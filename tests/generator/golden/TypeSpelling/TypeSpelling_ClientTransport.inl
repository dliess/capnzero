
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TypeSpelling.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::TypeSpelling
{

inline
TypeSpellingClientRpcTransport::TypeSpellingClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}

template<typename Callable>
void TypeSpellingClientRpcTransport::_bytes(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::BYTES));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
    zmq::message_t revcMsg;
    auto recvRes = m_zmqReqSocket.recv(revcMsg);
    if(!recvRes)
    {
        throw std::runtime_error("recv failed, nothing received");
    }
    ::capnp::FlatArrayMessageReader readMessage(
	    kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(revcMsg.data()), revcMsg.size() / sizeof(capnp::word) )
	);
    rcvCb(readMessage);
}



} // namespace capnzero::TypeSpelling
