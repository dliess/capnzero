#include "TemperatureQtSignalsOnly_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TemperatureQtSignalsOnly.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQtSignalsOnly;

TemperatureQtSignalsOnlyServer::TemperatureQtSignalsOnlyServer(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_signals(rZmqContext, signalBindAddr)
{
}
TemperatureQtSignalsOnlyServer::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::xpub)
{
    m_rZmqPubSocket.bind(signalBindAddr);
    m_rZmqPubSocket.set(zmq::sockopt::xpub_verbose, true);
}

void TemperatureQtSignalsOnlyServer::Signals::unbind(const std::string& signalBindAddr) { m_rZmqPubSocket.unbind(signalBindAddr); }

int TemperatureQtSignalsOnlyServer::Signals::getFd() const
{
    return m_rZmqPubSocket.get(zmq::sockopt::fd);
}

void TemperatureQtSignalsOnlyServer::Signals::handleAllSubscriptions()
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

void TemperatureQtSignalsOnlyServer::Signals::registerTempRangeChanged2SubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["tempRangeChanged2"] = cb;
}
void TemperatureQtSignalsOnlyServer::Signals::registerSensorNameChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["sensorNameChanged"] = cb;
}
void TemperatureQtSignalsOnlyServer::Signals::registerActualTemperatureChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["actualTemperatureChanged"] = cb;
}
void TemperatureQtSignalsOnlyServer::Signals::registerTempRangeChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["tempRangeChanged"] = cb;
}



void TemperatureQtSignalsOnlyServer::Signals::tempRangeChanged2(ETempRange tempRange)
{
	m_rZmqPubSocket.send(zmq::const_buffer("tempRangeChanged2", 17), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterTempRangeChanged2>();
	builder.setTempRange(tempRange);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtSignalsOnlyServer::Signals::sensorNameChanged(const Text& val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("sensorNameChanged", 17), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterSensorNameChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtSignalsOnlyServer::Signals::actualTemperatureChanged(Float32 val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("actualTemperatureChanged", 24), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterActualTemperatureChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtSignalsOnlyServer::Signals::tempRangeChanged(ETempRange val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("tempRangeChanged", 16), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterTempRangeChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}
