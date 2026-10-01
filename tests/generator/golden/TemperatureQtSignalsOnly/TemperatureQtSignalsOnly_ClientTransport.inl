
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TemperatureQtSignalsOnly.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::TemperatureQtSignalsOnly
{

inline
TemperatureQtSignalsOnlyClientRpcTransport::TemperatureQtSignalsOnlyClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}



} // namespace capnzero::TemperatureQtSignalsOnly
