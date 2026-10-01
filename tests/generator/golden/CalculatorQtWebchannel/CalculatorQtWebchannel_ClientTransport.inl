
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "CalculatorQtWebchannel.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::CalculatorQtWebchannel
{

inline
CalculatorQtWebchannelClientRpcTransport::CalculatorQtWebchannelClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}

inline
void CalculatorQtWebchannelClientRpcTransport::_setMode(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::SET_MODE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

template<typename Callable>
void CalculatorQtWebchannelClientRpcTransport::_Calculator__add(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::CALCULATOR));
    builder.setRpcId(to_underlying(CalculatorRpcIds::ADD));
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
void CalculatorQtWebchannelClientRpcTransport::_Calculator__sub(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::CALCULATOR));
    builder.setRpcId(to_underlying(CalculatorRpcIds::SUB));
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
void CalculatorQtWebchannelClientRpcTransport::_Screen__setBrightness(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_BRIGHTNESS));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void CalculatorQtWebchannelClientRpcTransport::_Screen__setColor(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_COLOR));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

template<typename Callable>
void CalculatorQtWebchannelClientRpcTransport::_Screen__getColor(Callable &&rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::GET_COLOR));
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

inline
void CalculatorQtWebchannelClientRpcTransport::_Screen__setTitle(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_TITLE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void CalculatorQtWebchannelClientRpcTransport::_Screen__setId(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::SET_ID));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

template<typename Callable>
void CalculatorQtWebchannelClientRpcTransport::_Screen__alltypes(capnp::MessageBuilder& sndData, Callable&& rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::SCREEN));
    builder.setRpcId(to_underlying(ScreenRpcIds::ALLTYPES));
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



} // namespace capnzero::CalculatorQtWebchannel
