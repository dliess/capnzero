import global_types
from common import *
from common_client import *

def create_string_comparisons(data, zmqSubSocket):
    string_comparisons = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                cb_type_name = "{}{}Cb".format(upperfirst(service_name), upperfirst(signal_name))
                cb_register_fn_name = "on{}{}".format(upperfirst(service_name), upperfirst(signal_name))
                cb_member = "m_{}".format(lowerfirst(cb_type_name))
                zmq_sub_key = "{}{}".format(service_name, signal_name)
                string_comparisons += "\t{}(key == \"{}\"){{\n".format("if" if (string_comparisons == "") else "else if", zmq_sub_key)
                cb_call_params = ""
                if "parameter" in signal_info:
                    param_info = signal_info["parameter"]
                    string_comparisons += "\t\tzmq::message_t paramBuf;\n"
                    string_comparisons += "\t\tauto res2 = {}.recv(paramBuf, zmq::recv_flags::none);\n".format(zmqSubSocket)
                    string_comparisons += "\t\tif (!res2) { throw std::runtime_error(\"No received msg\"); }\n"
                    if isinstance(signal_info["parameter"], dict):
                        string_comparisons += "\t\tauto paramReader = getReader<{}>(paramBuf);\n".format(create_capnp_signal_param_type_str(service_name, signal_name))
                        cb_call_params = ""
                        for param_name, param_type in param_info.items():
                            cpp_rpc_if_type = map_2_ca_call_param_type(param_type)
                            if cpp_rpc_if_type.startswith("Span"):
                                cb_call_params += "{0}(paramReader.get{1}().begin(), paramReader.get{1}().end())".format(cpp_rpc_if_type, upperfirst(param_name))
                            elif cpp_rpc_if_type == "TextView":
                                cb_call_params += "TextView(paramReader.get{0}().begin(), paramReader.get{0}().size())".format(upperfirst(param_name))
                            else:
                                cb_call_params += "paramReader.get{}()".format(upperfirst(param_name))
                            if list(param_info.keys())[-1] != param_name:
                                cb_call_params += ", "
                    elif signal_info["parameter"] == "__capnp__native__":
                        string_comparisons += "\t\t::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));\n"
                        cb_call_params += "reader"
                string_comparisons += "\t\tif({0}) {0}({1});\n".format(create_signal_cb_member(service_name, signal_name), cb_call_params)
                string_comparisons += "\t}\n"
    return string_comparisons

def create_capnzero_client_file_cpp_content_str(data, file_we):

    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    outStr = """\
#include "{0}_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "{0}.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::{0};
""".format(file_we)

    if has_rpc:
        outStr += """\
{0}ClientRpc::{0}ClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{{
    m_zmqReqSocket.connect(serverRpcAddr);
}}
""".format(file_we)

    if has_signal_handling:
        outStr += """\
{0}ClientSignals::{0}ClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{{
    m_zmqSubSocket.connect(serverSignalAddr);
}}

{0}ClientSignalsThreaded::{0}ClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{{
    m_zmqSubHelperSocket.bind("inproc://{0}SubHelper");
}}

{0}ClientSignalsThreaded::~{0}ClientSignalsThreaded()
{{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}}

void {0}ClientSignalsThreaded::StartThread()
{{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() {{ signalReceiverThreadFn(); }});
}}

""".format(file_we)

    # define rpc-s
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if not should_be_template(rpc_info):
                    outStr += create_client_definition_for_rpc(rpc_info, service_name, rpc_name, file_we)

    if has_signal_handling:
        outStr += """\
void {0}ClientSignalsThreaded::signalReceiverThreadFn()
{{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://{0}SubHelper");

    while(!m_terminateRequest.load())
    {{
        zmq::pollitem_t items[] = {{
            {{zmqSubSocket.handle(), 0, ZMQ_POLLIN, 0}},
			{{zmqSubHelperSocket.handle(), 0, ZMQ_POLLIN, 0}}
        }};
        zmq::poll(&items[0], 2);
		if (items[0].revents & ZMQ_POLLIN)
		{{
            handleIncomingSignal(zmqSubSocket);
		}}
		if (items[1].revents & ZMQ_POLLIN)
		{{
			handleSubscriptionRequest(zmqSubHelperSocket, zmqSubSocket);
		}}
    }}
}}

void {0}ClientSignals::handleIncomingSignal()
{{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) {{ throw std::runtime_error(\"No received msg\"); }}
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
{1}}}

void {0}ClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) {{ throw std::runtime_error(\"No received msg\"); }}
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
{2}}}

void {0}ClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) {{ throw std::runtime_error(\"No received msg\"); }}
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}}
""".format(file_we, create_string_comparisons(data, "m_zmqSubSocket"), create_string_comparisons(data, "zmqSubSocket"))

    outStr += "\n"
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                cb_type_name = "{}{}Cb".format(upperfirst(service_name), upperfirst(signal_name))
                cb_register_fn_name = "on{}{}".format(upperfirst(service_name), upperfirst(signal_name))
                cb_member = "m_{}".format(lowerfirst(cb_type_name))
                zmq_sub_key = "{}{}".format(service_name, signal_name)
                outStr += """\
void {0}ClientSignals::{1}({2} cb)
{{
    if({3}) return; // set it only once
    {3} = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "{4}");
}}

void {0}ClientSignalsThreaded::{1}({2} cb)
{{
    if({3}) return; // set it only once
    {3} = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("{4}", {5}));
}}

""".format(file_we, cb_register_fn_name, cb_type_name, cb_member, zmq_sub_key, len(zmq_sub_key))



    client_class_constructor_definition = ""
    if has_rpc and has_signal_handling:
        client_class_constructor_definition = """\
{0}Client::{0}Client(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr) :
    {0}ClientRpc(rZmqContext, serverRpcAddr),
    {0}ClientSignalsThreaded(rZmqContext, serverSignalAddr)
{{
    StartThread();
}}
""".format(file_we)
    elif has_rpc:
        client_class_constructor_definition = """\
{0}Client::{0}Client(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    {0}ClientRpc(rZmqContext, serverRpcAddr)
{{
}}
""".format(file_we)
    elif has_signal_handling:
        client_class_constructor_definition = """\
{0}Client::{0}Client(zmq::context_t& rZmqContext, const std::string& serverSignalAddr) :
    {0}ClientSignalsThreaded(rZmqContext, serverSignalAddr)
{{
    StartThread();
}}
""".format(file_we)

    outStr += client_class_constructor_definition

    return outStr

