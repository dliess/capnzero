import global_types
from common import *

def create_subscr_cb_reg_method_declarations(data):
    ret = ""
    for service_name in data["services"]:
        if "signal" in data["services"][service_name]:
            for signal_name in data["services"][service_name]["signal"]:
                signal_info = data["services"][service_name]["signal"][signal_name]
                ret += "\t\t\tvoid register{}(SubscriptionCb cb);\n".format(create_signal_subscription_cb_type(service_name, signal_name))
    return ret

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
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            cbif_includes += "#include \"{}{}.h\"\n".format(file_we, create_member_cb_if_type(service_name))
            constructor_parameters += "std::unique_ptr<{}> {}".format(create_member_cb_if_type(service_name), \
                                                                      create_var_name_from_service(service_name))
            if list(data["services"].keys())[-1] != service_name:
                constructor_parameters += ", "
            cbif_members += "\tstd::unique_ptr<{}> {};\n".format(create_member_cb_if_type(service_name), \
                                                                create_member_cb_if(service_name))

    signal_fn_declarations = create_signal_fn_declarations(data, "\t\t")

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
    void processNextRequestAllNonBlock();
  	int getFd() const;
"""

    public_signals_part = ""
    if has_signal_handling:
        public_signals_part = """\
    class Signals
    {{
    public:
        Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr);
        int getFd() const;
        void handleAllSubscriptions();
{0}
        using SubscriptionCb = std::function<void(Signals&)>;
{1}
    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    }};
    Signals &signals() {{ return m_signals; }}
""".format(signal_fn_declarations, create_subscr_cb_reg_method_declarations(data))


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
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"

{1}

namespace capnzero::{2}
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

}} // namespace capnzero::{2}
#endif
""".format(file_we.upper(), cbif_includes, file_we, constructor_decl, public_rpc_part, public_signals_part, private_rpc_part, private_signals_part)

    return outStr
