#ifndef CALCULATORQTWEBCHANNEL_CLIENT_TRANSPORT_H
#define CALCULATORQTWEBCHANNEL_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::CalculatorQtWebchannel
{

class CalculatorQtWebchannelClientRpcTransport
{
protected:
    inline
    CalculatorQtWebchannelClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	inline
	void _setMode(capnp::MessageBuilder& sndData);
	template<typename Callable>
	void _Calculator__add(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _Calculator__sub(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	inline
	void _Screen__setBrightness(capnp::MessageBuilder& sndData);
	inline
	void _Screen__setColor(capnp::MessageBuilder& sndData);
	template<typename Callable>
	void _Screen__getColor(Callable&& rcvCb);
	inline
	void _Screen__setTitle(capnp::MessageBuilder& sndData);
	inline
	void _Screen__setId(capnp::MessageBuilder& sndData);
	template<typename Callable>
	void _Screen__alltypes(capnp::MessageBuilder& sndData, Callable&& rcvCb);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::CalculatorQtWebchannel
#include "CalculatorQtWebchannel_ClientTransport.inl"
#endif
