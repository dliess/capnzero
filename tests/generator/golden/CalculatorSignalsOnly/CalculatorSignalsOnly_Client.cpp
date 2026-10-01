#include "CalculatorSignalsOnly_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "CalculatorSignalsOnly.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorSignalsOnly;
CalculatorSignalsOnlyClientSignals::CalculatorSignalsOnlyClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(serverSignalAddr);
}

int CalculatorSignalsOnlyClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

CalculatorSignalsOnlyClientSignalsThreaded::CalculatorSignalsOnlyClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{
    m_zmqSubHelperSocket.bind("inproc://CalculatorSignalsOnlySubHelper");
}

CalculatorSignalsOnlyClientSignalsThreaded::~CalculatorSignalsOnlyClientSignalsThreaded()
{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}

void CalculatorSignalsOnlyClientSignalsThreaded::StartThread()
{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() { signalReceiverThreadFn(); });
}

void CalculatorSignalsOnlyClientSignalsThreaded::signalReceiverThreadFn()
{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://CalculatorSignalsOnlySubHelper");

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

void CalculatorSignalsOnlyClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "Screen__brightnessChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenBrightnessChanged>();
		if(m_screenBrightnessChangedCb) m_screenBrightnessChangedCb(paramReader.getBrightness());
	}
	else if(key == "Screen__colorChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenColorChanged>();
		if(m_screenColorChangedCb) m_screenColorChangedCb(paramReader.getColor());
	}

}

void CalculatorSignalsOnlyClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

void CalculatorSignalsOnlyClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "Screen__brightnessChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenBrightnessChanged>();
		if(m_screenBrightnessChangedCb) m_screenBrightnessChangedCb(paramReader.getBrightness());
	}
	else if(key == "Screen__colorChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenColorChanged>();
		if(m_screenColorChangedCb) m_screenColorChangedCb(paramReader.getColor());
	}
}

void CalculatorSignalsOnlyClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}

void CalculatorSignalsOnlyClientSignals::onScreenBrightnessChanged(ScreenBrightnessChangedCb cb)
{
    if(m_screenBrightnessChangedCb) return; // set it only once
    m_screenBrightnessChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__brightnessChanged");
}

void CalculatorSignalsOnlyClientSignalsThreaded::onScreenBrightnessChanged(ScreenBrightnessChangedCb cb)
{
    if(m_screenBrightnessChangedCb) return; // set it only once
    m_screenBrightnessChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__brightnessChanged", 25));
}

void CalculatorSignalsOnlyClientSignals::onScreenColorChanged(ScreenColorChangedCb cb)
{
    if(m_screenColorChangedCb) return; // set it only once
    m_screenColorChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__colorChanged");
}

void CalculatorSignalsOnlyClientSignalsThreaded::onScreenColorChanged(ScreenColorChangedCb cb)
{
    if(m_screenColorChangedCb) return; // set it only once
    m_screenColorChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__colorChanged", 20));
}

CalculatorSignalsOnlyClient::CalculatorSignalsOnlyClient(zmq::context_t& rZmqContext, const std::string& serverSignalAddr) :
    CalculatorSignalsOnlyClientSignalsThreaded(rZmqContext, serverSignalAddr)
{
    StartThread();
}
