#include "TemperatureQtWithServiceName_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "TemperatureQtWithServiceName.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQtWithServiceName;
TemperatureQtWithServiceNameClientRpc::TemperatureQtWithServiceNameClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    Super(rZmqContext, serverRpcAddr)
{
}
TemperatureQtWithServiceNameClientSignals::TemperatureQtWithServiceNameClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(serverSignalAddr);
}

int TemperatureQtWithServiceNameClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

TemperatureQtWithServiceNameClientSignalsThreaded::TemperatureQtWithServiceNameClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{
    m_zmqSubHelperSocket.bind("inproc://TemperatureQtWithServiceNameSubHelper");
}

TemperatureQtWithServiceNameClientSignalsThreaded::~TemperatureQtWithServiceNameClientSignalsThreaded()
{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}

void TemperatureQtWithServiceNameClientSignalsThreaded::StartThread()
{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() { signalReceiverThreadFn(); });
}

void TemperatureQtWithServiceNameClientRpc::TemperatureService__aTestRpc(const Text& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterTemperatureServiceATestRpc>();
	paramBuilder.setTitle(title);


    _TemperatureService__aTestRpc(
		message

    );

}

void TemperatureQtWithServiceNameClientRpc::TemperatureService__setCommandedTemperature(Float32 val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterTemperatureServiceSetCommandedTemperature>();
	paramBuilder.setVal(val);


    _TemperatureService__setCommandedTemperature(
		message

    );

}

void TemperatureQtWithServiceNameClientSignalsThreaded::signalReceiverThreadFn()
{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://TemperatureQtWithServiceNameSubHelper");

    while(!m_terminateRequest.load())
    {
        zmq::pollitem_t items[] = {
            {zmqSubSocket.handle(), 0, ZMQ_POLLIN, 0},
			{zmqSubHelperSocket.handle(), 0, ZMQ_POLLIN, 0}
        };
        zmq::poll(&items[0], 2);
		if (items[0].revents & ZMQ_POLLIN)
		{
            handleIncomingSignal(zmqSubSocket);
		}
		if (items[1].revents & ZMQ_POLLIN)
		{
			handleSubscriptionRequest(zmqSubHelperSocket, zmqSubSocket);
		}
    }
}

void TemperatureQtWithServiceNameClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "TemperatureService__sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceSensorNameChanged>();
		if(m_temperatureServiceSensorNameChangedCb) m_temperatureServiceSensorNameChangedCb(TextView(paramReader.getVal().begin(), paramReader.getVal().size()));
	}
	else if(key == "TemperatureService__actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceActualTemperatureChanged>();
		if(m_temperatureServiceActualTemperatureChangedCb) m_temperatureServiceActualTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "TemperatureService__commandedTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceCommandedTemperatureChanged>();
		if(m_temperatureServiceCommandedTemperatureChangedCb) m_temperatureServiceCommandedTemperatureChangedCb(paramReader.getVal());
	}

}

void TemperatureQtWithServiceNameClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

void TemperatureQtWithServiceNameClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "TemperatureService__sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceSensorNameChanged>();
		if(m_temperatureServiceSensorNameChangedCb) m_temperatureServiceSensorNameChangedCb(TextView(paramReader.getVal().begin(), paramReader.getVal().size()));
	}
	else if(key == "TemperatureService__actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceActualTemperatureChanged>();
		if(m_temperatureServiceActualTemperatureChangedCb) m_temperatureServiceActualTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "TemperatureService__commandedTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceCommandedTemperatureChanged>();
		if(m_temperatureServiceCommandedTemperatureChangedCb) m_temperatureServiceCommandedTemperatureChangedCb(paramReader.getVal());
	}
}

void TemperatureQtWithServiceNameClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}

void TemperatureQtWithServiceNameClientSignals::onTemperatureServiceSensorNameChanged(TemperatureServiceSensorNameChangedCb cb)
{
    if(m_temperatureServiceSensorNameChangedCb) return; // set it only once
    m_temperatureServiceSensorNameChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "TemperatureService__sensorNameChanged");
}

void TemperatureQtWithServiceNameClientSignalsThreaded::onTemperatureServiceSensorNameChanged(TemperatureServiceSensorNameChangedCb cb)
{
    if(m_temperatureServiceSensorNameChangedCb) return; // set it only once
    m_temperatureServiceSensorNameChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("TemperatureService__sensorNameChanged", 37));
}

void TemperatureQtWithServiceNameClientSignals::onTemperatureServiceActualTemperatureChanged(TemperatureServiceActualTemperatureChangedCb cb)
{
    if(m_temperatureServiceActualTemperatureChangedCb) return; // set it only once
    m_temperatureServiceActualTemperatureChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "TemperatureService__actualTemperatureChanged");
}

void TemperatureQtWithServiceNameClientSignalsThreaded::onTemperatureServiceActualTemperatureChanged(TemperatureServiceActualTemperatureChangedCb cb)
{
    if(m_temperatureServiceActualTemperatureChangedCb) return; // set it only once
    m_temperatureServiceActualTemperatureChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("TemperatureService__actualTemperatureChanged", 44));
}

void TemperatureQtWithServiceNameClientSignals::onTemperatureServiceCommandedTemperatureChanged(TemperatureServiceCommandedTemperatureChangedCb cb)
{
    if(m_temperatureServiceCommandedTemperatureChangedCb) return; // set it only once
    m_temperatureServiceCommandedTemperatureChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "TemperatureService__commandedTemperatureChanged");
}

void TemperatureQtWithServiceNameClientSignalsThreaded::onTemperatureServiceCommandedTemperatureChanged(TemperatureServiceCommandedTemperatureChangedCb cb)
{
    if(m_temperatureServiceCommandedTemperatureChangedCb) return; // set it only once
    m_temperatureServiceCommandedTemperatureChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("TemperatureService__commandedTemperatureChanged", 47));
}

TemperatureQtWithServiceNameClient::TemperatureQtWithServiceNameClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr) :
    TemperatureQtWithServiceNameClientRpc(rZmqContext, serverRpcAddr),
    TemperatureQtWithServiceNameClientSignalsThreaded(rZmqContext, serverSignalAddr)
{
    StartThread();
}
