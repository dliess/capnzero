import global_types
from common import *

def create_rpc_declarations_for_qt_webchannel_obj(data, tabs, prefix):
    ret_str = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                return_type_str = create_rpc_return_type_for_qt_webchannel_obj(rpc_info)
                parameter_str = create_fn_parameter_str_from_dict(rpc_info, map_type_to_qt_type)
                method_name = service_name + "__" + rpc_name
                ret_str +=  tabs + prefix + " " + return_type_str
                ret_str += " " if len(prefix + " " + return_type_str) < 40 else ("\n" + tabs)
                ret_str += method_name + "(" + parameter_str + ");\n"
    return ret_str

def create_capnzero_qobject_client_file_h_content_str(data, file_we):
    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    qinvokable_declarations = create_rpc_declarations_for_qt_webchannel_obj(data, "\t", "Q_INVOKABLE")
    signal_declarations = create_signal_fn_declarations(data, "\t", map_type_to_qt_type)
    qclient_constructor_declaration = ""
    if has_rpc and has_signal_handling:
        qclient_constructor_declaration = "explicit {}QObjectClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent = nullptr);".format(file_we)
    elif has_rpc:
        qclient_constructor_declaration = "explicit {}QObjectClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject* pParent = nullptr);".format(file_we)
    elif has_signal_handling:
        qclient_constructor_declaration = "explicit {}QObjectClient(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject* pParent = nullptr);".format(file_we)
    outStr = """
#ifndef {0}_QOBJECT_CLIENT_H
#define {0}_QOBJECT_CLIENT_H

#include <QObject>
#include "{1}_Client.h"

namespace capnzero
{{

class {1}QObjectClient : public QObject, public {1}Client
{{
    Q_OBJECT
public:
    {2}
{3}
signals:
{4}
private:
    using Super = {1}Client;
}};

}} // namespace capnzero
#endif
""".format(to_snake_case(file_we).upper(), file_we, qclient_constructor_declaration, qinvokable_declarations, signal_declarations)
    return outStr
