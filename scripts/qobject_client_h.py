import global_types
from common import *
from common_client import *

def create_qgadget_declaration(ret_info, service_name, rpc_name):
    struct_name = "Return" + service_name +  upperfirst(rpc_name)
    qproperty_decl = ""
    for element_name, element_type in ret_info.items():
        if qproperty_decl != "":
            qproperty_decl += "\n"
        qproperty_decl += "\tQ_PROPERTY({0} {1} MEMBER {1})".format(map_type_to_qt_type(element_type), element_name)
    gadget_members = ""
    for element_name, element_type in ret_info.items():
        if gadget_members != "":
            gadget_members += "\n"
        gadget_members += "\t{} {};".format(map_type_to_qt_type(element_type), element_name)
    return """\
class {0}
{{
    Q_GADGET
{1}
public:
{2}
}};

""".format(struct_name, qproperty_decl, gadget_members)
    

def create_return_type_gadget_declarations(data):
    ret_str = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                return_type = rpc_return_type(rpc_info)
                if return_type == RPCType.Dict:
                    ret_str += create_qgadget_declaration(rpc_info["returns"], service_name, rpc_name)
    return ret_str

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
    return ret_str


def create_rpc_declarations_for_qt_obj(data, tabs, prefix):
    ret_str = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if is_valid_for_qt_rpc_client(rpc_info):
                    return_type_str = create_return_type_str_client(rpc_info, service_name, rpc_name, type_mapper_fn = map_type_to_qt_type)
                    parameter_str = create_fn_parameter_str_from_dict(rpc_info, map_type_to_qt_type)
                    method_name = create_rpc_method_name(service_name, rpc_name)
                    ret_str +=  tabs + prefix + " " + return_type_str
                    ret_str += " " if len(prefix + " " + return_type_str) < 40 else ("\n" + tabs)
                    ret_str += method_name + "(" + parameter_str + ");\n"
    return ret_str

def create_rpc_part(file_we, data):
    if not has_rpc(data):
        return ""

    return """\
#include "{1}_ClientTransport.h"
namespace capnzero::{1}
{{

{2}
inline void registerMetatTypes()
{{
{3}
}}

class {1}QObjectClientRpc : public QObject, public {1}ClientRpcTransport
{{
    Q_OBJECT
public:
    {1}QObjectClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
{4}
}};

}} // namespace capnzero::{1}
""".format(to_snake_case(file_we).upper(), file_we, create_return_type_gadget_declarations(data), create_metatype_registration_content(data), create_rpc_declarations_for_qt_obj(data, "\t", "Q_INVOKABLE"))




def create_capnzero_qobject_client_file_h_content_str(data, file_we):

#    has_rpc = has_rpc(data)
#    has_signal_handling = has_signals(data)
#    return_type_gadget_declarations = create_return_type_gadget_declarations(data)
#    qinvokable_declarations = create_rpc_declarations_for_qt_obj(data, "\t", "Q_INVOKABLE")
#    signal_declarations = create_signal_fn_declarations(data, "\t", map_type_to_qt_type)
#    qclient_constructor_declaration = ""
#    if has_rpc and has_signal_handling:
#        qclient_constructor_declaration = "explicit {}QObjectClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent = nullptr);".format(file_we)
#    elif has_rpc:
#        qclient_constructor_declaration = "explicit {}QObjectClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject* pParent = nullptr);".format(file_we)
#    elif has_signal_handling:
#        qclient_constructor_declaration = "explicit {}QObjectClient(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject* pParent = nullptr);".format(file_we)

    return """\
#ifndef {0}_QOBJECT_CLIENT_H
#define {0}_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>

{1}

#endif
""".format(to_snake_case(file_we).upper(), create_rpc_part(file_we, data))