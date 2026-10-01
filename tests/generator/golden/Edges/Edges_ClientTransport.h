#ifndef EDGES_CLIENT_TRANSPORT_H
#define EDGES_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::Edges
{

class EdgesClientRpcTransport
{
protected:
    inline
    EdgesClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	template<typename Callable>
	void _Zebra__empty(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	template<typename Callable>
	void _Zebra__noArgs(Callable&& rcvCb);
	template<typename Callable>
	void _Zebra__bytes(capnp::MessageBuilder& sndData, Callable&& rcvCb);
	inline
	void _Zebra__setActive(capnp::MessageBuilder& sndData);
	inline
	void _Zebra__toggleActive();
	inline
	void _fire();

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::Edges
#include "Edges_ClientTransport.inl"
#endif
