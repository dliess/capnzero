#include "Edges_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "Edges.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::Edges;

EdgesServer::EdgesServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<ZebraRpcIf> zebra, std::unique_ptr<RpcIf> _) :
	m_pZebraRpcIf(std::move(zebra)),
	m_p_RpcIf(std::move(_)),
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router),
    m_signals(rZmqContext, signalBindAddr)
{
    m_zmqRepSocket.bind(rpcBindAddr);
}
void EdgesServer::unbind(const std::string& rpcBindAddr) { m_zmqRepSocket.unbind(rpcBindAddr); }   
void EdgesServer::processNextRequest(WaitMode waitMode) {
    zmq::message_t routerId;
    auto res0 = m_zmqRepSocket.recv(routerId, (waitMode == WaitMode::Blocking) ? zmq::recv_flags::none : zmq::recv_flags::dontwait);
    if(waitMode == WaitMode::NonBlocking && !res0) return;
    zmq::message_t rpcCoordBuf;
    auto res1 = m_zmqRepSocket.recv(rpcCoordBuf, zmq::recv_flags::none);
    ::capnp::FlatArrayMessageReader msgReader(asCapnpArr(rpcCoordBuf));
    auto coordReader = msgReader.getRoot<RpcCoord>();
    switch(coordReader.getServiceId())
    {
		case to_underlying(ServiceId::ZEBRA):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(ZebraRpcIds::EMPTY):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterZebraEmpty>();
					auto ret = m_pZebraRpcIf->empty();
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnZebraEmpty>();
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(ZebraRpcIds::NO_ARGS):
				{
					auto ret = m_pZebraRpcIf->noArgs();
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnZebraNoArgs>();
					builder.setRetParam(ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(ZebraRpcIds::BYTES):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterZebraBytes>();
					auto ret = m_pZebraRpcIf->bytes(Span(paramReader.getDynamic().begin(), paramReader.getDynamic().end()), SpanCL<4>(paramReader.getFixed().begin(), paramReader.getFixed().end()), TextView(paramReader.getText().begin(), paramReader.getText().size()), paramReader.getMode());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnZebraBytes>();
					builder.setRetParam(capnp::Data::Reader(ret.data(), ret.size()));
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(ZebraRpcIds::SET_ACTIVE):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterZebraSetActive>();
					m_pZebraRpcIf->setActive(paramReader.getVal());
					break;
				}
				case to_underlying(ZebraRpcIds::TOGGLE_ACTIVE):
				{
					m_pZebraRpcIf->toggleActive();
					break;
				}
			}
			break;
		}
		case to_underlying(ServiceId::MAIN):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(RpcIds::FIRE):
				{
					m_p_RpcIf->fire();
					break;
				}
			}
			break;
		}

    }
}

void EdgesServer::processNextRequestAllNonBlock() {
    while(m_zmqRepSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        processNextRequest(WaitMode::Blocking);
    }
}

int EdgesServer::getFd() const
{
    return m_zmqRepSocket.get(zmq::sockopt::fd);
}

EdgesServer::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::xpub)
{
    m_rZmqPubSocket.bind(signalBindAddr);
    m_rZmqPubSocket.set(zmq::sockopt::xpub_verbose, true);
}

void EdgesServer::Signals::unbind(const std::string& signalBindAddr) { m_rZmqPubSocket.unbind(signalBindAddr); }

int EdgesServer::Signals::getFd() const
{
    return m_rZmqPubSocket.get(zmq::sockopt::fd);
}

void EdgesServer::Signals::handleAllSubscriptions()
{
    while(m_rZmqPubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
	{
		zmq::message_t msg;
		auto res = m_rZmqPubSocket.recv(msg, zmq::recv_flags::none);
		bool subscribe = static_cast<const char*>(msg.data())[0];
		std::string key(&(static_cast<const char*>(msg.data())[1]), msg.size() - 1);
		if(subscribe)
		{
            auto it = m_subscrCbMap.find(key);
            if(it != m_subscrCbMap.end())
            {
                (it->second)(*this);
            }
		}
	}
}

void EdgesServer::Signals::registerZebraActiveChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Zebra__activeChanged"] = cb;
}
void EdgesServer::Signals::registerEmptySubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["empty"] = cb;
}
void EdgesServer::Signals::registerTickSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["tick"] = cb;
}



void EdgesServer::Signals::Zebra__activeChanged(Bool val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Zebra__activeChanged", 20), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterZebraActiveChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void EdgesServer::Signals::empty()
{
	m_rZmqPubSocket.send(zmq::const_buffer("empty", 5), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterEmpty>();
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void EdgesServer::Signals::tick()
{
	m_rZmqPubSocket.send(zmq::const_buffer("tick", 4), zmq::send_flags::none);
}
