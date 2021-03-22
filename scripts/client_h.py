import global_types
from common import *
from common_client import *

def create_protected_section_rpc_decl(data):
    ret = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                method_name = create_rpc_method_name(service_name, rpc_name)
                if ("parameter" in rpc_info) and ("returns" in rpc_info):
                    ret += "\ttemplate<typename Callable>\n"
                    ret += "\tvoid _{0}(capnp::MessageBuilder& sndData, Callable&& rcvCb);\n".format(method_name)
                elif "parameter" in rpc_info:
                    ret += "\tinline\n"
                    ret += "\tvoid _{0}(capnp::MessageBuilder& sndData);\n".format(method_name)
                elif "returns" in rpc_info:
                    ret += "\ttemplate<typename Callable>\n"
                    ret += "\tvoid _{0}(Callable&& rcvCb);\n".format(method_name)
                else:
                    ret += "\tinline\n"
                    ret += "\tvoid _{0}();\n".format(method_name)
    return ret

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
        protected_section_rpc_decl = create_protected_section_rpc_decl(data)
        for service_name in data["services"]:
            if "rpc" in data["services"][service_name]:
                for rpc_name in data["services"][service_name]["rpc"]:
                    rpc_info = data["services"][service_name]["rpc"][rpc_name]
                    return_type_str = "void"
                    param1 = ""
                    param2 = ""
                    if "returns" in rpc_info:
                        members = rpc_info["returns"]
                        if isinstance(members, dict):
                            return_type_str = create_return_type_str_client(rpc_info, service_name, rpc_name)
                            return_struct_str = "\tstruct " + return_type_str + " {\n"
                            for member_name, member_type in members.items():
                                return_struct_str += "\t\t" + map_2_ret_type(member_type) + " " + member_name + ";\n"  
                            return_struct_str += "\t};\n"
                            public_section_rpc += return_struct_str
                        elif members == '__capnp__native__':
                            param2 = "Callable&& retCb"

                    parameter_str = create_fn_parameter_str_from_dict(rpc_info)
                    if param2 != "":
                        public_section_rpc += "\ttemplate<typename Callable>\n"
                        parameter_str += ", "
                    parameter_str += param2
                    method_name = create_rpc_method_name(service_name, rpc_name)
                    public_section_rpc +=  "\t" + return_type_str
                    public_section_rpc += " " if len(return_type_str) < 8 else "\n\t"
                    public_section_rpc += method_name + "(" + parameter_str + ");\n"
                    public_section_rpc += "\n"

        client_rpc_class = """\
namespace capnzero::{0}
{{

class {0}ClientRpc
{{
public:
    {0}ClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);  
{1}
protected:
{2}
private:
    zmq::socket_t m_zmqReqSocket;
}};

}} // namespace capnzero::{0}
""".format(file_we, public_section_rpc, protected_section_rpc_decl)

    client_signal_threaded_class = ""
    client_signal_class = ""
    if has_signal_handling:
        public_section_signal = ""
        if has_signal_handling:
            for service_name in data["services"]:
                if "signal" in data["services"][service_name]:
                    for signal_name in data["services"][service_name]["signal"]:
                        signal_info = data["services"][service_name]["signal"][signal_name]
                        cb_type_name = create_signal_cb_type(service_name, signal_name)
                        public_section_signal += "\tusing {} = std::function<void({})>;\n".format(cb_type_name, create_rpc_handler_fn_arguments_str(signal_info))
                        cb_register_fn_name =  create_signal_registration_method_name(service_name, signal_name)
                        public_section_signal += "\tvoid {}({} cb);\n".format(cb_register_fn_name, cb_type_name)

        signal_handling_cb_members = ""
        for service_name in data["services"]:
            if "signal" in data["services"][service_name]:
                for signal_name in data["services"][service_name]["signal"]:
                    signal_info = data["services"][service_name]["signal"][signal_name]
                    signal_handling_cb_members += "\t{} {};\n".format(create_signal_cb_type(service_name, signal_name), create_signal_cb_member(service_name, signal_name))

        client_signal_class = """\
#include <functional>
#include <capnp/message.h>

namespace capnzero::{0}
{{

class {0}ClientSignals
{{
public:
    {0}ClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
{1}
private:
    zmq::socket_t m_zmqSubSocket;
{2}
}};

}} // namespace capnzero::{0}
""".format(file_we, public_section_signal, signal_handling_cb_members)


        client_signal_threaded_class = """\
#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::{0}
{{

class {0}ClientSignalsThreaded
{{
public:
    {0}ClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~{0}ClientSignalsThreaded();
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

}} // namespace capnzero::{0}
""".format(file_we, public_section_signal, signal_handling_cb_members)


    client_class = ""
    if has_rpc and has_signal_handling:
        client_class = """\
class {0}Client : public {0}ClientRpc, public {0}ClientSignalsThreaded
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
class {0}Client : public {0}ClientSignalsThreaded
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
{4}

namespace capnzero::{1}
{{

{5}


}} // namespace capnzero::{1}
#include "{1}_Client.inl"
#endif
""".format(file_we.upper(), file_we, client_rpc_class, client_signal_class, client_signal_threaded_class, client_class)
    return outStr
