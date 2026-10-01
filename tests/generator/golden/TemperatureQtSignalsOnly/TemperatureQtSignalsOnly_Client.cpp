#include "TemperatureQtSignalsOnly_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "TemperatureQtSignalsOnly.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQtSignalsOnly;
TemperatureQtSignalsOnlyClientSignals::TemperatureQtSignalsOnlyClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(serverSignalAddr);
}

int TemperatureQtSignalsOnlyClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

TemperatureQtSignalsOnlyClientSignalsThreaded::TemperatureQtSignalsOnlyClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{
    m_zmqSubHelperSocket.bind("inproc://TemperatureQtSignalsOnlySubHelper");
}

TemperatureQtSignalsOnlyClientSignalsThreaded::~TemperatureQtSignalsOnlyClientSignalsThreaded()
{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::StartThread()
{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() { signalReceiverThreadFn(); });
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::signalReceiverThreadFn()
{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://TemperatureQtSignalsOnlySubHelper");

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

void TemperatureQtSignalsOnlyClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "tempRangeChanged2"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTempRangeChanged2>();
		if(m_tempRangeChanged2Cb) m_tempRangeChanged2Cb(paramReader.getTempRange());
	}
	else if(key == "sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterSensorNameChanged>();
		if(m_sensorNameChangedCb) m_sensorNameChangedCb(TextView(paramReader.getVal().begin(), paramReader.getVal().size()));
	}
	else if(key == "actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterActualTemperatureChanged>();
		if(m_actualTemperatureChangedCb) m_actualTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "tempRangeChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTempRangeChanged>();
		if(m_tempRangeChangedCb) m_tempRangeChangedCb(paramReader.getVal());
	}

}

void TemperatureQtSignalsOnlyClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "tempRangeChanged2"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTempRangeChanged2>();
		if(m_tempRangeChanged2Cb) m_tempRangeChanged2Cb(paramReader.getTempRange());
	}
	else if(key == "sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterSensorNameChanged>();
		if(m_sensorNameChangedCb) m_sensorNameChangedCb(TextView(paramReader.getVal().begin(), paramReader.getVal().size()));
	}
	else if(key == "actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterActualTemperatureChanged>();
		if(m_actualTemperatureChangedCb) m_actualTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "tempRangeChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTempRangeChanged>();
		if(m_tempRangeChangedCb) m_tempRangeChangedCb(paramReader.getVal());
	}
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}

void TemperatureQtSignalsOnlyClientSignals::onTempRangeChanged2(TempRangeChanged2Cb cb)
{
    if(m_tempRangeChanged2Cb) return; // set it only once
    m_tempRangeChanged2Cb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "tempRangeChanged2");
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::onTempRangeChanged2(TempRangeChanged2Cb cb)
{
    if(m_tempRangeChanged2Cb) return; // set it only once
    m_tempRangeChanged2Cb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("tempRangeChanged2", 17));
}

void TemperatureQtSignalsOnlyClientSignals::onSensorNameChanged(SensorNameChangedCb cb)
{
    if(m_sensorNameChangedCb) return; // set it only once
    m_sensorNameChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "sensorNameChanged");
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::onSensorNameChanged(SensorNameChangedCb cb)
{
    if(m_sensorNameChangedCb) return; // set it only once
    m_sensorNameChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("sensorNameChanged", 17));
}

void TemperatureQtSignalsOnlyClientSignals::onActualTemperatureChanged(ActualTemperatureChangedCb cb)
{
    if(m_actualTemperatureChangedCb) return; // set it only once
    m_actualTemperatureChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "actualTemperatureChanged");
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::onActualTemperatureChanged(ActualTemperatureChangedCb cb)
{
    if(m_actualTemperatureChangedCb) return; // set it only once
    m_actualTemperatureChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("actualTemperatureChanged", 24));
}

void TemperatureQtSignalsOnlyClientSignals::onTempRangeChanged(TempRangeChangedCb cb)
{
    if(m_tempRangeChangedCb) return; // set it only once
    m_tempRangeChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "tempRangeChanged");
}

void TemperatureQtSignalsOnlyClientSignalsThreaded::onTempRangeChanged(TempRangeChangedCb cb)
{
    if(m_tempRangeChangedCb) return; // set it only once
    m_tempRangeChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("tempRangeChanged", 16));
}

TemperatureQtSignalsOnlyClient::TemperatureQtSignalsOnlyClient(zmq::context_t& rZmqContext, const std::string& serverSignalAddr) :
    TemperatureQtSignalsOnlyClientSignalsThreaded(rZmqContext, serverSignalAddr)
{
    StartThread();
}
