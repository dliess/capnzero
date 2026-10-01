#ifndef CALCULATORQTWEBCHANNEL_CLIENT_H
#define CALCULATORQTWEBCHANNEL_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "CalculatorQtWebchannel.capnp.h"

#include "CalculatorQtWebchannel_ClientTransport.h"
namespace capnzero::CalculatorQtWebchannel
{

class CalculatorQtWebchannelClientRpc : public CalculatorQtWebchannelClientRpcTransport
{
public:
    using Super = CalculatorQtWebchannelClientRpcTransport;
    CalculatorQtWebchannelClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void setMode(EMode val);

	Int32 Calculator__add(Int32 a, Int32 b);

	Int32 Calculator__sub(Int32 a, Int32 b);

	void Screen__setBrightness(UInt32 brightness);

	void Screen__setColor(EColor color);

	EColor Screen__getColor();

	void Screen__setTitle(const Text& title);

	void Screen__setId(const Data<8>& title);

	struct ReturnScreenAlltypes {
		Int32 a;
		Float32 b;
		Float64 c;
		Text d;
		Data<8> f;
		EColor color;
		EMode mode;
	};
	ReturnScreenAlltypes
	Screen__alltypes(Int32 a, Float32 b, Float64 c, const Text& d, const Span& e, const Data<8>& f, EColor color, EMode mode);


};

} // namespace capnzero::CalculatorQtWebchannel

#include <functional>
#include <capnp/message.h>

namespace capnzero::CalculatorQtWebchannel
{

class CalculatorQtWebchannelClientSignals
{
public:
    CalculatorQtWebchannelClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;

	using ModeChangedCb = std::function<void(EMode val)>;
	void onModeChanged(ModeChangedCb cb);
	using ScreenColorChangedCb = std::function<void(EColor color)>;
	void onScreenColorChanged(ScreenColorChangedCb cb);
	using ScreenBrightnessChangedCb = std::function<void(UInt32 brightness)>;
	void onScreenBrightnessChanged(ScreenBrightnessChangedCb cb);
	using ScreenAlltypessCb = std::function<void(Int32 a, Float32 b, Float64 c, const TextView& d, const Span& e, const SpanCL<8>& f, EColor color, EMode mode)>;
	void onScreenAlltypess(ScreenAlltypessCb cb);

private:
    zmq::socket_t m_zmqSubSocket;
	ModeChangedCb m_modeChangedCb;
	ScreenColorChangedCb m_screenColorChangedCb;
	ScreenBrightnessChangedCb m_screenBrightnessChangedCb;
	ScreenAlltypessCb m_screenAlltypessCb;

};

} // namespace capnzero::CalculatorQtWebchannel

#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::CalculatorQtWebchannel
{

class CalculatorQtWebchannelClientSignalsThreaded
{
public:
    CalculatorQtWebchannelClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~CalculatorQtWebchannelClientSignalsThreaded();
    void StartThread();
	void signalReceiverThreadFn();
	using ModeChangedCb = std::function<void(EMode val)>;
	void onModeChanged(ModeChangedCb cb);
	using ScreenColorChangedCb = std::function<void(EColor color)>;
	void onScreenColorChanged(ScreenColorChangedCb cb);
	using ScreenBrightnessChangedCb = std::function<void(UInt32 brightness)>;
	void onScreenBrightnessChanged(ScreenBrightnessChangedCb cb);
	using ScreenAlltypessCb = std::function<void(Int32 a, Float32 b, Float64 c, const TextView& d, const Span& e, const SpanCL<8>& f, EColor color, EMode mode)>;
	void onScreenAlltypess(ScreenAlltypessCb cb);

private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{false};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
	ModeChangedCb m_modeChangedCb;
	ScreenColorChangedCb m_screenColorChangedCb;
	ScreenBrightnessChangedCb m_screenBrightnessChangedCb;
	ScreenAlltypessCb m_screenAlltypessCb;

};

} // namespace capnzero::CalculatorQtWebchannel


namespace capnzero::CalculatorQtWebchannel
{

class CalculatorQtWebchannelClient : public CalculatorQtWebchannelClientRpc, public CalculatorQtWebchannelClientSignalsThreaded
{
public:
    CalculatorQtWebchannelClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr);
};



} // namespace capnzero::CalculatorQtWebchannel
#include "CalculatorQtWebchannel_Client.inl"
#endif
