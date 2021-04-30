import global_types
from common import *
from common_client import *

def create_protected_section_rpc_def(data, file_we):
    protected_section_rpc_def = ""
    for service_name in data["services"]:
        if "rpc" in data["services"][service_name]:
            for rpc_name in data["services"][service_name]["rpc"]:
                rpc_info = data["services"][service_name]["rpc"][rpc_name]
                method_name = create_rpc_method_name(service_name, rpc_name)
                rpc_coord_send_str = """\
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::{0}));
    builder.setRpcId(to_underlying({1}::{2}));
    sendOverZmq(message, m_zmqReqSocket, {3});
""".format(create_rpc_service_id_enum(service_name), \
          create_rpc_id_enum(service_name), \
          to_snake_case(rpc_name).upper(), \
          "zmq::send_flags::sndmore" if "parameter" in rpc_info else "zmq::send_flags::none")

                if ("parameter" in rpc_info) and ("returns" in rpc_info):
                    protected_section_rpc_def += """\
template<typename Callable>
void {0}ClientRpcTransport::_{1}(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{{
{2}
    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
    zmq::message_t revcMsg;
    auto recvRes = m_zmqReqSocket.recv(revcMsg);
    if(!recvRes)
    {{
        throw std::runtime_error("recv failed, nothing received");
    }}
    ::capnp::UnalignedFlatArrayMessageReader readMessage(
	    kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(revcMsg.data()), revcMsg.size() / sizeof(capnp::word) )
	);
    rcvCb(readMessage);
}}

""".format(file_we, method_name, rpc_coord_send_str)
                elif "parameter" in rpc_info:
                    protected_section_rpc_def += """\
inline
void {0}ClientRpcTransport::_{1}(capnp::MessageBuilder& sndData)
{{
{2}
    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}}

""".format(file_we, method_name, rpc_coord_send_str)

                elif "returns" in rpc_info:
                    protected_section_rpc_def += """\
template<typename Callable>
void {0}ClientRpcTransport::_{1}(Callable &&rcvCb)
{{
{2}
    zmq::message_t revcMsg;
    auto recvRes = m_zmqReqSocket.recv(revcMsg);
    if(!recvRes)
    {{
        throw std::runtime_error("recv failed, nothing received");
    }}
    ::capnp::UnalignedFlatArrayMessageReader readMessage(
	    kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(revcMsg.data()), revcMsg.size() / sizeof(capnp::word) )
	);
    rcvCb(readMessage);
}}

""".format(file_we, method_name, rpc_coord_send_str)
                else:
                    protected_section_rpc_def += """\
inline
void {0}ClientRpcTransport::_{1}()
{{
{2}
}}
""".format(file_we, method_name, rpc_coord_send_str)
    return protected_section_rpc_def


def create_capnzero_client_transport_file_inl_content_str(data, file_we):
    return """
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "{0}.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::{0}
{{

inline
{0}ClientRpcTransport::{0}ClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{{
    m_zmqReqSocket.connect(serverRpcAddr);
}}

{1}

}} // namespace capnzero::{0}
""".format(file_we, create_protected_section_rpc_def(data, file_we))
