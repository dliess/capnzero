#ifndef CALCULATOR_CLIENT_H
#define CALCULATOR_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "Calculator.capnp.h"

#include "Calculator_ClientTransport.h"
namespace capnzero::Calculator
{

class CalculatorClientRpc : public CalculatorClientRpcTransport
{
public:
    using Super = CalculatorClientRpcTransport;
    CalculatorClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	struct Return_Add {
		Int32 ret;
	};
	Return_Add
	add(Int32 a, Int32 b);

	struct Return_Sub {
		Int32 ret;
	};
	Return_Sub
	sub(Int32 a, Int32 b);

	Int32 addDirectRet(Int32 a, Int32 b);

	Int32 subDirectRet(Int32 a, Int32 b);

	void multiply(::capnp::MessageBuilder& msgBuilder);

	struct Return_Multiply2 {
		Int32 ret;
	};
	Return_Multiply2
	multiply2(::capnp::MessageBuilder& msgBuilder);

	template<typename Callable>
	void multiply3(Int32 a, Int32 b, Callable&& retCb);

	template<typename Callable>
	void multiply4(::capnp::MessageBuilder& msgBuilder, Callable&& retCb);

	Int32 DirectRet__add(Int32 a, Int32 b);

	Int32 DirectRet__sub(Int32 a, Int32 b);

	void Screen__setBrightness(UInt32 brightness);

	void Screen__setColor(EColor color);

	void Screen__setTitle(const Text& title);

	void Screen__setId(const Data<8>& title);


};

} // namespace capnzero::Calculator

#include <functional>
#include <capnp/message.h>

namespace capnzero::Calculator
{

class CalculatorClientSignals
{
public:
    CalculatorClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;

	using JustASignalCb = std::function<void()>;
	void onJustASignal(JustASignalCb cb);
	using ScreenListChangedCb = std::function<void(::capnp::MessageReader& recvData)>;
	void onScreenListChanged(ScreenListChangedCb cb);
	using ScreenColorChangedCb = std::function<void(EColor color)>;
	void onScreenColorChanged(ScreenColorChangedCb cb);
	using ScreenBrightnessChangedCb = std::function<void(UInt32 brightness)>;
	void onScreenBrightnessChanged(ScreenBrightnessChangedCb cb);

private:
    zmq::socket_t m_zmqSubSocket;
	JustASignalCb m_justASignalCb;
	ScreenListChangedCb m_screenListChangedCb;
	ScreenColorChangedCb m_screenColorChangedCb;
	ScreenBrightnessChangedCb m_screenBrightnessChangedCb;

};

} // namespace capnzero::Calculator

#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::Calculator
{

class CalculatorClientSignalsThreaded
{
public:
    CalculatorClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~CalculatorClientSignalsThreaded();
    void StartThread();
	void signalReceiverThreadFn();
	using JustASignalCb = std::function<void()>;
	void onJustASignal(JustASignalCb cb);
	using ScreenListChangedCb = std::function<void(::capnp::MessageReader& recvData)>;
	void onScreenListChanged(ScreenListChangedCb cb);
	using ScreenColorChangedCb = std::function<void(EColor color)>;
	void onScreenColorChanged(ScreenColorChangedCb cb);
	using ScreenBrightnessChangedCb = std::function<void(UInt32 brightness)>;
	void onScreenBrightnessChanged(ScreenBrightnessChangedCb cb);

private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{false};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
	JustASignalCb m_justASignalCb;
	ScreenListChangedCb m_screenListChangedCb;
	ScreenColorChangedCb m_screenColorChangedCb;
	ScreenBrightnessChangedCb m_screenBrightnessChangedCb;

};

} // namespace capnzero::Calculator


namespace capnzero::Calculator
{

class CalculatorClient : public CalculatorClientRpc, public CalculatorClientSignalsThreaded
{
public:
    CalculatorClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr);
};



} // namespace capnzero::Calculator
#include "Calculator_Client.inl"
#endif
