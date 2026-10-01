"""Shared wire names and schema mappings. These are protocol compatibility rules."""
import re

def upperfirst(x):
    return x[:1].upper() + x[1:] if x else ''


def lowerfirst(x):
    return x[:1].lower() + x[1:] if x else ''


def create_signal_key_name(service_name, signal_name):
    return signal_name if service_name == "_" else service_name + "__" + signal_name


def create_rpc_service_id_capnp_enum(service_name):
    if service_name == "_":
        return "main"
    else:
        return lowerfirst(service_name)


def create_rpc_id_enum(service_name):
    if service_name == "_":
        return "RpcIds"
    else:
        return service_name + "RpcIds"


def create_capnp_rpc_parameter_type_str(service_name, rpc_name):
    if service_name == "_":
        return "CAPNPParameter" + upperfirst(rpc_name)
    else:
        return "CAPNPParameter" + service_name +  upperfirst(rpc_name)


def create_capnp_rpc_return_type_str(service_name, rpc_name):
    if service_name == "_":
        return "CAPNPReturn" + upperfirst(rpc_name)
    else: 
        return "CAPNPReturn" + service_name +  upperfirst(rpc_name)


def create_capnp_signal_param_type_str(service_name, signal_name):
    if service_name == "_":
        return "CAPNPSignalParameter" + upperfirst(signal_name)
    else:
        return "CAPNPSignalParameter" + service_name +  upperfirst(signal_name)


def map_descr_type_to_capnp_type(type):
    import re
    p = re.compile(r'Data<\d+>')
    if p.match(type) or type == "Span":
        return "Data"
    else:
        return type

