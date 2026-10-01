#ifndef TEMPERATUREQT_CLIENT_TRANSPORT_H
#define TEMPERATUREQT_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TemperatureQt
{

class TemperatureQtClientRpcTransport
{
protected:
    inline
    TemperatureQtClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	inline
	void _explicitSetUUID(capnp::MessageBuilder& sndData);
	template<typename Callable>
	void _explicitGetUUID(Callable&& rcvCb);
	inline
	void _setCommandedTemperature(capnp::MessageBuilder& sndData);
	inline
	void _setEnabled(capnp::MessageBuilder& sndData);
	inline
	void _toggleEnabled();
	inline
	void _setUuid(capnp::MessageBuilder& sndData);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::TemperatureQt
#include "TemperatureQt_ClientTransport.inl"
#endif
