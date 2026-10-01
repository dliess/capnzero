#include "CalculatorSignalsOnly_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "CalculatorSignalsOnly.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorSignalsOnly;

CalculatorSignalsOnlyServer::CalculatorSignalsOnlyServer(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_signals(rZmqContext, signalBindAddr)
{
}
CalculatorSignalsOnlyServer::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::xpub)
{
    m_rZmqPubSocket.bind(signalBindAddr);
    m_rZmqPubSocket.set(zmq::sockopt::xpub_verbose, true);
}

void CalculatorSignalsOnlyServer::Signals::unbind(const std::string& signalBindAddr) { m_rZmqPubSocket.unbind(signalBindAddr); }

int CalculatorSignalsOnlyServer::Signals::getFd() const
{
    return m_rZmqPubSocket.get(zmq::sockopt::fd);
}

void CalculatorSignalsOnlyServer::Signals::handleAllSubscriptions()
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

void CalculatorSignalsOnlyServer::Signals::registerScreenBrightnessChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__brightnessChanged"] = cb;
}
void CalculatorSignalsOnlyServer::Signals::registerScreenColorChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__colorChanged"] = cb;
}



void CalculatorSignalsOnlyServer::Signals::Screen__brightnessChanged(UInt32 brightness)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__brightnessChanged", 25), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterScreenBrightnessChanged>();
	builder.setBrightness(brightness);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void CalculatorSignalsOnlyServer::Signals::Screen__colorChanged(EColor color)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__colorChanged", 20), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterScreenColorChanged>();
	builder.setColor(color);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}
