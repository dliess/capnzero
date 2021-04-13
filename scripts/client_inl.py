import global_types
from common import *
from common_client import *

def create_public_section_rpc_def(data, file_we):
    ret_str = "// public section rpc definitions\n\n"
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if should_be_template(rpc_info):
                    ret_str += create_client_definition_for_rpc(rpc_info, service_name, rpc_name, file_we, class_namespace = "{}ClientRpc".format(file_we))
    return ret_str


def create_capnzero_client_file_inl_content_str(data, file_we):
    return """
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "capnzero_utils.h"
namespace capnzero::{0}
{{

{1}

}} // namespace capnzero::{0}
""".format(file_we, create_public_section_rpc_def(data, file_we))
