import global_types
from common import *
from common_client import *

def create_all_client_definition_for_rpc(file_we, data, webchannel_support):
    if not has_rpc(data):
        return ""
    client_definition_for_rpc = """
QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    {0}ClientRpcTransport(rZmqContext, rpcAddr)
{{
}}    
        
""".format(file_we)
    # define rpc-s
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if is_valid_for_qt_rpc_client(rpc_info):
                    client_definition_for_rpc += create_client_definition_for_rpc(rpc_info, service_name, rpc_name, file_we, class_namespace = "QClientRpc", type_converter_fn = map_type_to_qt_type, webchannel_support = webchannel_support)
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

def create_string_comparisons(data, webchannel_support = False):
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
                        string_comparisons += "\t\t::capnp::UnalignedFlatArrayMessageReader msgReader(asCapnpArr(paramBuf));\n"
                        string_comparisons += "\t\tauto paramReader = msgReader.getRoot<{}>();\n".format(create_capnp_signal_param_type_str(service_name, signal_name))
                        cb_call_params = ""
                        for param_name, param_type in param_info.items():
                            qt_mapped_type = map_type_to_qt_type(param_type)
                            if qt_mapped_type == "QByteArray" or qt_mapped_type == "QString":
                                cb_call_params += "QByteArray(reinterpret_cast<const char *>(paramReader.get{0}().begin()), int(paramReader.get{0}().size()))".format(upperfirst(param_name))
                                if qt_mapped_type == "QByteArray" and webchannel_support == True:
                                    cb_call_params += ".toBase64()"
                            else:
                                cb_call_params += "paramReader.get{}()".format(upperfirst(param_name))
                            if list(param_info.keys())[-1] != param_name:
                                cb_call_params += ", "
                    elif signal_info["parameter"] == "__capnp__native__":
                        string_comparisons += "\t\t::capnp::UnalignedFlatArrayMessageReader reader(asCapnpArr(paramBuf));\n"
                        cb_call_params += "reader"
                string_comparisons += "\t\temit {0}({1});\n".format(create_signal_method_name_qt(service_name, signal_name), cb_call_params)
                string_comparisons += "\t}\n"
    return string_comparisons

def create_qclient_signal_connections(data, file_we):
    ret = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                arguments = get_arguments(signal_info)
                ### create signal args
                # > convert types to qt type
                arguments_qt = map(lambda arg: { 'result': map_type_to_qt_type(arg['capnz_type']),\
                                                 'capnz_type': arg['capnz_type'], \
                                                 'name': arg['parameter_name'] }, arguments)
                # > we have to namespace the enumerations
                def add_ns_if_is_enum(capnz_type, type_to_decorate):
                    return "capnzero::" + file_we + "::" + type_to_decorate if is_enum_type(capnz_type) else type_to_decorate
                arguments_qt_ns = map(lambda arg: { 'result': add_ns_if_is_enum(arg['capnz_type'], arg['result']), \
                                                    'capnz_type':  arg['capnz_type'], \
                                                    'name': arg['name'] }, arguments_qt)
                # > add const& to non-trivial types
                arguments_qt_ns_constref = map(lambda arg: { 'result': constref_nontrivial(arg['capnz_type'], arg['result']), \
                                                             'name': arg['name'] }, arguments_qt_ns)
                # > to string
                arguments_qt_ns_constref_str = ",".join(map(lambda arg: "{} {}".format(arg['result'], arg['name']), arguments_qt_ns_constref))
                ### create signal call args
                # > we have to static cast the enum parameters
                def add_cast_if_is_enum(capnz_type, parameter_name):
                    return "static_cast<QClient::{}>({})".format(capnz_type, parameter_name) if is_enum_type(capnz_type) else parameter_name
                call_parameters = map(lambda arg: add_cast_if_is_enum(arg['capnz_type'], arg['parameter_name']) , arguments)
                # > to string (names only)
                call_parameters_str = ",".join(call_parameters)

                property_name = signal_name.removesuffix("Changed")
                if has_property(data["services"][service_name], property_name):
                    property_name_with_prefix = create_property_var_name(service_name, property_name)
                    if is_enum_type(arguments[0]['capnz_type']):
                        ret += """\
\tQObject::connect(&m_qclientSignals, &QClientSignals::{0}, [this]({1}){{
\t\tif(m_{2} != static_cast<decltype(m_{2})>(val)){{
\t\t\tm_{2} = static_cast<decltype(m_{2})>(val);
\t\t\temit {0}();
\t\t}}
\t}});
""".format(create_signal_method_name_qt(service_name, signal_name), \
           arguments_qt_ns_constref_str, \
           property_name_with_prefix)
                    else:
                        ret += """\
\tQObject::connect(&m_qclientSignals, &QClientSignals::{0}, [this]({1}){{
\t\tif(m_{2} != val){{
\t\t\tm_{2} = val;
\t\t\temit {0}();
\t\t}}
\t}});
""".format(create_signal_method_name_qt(service_name, signal_name), \
           arguments_qt_ns_constref_str, \
           property_name_with_prefix)
                else:
                    ret += """\
\tQObject::connect(&m_qclientSignals, &QClientSignals::{0}, [this]({1}){{
\t\t\temit {0}({2});
\t}});
""".format(create_signal_method_name_qt(service_name, signal_name), \
           arguments_qt_ns_constref_str, \
           call_parameters_str)
    return ret

def create_qclient_constructor_definition(data, file_we):
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
    registerMetaTypes();
{0}
}}
""".format(create_qclient_signal_connections(data, file_we))
    elif has_rpc(data):
        ret += """\
QClient::QClient(zmq::context_t& rZmqContext,
                 const std::string& rpcAddr, 
                 QObject* pParent) :
    QObject(pParent),
    m_qclientRpc(rZmqContext, rpcAddr)
{{
    registerMetaTypes();
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
    registerMetaTypes();
{0}
}}
""".format(create_qclient_signal_connections(data, file_we))
    return ret

def create_qclient_invokable_definitions(data, file_we):
    ret = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if is_valid_for_qt_rpc_client(rpc_info):
                    callParams = ""
                    if "parameter" in rpc_info:
                        for prm, prm_type in rpc_info["parameter"].items():
                            if callParams != "":
                                callParams += ", "
                            if is_enum_type(prm_type):
                                prm="static_cast<capnzero::"+file_we+"::"+prm_type+">("+prm+")"
                            callParams += prm
                    ret += create_rpc_client_method_definition_head(rpc_info, service_name, rpc_name, "QClient", type_converter_fn = map_type_to_qt_type, forced_enum_namespace = "QClient") + "\n"
                    ret += "{\n"
                    m_qclientRpc_call = "m_qclientRpc.{}({})".format(public_method_name_qt(service_name, rpc_name), callParams)
                    if "returns" in rpc_info:
                        if rpc_return_type(rpc_info) == RPCType.DirectType and is_enum_type(rpc_info["returns"]):
                            ret += "\treturn static_cast<QClient::{}>({});\n".format(rpc_info["returns"], m_qclientRpc_call)
                        else:
                            ret += "\treturn {};\n".format(m_qclientRpc_call)
                    else:
                        ret += "\t{};\n".format(m_qclientRpc_call)
                    ret += "}\n\n"
    for service_name in data["services"]:
        service = data["services"][service_name]
        if "properties" in service:
            for key, descr in service["properties"].items():
                property_name_with_prefix = create_property_var_name(service_name, key)
                return_type = map_type_to_qt_type(descr["type"])
                if is_enum_type(descr["type"]):
                    return_type = "QClient::" + return_type
                ret += "{} QClient::{}() const\n".format(return_type, property_name_with_prefix)
                ret += "{\n"
                ret += "\treturn m_{};\n".format(property_name_with_prefix)
                ret += "}\n\n"
    return ret

def create_metatype_registration_content(data):
    ret_str = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                return_type = rpc_return_type(rpc_info)
                if return_type == RPCType.Dict:
                    struct_name = "Return" + service_name +  upperfirst(rpc_name)
                    if ret_str != "":
                        ret_str += "\n"
                    ret_str += "\tqRegisterMetaType<{}>();".format(struct_name)
    for enum_name in global_types.enumerations:
        ret_str += "\n\tqRegisterMetaType<{0}>(\"{0}\");".format(enum_name)
    return ret_str

def create_capnzero_qobject_client_file_cpp_content_str(data, file_we, webchannel_support = False):
    return """\
#include "{0}_QObjectClient.h"
#include "{0}.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::{0};

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

void QClient::registerMetaTypes()
{{
{6}
}}


""".format(file_we, \
           create_all_client_definition_for_rpc(file_we, data, webchannel_support), \
           create_subscriptions(data), \
           create_string_comparisons(data, webchannel_support), \
           create_qclient_constructor_definition(data, file_we), \
           create_qclient_invokable_definitions(data, file_we),
           create_metatype_registration_content(data))