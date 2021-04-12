import global_types
from common import *
from common_client import *

def create_all_client_definition_for_rpc(file_we, data):
    client_definition_for_rpc = ""
    # define rpc-s
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if is_valid_for_qt_rpc_client(rpc_info):
                    client_definition_for_rpc += create_client_definition_for_rpc(rpc_info, service_name, rpc_name, file_we, class_namespace = "QClientRpc", type_converter_fn = map_type_to_qt_type)
    return client_definition_for_rpc

def create_subscriptions(data):
    ret_str = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                if ret_str != "":
                    ret_str += "\n"
                ret_str += "\tm_zmqSubSocket.set(zmq::sockopt::subscribe, \"{}\");".format(create_signal_method_name(service_name, signal_name))
    return ret_str

def create_string_comparisons(data):
    string_comparisons = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                cb_type_name = "{}{}Cb".format(upperfirst(service_name), upperfirst(signal_name))
                cb_register_fn_name = "on{}{}".format(upperfirst(service_name), upperfirst(signal_name))
                cb_member = "m_{}".format(lowerfirst(cb_type_name))
                zmq_sub_key = create_signal_key_name(service_name, signal_name)
                string_comparisons += "\t{}(key == \"{}\"){{\n".format("if" if (string_comparisons == "") else "else if", zmq_sub_key)
                cb_call_params = ""
                if "parameter" in signal_info:
                    param_info = signal_info["parameter"]
                    string_comparisons += "\t\tzmq::message_t paramBuf;\n"
                    string_comparisons += "\t\tauto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);\n"
                    string_comparisons += "\t\tif (!res2) { throw std::runtime_error(\"No received msg\"); }\n"
                    if isinstance(signal_info["parameter"], dict):
                        string_comparisons += "\t\t::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));\n"
                        string_comparisons += "\t\tauto paramReader = msgReader.getRoot<{}>();\n".format(create_capnp_signal_param_type_str(service_name, signal_name))
                        cb_call_params = ""
                        for param_name, param_type in param_info.items():
                            cpp_rpc_if_type = map_type_to_qt_type(param_type)
                            if cpp_rpc_if_type == "QByteArray" or cpp_rpc_if_type == "QString":
                                cb_call_params += "QByteArray(reinterpret_cast<const char *>(paramReader.get{0}().begin()), int(paramReader.get{0}().size()))".format(upperfirst(param_name))
                            else:
                                cb_call_params += "paramReader.get{}()".format(upperfirst(param_name))
                            if list(param_info.keys())[-1] != param_name:
                                cb_call_params += ", "
                    elif signal_info["parameter"] == "__capnp__native__":
                        string_comparisons += "\t\t::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));\n"
                        cb_call_params += "reader"
                string_comparisons += "\t\temit {0}({1});\n".format(create_signal_method_name(service_name, signal_name), cb_call_params)
                string_comparisons += "\t}\n"
    return string_comparisons

def create_qclient_signal_connections(data):
    ret = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                if has_property(data, signal_name.removesuffix("Changed")):
                    ret += """\
\tQObject::connect(&m_qclientSignals, &QClientSignals::{0}, [this]({1}){{
\t\tif(m_{2} != val){{
\t\t\tm_{2} = val;
\t\t\temit {0}();
\t\t}}
\t}});
""".format(create_signal_method_name(service_name, signal_name), \
           create_fn_input_parameter_str_sender(signal_info, map_type_to_qt_type), \
           signal_name.removesuffix("Changed"))
                else:
                    ret += "\tQObject::connect(&m_qclientSignals, &QClientSignals::{0}, this, &QClient::{0});\n".format(create_signal_method_name(service_name, signal_name))
    return ret

def create_qclient_constructor_definition(data):
    ret = ""
    if has_rpc(data) and has_signals(data):
        ret += """\
QClient::QClient(zmq::context_t& rZmqContext, 
                 const std::string& rpcAddr, 
                 const std::string& signalAddr, 
                 QObject* pParent) :
    QObject(pParent),
    m_qclientRpc(rZmqContext, rpcAddr),
    m_qclientSignals(rZmqContext, signalAddr)
{{
{0}
}}
""".format(create_qclient_signal_connections(data))
    elif has_rpc(data):
        ret += """\
QClient::QClient(zmq::context_t& rZmqContext,
                 const std::string& rpcAddr, 
                 QObject* pParent) :
    QObject(pParent),
    m_qclientRpc(rZmqContext, rpcAddr)
{{
}}
"""
    elif has_signals(data):
        ret += """\
QClient::QClient(zmq::context_t& rZmqContext, 
                 const std::string& signalAddr,
                 QObject* pParent) :
    QObject(pParent),
    m_qclientSignals(rZmqContext, signalAddr)
{{
{0}
}}
""".format(create_qclient_signal_connections(data))
    return ret

def create_qclient_invokable_definitions(data):
    ret = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if is_valid_for_qt_rpc_client(rpc_info):
                    callParams = ""
                    if "parameter" in rpc_info:
                        for prm in rpc_info["parameter"]:
                            if callParams != "":
                                callParams += ", "
                            callParams += prm
                    ret += create_rpc_client_method_head(rpc_info, service_name, rpc_name, "QClient", type_converter_fn = map_type_to_qt_type) + "\n"
                    ret += "{\n"
                    ret += "\t{}m_qclientRpc.{}({});\n".format("return " if ("returns" in rpc_info) else "", public_method_name(service_name, rpc_name), callParams)
                    ret += "}\n\n"
    for service_name in data["services"]:
        service = data["services"][service_name]
        if "properties" in service:
            for key, descr in service["properties"].items():
                ret += "{} QClient::{}() const\n".format(map_type_to_qt_type(descr["type"]), create_property_var_name(service_name, key))
                ret += "{\n"
                ret += "\treturn m_{};\n".format(key)
                ret += "}\n\n"
    return ret

def create_capnzero_qobject_client_file_cpp_content_str(data, file_we):
    return """\
#include "{0}_QObjectClient.h"
#include <QSocketNotifier>

using namespace capnzero;
using namespace capnzero::{0};

QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    {0}ClientRpcTransport(rZmqContext, rpcAddr)
{{
}}

{1}

QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{{
    m_zmqSubSocket.connect(signalAddr);
{2}
    QSocketNotifier* socketNotifier = new QSocketNotifier(getFd(), QSocketNotifier::Read, this);
    QObject::connect(socketNotifier, &QSocketNotifier::activated, [this](int fd){{
        handleIncomingSignalAllNonBlock();
    }});
}}

void QClientSignals::handleIncomingSignal()
{{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) {{ throw std::runtime_error(\"No received msg\"); }}
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
{3}
}}

void QClientSignals::handleIncomingSignalAllNonBlock()
{{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {{
        handleIncomingSignal();
    }}
}}

int QClientSignals::getFd() const
{{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}}

{4}

{5}

""".format(file_we, \
           create_all_client_definition_for_rpc(file_we, data), \
           create_subscriptions(data), \
           create_string_comparisons(data), \
           create_qclient_constructor_definition(data), \
           create_qclient_invokable_definitions(data))