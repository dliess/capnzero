#include "TemperatureQtWithServiceName_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "TemperatureQtWithServiceName.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQtWithServiceName;

TemperatureQtWithServiceNameServer::TemperatureQtWithServiceNameServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<TemperatureServiceRpcIf> temperatureService) :
	m_pTemperatureServiceRpcIf(std::move(temperatureService)),
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router),
    m_signals(rZmqContext, signalBindAddr)
{
    m_zmqRepSocket.bind(rpcBindAddr);
}
void TemperatureQtWithServiceNameServer::unbind(const std::string& rpcBindAddr) { m_zmqRepSocket.unbind(rpcBindAddr); }   
void TemperatureQtWithServiceNameServer::processNextRequest(WaitMode waitMode) {
    zmq::message_t routerId;
    auto res0 = m_zmqRepSocket.recv(routerId, (waitMode == WaitMode::Blocking) ? zmq::recv_flags::none : zmq::recv_flags::dontwait);
    if(waitMode == WaitMode::NonBlocking && !res0) return;
    zmq::message_t rpcCoordBuf;
    auto res1 = m_zmqRepSocket.recv(rpcCoordBuf, zmq::recv_flags::none);
    ::capnp::FlatArrayMessageReader msgReader(asCapnpArr(rpcCoordBuf));
    auto coordReader = msgReader.getRoot<RpcCoord>();
    switch(coordReader.getServiceId())
    {
		case to_underlying(ServiceId::TEMPERATURE_SERVICE):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(TemperatureServiceRpcIds::A_TEST_RPC):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterTemperatureServiceATestRpc>();
					m_pTemperatureServiceRpcIf->aTestRpc(TextView(paramReader.getTitle().begin(), paramReader.getTitle().size()));
					break;
				}
				case to_underlying(TemperatureServiceRpcIds::SET_COMMANDED_TEMPERATURE):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterTemperatureServiceSetCommandedTemperature>();
					m_pTemperatureServiceRpcIf->setCommandedTemperature(paramReader.getVal());
					break;
				}
			}
			break;
		}

    }
}

void TemperatureQtWithServiceNameServer::processNextRequestAllNonBlock() {
    while(m_zmqRepSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        processNextRequest(WaitMode::Blocking);
    }
}

int TemperatureQtWithServiceNameServer::getFd() const
{
    return m_zmqRepSocket.get(zmq::sockopt::fd);
}

TemperatureQtWithServiceNameServer::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::xpub)
{
    m_rZmqPubSocket.bind(signalBindAddr);
    m_rZmqPubSocket.set(zmq::sockopt::xpub_verbose, true);
}

void TemperatureQtWithServiceNameServer::Signals::unbind(const std::string& signalBindAddr) { m_rZmqPubSocket.unbind(signalBindAddr); }

int TemperatureQtWithServiceNameServer::Signals::getFd() const
{
    return m_rZmqPubSocket.get(zmq::sockopt::fd);
}

void TemperatureQtWithServiceNameServer::Signals::handleAllSubscriptions()
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

void TemperatureQtWithServiceNameServer::Signals::registerTemperatureServiceSensorNameChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["TemperatureService__sensorNameChanged"] = cb;
}
void TemperatureQtWithServiceNameServer::Signals::registerTemperatureServiceActualTemperatureChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["TemperatureService__actualTemperatureChanged"] = cb;
}
void TemperatureQtWithServiceNameServer::Signals::registerTemperatureServiceCommandedTemperatureChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["TemperatureService__commandedTemperatureChanged"] = cb;
}



void TemperatureQtWithServiceNameServer::Signals::TemperatureService__sensorNameChanged(const Text& val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("TemperatureService__sensorNameChanged", 37), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterTemperatureServiceSensorNameChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtWithServiceNameServer::Signals::TemperatureService__actualTemperatureChanged(Float32 val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("TemperatureService__actualTemperatureChanged", 44), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterTemperatureServiceActualTemperatureChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void TemperatureQtWithServiceNameServer::Signals::TemperatureService__commandedTemperatureChanged(Float32 val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("TemperatureService__commandedTemperatureChanged", 47), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterTemperatureServiceCommandedTemperatureChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}
