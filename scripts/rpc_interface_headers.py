import global_types
from common import *

def create_capnzero_cbif_h_content_str(service_name, rpc_infos, file_we, file_base_we):
    if_member_fns = ""
    for rpc_name in rpc_infos:
        return_type = create_return_type_str_server(rpc_infos[rpc_name], rpc_name)
        if "void" != return_type:
            if_member_fns += create_return_type_definition(return_type, rpc_infos[rpc_name]["returns"], "\t")
        parameter = create_fn_arguments_str(rpc_infos[rpc_name])
        if_member_fns += "\tvirtual {} {}({}) = 0;\n".format(return_type, rpc_name, parameter)

    outStr = """\
#ifndef {0}_H
#define {0}_H

#include "capnzero_typedefs.h"
#include "{1}.capnp.h"

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
