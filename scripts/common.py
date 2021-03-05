import re
import global_types

def upperfirst(x):
    return x[:1].upper() + x[1:] if x else ''

def lowerfirst(x):
    return x[:1].lower() + x[1:] if x else ''

def to_snake_case(name):
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name)

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

def create_fn_parameter_str(rpc_info, converter_fn = None):
    if not "parameter" in rpc_info:
        return ""
    params = rpc_info["parameter"]
    ret = ""
    for param_name, param_type in params.items():
        ret += type_to_fn_parameter_pass_str(param_type, converter_fn) + " " + param_name
        if list(params.keys())[-1] != param_name:
            ret += ", "
    return ret

def create_fn_arguments_str(rpc_info):
    if not "parameter" in rpc_info:
        return ""
    params = rpc_info["parameter"]
    ret = ""
    for param_name, param_type in params.items():
        ret += type_to_fn_parameter_pass_str(map_2_ca_call_param_type(param_type)) + " " + param_name
        if list(params.keys())[-1] != param_name:
            ret += ", "
    return ret

def create_fn_arguments_param_only_str(rpc_info):
    if not "parameter" in rpc_info:
        return ""
    ret = ""
    for param_name in rpc_info["parameter"]:
        ret += param_name
        if list(rpc_info["parameter"].keys())[-1] != param_name:
            ret += ", "
    return ret

def create_return_type_str_client(service_name, rpc_name):
    return "Return" + service_name +  upperfirst(rpc_name)

def create_return_type_str_server(rpc_info, rpc_name):
    if "returns" in rpc_info:
        return "Return{}".format(upperfirst(rpc_name))
    else: 
        return "void"

def create_capnp_rpc_parameter_type_str(service_name, rpc_name):
    return "CAPNPParameter" + service_name +  upperfirst(rpc_name)

def create_capnp_rpc_return_type_str(service_name, rpc_name):
    return "CAPNPReturn" + service_name +  upperfirst(rpc_name)

def create_capnp_signal_param_type_str(service_name, signal_name):
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
    return "m_p{}If".format(upperfirst(service_name))

def create_member_cb_if_type(service_name):
    return "{}If".format(upperfirst(service_name))

def create_return_type_definition(return_type, content, tabs):
    struct_content = ""
    for member_name, member_type in content.items():
        struct_content += "{}\t{} {};\n".format(tabs, map_2_ret_type(member_type), member_name)
    return """\
{0}struct {1}
{0}{{
{2}{0}}};
""".format(tabs, return_type, struct_content)

def create_signal_cb_type(service_name, signal_name):
    return "{}{}Cb".format(upperfirst(service_name), upperfirst(signal_name))

def create_signal_cb_member(service_name, signal_name):
    return "m_{}".format(lowerfirst(create_signal_cb_type(service_name, signal_name)))

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
    elif re_data.match(capnz_type):
        return "QString" # 64 encoding
    elif capnz_type == "Span":
        return "QString" # 64 encoding
    elif capnz_type == "Text":
        return "QString"
    else:
        return capnz_type
        #raise Exception("Invalid capnzero data type")

def create_rpc_return_type_for_qt_webchannel_obj(rpc_info):
    return_type_str = "void"
    if "returns" in rpc_info:
        if isinstance(rpc_info["returns"], dict):
            return_type_str = "QString"
        else:
            return_type_str = map_type_to_qt_type(rpc_info["returns"])
    return return_type_str
   
def create_signal_fn_declarations(data, tabs, converter_fn = None):
    signal_fn_declarations = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_fn_declarations += tabs + "void {}__{}({});\n".format(service_name, signal_name, create_fn_parameter_str(data["services"][service_name]["signal"][signal_name], converter_fn))
    return signal_fn_declarations
