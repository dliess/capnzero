import global_types
from common import *

def create_capnzero_server_file_cpp_content_str(data, file_we):
    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    constructor_parameters = ""
    constructor_initializer_list = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            constructor_parameters += "std::unique_ptr<{}> {}".format(create_member_cb_if_type(service_name), \
                                                                lowerfirst(service_name))
            constructor_initializer_list += "\t{}(std::move({})),".format(create_member_cb_if(service_name), lowerfirst(service_name))

            if list(data["services"].keys())[-1] != service_name:
                constructor_parameters += ", "
                constructor_initializer_list += "\n"

    cases_str = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            cases_str += "\t\tcase to_underlying(ServiceId::{}):\n".format(to_snake_case(service_name).upper())
            cases_str +=  "\t\t{\n"
            cases_str += "\t\t\tswitch(coordReader.getRpcId())\n"
            cases_str += "\t\t\t{\n"
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                cases_str += "\t\t\t\tcase to_underlying({0}RpcIds::{1}):\n".format(service_name, to_snake_case(rpc_name).upper())
                cases_str += "\t\t\t\t{\n"
                params = ""
                if "parameter" in rpc_info:
                    cases_str += "\t\t\t\t\tzmq::message_t paramBuf;\n"
                    cases_str += "\t\t\t\t\tauto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);\n"
                    cases_str += "\t\t\t\t\tif (!res2) { throw std::runtime_error(\"No received msg\"); }\n"
                    cases_str += "\t\t\t\t\tauto paramReader = getReader<{}>(paramBuf);\n".format(create_capnp_rpc_parameter_type_str(service_name, rpc_name))
                    param_info = rpc_info["parameter"]
                    for param_name, param_type in param_info.items():
                        cpp_rpc_if_type = map_2_ca_call_param_type(param_type)
                        if cpp_rpc_if_type.startswith("Span"):
                            params += "{0}(paramReader.get{1}().begin(), paramReader.get{1}().end())".format(cpp_rpc_if_type, upperfirst(param_name))
                        elif cpp_rpc_if_type == "TextView":
                            params += "TextView(paramReader.get{0}().begin(), paramReader.get{0}().size())".format(upperfirst(param_name))
                        else:
                            params += "paramReader.get{}()".format(upperfirst(param_name))
                        if list(param_info.keys())[-1] != param_name:
                            params += ", "
                return_expr = ""
                if "returns" in rpc_info:
                    return_expr = "auto ret = "
                cases_str += "\t\t\t\t\t{}{}->{}({});\n".format(return_expr, create_member_cb_if(service_name), rpc_name, params)
                if "returns" in rpc_info:
                    cases_str += "\t\t\t\t\t::capnp::MallocMessageBuilder retMessage;\n"
                    cases_str += "\t\t\t\t\tauto builder = retMessage.initRoot<{}>();\n".format(create_capnp_rpc_return_type_str(service_name, rpc_name))
                    for return_name, return_type in rpc_info["returns"].items():
                        if map_descr_type_to_capnp_type(return_type) == "Data":
                            cases_str += "\t\t\t\t\tbuilder.set{0}(capnp::Data::Reader(ret.{1}.data(), ret.{1}.size()));\n".format(upperfirst(return_name), return_name)
                        else:
                            cases_str += "\t\t\t\t\tbuilder.set{}(ret.{});\n".format(upperfirst(return_name), return_name)
                    cases_str += "\t\t\t\t\tm_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);\n"
                    cases_str += "\t\t\t\t\tsendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);\n"
                cases_str += "\t\t\t\t\tbreak;\n"
                cases_str += "\t\t\t\t}\n"
            cases_str += "\t\t\t}\n"
            cases_str +=  "\t\t\tbreak;\n"
            cases_str +=  "\t\t}\n"

    signal_fn_definitions = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_fn_definitions += "void {0}Server::Signals::{1}__{2}({3})\n".format(file_we, service_name, signal_name, create_fn_parameter_str(data["services"][service_name]["signal"][signal_name]))
                signal_fn_definitions += "{\n"
                signal_info = data["services"][service_name]["signal"][signal_name]
                send_flag = "zmq::send_flags::sndmore" if ("parameter" in signal_info) else "zmq::send_flags::none"
                signal_fn_definitions += "\tm_rZmqPubSocket.send(zmq::const_buffer(\"{}{}\", {}), {});\n".format(service_name, signal_name, len(service_name) + len(signal_name), send_flag)
                if "parameter" in signal_info:
                    signal_fn_definitions += "\t::capnp::MallocMessageBuilder message;\n"
                    signal_fn_definitions += "\tauto builder = message.initRoot<{}>();\n".format(format(create_capnp_signal_param_type_str(service_name, signal_name)))
                    for param_name, param_type in signal_info["parameter"].items():
                        if map_descr_type_to_capnp_type(param_type) == "Data":
                            signal_fn_definitions += "\tbuilder.set{0}(capnp::Data::Reader({1}.data(), {1}.size()));\n".format(upperfirst(param_name), param_name)
                        else:
                            signal_fn_definitions += "\tbuilder.set{}({});\n".format(upperfirst(param_name), param_name)
                    signal_fn_definitions += "\tsendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);\n"
                signal_fn_definitions += "}\n\n"

    server_constructor_definition = ""
    if has_rpc and has_signal_handling:
        server_constructor_definition = """\
{0}Server::{0}Server(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr, {1}) :
{2}
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router),
    m_signals(rZmqContext, signalBindAddr)
{{
    m_zmqRepSocket.bind(rpcBindAddr);
}}
""".format(file_we, constructor_parameters, constructor_initializer_list)
    elif has_rpc:
        server_constructor_definition = """\
{0}Server::{0}Server(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, {1}) :
{2}
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router)
{{
    m_zmqRepSocket.bind(rpcBindAddr);
}}
""".format(file_we, constructor_parameters, constructor_initializer_list)
    elif has_signal_handling:
        server_constructor_definition = """\
{0}Server::{0}Server(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_signals(rZmqContext, signalBindAddr)
{{
}}
""".format(file_we)

    signal_constructor_definitions = ""
    if has_signal_handling:
        signal_constructor_definitions = """\
{0}Server::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::pub)
{{
    m_rZmqPubSocket.bind(signalBindAddr);
}}
""".format(file_we)



    outStr = """\
#include "{0}_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "{0}.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;

""".format(file_we)

    outStr += server_constructor_definition

    if has_rpc:
        outStr += """\
void {0}Server::processNextRequest(WaitMode waitMode) {{
    zmq::message_t routerId;
    auto res0 = m_zmqRepSocket.recv(routerId, (waitMode == WaitMode::Blocking) ? zmq::recv_flags::none : zmq::recv_flags::dontwait);
    if(waitMode == WaitMode::NonBlocking && !res0) return;
    zmq::message_t rpcCoordBuf;
    auto res1 = m_zmqRepSocket.recv(rpcCoordBuf, zmq::recv_flags::none);
    auto coordReader = getReader<RpcCoord>(rpcCoordBuf);
    switch(coordReader.getServiceId())
    {{
{1}
    }}
}}
""".format(file_we, cases_str)

    outStr += signal_constructor_definitions
    outStr += signal_fn_definitions

    return outStr
