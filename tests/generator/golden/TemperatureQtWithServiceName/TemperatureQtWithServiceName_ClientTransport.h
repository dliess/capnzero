#ifndef TEMPERATUREQTWITHSERVICENAME_CLIENT_TRANSPORT_H
#define TEMPERATUREQTWITHSERVICENAME_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TemperatureQtWithServiceName
{

class TemperatureQtWithServiceNameClientRpcTransport
{
protected:
    inline
    TemperatureQtWithServiceNameClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	inline
	void _TemperatureService__aTestRpc(capnp::MessageBuilder& sndData);
	inline
	void _TemperatureService__setCommandedTemperature(capnp::MessageBuilder& sndData);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::TemperatureQtWithServiceName
#include "TemperatureQtWithServiceName_ClientTransport.inl"
#endif
