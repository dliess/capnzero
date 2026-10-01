#ifndef CALCULATOR_CLIENT_TRANSPORT_H
#define CALCULATOR_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::Calculator
{

class CalculatorClientRpcTransport
{
protected:
    inline
    CalculatorClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	template<typename Callable>
	void _add(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _sub(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _addDirectRet(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _subDirectRet(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	inline
	void _multiply(capnp::MessageBuilder& sndData);
	template<typename Callable>
	void _multiply2(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _multiply3(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _multiply4(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _DirectRet__add(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _DirectRet__sub(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	inline
	void _Screen__setBrightness(capnp::MessageBuilder& sndData);
	inline
	void _Screen__setColor(capnp::MessageBuilder& sndData);
	inline
	void _Screen__setTitle(capnp::MessageBuilder& sndData);
	inline
	void _Screen__setId(capnp::MessageBuilder& sndData);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::Calculator
#include "Calculator_ClientTransport.inl"
#endif
