import global_types
from common import *

def create_capnzero_qobject_client_file_cpp_content_str(data, file_we):
    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    signal_registrations = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                signal_registrations += """\
    Super::on{0}{1}([this]({2}){{
        emit {3}({4});
    }});
""".format(upperfirst(service_name), upperfirst(signal_name), create_fn_arguments_str(signal_info), create_signal_method_name(service_name, signal_name), create_fn_arguments_param_only_str(signal_info))

    qclient_constructor_definition = ""
    if has_rpc and has_signal_handling:
        qclient_constructor_definition = """\
{0}QObjectClient::{0}QObjectClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent) :
    QObject(pParent),
    {0}Client(rZmqContext, rpcAddr, signalAddr)
{{
{1}
}}
""".format(file_we, signal_registrations)
    elif has_rpc:
        qclient_constructor_definition = """\
{0}QObjectClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject* pParent) :
    QObject(pParent),
    {0}Client(rZmqContext, rpcAddr)
{{
{1}
}}
""".format(file_we, signal_registrations)
    elif has_signal_handling:
        qclient_constructor_definition = """\
{0}QObjectClient(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject* pParent) :
    QObject(pParent),
    {0}Client(rZmqContext, signalAddr)
{{
{1}
}}
""".format(file_we, signal_registrations)


    qinvokable_definitions = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                return_type_str = create_rpc_return_type_for_qt_webchannel_obj(rpc_info)
                parameter_str = ""
                if "parameter" in rpc_info:
                    parameter_str = create_fn_parameter_str_from_dict(rpc_info)
                method_name = create_rpc_method_name(service_name, rpc_name)
                qinvokable_definitions += return_type_str
                qinvokable_definitions += " " if len(return_type_str) < 40 else ("\n")
                qinvokable_definitions += "{0}QObjectClient::{1}({2}) {{\n".format(file_we, method_name, parameter_str)
                qinvokable_definitions += "\t{}Super::{}({});\n".format("return " if (return_type_str != "void") else "", method_name, create_fn_arguments_param_only_str(rpc_info))
                qinvokable_definitions += "}\n\n"


    outStr = """
#include "{0}_QObjectClient.h"

using namespace capnzero;
using namespace capnzero::{0}

{0}QObjectClientRpc::{0}QObjectClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    {0}ClientRpcTransport(rZmqContext, rpcAddr)
{{
}}

{1}
{2}
{3}

void {0}QObjectClientRpc::registerMetatTypes()
{{

}}

""".format(file_we, qclient_constructor_definition, qinvokable_definitions, meta_type_registration_definition)
    return outStr
