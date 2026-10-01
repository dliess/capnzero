
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "Calculator.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::Calculator
{

inline
CalculatorClientRpcTransport::CalculatorClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}

template<typename Callable>
void CalculatorClientRpcTransport::_add(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::ADD));
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
void CalculatorClientRpcTransport::_sub(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::SUB));
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
void CalculatorClientRpcTransport::_addDirectRet(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::ADD_DIRECT_RET));
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
void CalculatorClientRpcTransport::_subDirectRet(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::SUB_DIRECT_RET));
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
void CalculatorClientRpcTransport::_multiply(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::MULTIPLY));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

template<typename Callable>
void CalculatorClientRpcTransport::_multiply2(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::MULTIPLY2));
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
void CalculatorClientRpcTransport::_multiply3(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::MULTIPLY3));
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
void CalculatorClientRpcTransport::_multiply4(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::MULTIPLY4));
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
void CalculatorClientRpcTransport::_DirectRet__add(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::DIRECT_RET));
    builder.setRpcId(to_underlying(DirectRetRpcIds::ADD));
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
void CalculatorClientRpcTransport::_DirectRet__sub(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::DIRECT_RET));
    builder.setRpcId(to_underlying(DirectRetRpcIds::SUB));
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
void CalculatorClientRpcTransport::_Screen__setBrightness(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_BRIGHTNESS));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void CalculatorClientRpcTransport::_Screen__setColor(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_COLOR));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void CalculatorClientRpcTransport::_Screen__setTitle(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_TITLE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void CalculatorClientRpcTransport::_Screen__setId(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_ID));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}



} // namespace capnzero::Calculator
