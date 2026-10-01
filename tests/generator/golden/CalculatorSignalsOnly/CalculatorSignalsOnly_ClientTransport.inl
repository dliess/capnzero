
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "CalculatorSignalsOnly.capnp.h"
#include "capnzero_utils.h"


namespace capnzero::CalculatorSignalsOnly
{

inline
CalculatorSignalsOnlyClientRpcTransport::CalculatorSignalsOnlyClientRpcTransport(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    m_zmqReqSocket(rZmqContext, zmq::socket_type::dealer)
{
    m_zmqReqSocket.connect(serverRpcAddr);
}



} // namespace capnzero::CalculatorSignalsOnly
