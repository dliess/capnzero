import global_types
from common import *

def create_capnp_file_content_str(data):
    outStr = """\
@0x9c9f9131bf231692;

"""

    if "capnpdata" in data:
        outStr += data["capnpdata"]

    # Create capnp enum for ServiceId 
    service_enum_str = "enum ServiceId {\n"
    for idx, service_name in enumerate(data["services"]):
        service_enum_str += "\t" + lowerfirst(service_name) + " @" + str(idx) + ";\n"
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
            rpc_enum_string = "enum " + service_name + "RpcIds {\n"
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
                if "parameter" in data["services"][service_name]["rpc"][rpc_name]:
                    parameter_struct_str = "struct {}{{\n".format(create_capnp_rpc_parameter_type_str(service_name, rpc_name))
                    params = data["services"][service_name]["rpc"][rpc_name]["parameter"]
                    for idx, key in enumerate(params.keys()):
                        parameter_struct_str += "\t" + key + " @" + str(idx) + " :" + map_descr_type_to_capnp_type(params[key]) + ";\n"  
                    parameter_struct_str += "}\n"
                    outStr += parameter_struct_str
                if "returns" in data["services"][service_name]["rpc"][rpc_name]:
                    return_struct_str = "struct " + create_capnp_rpc_return_type_str(service_name, rpc_name) + " {\n"
                    members = data["services"][service_name]["rpc"][rpc_name]["returns"]
                    for idx, key in enumerate(members.keys()):
                        return_struct_str += "\t" + key + " @" + str(idx) + " :" + map_descr_type_to_capnp_type(members[key]) + ";\n"  
                    return_struct_str += "}\n"
                    outStr += return_struct_str

        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                if "parameter" in data["services"][service_name]["signal"][signal_name]:
                    parameter_struct_str = "struct {}{{\n".format(create_capnp_signal_param_type_str(service_name, signal_name))
                    params = data["services"][service_name]["signal"][signal_name]["parameter"]
                    for idx, key in enumerate(params.keys()):
                        parameter_struct_str += "\t" + key + " @" + str(idx) + " :" + map_descr_type_to_capnp_type(params[key]) + ";\n"  
                    parameter_struct_str += "}\n"
                    outStr += parameter_struct_str

    return outStr
