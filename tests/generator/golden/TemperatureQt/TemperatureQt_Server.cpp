#include "TemperatureQt_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TemperatureQt.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQt;

TemperatureQtServer::TemperatureQtServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<RpcIf> _) :
	m_p_RpcIf(std::move(_)),
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router),
    m_signals(rZmqContext, signalBindAddr)
{
    m_zmqRepSocket.bind(rpcBindAddr);
}
void TemperatureQtServer::unbind(const std::string& rpcBindAddr) { m_zmqRepSocket.unbind(rpcBindAddr); }   
void TemperatureQtServer::processNextRequest(WaitMode waitMode) {
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
				case to_underlying(RpcIds::EXPLICIT_SET_U_U_I_D):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterExplicitSetUUID>();
					m_p_RpcIf->explicitSetUUID(SpanCL<16>(paramReader.getUuid().begin(), paramReader.getUuid().end()));
					break;
				}
				case to_underlying(RpcIds::EXPLICIT_GET_U_U_I_D):
				{
					auto ret = m_p_RpcIf->explicitGetUUID();
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnExplicitGetUUID>();
					builder.setRetParam(capnp::Data::Reader(ret.data(), ret.size()));
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(RpcIds::SET_COMMANDED_TEMPERATURE):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterSetCommandedTemperature>();
					m_p_RpcIf->setCommandedTemperature(paramReader.getVal());
					break;
				}
				case to_underlying(RpcIds::SET_ENABLED):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterSetEnabled>();
					m_p_RpcIf->setEnabled(paramReader.getVal());
					break;
				}
				case to_underlying(RpcIds::TOGGLE_ENABLED):
				{
					m_p_RpcIf->toggleEnabled();
					break;
				}
				case to_underlying(RpcIds::SET_UUID):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterSetUuid>();
					m_p_RpcIf->setUuid(SpanCL<16>(paramReader.getVal().begin(), paramReader.getVal().end()));
					break;
				}
			}
			break;
		}

    }
}

void TemperatureQtServer::processNextRequestAllNonBlock() {
    while(m_zmqRepSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        processNextRequest(WaitMode::Blocking);
    }
}

int TemperatureQtServer::getFd() const
{
    return m_zmqRepSocket.get(zmq::sockopt::fd);
}

TemperatureQtServer::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::xpub)
{
    m_rZmqPubSocket.bind(signalBindAddr);
    m_rZmqPubSocket.set(zmq::sockopt::xpub_verbose, true);
}

void TemperatureQtServer::Signals::unbind(const std::string& signalBindAddr) { m_rZmqPubSocket.unbind(signalBindAddr); }

int TemperatureQtServer::Signals::getFd() const
{
    return m_rZmqPubSocket.get(zmq::sockopt::fd);
}

void TemperatureQtServer::Signals::handleAllSubscriptions()
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

void TemperatureQtServer::Signals::registerExplicitSignalUUIDSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["explicitSignalUUID"] = cb;
}
void TemperatureQtServer::Signals::registerSensorNameChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["sensorNameChanged"] = cb;
}
void TemperatureQtServer::Signals::registerActualTemperatureChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["actualTemperatureChanged"] = cb;
}
void TemperatureQtServer::Signals::registerCommandedTemperatureChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["commandedTemperatureChanged"] = cb;
}
void TemperatureQtServer::Signals::registerEnabledChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["enabledChanged"] = cb;
}
void TemperatureQtServer::Signals::registerUuidChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["uuidChanged"] = cb;
}



void TemperatureQtServer::Signals::explicitSignalUUID(const Data<16>& uuid)
{
	m_rZmqPubSocket.send(zmq::const_buffer("explicitSignalUUID", 18), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterExplicitSignalUUID>();
	builder.setUuid(capnp::Data::Reader(uuid.data(), uuid.size()));
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtServer::Signals::sensorNameChanged(const Text& val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("sensorNameChanged", 17), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterSensorNameChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtServer::Signals::actualTemperatureChanged(Float32 val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("actualTemperatureChanged", 24), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterActualTemperatureChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtServer::Signals::commandedTemperatureChanged(Float32 val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("commandedTemperatureChanged", 27), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterCommandedTemperatureChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtServer::Signals::enabledChanged(Bool val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("enabledChanged", 14), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterEnabledChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtServer::Signals::uuidChanged(const Data<16>& val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("uuidChanged", 11), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterUuidChanged>();
	builder.setVal(capnp::Data::Reader(val.data(), val.size()));
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}
