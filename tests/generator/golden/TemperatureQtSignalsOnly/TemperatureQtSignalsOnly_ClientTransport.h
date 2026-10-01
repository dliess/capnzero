#ifndef TEMPERATUREQTSIGNALSONLY_CLIENT_TRANSPORT_H
#define TEMPERATUREQTSIGNALSONLY_CLIENT_TRANSPORT_H

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TemperatureQtSignalsOnly
{

class TemperatureQtSignalsOnlyClientRpcTransport
{
protected:
    inline
    TemperatureQtSignalsOnlyClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);

private:
    zmq::socket_t m_zmqReqSocket;
};

} // namespace capnzero::TemperatureQtSignalsOnly
#include "TemperatureQtSignalsOnly_ClientTransport.inl"
#endif
