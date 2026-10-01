#ifndef CALCULATORSIGNALSONLY_CLIENT_TRANSPORT_H
#define CALCULATORSIGNALSONLY_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::CalculatorSignalsOnly
{

class CalculatorSignalsOnlyClientRpcTransport
{
protected:
    inline
    CalculatorSignalsOnlyClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::CalculatorSignalsOnly
#include "CalculatorSignalsOnly_ClientTransport.inl"
#endif
