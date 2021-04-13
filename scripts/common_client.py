import global_types
from common import *


def should_be_template(rpc_info):
    if "returns" in rpc_info:
        if rpc_info["returns"] == "__capnp__native__":
            return True
    return False

def public_method_name(service_name, rpc_name):
    if service_name == "_":
        return rpc_name
    else:
        return service_name + "__" + rpc_name

def protected_method_name(service_name, rpc_name):
    return "_" + public_method_name(service_name, rpc_name)

def create_signal_registration_method_name(service_name, signal_name):
    if service_name == "_":
        return "on{}".format(upperfirst(signal_name))
    else:
        return "on{}{}".format(upperfirst(service_name), upperfirst(signal_name))

def create_return_type_str_client(rpc_info, service_name, rpc_name, class_namespace = None, type_mapper_fn = None):
    return_type = rpc_return_type(rpc_info)
    if return_type == RPCType.Void or return_type == RPCType.CapnpNative:
        return "void"
    elif return_type == RPCType.DirectType:
        if type_mapper_fn:
            return type_mapper_fn(rpc_info["returns"])
        else:
            return rpc_info["returns"]
    else:
        if class_namespace:
            return "{}::{}".format(class_namespace, "Return" + service_name +  upperfirst(rpc_name))
        else:
            return "{}".format("Return" + service_name +  upperfirst(rpc_name))

def create_rpc_client_method_head(rpc_info, service_name, rpc_name, class_namespace, indent = "", type_converter_fn = None):
    ret_str = ""
    #TODO: this is a dirty hack:
    if type_converter_fn == map_type_to_qt_type:
        the_class_namespace = ""
    else:
        the_class_namespace = class_namespace
    return_type_str = create_return_type_str_client(rpc_info, service_name, rpc_name, the_class_namespace, type_mapper_fn = type_converter_fn)

    method_name = public_method_name(service_name, rpc_name)

    parameter_str = create_fn_input_parameter_str_sender(rpc_info, converter_fn = type_converter_fn)
    return_type = rpc_return_type(rpc_info)
    if return_type == RPCType.CapnpNative:
        if parameter_str != "":
            parameter_str += ", "
        ret_str += indent + "template <typename Callable>\n"
        parameter_str += "Callable&& cb"

    if class_namespace:
        ret_str += indent + "{0}{1}::{2}({3})".format(add_whitespace(return_type_str, 10), class_namespace, method_name, parameter_str)
    else:
        ret_str += indent + "{0}{2}({3})".format(add_whitespace(return_type_str, 10), method_name, parameter_str)
    return ret_str

def create_client_definition_for_rpc(rpc_info, service_name, rpc_name, file_we, class_namespace, type_converter_fn = None):
    method_head = create_rpc_client_method_head(rpc_info, service_name, rpc_name, class_namespace, type_converter_fn = type_converter_fn)
    param_type = rpc_param_type(rpc_info)
    return_type = rpc_return_type(rpc_info)

    req_msg_builder = ""
    if param_type == RPCType.Dict:
        req_msg_builder += "\t::capnp::MallocMessageBuilder message;\n"
        req_msg_builder += "\tauto paramBuilder = message.initRoot<{0}>();\n".format(create_capnp_rpc_parameter_type_str(service_name, rpc_name))
        for param_name, p_type in rpc_info["parameter"].items():
            if(map_descr_type_to_capnp_type(p_type) == 'Data'):
                if type_converter_fn and type_converter_fn(p_type) == "QByteArray":
                    req_msg_builder += "\tparamBuilder.set{0}(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>({1}.data()), {1}.size()));\n".format(upperfirst(param_name), param_name)
                else:
                    req_msg_builder += "\tparamBuilder.set{0}(capnp::Data::Reader({1}.data(), {1}.size()));\n".format(upperfirst(param_name), param_name)
            else:
                if type_converter_fn and type_converter_fn(p_type) == "QString":
                    req_msg_builder += "\tparamBuilder.set{0}({1}.toUtf8().constData());\n".format(upperfirst(param_name), param_name)
                else:
                    req_msg_builder += "\tparamBuilder.set{0}({1});\n".format(upperfirst(param_name), param_name)
        
    param1 = ""
    if param_type == RPCType.Void:
        pass
    elif param_type == RPCType.Dict:
        param1 = "\t\tmessage"
    elif param_type == RPCType.CapnpNative:
        param1 = "\t\tsndData"

    param2 = ""
    if return_type == RPCType.Void:
        pass
    elif return_type == RPCType.Dict:
        lambda_content = ""
        lambda_content += "\tauto reader = message.getRoot<{}>();\n".format(create_capnp_rpc_return_type_str(service_name, rpc_name))
        for member_name, member_type in rpc_info["returns"].items():
            if map_descr_type_to_capnp_type(member_type) == 'Data':
                lambda_content += "\t\t\tauto src = reader.get{}();\n".format(upperfirst(member_name))
                lambda_content += "\t\t\tassert(src.size() == retVal.{}.size());\n".format(member_name)
                lambda_content += "\t\t\tstd::copy(src.begin(), src.end(), retVal.{}.begin());\n".format(member_name)
            else:
                if type_converter_fn and type_converter_fn(member_type) == "QString":
                    lambda_content += "\t\t\tretVal.{0} = reader.get{1}().cStr();\n".format(member_name, upperfirst(member_name))
                else:
                    lambda_content += "\t\t\tretVal.{0} = reader.get{1}();\n".format(member_name, upperfirst(member_name))
        param2 += "\t\t[&retVal](capnp::MessageReader& message){{\n\t\t{0}\n\t}}".format(lambda_content)
    elif return_type == RPCType.CapnpNative:
        param2 += "\t\tcb"
    elif return_type == RPCType.DirectType:
        lambda_content = ""
        lambda_content += "\tauto reader = message.getRoot<{}>();\n".format(create_capnp_rpc_return_type_str(service_name, rpc_name))
        member_name = "retParam"
        member_type = rpc_info["returns"]
        if map_descr_type_to_capnp_type(member_type) == 'Data':
            lambda_content += "\t\t\tauto src = reader.get{}();\n".format(upperfirst(member_name))
            lambda_content += "\t\t\tassert(src.size() == retVal.{}.size());\n".format(member_name)
            lambda_content += "\t\t\tstd::copy(src.begin(), src.end(), retVal.{}.begin());\n".format(member_name)
        else:
            if type_converter_fn and type_converter_fn(member_type) == "QString":
                lambda_content += "\t\t\tretVal = reader.get{0}().cStr();\n".format(upperfirst(member_name))
            else:
                lambda_content += "\t\t\tretVal = reader.get{0}();\n".format(upperfirst(member_name))
        param2 += "\t\t[&retVal](capnp::MessageReader& message){{\n\t\t{0}\n\t}}".format(lambda_content)

    if param1 != "" and param2 != "":
        param1 += ", "

    return_var = ""
    return_retval = ""
    if return_type != RPCType.Void and return_type != RPCType.CapnpNative:
        return_var += "\t" + create_return_type_str_client(rpc_info, service_name, rpc_name, type_mapper_fn = type_converter_fn) + " retVal;\n"
        return_retval += "\treturn retVal;"

    method_name = public_method_name(service_name, rpc_name)

    return """\
{0}
{{
{1}
{2}
    _{3}(
{4}
{5}
    );
{6}
}}

""".format(method_head, req_msg_builder, return_var, method_name, param1, param2, return_retval)

def is_valid_for_qt_rpc_client(rpc_info):
    return rpc_param_type(rpc_info) != RPCType.CapnpNative and rpc_return_type(rpc_info) != RPCType.CapnpNative