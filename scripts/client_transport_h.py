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


def create_capnzero_client_transport_file_h_content_str(data, file_we):
    return """\
#ifndef {0}_CLIENT_TRANSPORT_H
#define {0}_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::{1}
{{

class {1}ClientRpcTransport
{{
protected:
    inline
    {1}ClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
{2}
private:
    zmq::socket_t m_zmqReqSocket;
}};

}} // namespace capnzero::{1}
#include "{1}_ClientTransport.inl"
#endif
""".format(file_we.upper(), file_we, create_protected_section_rpc_decl(data))