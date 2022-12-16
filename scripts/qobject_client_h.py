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

def create_rpc_declarations_for_qt_obj(data, tabs, prefix):
    ret_str = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if is_valid_for_qt_rpc_client(rpc_info):
                    return_type_str = create_return_type_str_client(rpc_info, service_name, rpc_name, type_mapper_fn = map_type_to_qt_type)
                    parameter_str = create_fn_parameter_str_from_dict(rpc_info, map_type_to_qt_type)
                    method_name = create_rpc_method_name_qt(service_name, rpc_name)
                    ret_str +=  tabs + prefix + " " + return_type_str
                    ret_str += " " if len(prefix + " " + return_type_str) < 40 else ("\n" + tabs)
                    ret_str += method_name + "(" + parameter_str + ");\n"
    return ret_str

def create_rpc_part(file_we, data):
    if not has_rpc(data):
        return ""

    return """\
#include "{0}_ClientTransport.h"
namespace capnzero::{0}
{{

{1}

class QClientRpc : public QObject, public {0}ClientRpcTransport
{{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
{2}
}};

}} // namespace capnzero::{0}
""".format(file_we, create_return_type_gadget_declarations(data), create_rpc_declarations_for_qt_obj(data, "\t", "Q_INVOKABLE"))

def create_signal_part(file_we, data):
    return """\
#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::{0}
{{

class QClientSignals : public QObject
{{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
{1}
private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
}};

}} // namespace capnzero::{0}
""".format(file_we, create_signal_fn_declarations_qt(data, "\t", map_type_to_qt_type))

def create_qproperty_declarations(data):
    ret = ""
    for service_name in data["services"]:
        service = data["services"][service_name]
        if "properties" in service:
            for key, descr in service["properties"].items():
                propName = create_property_var_name(service_name, key)
                property_setter_name = create_qproperty_setter_method_name(service_name, key)
                property_notification_signal_name = create_qproperty_notification_signal_name(service_name, key)
                if descr["access"] == "read-only":
                    ret += "\tQ_PROPERTY({0} {1} READ {2} NOTIFY {3})\n".format(map_type_to_qt_type(descr["type"]), propName, propName, property_notification_signal_name )
                else:
                    ret += "\tQ_PROPERTY({0} {1} READ {2} WRITE {3} NOTIFY {4})\n".format(map_type_to_qt_type(descr["type"]), propName, propName, property_setter_name, property_notification_signal_name )
    return ret

def create_qclient_invokable_declarations(data):
    ret = create_rpc_declarations_for_qt_obj(data, "\t", "Q_INVOKABLE")
    for service_name in data["services"]:
        service = data["services"][service_name]
        if "properties" in service:
            for key, descr in service["properties"].items():
                ret += "\tQ_INVOKABLE {} {}() const;\n".format(map_type_to_qt_type(descr["type"]), create_property_var_name(service_name, key))
    return ret

def create_qclient_signal_fn_declarations(data, tabs, converter_fn = None):
    signal_fn_declarations = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                if has_property(data["services"][service_name], signal_name.removesuffix("Changed")):
                    signal_fn_declarations += tabs + "void {}();\n".format(create_signal_method_name_qt(service_name, signal_name))
                else:
                    signal_fn_declarations += tabs + "void {}({});\n".format(create_signal_method_name_qt(service_name, signal_name), create_fn_input_parameter_str_sender(signal_info, converter_fn))
    return signal_fn_declarations

def create_qobject_private_members(data):
    ret = ""
    if has_rpc(data):
        ret += "\tQClientRpc m_qclientRpc;\n"
    if has_signals(data):
        ret += "\tQClientSignals m_qclientSignals;\n"
    for service_name in data["services"]:
        service = data["services"][service_name]
        if "properties" in service:
            for key, descr in service["properties"].items():
                ret += "\t{} m_{};\n".format(map_type_to_qt_type(descr["type"]), create_property_var_name(service_name, key))
    return ret

def create_qclient_constructor_declaration(data):
    ret = ""
    if has_rpc(data) and has_signals(data):
        ret += "\texplicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent = nullptr);"
    elif has_rpc(data):
        ret += "\texplicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject* pParent = nullptr);"
    elif has_signals(data):
        ret += "\texplicit QClient(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject* pParent = nullptr);"
    return ret

def create_qclient_enum_declarations():
    enum_str = "\n"
    for enum_name in global_types.enumerations:
        qenum_name = enum_name
        enum_str += "\tenum class {} {{\n".format(qenum_name)
        enum_data = global_types.enumerations[enum_name]
        for i, (enum_element_name, enum_element_number) in enumerate(enum_data.items()):
            enum_str += "\t\t{} = {}".format(enum_element_name, enum_element_number)
            if i < len(enum_data) - 1:
                enum_str += ","
            enum_str += "\n"
        enum_str += "\t}};\n\tQ_ENUM({})\n".format(qenum_name)
    return enum_str

def create_qtclient(file_we, data):
    return """\
namespace capnzero::{0}
{{

class QClient : public QObject
{{
    Q_OBJECT
{1}
public:
{2}
{3}
{4}
signals:
{5}
private:
{6}
    void registerMetaTypes();
}};

}} // namespace capnzero::{0}
""".format(file_we, \
           create_qproperty_declarations(data), \
           create_qclient_constructor_declaration(data), \
           create_qclient_enum_declarations(), \
           create_qclient_invokable_declarations(data), \
           create_qclient_signal_fn_declarations(data, "\t", map_type_to_qt_type), \
           create_qobject_private_members(data))

def create_capnzero_qobject_client_file_h_content_str(data, file_we):
    return """\
#ifndef {0}_QOBJECT_CLIENT_H
#define {0}_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "{1}.capnp.h"

{2}
{3}
{4}

#endif
""".format(to_snake_case(file_we).upper(), file_we, create_rpc_part(file_we, data), create_signal_part(file_we, data), create_qtclient(file_we, data))