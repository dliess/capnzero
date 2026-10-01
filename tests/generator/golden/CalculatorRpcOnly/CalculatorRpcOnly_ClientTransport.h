#ifndef CALCULATORRPCONLY_CLIENT_TRANSPORT_H
#define CALCULATORRPCONLY_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::CalculatorRpcOnly
{

class CalculatorRpcOnlyClientRpcTransport
{
protected:
    inline
    CalculatorRpcOnlyClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	template<typename Callable>
	void _Calculator__add(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _Calculator__sub(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	inline
	void _Screen__setBrightness(capnp::MessageBuilder& sndData);
	inline
	void _Screen__setTitle(capnp::MessageBuilder& sndData);
	inline
	void _Screen__setId(capnp::MessageBuilder& sndData);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::CalculatorRpcOnly
#include "CalculatorRpcOnly_ClientTransport.inl"
#endif
