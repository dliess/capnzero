import global_types
from common import *
from common_client import *

def create_capnzero_qobject_client_file_cpp_content_str(data, file_we):
    client_definition_for_rpc = ""
    # define rpc-s
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if is_valid_for_qt_rpc_client(rpc_info):
                    client_definition_for_rpc += create_client_definition_for_rpc(rpc_info, service_name, rpc_name, file_we, class_namespace = "{}QObjectClientRpc".format(file_we), type_converter_fn = map_type_to_qt_type)

    return """\
#include "{0}_QObjectClient.h"

using namespace capnzero;
using namespace capnzero::{0};

{0}QObjectClientRpc::{0}QObjectClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    {0}ClientRpcTransport(rZmqContext, rpcAddr)
{{
}}

{1}
""".format(file_we, client_definition_for_rpc)