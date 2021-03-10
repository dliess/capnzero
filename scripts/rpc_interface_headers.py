import global_types
from common import *

def create_capnzero_cbif_h_content_str(service_name, rpc_infos, file_we, file_base_we):
    if_member_fns = ""
    for rpc_name in rpc_infos:
        parameter1 = ""
        parameter2 = ""
        ret_type =  rpc_return_type(rpc_infos[rpc_name])
        return_type = create_return_type_str_server(rpc_infos[rpc_name], rpc_name)
        if ret_type == RPCType.Dict:
            if_member_fns += create_return_struct_definition(return_type, rpc_infos[rpc_name]["returns"], "\t")
        elif ret_type == RPCType.CapnpNative:
            parameter2 = "NativeCapnpMsgWriter &writer"
        parameter1 = create_rpc_handler_fn_arguments_str(rpc_infos[rpc_name])
        if parameter1 != "" and parameter2 != "":
            parameter1 += ", "
        if_member_fns += "\tvirtual {} {}({}{}) = 0;\n".format(return_type, rpc_name, parameter1, parameter2)

    outStr = """\
#ifndef {0}_H
#define {0}_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "{1}.capnp.h"
#include "capnp/message.h"

namespace capnzero
{{

class {2}
{{
public:
    virtual ~{2}() = default;
{3}}};

}}
#endif
""".format(to_snake_case(file_we).upper(), file_base_we, create_member_cb_if_type(service_name), if_member_fns)

    return outStr
