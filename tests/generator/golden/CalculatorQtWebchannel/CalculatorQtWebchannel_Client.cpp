#include "CalculatorQtWebchannel_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "CalculatorQtWebchannel.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorQtWebchannel;
CalculatorQtWebchannelClientRpc::CalculatorQtWebchannelClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    Super(rZmqContext, serverRpcAddr)
{
}
CalculatorQtWebchannelClientSignals::CalculatorQtWebchannelClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(serverSignalAddr);
}

int CalculatorQtWebchannelClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

CalculatorQtWebchannelClientSignalsThreaded::CalculatorQtWebchannelClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{
    m_zmqSubHelperSocket.bind("inproc://CalculatorQtWebchannelSubHelper");
}

CalculatorQtWebchannelClientSignalsThreaded::~CalculatorQtWebchannelClientSignalsThreaded()
{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}

void CalculatorQtWebchannelClientSignalsThreaded::StartThread()
{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() { signalReceiverThreadFn(); });
}

void CalculatorQtWebchannelClientRpc::setMode(EMode val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetMode>();
	paramBuilder.setVal(val);


    _setMode(
		message

    );

}

Int32 CalculatorQtWebchannelClientRpc::Calculator__add(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Int32 retVal;

    _Calculator__add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorAdd>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

Int32 CalculatorQtWebchannelClientRpc::Calculator__sub(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Int32 retVal;

    _Calculator__sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorSub>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

void CalculatorQtWebchannelClientRpc::Screen__setBrightness(UInt32 brightness)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetBrightness>();
	paramBuilder.setBrightness(brightness);


    _Screen__setBrightness(
		message

    );

}

void CalculatorQtWebchannelClientRpc::Screen__setColor(EColor color)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetColor>();
	paramBuilder.setColor(color);


    _Screen__setColor(
		message

    );

}

EColor CalculatorQtWebchannelClientRpc::Screen__getColor()
{

	EColor retVal;

    _Screen__getColor(

		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnScreenGetColor>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

void CalculatorQtWebchannelClientRpc::Screen__setTitle(const Text& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetTitle>();
	paramBuilder.setTitle(title);


    _Screen__setTitle(
		message

    );

}

void CalculatorQtWebchannelClientRpc::Screen__setId(const Data<8>& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetId>();
	paramBuilder.setTitle(capnp::Data::Reader(title.data(), title.size()));


    _Screen__setId(
		message

    );

}

CalculatorQtWebchannelClientRpc::ReturnScreenAlltypes
CalculatorQtWebchannelClientRpc::Screen__alltypes(Int32 a, Float32 b, Float64 c, const Text& d, const Span& e, const Data<8>& f, EColor color, EMode mode)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenAlltypes>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);
	paramBuilder.setC(c);
	paramBuilder.setD(d);
	paramBuilder.setE(capnp::Data::Reader(e.data(), e.size()));
	paramBuilder.setF(capnp::Data::Reader(f.data(), f.size()));
	paramBuilder.setColor(color);
	paramBuilder.setMode(mode);

	ReturnScreenAlltypes retVal;

    _Screen__alltypes(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnScreenAlltypes>();
			retVal.a = reader.getA();
			retVal.b = reader.getB();
			retVal.c = reader.getC();
			retVal.d = reader.getD();
			auto src = reader.getF();
			assert(src.size() == retVal.f.size());
			std::copy(src.begin(), src.end(), retVal.f.begin());
			retVal.color = reader.getColor();
			retVal.mode = reader.getMode();

	}
    );
	return retVal;
}

void CalculatorQtWebchannelClientSignalsThreaded::signalReceiverThreadFn()
{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://CalculatorQtWebchannelSubHelper");

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

void CalculatorQtWebchannelClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "modeChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterModeChanged>();
		if(m_modeChangedCb) m_modeChangedCb(paramReader.getVal());
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
	else if(key == "Screen__alltypess"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenAlltypess>();
		if(m_screenAlltypessCb) m_screenAlltypessCb(paramReader.getA(), paramReader.getB(), paramReader.getC(), TextView(paramReader.getD().begin(), paramReader.getD().size()), Span(paramReader.getE().begin(), paramReader.getE().end()), SpanCL<8>(paramReader.getF().begin(), paramReader.getF().end()), paramReader.getColor(), paramReader.getMode());
	}

}

void CalculatorQtWebchannelClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

void CalculatorQtWebchannelClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "modeChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterModeChanged>();
		if(m_modeChangedCb) m_modeChangedCb(paramReader.getVal());
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
	else if(key == "Screen__alltypess"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenAlltypess>();
		if(m_screenAlltypessCb) m_screenAlltypessCb(paramReader.getA(), paramReader.getB(), paramReader.getC(), TextView(paramReader.getD().begin(), paramReader.getD().size()), Span(paramReader.getE().begin(), paramReader.getE().end()), SpanCL<8>(paramReader.getF().begin(), paramReader.getF().end()), paramReader.getColor(), paramReader.getMode());
	}
}

void CalculatorQtWebchannelClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}

void CalculatorQtWebchannelClientSignals::onModeChanged(ModeChangedCb cb)
{
    if(m_modeChangedCb) return; // set it only once
    m_modeChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "modeChanged");
}

void CalculatorQtWebchannelClientSignalsThreaded::onModeChanged(ModeChangedCb cb)
{
    if(m_modeChangedCb) return; // set it only once
    m_modeChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("modeChanged", 11));
}

void CalculatorQtWebchannelClientSignals::onScreenColorChanged(ScreenColorChangedCb cb)
{
    if(m_screenColorChangedCb) return; // set it only once
    m_screenColorChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__colorChanged");
}

void CalculatorQtWebchannelClientSignalsThreaded::onScreenColorChanged(ScreenColorChangedCb cb)
{
    if(m_screenColorChangedCb) return; // set it only once
    m_screenColorChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__colorChanged", 20));
}

void CalculatorQtWebchannelClientSignals::onScreenBrightnessChanged(ScreenBrightnessChangedCb cb)
{
    if(m_screenBrightnessChangedCb) return; // set it only once
    m_screenBrightnessChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__brightnessChanged");
}

void CalculatorQtWebchannelClientSignalsThreaded::onScreenBrightnessChanged(ScreenBrightnessChangedCb cb)
{
    if(m_screenBrightnessChangedCb) return; // set it only once
    m_screenBrightnessChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__brightnessChanged", 25));
}

void CalculatorQtWebchannelClientSignals::onScreenAlltypess(ScreenAlltypessCb cb)
{
    if(m_screenAlltypessCb) return; // set it only once
    m_screenAlltypessCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__alltypess");
}

void CalculatorQtWebchannelClientSignalsThreaded::onScreenAlltypess(ScreenAlltypessCb cb)
{
    if(m_screenAlltypessCb) return; // set it only once
    m_screenAlltypessCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Screen__alltypess", 17));
}

CalculatorQtWebchannelClient::CalculatorQtWebchannelClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr) :
    CalculatorQtWebchannelClientRpc(rZmqContext, serverRpcAddr),
    CalculatorQtWebchannelClientSignalsThreaded(rZmqContext, serverSignalAddr)
{
    StartThread();
}
