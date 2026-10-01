
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "Edges.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::Edges
{

inline
EdgesClientRpcTransport::EdgesClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}

template<typename Callable>
void EdgesClientRpcTransport::_Zebra__empty(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::ZEBRA));
    builder.setRpcId(to_underlying(ZebraRpcIds::EMPTY));
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

template<typename Callable>
void EdgesClientRpcTransport::_Zebra__noArgs(Callable &&rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::ZEBRA));
    builder.setRpcId(to_underlying(ZebraRpcIds::NO_ARGS));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::none);

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

template<typename Callable>
void EdgesClientRpcTransport::_Zebra__bytes(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::ZEBRA));
    builder.setRpcId(to_underlying(ZebraRpcIds::BYTES));
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

inline
void EdgesClientRpcTransport::_Zebra__setActive(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::ZEBRA));
    builder.setRpcId(to_underlying(ZebraRpcIds::SET_ACTIVE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void EdgesClientRpcTransport::_Zebra__toggleActive()
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::ZEBRA));
    builder.setRpcId(to_underlying(ZebraRpcIds::TOGGLE_ACTIVE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::none);

}
inline
void EdgesClientRpcTransport::_fire()
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::FIRE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::none);

}


} // namespace capnzero::Edges
