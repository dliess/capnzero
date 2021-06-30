import re
import global_types

def upperfirst(x):
    return x[:1].upper() + x[1:] if x else ''

def lowerfirst(x):
    return x[:1].lower() + x[1:] if x else ''

def to_snake_case(name):
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name)

def add_whitespace(str_to_wrap, threshold):
    if len(str_to_wrap) > threshold:
        return str_to_wrap + "\n"
    else:
        return str_to_wrap + " "

def is_integral_type(capnz_type):
    for enum_name in global_types.enumerations:
        if capnz_type == enum_name:
            return True
    return capnz_type == "Int8" or \
           capnz_type == "Int16" or \
           capnz_type == "Int32" or \
           capnz_type == "Int64" or \
           capnz_type == "UInt8" or \
           capnz_type == "UInt16" or \
           capnz_type == "UInt32" or \
           capnz_type == "UInt64" or \
           capnz_type == "Float32" or \
           capnz_type == "Float64"

def type_to_fn_parameter_pass_str(capnz_type, converter_fn = None):
    converted_type = capnz_type if not converter_fn else converter_fn(capnz_type)
    if is_integral_type(capnz_type):
        return converted_type
    else:
        return "const {}&".format(converted_type)

class RPCType:
    Void = 0
    Dict = 1
    CapnpNative = 2
    DirectType = 3

def rpc_param_type(rpc_info):
    if "parameter" in rpc_info:
        if isinstance(rpc_info["parameter"], dict):
            return RPCType.Dict
        elif rpc_info["parameter"] == "__capnp__native__":
            return RPCType.CapnpNative
        else:
            raise Exception("Invalid type in rpc_info")
    else:
        return RPCType.Void

def rpc_return_type(rpc_info):
    if "returns" in rpc_info:
        if isinstance(rpc_info["returns"], dict):
            return RPCType.Dict
        elif rpc_info["returns"] == "__capnp__native__":
            return RPCType.CapnpNative
        else:
            return RPCType.DirectType
    else:
        return RPCType.Void

def create_rpc_method_name(service_name, rpc_name):
    if service_name == "_":
        return rpc_name
    else:
        return service_name + "__" + rpc_name

def create_signal_method_name(service_name, signal_name):
    if service_name == "_":
        return signal_name
    else:
        return service_name + "__" + signal_name

def create_signal_key_name(service_name, signal_name):
    return create_signal_method_name(service_name, signal_name)

def create_rpc_service_id_capnp_enum(service_name):
    if service_name == "_":
        return "main"
    else:
        return lowerfirst(service_name)

def create_var_name_from_service(service_name):
    if service_name == "_":
        return "_"
    else:
        return lowerfirst(service_name)

def create_rpc_service_id_enum(service_name):
    return to_snake_case(create_rpc_service_id_capnp_enum(service_name)).upper()

def create_rpc_id_enum(service_name):
    if service_name == "_":
        return "RpcIds"
    else:
        return service_name + "RpcIds"

def create_property_var_name(service_name, var_name):
    if service_name == "_":
        return lowerfirst(var_name)
    else:
        return lowerfirst(service_name) + "__" + var_name

def create_fn_parameter_str_from_dict(fn_info, converter_fn = None):
    param_type = rpc_param_type(fn_info)
    if param_type == RPCType.Void:
        return ""
    elif param_type == RPCType.Dict:
        return create_fn_parameter_str(fn_info["parameter"], converter_fn)
    elif param_type == RPCType.CapnpNative:
        return "::capnp::MessageBuilder& msgBuilder"

def create_fn_parameter_str(params, converter_fn = None):
    ret = ""
    for param_name, param_type in params.items():
        ret += type_to_fn_parameter_pass_str(param_type, converter_fn) + " " + param_name
        if list(params.keys())[-1] != param_name:
            ret += ", "
    return ret

def create_fn_input_parameter_str_sender(rpc_info, converter_fn = None):
    param_type = rpc_param_type(rpc_info)
    if param_type == RPCType.Void:
        return ""
    elif param_type == RPCType.Dict:
        return create_fn_parameter_str(rpc_info["parameter"], converter_fn)
    elif param_type == RPCType.CapnpNative:
        return "::capnp::MessageBuilder& sndData"

def create_fn_arguments_str(rpc_info):
    if not "parameter" in rpc_info:
        return ""
    params = rpc_info["parameter"]
    ret = ""
    if isinstance(params, dict):
        for param_name, param_type in params.items():
            ret += type_to_fn_parameter_pass_str(map_2_ca_call_param_type(param_type)) + " " + param_name
            if list(params.keys())[-1] != param_name:
                ret += ", "
    elif params == "__capnp__native__":
        ret += "::capnp::MessageBuilder& sndData"
    return ret

def create_rpc_handler_fn_arguments_str(rpc_info):
    param_type = rpc_param_type(rpc_info)
    if param_type == RPCType.Void:
        return ""
    elif param_type == RPCType.Dict:
        ret = ""
        params = rpc_info["parameter"]
        for param_name, p_type in params.items():
            ret += type_to_fn_parameter_pass_str(map_2_ca_call_param_type(p_type)) + " " + param_name
            if list(params.keys())[-1] != param_name:
                ret += ", "
        return ret
    elif param_type == RPCType.CapnpNative:
        return "::capnp::MessageReader& recvData"

def create_fn_arguments_param_only_str(rpc_info):
    if not "parameter" in rpc_info:
        return ""
    params = rpc_info["parameter"]
    ret = ""
    if isinstance(params, dict):
        for param_name in params:
            ret += param_name
            if list(params.keys())[-1] != param_name:
                ret += ", "
    elif params == "__capnp__native__":
        ret += "capnp::MessageBuilder& sndData"
    return ret

def create_return_type_str_server(rpc_info, rpc_name):
    ret_type = rpc_return_type(rpc_info)
    if ret_type == RPCType.Void:
        return "void"
    elif ret_type == RPCType.Dict:
        return "Return{}".format(upperfirst(rpc_name))
    elif ret_type == RPCType.CapnpNative:
        return "void"
    elif ret_type == RPCType.DirectType:
        return rpc_info["returns"]

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

def map_2_ret_type(type):
    if type == "Data":
        return "std::vector<uint8_t>"
    elif type == "Span":
        return "NOT A TYPE"  # TODO: replace with toml verification function
    else:
        return type

def map_2_ca_call_param_type(type):
    p = re.compile(r'Data<(\d+)>')
    m = p.match(type) 
    if m:
        return "SpanCL<{}>".format(m.group(1))
    elif type == "Data":
        return "Span"
    elif type == "Span":
        return "Span"
    elif type == "Text":
        return "TextView"
    else:
        return type

def create_member_cb_if(service_name):
    return "m_p{}RpcIf".format(upperfirst(service_name))

def create_member_cb_if_type(service_name):
    if service_name == "_":
        return "RpcIf"
    else:
        return "{}RpcIf".format(upperfirst(service_name))

def create_return_struct_definition(return_type, content, tabs):
    struct_content = ""
    for member_name, member_type in content.items():
        struct_content += "{}\t{} {};\n".format(tabs, map_2_ret_type(member_type), member_name)
    return """\
{0}struct {1}
{0}{{
{2}{0}}};
""".format(tabs, return_type, struct_content)

def create_signal_cb_type(service_name, signal_name):
    if service_name == "_":
        return "{}Cb".format(upperfirst(signal_name))
    else:
        return "{}{}Cb".format(upperfirst(service_name), upperfirst(signal_name))

def create_signal_subscription_cb_type(service_name, signal_name):
    if service_name == "_":
        return "{}SubscrCb".format(upperfirst(signal_name))
    else:
        return "{}{}SubscrCb".format(upperfirst(service_name), upperfirst(signal_name))

def create_signal_cb_member(service_name, signal_name):
    return "m_{}".format(lowerfirst(create_signal_cb_type(service_name, signal_name)))

def create_signal_subscription_cb_member(service_name, signal_name):
    return "m_{}".format(lowerfirst(create_signal_subscription_cb_type(service_name, signal_name)))

def map_type_to_qt_type(capnz_type):
    re_data = re.compile(r'Data<(\d+)>')
    for enum_name in global_types.enumerations:
        if capnz_type == enum_name:
            return capnz_type
    if capnz_type == "Int8":
        return "qint8"
    elif capnz_type == "Int16":
        return "qint16"
    elif capnz_type == "Int32":
        return "qint32"
    elif capnz_type == "Int64":
        return "qint64"
    elif capnz_type == "UInt8":
        return "quint8"
    elif capnz_type == "UInt16":
        return "quint16"
    elif capnz_type == "UInt32":
        return "quint32"
    elif capnz_type == "UInt64":
        return "quint64"
    elif capnz_type == "Float32":
        return "qreal"
    elif capnz_type == "Float64":
        return "qreal"
# This would be the case for a Qt Webchannel interface
#    elif re_data.match(capnz_type):
#        return "QString" # 64 encoding
#    elif capnz_type == "Span":
#        return "QString" # 64 encoding
    elif re_data.match(capnz_type):
        return "QByteArray"
    elif capnz_type == "Span":
        return "QByteArray"
    elif capnz_type == "Text":
        return "QString"
    else:
        return capnz_type
        #raise Exception("Invalid capnzero data type")

def create_rpc_return_type_for_qt_webchannel_obj(rpc_info):
    ret_type = rpc_return_type(rpc_info)
    if ret_type == RPCType.Void:
        return "void"
    elif ret_type == RPCType.Dict:
        return "QString"
    elif ret_type == RPCType.CapnpNative:
        return "QString"
    elif ret_type == RPCType.DirectType:
        return map_type_to_qt_type(rpc_info["returns"])

def create_rpc_return_type_for_qt_obj(rpc_info):
    ret_type = rpc_return_type(rpc_info)
    if ret_type == RPCType.Void:
        return "void"
    elif ret_type == RPCType.Dict:
        return "QString"
    elif ret_type == RPCType.CapnpNative:
        return "QString"
    elif ret_type == RPCType.DirectType:
        return map_type_to_qt_type(rpc_info["returns"])

def create_signal_fn_declarations(data, tabs, converter_fn = None):
    signal_fn_declarations = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                signal_fn_declarations += tabs + "void {}({});\n".format(create_signal_method_name(service_name, signal_name), create_fn_input_parameter_str_sender(signal_info, converter_fn))
    return signal_fn_declarations

def has_signals(data):
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            return True
    return False

def has_rpc(data):
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            return True
    return False