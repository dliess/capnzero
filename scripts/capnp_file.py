import global_types
import subprocess
from common import *

def create_capnp_file_content_str(data, file_we):
    the_id = subprocess.check_output(['capnp', 'id']).decode('utf-8').rstrip()

    outStr = """\
{};

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::{}");

""".format(the_id, file_we)

    if "capnp-inline" in data:
        outStr += data["capnp-inline"]

    # Create capnp enum for ServiceId 
    service_enum_str = "enum ServiceId {\n"
    for idx, service_name in enumerate(data["services"]):
        service_enum_str += "\t" + create_rpc_service_id_capnp_enum(service_name) + " @" + str(idx) + ";\n"
    service_enum_str += "}\n"

    outStr += service_enum_str

    #create user defined enumerations
    schema_enum_str = ""
    for enum_name in global_types.enumerations:
        schema_enum_str += "enum {} {{\n".format(enum_name)
        for enum_element_name, enum_element_number in global_types.enumerations[enum_name].items():
            schema_enum_str += "\t{} @{};\n".format(lowerfirst(enum_element_name), enum_element_number)
        schema_enum_str +="}\n"
    outStr += schema_enum_str

    # Create capnp enum for rpc Ids 
    # convention for enum name: <service_name> + "RpcIds
    rpc_enum_strings = []
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:

            rpc_enum_string = "enum " + create_rpc_id_enum(service_name) + "{\n"
            for idx, rpc_name in enumerate(data["services"][service_name]["rpc"]):
                rpc_enum_string += "\t" + rpc_name + " @" + str(idx) + ";\n"
            rpc_enum_string += "}\n"
            rpc_enum_strings.append(rpc_enum_string)

    for rpc_enum in rpc_enum_strings:
        outStr += rpc_enum

    outStr += "struct RpcCoord {\n"
    outStr += "\tserviceId @0 :UInt16;\n"
    outStr += "\trpcId @1 :UInt16;\n"
    outStr += "}\n"

    # Create capnp type for parameter and return types
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                if "parameter" in rpc_info and isinstance(rpc_info["parameter"], dict):
                    parameter_struct_str = "struct {}{{\n".format(create_capnp_rpc_parameter_type_str(service_name, rpc_name))
                    params = rpc_info["parameter"]
                    for idx, key in enumerate(params.keys()):
                        parameter_struct_str += "\t" + key + " @" + str(idx) + " :" + map_descr_type_to_capnp_type(params[key]) + ";\n"  
                    parameter_struct_str += "}\n"
                    outStr += parameter_struct_str
                return_type = rpc_return_type(rpc_info)
                if return_type == RPCType.Void:
                    pass
                elif return_type == RPCType.Dict:
                    return_struct_str = "struct " + create_capnp_rpc_return_type_str(service_name, rpc_name) + " {\n"
                    members = rpc_info["returns"]
                    for idx, key in enumerate(members.keys()):
                        return_struct_str += "\t" + key + " @" + str(idx) + " :" + map_descr_type_to_capnp_type(members[key]) + ";\n"  
                    return_struct_str += "}\n"
                    outStr += return_struct_str
                elif return_type == RPCType.CapnpNative:
                    pass
                elif return_type == RPCType.DirectType:
                    return_struct_str = "struct " + create_capnp_rpc_return_type_str(service_name, rpc_name) + " {\n"
                    return_struct_str += "\tretParam @0 :" + map_descr_type_to_capnp_type(rpc_info["returns"]) + ";\n"  
                    return_struct_str += "}\n"
                    outStr += return_struct_str

        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                if "parameter" in signal_info and isinstance(signal_info["parameter"], dict):
                    params = signal_info["parameter"]
                    parameter_struct_str = "struct {}{{\n".format(create_capnp_signal_param_type_str(service_name, signal_name))
                    for idx, key in enumerate(params.keys()):
                        parameter_struct_str += "\t" + key + " @" + str(idx) + " :" + map_descr_type_to_capnp_type(params[key]) + ";\n"  
                    parameter_struct_str += "}\n"
                    outStr += parameter_struct_str

    return outStr
