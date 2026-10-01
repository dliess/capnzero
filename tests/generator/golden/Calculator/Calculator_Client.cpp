#include "Calculator_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "Calculator.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::Calculator;
CalculatorClientRpc::CalculatorClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    Super(rZmqContext, serverRpcAddr)
{
}
CalculatorClientSignals::CalculatorClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(serverSignalAddr);
}

int CalculatorClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

CalculatorClientSignalsThreaded::CalculatorClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{
    m_zmqSubHelperSocket.bind("inproc://CalculatorSubHelper");
}

CalculatorClientSignalsThreaded::~CalculatorClientSignalsThreaded()
{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}

void CalculatorClientSignalsThreaded::StartThread()
{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() { signalReceiverThreadFn(); });
}

CalculatorClientRpc::Return_Add
CalculatorClientRpc::add(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Return_Add retVal;

    _add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnAdd>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

CalculatorClientRpc::Return_Sub
CalculatorClientRpc::sub(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Return_Sub retVal;

    _sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnSub>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

Int32 CalculatorClientRpc::addDirectRet(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterAddDirectRet>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Int32 retVal;

    _addDirectRet(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnAddDirectRet>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

Int32 CalculatorClientRpc::subDirectRet(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSubDirectRet>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Int32 retVal;

    _subDirectRet(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnSubDirectRet>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

void CalculatorClientRpc::multiply(::capnp::MessageBuilder& sndData)
{


    _multiply(
		sndData

    );

}

CalculatorClientRpc::Return_Multiply2
CalculatorClientRpc::multiply2(::capnp::MessageBuilder& sndData)
{

	Return_Multiply2 retVal;

    _multiply2(
		sndData, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnMultiply2>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

Int32 CalculatorClientRpc::DirectRet__add(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterDirectRetAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Int32 retVal;

    _DirectRet__add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnDirectRetAdd>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

Int32 CalculatorClientRpc::DirectRet__sub(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterDirectRetSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Int32 retVal;

    _DirectRet__sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnDirectRetSub>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

void CalculatorClientRpc::Screen__setBrightness(UInt32 brightness)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetBrightness>();
	paramBuilder.setBrightness(brightness);


    _Screen__setBrightness(
		message

    );

}

void CalculatorClientRpc::Screen__setColor(EColor color)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetColor>();
	paramBuilder.setColor(color);


    _Screen__setColor(
		message

    );

}

void CalculatorClientRpc::Screen__setTitle(const Text& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetTitle>();
	paramBuilder.setTitle(title);


    _Screen__setTitle(
		message

    );

}

void CalculatorClientRpc::Screen__setId(const Data<8>& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetId>();
	paramBuilder.setTitle(capnp::Data::Reader(title.data(), title.size()));


    _Screen__setId(
		message

    );

}

void CalculatorClientSignalsThreaded::signalReceiverThreadFn()
{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://CalculatorSubHelper");

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

void CalculatorClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "justASignal"){
		if(m_justASignalCb) m_justASignalCb();
	}
	else if(key == "Screen__listChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));
		if(m_screenListChangedCb) m_screenListChangedCb(reader);
	}
	else if(key == "Screen__colorChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenColorChanged>();
		if(m_screenColorChangedCb) m_screenColorChangedCb(paramReader.getColor());
	}
	else if(key == "Screen__brightnessChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenBrightnessChanged>();
		if(m_screenBrightnessChangedCb) m_screenBrightnessChangedCb(paramReader.getBrightness());
	}

}

void CalculatorClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

void CalculatorClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "justASignal"){
		if(m_justASignalCb) m_justASignalCb();
	}
	else if(key == "Screen__listChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));
		if(m_screenListChangedCb) m_screenListChangedCb(reader);
	}
	else if(key == "Screen__colorChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenColorChanged>();
		if(m_screenColorChangedCb) m_screenColorChangedCb(paramReader.getColor());
	}
	else if(key == "Screen__brightnessChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenBrightnessChanged>();
		if(m_screenBrightnessChangedCb) m_screenBrightnessChangedCb(paramReader.getBrightness());
	}
}

void CalculatorClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}

void CalculatorClientSignals::onJustASignal(JustASignalCb cb)
{
    if(m_justASignalCb) return; // set it only once
    m_justASignalCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "justASignal");
}

void CalculatorClientSignalsThreaded::onJustASignal(JustASignalCb cb)
{
    if(m_justASignalCb) return; // set it only once
    m_justASignalCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("justASignal", 11));
}

void CalculatorClientSignals::onScreenListChanged(ScreenListChangedCb cb)
{
    if(m_screenListChangedCb) return; // set it only once
    m_screenListChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__listChanged");
}

void CalculatorClientSignalsThreaded::onScreenListChanged(ScreenListChangedCb cb)
{
    if(m_screenListChangedCb) return; // set it only once
    m_screenListChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__listChanged", 19));
}

void CalculatorClientSignals::onScreenColorChanged(ScreenColorChangedCb cb)
{
    if(m_screenColorChangedCb) return; // set it only once
    m_screenColorChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__colorChanged");
}

void CalculatorClientSignalsThreaded::onScreenColorChanged(ScreenColorChangedCb cb)
{
    if(m_screenColorChangedCb) return; // set it only once
    m_screenColorChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__colorChanged", 20));
}

void CalculatorClientSignals::onScreenBrightnessChanged(ScreenBrightnessChangedCb cb)
{
    if(m_screenBrightnessChangedCb) return; // set it only once
    m_screenBrightnessChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__brightnessChanged");
}

void CalculatorClientSignalsThreaded::onScreenBrightnessChanged(ScreenBrightnessChangedCb cb)
{
    if(m_screenBrightnessChangedCb) return; // set it only once
    m_screenBrightnessChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__brightnessChanged", 25));
}

CalculatorClient::CalculatorClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr) :
    CalculatorClientRpc(rZmqContext, serverRpcAddr),
    CalculatorClientSignalsThreaded(rZmqContext, serverSignalAddr)
{
    StartThread();
}
