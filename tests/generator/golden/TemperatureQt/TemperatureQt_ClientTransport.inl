
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TemperatureQt.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::TemperatureQt
{

inline
TemperatureQtClientRpcTransport::TemperatureQtClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}

inline
void TemperatureQtClientRpcTransport::_explicitSetUUID(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::EXPLICIT_SET_U_U_I_D));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

template<typename Callable>
void TemperatureQtClientRpcTransport::_explicitGetUUID(Callable &&rcvCb)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::EXPLICIT_GET_U_U_I_D));
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
void TemperatureQtClientRpcTransport::_setCommandedTemperature(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::SET_COMMANDED_TEMPERATURE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void TemperatureQtClientRpcTransport::_setEnabled(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::SET_ENABLED));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void TemperatureQtClientRpcTransport::_toggleEnabled()
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::TOGGLE_ENABLED));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::none);

}
inline
void TemperatureQtClientRpcTransport::_setUuid(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::MAIN));
    builder.setRpcId(to_underlying(RpcIds::SET_UUID));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}



} // namespace capnzero::TemperatureQt
