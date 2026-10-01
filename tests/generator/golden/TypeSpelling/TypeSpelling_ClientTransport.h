#ifndef TYPESPELLING_CLIENT_TRANSPORT_H
#define TYPESPELLING_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TypeSpelling
{

class TypeSpellingClientRpcTransport
{
protected:
    inline
    TypeSpellingClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	template<typename Callable>
	void _bytes(capnp::MessageBuilder& sndData, Callable&& rcvCb);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::TypeSpelling
#include "TypeSpelling_ClientTransport.inl"
#endif
