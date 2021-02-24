#!/usr/bin/python3

import toml
import getopt
import sys
import os.path

def upperfirst(x):
    return x[:1].upper() + x[1:] if x else ''

def lowerfirst(x):
    return x[:1].lower() + x[1:] if x else ''

def to_snake_case(name):
    import re
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name)

def is_integral_type(type):
    if "enumerations" in data:
        for enum_name in data["enumerations"]:
            if type == enum_name:
                return True
    return type == "Int8" or \
           type == "Int16" or \
           type == "Int32" or \
           type == "Int64" or \
           type == "UInt8" or \
           type == "UInt16" or \
           type == "UInt32" or \
           type == "UInt64" or \
           type == "Float32" or \
           type == "Float64"

def type_to_fn_parameter_pass_str(type):
    if is_integral_type(type):
        return type
    else:
        return "const {}&".format(type)

def create_fn_parameter_str(rpc_info):
    if not "parameter" in rpc_info:
        return ""
    params = rpc_info["parameter"]
    ret = ""
    for param_name, param_type in params.items():
        ret += type_to_fn_parameter_pass_str(param_type) + " " + param_name
        if list(params.keys())[-1] != param_name:
            ret += ", "
    return ret

def create_rpc_if_fn_parameter_str(rpc_info):
    if not "parameter" in rpc_info:
        return ""
    params = rpc_info["parameter"]
    ret = ""
    for param_name, param_type in params.items():
        ret += type_to_fn_parameter_pass_str(map_2_ca_call_param_type(param_type)) + " " + param_name
        if list(params.keys())[-1] != param_name:
            ret += ", "
    return ret

def create_return_type_str_client(service_name, rpc_name):
    return "Return" + service_name +  upperfirst(rpc_name)

def create_return_type_str_server(rpc_info, rpc_name):
    return "Return{}".format(upperfirst(rpc_name)) if "returns" in rpc_info else "void"

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
    import re
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

#####################################################
################### CAPNP FILE ######################
#####################################################
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
    if "enumerations" in data:
        for enum_name in data["enumerations"]:
            schema_enum_str += "enum {} {{\n".format(enum_name)
            for enum_element_name, enum_element_number in data["enumerations"][enum_name].items():
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

#####################################################
################### CLIENT H ########################
#####################################################
def create_capnzero_client_file_h_content_str(data, file_we):

    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    client_rpc_class = ""
    if has_rpc:
        public_section_rpc = ""
        if has_rpc:
            for service_name in data["services"]:
                if "rpc" in data["services"][service_name]:
                    for rpc_name in data["services"][service_name]["rpc"]:
                        return_type_str = "void"
                        if "returns" in data["services"][service_name]["rpc"][rpc_name]:
                            return_type_str = create_return_type_str_client(service_name, rpc_name)
                            return_struct_str = "\tstruct " + return_type_str + " {\n"
                            members = data["services"][service_name]["rpc"][rpc_name]["returns"]
                            for member_name, member_type in members.items():
                                return_struct_str += "\t\t" + map_2_ret_type(member_type) + " " + member_name + ";\n"  
                            return_struct_str += "\t};\n"
                            public_section_rpc += return_struct_str

                        parameter_str = ""
                        if "parameter" in data["services"][service_name]["rpc"][rpc_name]:
                            parameter_str = create_fn_parameter_str(data["services"][service_name]["rpc"][rpc_name])
                        method_name = service_name + "__" + rpc_name
                        public_section_rpc +=  "\t" + return_type_str
                        public_section_rpc += " " if len(return_type_str) < 8 else "\n\t"
                        public_section_rpc += method_name + "(" + parameter_str + ");\n"
            public_section_rpc += "\n"

        client_rpc_class = """\
namespace capnzero
{{

class {0}ClientRpc
{{
public:
    {0}ClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);  
{1}
private:
    zmq::socket_t m_zmqReqSocket;
}};

}} // namespace capnzero
""".format(file_we, public_section_rpc)

    client_signal_class = ""
    if has_signal_handling:
        public_section_signal = ""
        if has_signal_handling:
            for service_name in data["services"]:
                if "signal" in data["services"][service_name]:
                    for signal_name in data["services"][service_name]["signal"]:
                        signal_info = data["services"][service_name]["signal"][signal_name]
                        cb_type_name = create_signal_cb_type(service_name, signal_name)
                        public_section_signal += "\tusing {} = std::function<void({})>;\n".format(cb_type_name, create_rpc_if_fn_parameter_str(signal_info))
                        cb_register_fn_name = "on{}{}".format(upperfirst(service_name), upperfirst(signal_name))
                        public_section_signal += "\tvoid {}({} cb);\n".format(cb_register_fn_name, cb_type_name)

        signal_handling_cb_members = ""
        for service_name in data["services"]:
            if "signal" in data["services"][service_name]:
                for signal_name in data["services"][service_name]["signal"]:
                    signal_info = data["services"][service_name]["signal"][signal_name]
                    signal_handling_cb_members += "\t{} {};\n".format(create_signal_cb_type(service_name, signal_name), create_signal_cb_member(service_name, signal_name))

        client_signal_class = """\
#include <thread>
#include <functional>
#include <atomic>

namespace capnzero
{{

class {0}ClientSignals
{{
public:
    {0}ClientSignals(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~{0}ClientSignals();
    void StartThread();
	void signalReceiverThreadFn();
{1}
private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{{false}};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
{2}
}};

}} // namespace capnzero
""".format(file_we, public_section_signal, signal_handling_cb_members)


    client_class = ""
    if has_rpc and has_signal_handling:
        client_class = """\
class {0}Client : public {0}ClientRpc, public {0}ClientSignals
{{
public:
    {0}Client(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr);
}};
""".format(file_we)
    elif has_rpc:
        client_class = """\
class {0}Client : public {0}ClientRpc
{{
public:
    {0}Client(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
}};
""".format(file_we)
    elif has_signal_handling:
        client_class = """\
class {0}Client : public {0}ClientSignals
{{
public:
    {0}Client(zmq::context_t& rZmqContext, const std::string& serverSignalAddr);
}};
""".format(file_we)

    outStr = """\
#ifndef {0}_CLIENT_H
#define {0}_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "{1}.capnp.h"

{2}
{3}

namespace capnzero
{{

{4}

}} // namespace capnzero
#endif
""".format(file_we.upper(), file_we, client_rpc_class, client_signal_class, client_class)
    return outStr

#####################################################
################### CLIENT CPP ######################
#####################################################
def create_capnzero_client_file_cpp_content_str(data, file_we):

    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    outStr = """\
#include "{0}_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "{0}.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
""".format(file_we)

    if has_rpc:
        outStr += """\
{0}ClientRpc::{0}ClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{{
    m_zmqReqSocket.connect(serverRpcAddr);
}}
""".format(file_we)

    if has_signal_handling:
        outStr += """\
{0}ClientSignals::{0}ClientSignals(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{{
    m_zmqSubHelperSocket.bind("inproc://{0}SubHelper");
}}

{0}ClientSignals::~{0}ClientSignals()
{{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}}

void {0}ClientSignals::StartThread()
{{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() {{ signalReceiverThreadFn(); }});
}}

""".format(file_we)


    # define rpc-s
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                return_type_str = "void "
                if "returns" in data["services"][service_name]["rpc"][rpc_name]:
                    return_type_str = file_we + "ClientRpc::" + create_return_type_str_client(service_name, rpc_name) + "\n"
                parameter_str = ""
                if "parameter" in data["services"][service_name]["rpc"][rpc_name]:
                    parameter_str = create_fn_parameter_str(data["services"][service_name]["rpc"][rpc_name])
                method_name = service_name + "__" + rpc_name
                outStr +=  return_type_str + file_we + "ClientRpc::" + method_name + "(" + parameter_str + "){\n"
                outStr += """\
    ::capnp::MallocMessageBuilder message;
    {{
        auto builder = message.initRoot<RpcCoord>();
        builder.setServiceId(to_underlying(ServiceId::{0}));
        builder.setRpcId(to_underlying({1}RpcIds::{2}));
""".format(to_snake_case(service_name).upper(), \
                service_name, to_snake_case(rpc_name).upper())

                if "parameter" in data["services"][service_name]["rpc"][rpc_name]:
                    outStr += "\t\tsendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);\n"
                else:
                    outStr += "\t\tsendOverZmq(message, m_zmqReqSocket, zmq::send_flags::none);\n"
                outStr += "\t}\n"
                if "parameter" in data["services"][service_name]["rpc"][rpc_name]:
                    outStr += " \t{\n"
                    outStr += "\t\tauto paramBuilder = message.initRoot<{0}>();\n".format(create_capnp_rpc_parameter_type_str(service_name, rpc_name))
                    params = data["services"][service_name]["rpc"][rpc_name]["parameter"]
                    for param_name, param_type in params.items():
                        if(map_descr_type_to_capnp_type(param_type) == 'Data'):
                            outStr += "\t\tparamBuilder.set{0}(capnp::Data::Reader({1}.data(), {1}.size()));\n".format(upperfirst(param_name), param_name)
                        else:
                            outStr += "\t\tparamBuilder.set{0}({1});\n".format(upperfirst(param_name), param_name)
                    outStr += "\t\tsendOverZmq(message, m_zmqReqSocket, zmq::send_flags::none);\n"
                    outStr += "\t}\n"

                if "returns" in data["services"][service_name]["rpc"][rpc_name]:
                    outStr += "\tzmq::message_t revcMsg;\n"
                    outStr += "\tauto recvRes = m_zmqReqSocket.recv(revcMsg);\n"
                    outStr += "\tif(!recvRes)\n"
                    outStr += "\t{\n"
                    outStr += "\t\tthrow std::runtime_error(\"recv failed, nothing received\");\n"
                    outStr += "\t}\n"
                    outStr += "\t::capnp::FlatArrayMessageReader readMessage(\n"
                    outStr += "\tkj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(revcMsg.data()), revcMsg.size() / sizeof(capnp::word) )\n"
                    outStr += "\t);\n"
                    outStr += "\tauto reader = readMessage.getRoot<{}>();\n".format(create_capnp_rpc_return_type_str(service_name, rpc_name))
                    outStr += "\t" + create_return_type_str_client(service_name, rpc_name) + " retVal;\n"
                    ret_members = data["services"][service_name]["rpc"][rpc_name]["returns"]
                    for member_name, member_type in ret_members.items():
                        if map_descr_type_to_capnp_type(member_type) == 'Data':
                            outStr += "\tauto src = reader.get{}();\n".format(upperfirst(member_name))
                            outStr += "\tassert(src.size() == retVal.{}.size());\n".format(member_name)
                            outStr += "\tstd::copy(src.begin(), src.end(), retVal.{}.begin());\n".format(member_name)
                        else:
                            outStr += "\tretVal.{0} = reader.get{1}();\n".format(member_name, upperfirst(member_name))
                    outStr += "\treturn retVal;\n"
                outStr +=  "}\n\n"

    string_comparisons = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                cb_type_name = "{}{}Cb".format(upperfirst(service_name), upperfirst(signal_name))
                cb_register_fn_name = "on{}{}".format(upperfirst(service_name), upperfirst(signal_name))
                cb_member = "m_{}".format(lowerfirst(cb_type_name))
                zmq_sub_key = "{}{}".format(service_name, signal_name)
                string_comparisons += "\t{}(key == \"{}\"){{\n".format("if" if (string_comparisons == "") else "else if", zmq_sub_key)
                cb_call_params = ""
                if "parameter" in signal_info:
                    string_comparisons += "\t\tzmq::message_t paramBuf;\n"
                    string_comparisons += "\t\tauto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);\n"
                    string_comparisons += "\t\tif (!res2) { throw std::runtime_error(\"No received msg\"); }\n"
                    string_comparisons += "\t\tauto paramReader = getReader<{}>(paramBuf);\n".format(create_capnp_signal_param_type_str(service_name, signal_name))
                    param_info = signal_info["parameter"]
                    cb_call_params = ""
                    for param_name, param_type in param_info.items():
                        cpp_rpc_if_type = map_2_ca_call_param_type(param_type)
                        if cpp_rpc_if_type.startswith("Span"):
                            cb_call_params += "{0}(paramReader.get{1}().begin(), paramReader.get{1}().end())".format(cpp_rpc_if_type, upperfirst(param_name))
                        elif cpp_rpc_if_type == "TextView":
                            cb_call_params += "TextView(paramReader.get{0}().begin(), paramReader.get{0}().size())".format(upperfirst(param_name))
                        else:
                            cb_call_params += "paramReader.get{}()".format(upperfirst(param_name))
                        if list(param_info.keys())[-1] != param_name:
                            cb_call_params += ", "
                string_comparisons += "\t\tif({0}) {0}({1});\n".format(create_signal_cb_member(service_name, signal_name), cb_call_params)
                string_comparisons += "\t}\n"

    if has_signal_handling:
        outStr += """\
void {0}ClientSignals::signalReceiverThreadFn()
{{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://{0}SubHelper");

    while(!m_terminateRequest.load())
    {{
        zmq::pollitem_t items[] = {{
            {{zmqSubSocket.handle(), 0, ZMQ_POLLIN, 0}},
			{{zmqSubHelperSocket.handle(), 0, ZMQ_POLLIN, 0}}
        }};
        zmq::poll(&items[0], 2);
		if (items[0].revents & ZMQ_POLLIN)
		{{
            handleIncomingSignal(zmqSubSocket);
		}}
		if (items[1].revents & ZMQ_POLLIN)
		{{
			handleSubscriptionRequest(zmqSubHelperSocket, zmqSubSocket);
		}}
    }}
}}

void {0}ClientSignals::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) {{ throw std::runtime_error(\"No received msg\"); }}
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
{1}}}

void {0}ClientSignals::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) {{ throw std::runtime_error(\"No received msg\"); }}
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}}
""".format(file_we, string_comparisons)

    outStr += "\n"
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                cb_type_name = "{}{}Cb".format(upperfirst(service_name), upperfirst(signal_name))
                cb_register_fn_name = "on{}{}".format(upperfirst(service_name), upperfirst(signal_name))
                cb_member = "m_{}".format(lowerfirst(cb_type_name))
                zmq_sub_key = "{}{}".format(service_name, signal_name)
                outStr += """\
void {0}ClientSignals::{1}({2} cb)
{{
    if({3}) return; // set it only once
    {3} = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("{4}", {5}));
}}

""".format(file_we, cb_register_fn_name, cb_type_name, cb_member, zmq_sub_key, len(zmq_sub_key))



    client_class_constructor_definition = ""
    if has_rpc and has_signal_handling:
        client_class_constructor_definition = """\
{0}Client::{0}Client(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr) :
    {0}ClientRpc(rZmqContext, serverRpcAddr),
    {0}ClientSignals(rZmqContext, serverSignalAddr)
{{
    StartThread();
}}
""".format(file_we)
    elif has_rpc:
        client_class_constructor_definition = """\
{0}Client::{0}Client(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    {0}ClientRpc(rZmqContext, serverRpcAddr)
{{
}}
""".format(file_we)
    elif has_signal_handling:
        client_class_constructor_definition = """\
{0}Client::{0}Client(zmq::context_t& rZmqContext, const std::string& serverSignalAddr) :
    {0}ClientSignals(rZmqContext, serverSignalAddr)
{{
    StartThread();
}}
""".format(file_we)

    outStr += client_class_constructor_definition

    return outStr

#####################################################
################### SERVER H ########################
#####################################################
def create_capnzero_server_file_h_content_str(data, file_we):
    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    cbif_includes = ""
    cbif_members = ""
    constructor_parameters = ""
    signal_fn_declarations = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            cbif_includes += "#include \"{}{}.h\"\n".format(file_we, create_member_cb_if_type(service_name))
            constructor_parameters += "std::unique_ptr<{}> {}".format(create_member_cb_if_type(service_name), \
                                                                      lowerfirst(service_name))
            if list(data["services"].keys())[-1] != service_name:
                constructor_parameters += ", "
            cbif_members += "\tstd::unique_ptr<{}> {};\n".format(create_member_cb_if_type(service_name), \
                                                                create_member_cb_if(service_name))

        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_fn_declarations += "\t\tvoid {}__{}({});\n".format(service_name, signal_name, create_fn_parameter_str(data["services"][service_name]["signal"][signal_name]))


    constructor_decl = ""
    if has_rpc and has_signal_handling:
        constructor_decl = "\t{}Server(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr, {});".format(file_we, constructor_parameters)
    elif has_rpc:
        constructor_decl = "\t{}Server(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, {});".format(file_we, constructor_parameters)
    elif has_signal_handling:
        constructor_decl = "\t{}Server(zmq::context_t& rZmqContext, const std::string& signalBindAddr);".format(file_we)


    public_rpc_part = ""
    if has_rpc:
        public_rpc_part = """\
    enum class WaitMode {
        Blocking,
        NonBlocking
    };
    void processNextRequest(WaitMode waitMode = WaitMode::Blocking);      
"""

    public_signals_part = ""
    if has_signal_handling:
        public_signals_part = """\
    class Signals
    {{
    public:
        Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr);
{0}
    private:
        zmq::socket_t m_rZmqPubSocket;
    }};
    Signals &signals() {{ return m_signals; }}
""".format(signal_fn_declarations)


    private_rpc_part = ""
    if has_rpc:
        private_rpc_part = cbif_members + "\tzmq::socket_t m_zmqRepSocket;\n"

    private_signals_part = ""
    if has_signal_handling:
        private_signals_part = "\tSignals m_signals;\n"

    outStr = """\
#ifndef {0}_SERVER_H
#define {0}_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include "capnzero_typedefs.h"

{1}

namespace capnzero
{{

class {2}Server
{{
public:
{3}
{4}
{5}
private:
{6}
{7}
}};

}} // namespace capnzero
#endif
""".format(file_we.upper(), cbif_includes, file_we, constructor_decl, public_rpc_part, public_signals_part, private_rpc_part, private_signals_part)

    return outStr

#####################################################
################### SERVER CPP ######################
#####################################################
def create_capnzero_server_file_cpp_content_str(data, file_we):
    has_rpc = False
    has_signal_handling = False
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            has_signal_handling = True
        if "rpc" in data["services"][service_name]:
            has_rpc = True

    constructor_parameters = ""
    constructor_initializer_list = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            constructor_parameters += "std::unique_ptr<{}> {}".format(create_member_cb_if_type(service_name), \
                                                                lowerfirst(service_name))
            constructor_initializer_list += "\t{}(std::move({})),".format(create_member_cb_if(service_name), lowerfirst(service_name))

            if list(data["services"].keys())[-1] != service_name:
                constructor_parameters += ", "
                constructor_initializer_list += "\n"

    cases_str = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            cases_str += "\t\tcase to_underlying(ServiceId::{}):\n".format(to_snake_case(service_name).upper())
            cases_str +=  "\t\t{\n"
            cases_str += "\t\t\tswitch(coordReader.getRpcId())\n"
            cases_str += "\t\t\t{\n"
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                cases_str += "\t\t\t\tcase to_underlying({0}RpcIds::{1}):\n".format(service_name, to_snake_case(rpc_name).upper())
                cases_str += "\t\t\t\t{\n"
                params = ""
                if "parameter" in rpc_info:
                    cases_str += "\t\t\t\t\tzmq::message_t paramBuf;\n"
                    cases_str += "\t\t\t\t\tauto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);\n"
                    cases_str += "\t\t\t\t\tif (!res2) { throw std::runtime_error(\"No received msg\"); }\n"
                    cases_str += "\t\t\t\t\tauto paramReader = getReader<{}>(paramBuf);\n".format(create_capnp_rpc_parameter_type_str(service_name, rpc_name))
                    param_info = rpc_info["parameter"]
                    for param_name, param_type in param_info.items():
                        cpp_rpc_if_type = map_2_ca_call_param_type(param_type)
                        if cpp_rpc_if_type.startswith("Span"):
                            params += "{0}(paramReader.get{1}().begin(), paramReader.get{1}().end())".format(cpp_rpc_if_type, upperfirst(param_name))
                        elif cpp_rpc_if_type == "TextView":
                            params += "TextView(paramReader.get{0}().begin(), paramReader.get{0}().size())".format(upperfirst(param_name))
                        else:
                            params += "paramReader.get{}()".format(upperfirst(param_name))
                        if list(param_info.keys())[-1] != param_name:
                            params += ", "
                return_expr = ""
                if "returns" in rpc_info:
                    return_expr = "auto ret = "
                cases_str += "\t\t\t\t\t{}{}->{}({});\n".format(return_expr, create_member_cb_if(service_name), rpc_name, params)
                if "returns" in rpc_info:
                    cases_str += "\t\t\t\t\t::capnp::MallocMessageBuilder retMessage;\n"
                    cases_str += "\t\t\t\t\tauto builder = retMessage.initRoot<{}>();\n".format(create_capnp_rpc_return_type_str(service_name, rpc_name))
                    for return_name, return_type in rpc_info["returns"].items():
                        if map_descr_type_to_capnp_type(return_type) == "Data":
                            cases_str += "\t\t\t\t\tbuilder.set{0}(capnp::Data::Reader(ret.{1}.data(), ret.{1}.size()));\n".format(upperfirst(return_name), return_name)
                        else:
                            cases_str += "\t\t\t\t\tbuilder.set{}(ret.{});\n".format(upperfirst(return_name), return_name)
                    cases_str += "\t\t\t\t\tm_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);\n"
                    cases_str += "\t\t\t\t\tsendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);\n"
                cases_str += "\t\t\t\t\tbreak;\n"
                cases_str += "\t\t\t\t}\n"
            cases_str += "\t\t\t}\n"
            cases_str +=  "\t\t\tbreak;\n"
            cases_str +=  "\t\t}\n"

    signal_fn_definitions = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_fn_definitions += "void {0}Server::Signals::{1}__{2}({3})\n".format(file_we, service_name, signal_name, create_fn_parameter_str(data["services"][service_name]["signal"][signal_name]))
                signal_fn_definitions += "{\n"
                signal_info = data["services"][service_name]["signal"][signal_name]
                send_flag = "zmq::send_flags::sndmore" if ("parameter" in signal_info) else "zmq::send_flags::none"
                signal_fn_definitions += "\tm_rZmqPubSocket.send(zmq::const_buffer(\"{}{}\", {}), {});\n".format(service_name, signal_name, len(service_name) + len(signal_name), send_flag)
                if "parameter" in signal_info:
                    signal_fn_definitions += "\t::capnp::MallocMessageBuilder message;\n"
                    signal_fn_definitions += "\tauto builder = message.initRoot<{}>();\n".format(format(create_capnp_signal_param_type_str(service_name, signal_name)))
                    for param_name, param_type in signal_info["parameter"].items():
                        if map_descr_type_to_capnp_type(param_type) == "Data":
                            signal_fn_definitions += "\tbuilder.set{0}(capnp::Data::Reader({1}.data(), {1}.size()));\n".format(upperfirst(param_name), param_name)
                        else:
                            signal_fn_definitions += "\tbuilder.set{}({});\n".format(upperfirst(param_name), param_name)
                    signal_fn_definitions += "\tsendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);\n"
                signal_fn_definitions += "}\n\n"

    server_constructor_definition = ""
    if has_rpc and has_signal_handling:
        server_constructor_definition = """\
{0}Server::{0}Server(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr, {1}) :
{2}
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router),
    m_signals(rZmqContext, signalBindAddr)
{{
    m_zmqRepSocket.bind(rpcBindAddr);
}}
""".format(file_we, constructor_parameters, constructor_initializer_list)
    elif has_rpc:
        server_constructor_definition = """\
{0}Server::{0}Server(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, {1}) :
{2}
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router)
{{
    m_zmqRepSocket.bind(rpcBindAddr);
}}
""".format(file_we, constructor_parameters, constructor_initializer_list)
    elif has_signal_handling:
        server_constructor_definition = """\
{0}Server::{0}Server(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_signals(rZmqContext, signalBindAddr)
{{
}}
""".format(file_we)

    signal_constructor_definitions = ""
    if has_signal_handling:
        signal_constructor_definitions = """\
{0}Server::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::pub)
{{
    m_rZmqPubSocket.bind(signalBindAddr);
}}
""".format(file_we)



    outStr = """\
#include "{0}_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "{0}.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;

""".format(file_we)

    outStr += server_constructor_definition

    if has_rpc:
        outStr += """\
void {0}Server::processNextRequest(WaitMode waitMode) {{
    zmq::message_t routerId;
    auto res0 = m_zmqRepSocket.recv(routerId, (waitMode == WaitMode::Blocking) ? zmq::recv_flags::none : zmq::recv_flags::dontwait);
    if(waitMode == WaitMode::NonBlocking && !res0) return;
    zmq::message_t rpcCoordBuf;
    auto res1 = m_zmqRepSocket.recv(rpcCoordBuf, zmq::recv_flags::none);
    auto coordReader = getReader<RpcCoord>(rpcCoordBuf);
    switch(coordReader.getServiceId())
    {{
{1}
    }}
}}
""".format(file_we, cases_str)

    outStr += signal_constructor_definitions
    outStr += signal_fn_definitions

    return outStr


#####################################################
############ RPC INTERFACE HEADERS ##################
#####################################################
def create_capnzero_cbif_h_content_str(service_name, rpc_infos, file_we, file_base_we):
    if_member_fns = ""
    for rpc_name in rpc_infos:
        return_type = create_return_type_str_server(rpc_infos[rpc_name], rpc_name)
        if "void" != return_type:
            if_member_fns += create_return_type_definition(return_type, rpc_infos[rpc_name]["returns"], "\t")
        parameter = create_rpc_if_fn_parameter_str(rpc_infos[rpc_name])
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


#####################################################
#################### MAIN ###########################
#####################################################
outdir="undefined"
descrfile="undefined"
options, remainder = getopt.getopt(sys.argv[1:], ['o:d:'], ['outdir=', 'descrfile='])
for opt, arg in options:
    if opt in ('-o', '--outdir'):
        outdir = arg
    elif opt in ('-d', '--descrfile'):
        descrfile = arg

file_we = os.path.splitext(os.path.basename(descrfile))[0]

print("outdir: " + outdir)
print("descrfile: " + descrfile)
print("file_we: " + file_we)

capnp_file = outdir + "/" + file_we + ".capnp"
client_h_file = outdir + "/" + file_we + "_Client.h"
client_cpp_file = outdir + "/" + file_we + "_Client.cpp"
server_h_file = outdir + "/" + file_we + "_Server.h"
server_cpp_file = outdir + "/" + file_we + "_Server.cpp"

data = toml.load(descrfile)
with open(capnp_file, 'w') as open_file:
    open_file.write(create_capnp_file_content_str(data))

with open(client_h_file, 'w') as open_file:
    open_file.write(create_capnzero_client_file_h_content_str(data, file_we))

with open(client_cpp_file, 'w') as open_file:
    open_file.write(create_capnzero_client_file_cpp_content_str(data, file_we))

with open(server_h_file, 'w') as open_file:
    open_file.write(create_capnzero_server_file_h_content_str(data, file_we))

with open(server_cpp_file, 'w') as open_file:
    open_file.write(create_capnzero_server_file_cpp_content_str(data, file_we))

for service_name in data["services"]:
    service = data["services"][service_name]
    if "rpc" in service:
        rpc_if_filename_we = file_we + create_member_cb_if_type(service_name)
        with open(outdir + "/" + rpc_if_filename_we + ".h", 'w') as open_file:
            open_file.write(create_capnzero_cbif_h_content_str(service_name, service["rpc"], rpc_if_filename_we, file_we))
