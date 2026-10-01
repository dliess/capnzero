
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TemperatureQtWithServiceName.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::TemperatureQtWithServiceName
{

inline
TemperatureQtWithServiceNameClientRpcTransport::TemperatureQtWithServiceNameClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}

inline
void TemperatureQtWithServiceNameClientRpcTransport::_TemperatureService__aTestRpc(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::TEMPERATURE_SERVICE));
    builder.setRpcId(to_underlying(TemperatureServiceRpcIds::A_TEST_RPC));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}

inline
void TemperatureQtWithServiceNameClientRpcTransport::_TemperatureService__setCommandedTemperature(capnp::MessageBuilder& sndData)
{
    ::capnp::MallocMessageBuilder message;
    auto builder = message.initRoot<RpcCoord>();
    builder.setServiceId(to_underlying(ServiceId::TEMPERATURE_SERVICE));
    builder.setRpcId(to_underlying(TemperatureServiceRpcIds::SET_COMMANDED_TEMPERATURE));
    sendOverZmq(message, m_zmqReqSocket, zmq::send_flags::sndmore);

    sendOverZmq(sndData, m_zmqReqSocket, zmq::send_flags::none);
}



} // namespace capnzero::TemperatureQtWithServiceName
