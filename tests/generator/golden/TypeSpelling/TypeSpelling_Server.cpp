#include "TypeSpelling_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TypeSpelling.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TypeSpelling;

TypeSpellingServer::TypeSpellingServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr , std::unique_ptr<RpcIf> _) :
	m_p_RpcIf(std::move(_)),
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router)
{
    m_zmqRepSocket.bind(rpcBindAddr);
}
void TypeSpellingServer::unbind(const std::string& rpcBindAddr) { m_zmqRepSocket.unbind(rpcBindAddr); }   
void TypeSpellingServer::processNextRequest(WaitMode waitMode) {
    zmq::message_t routerId;
    auto res0 = m_zmqRepSocket.recv(routerId, (waitMode == WaitMode::Blocking) ? zmq::recv_flags::none : zmq::recv_flags::dontwait);
    if(waitMode == WaitMode::NonBlocking && !res0) return;
    zmq::message_t rpcCoordBuf;
    auto res1 = m_zmqRepSocket.recv(rpcCoordBuf, zmq::recv_flags::none);
    ::capnp::FlatArrayMessageReader msgReader(asCapnpArr(rpcCoordBuf));
    auto coordReader = msgReader.getRoot<RpcCoord>();
    switch(coordReader.getServiceId())
    {
		case to_underlying(ServiceId::MAIN):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(RpcIds::BYTES):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterBytes>();
					auto ret = m_p_RpcIf->bytes(SpanCL<04>(paramReader.getValue().begin(), paramReader.getValue().end()));
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnBytes>();
					builder.setRetParam(capnp::Data::Reader(ret.data(), ret.size()));
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
			}
			break;
		}

    }
}

void TypeSpellingServer::processNextRequestAllNonBlock() {
    while(m_zmqRepSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        processNextRequest(WaitMode::Blocking);
    }
}

int TypeSpellingServer::getFd() const
{
    return m_zmqRepSocket.get(zmq::sockopt::fd);
}

