#ifndef CALCULATORSIGNALSONLY_CLIENT_H
#define CALCULATORSIGNALSONLY_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "CalculatorSignalsOnly.capnp.h"


#include <functional>
#include <capnp/message.h>

namespace capnzero::CalculatorSignalsOnly
{

class CalculatorSignalsOnlyClientSignals
{
public:
    CalculatorSignalsOnlyClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;

	using ScreenBrightnessChangedCb = std::function<void(UInt32 brightness)>;
	void onScreenBrightnessChanged(ScreenBrightnessChangedCb cb);
	using ScreenColorChangedCb = std::function<void(EColor color)>;
	void onScreenColorChanged(ScreenColorChangedCb cb);

private:
    zmq::socket_t m_zmqSubSocket;
	ScreenBrightnessChangedCb m_screenBrightnessChangedCb;
	ScreenColorChangedCb m_screenColorChangedCb;

};

} // namespace capnzero::CalculatorSignalsOnly

#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::CalculatorSignalsOnly
{

class CalculatorSignalsOnlyClientSignalsThreaded
{
public:
    CalculatorSignalsOnlyClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~CalculatorSignalsOnlyClientSignalsThreaded();
    void StartThread();
	void signalReceiverThreadFn();
	using ScreenBrightnessChangedCb = std::function<void(UInt32 brightness)>;
	void onScreenBrightnessChanged(ScreenBrightnessChangedCb cb);
	using ScreenColorChangedCb = std::function<void(EColor color)>;
	void onScreenColorChanged(ScreenColorChangedCb cb);

private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{false};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
	ScreenBrightnessChangedCb m_screenBrightnessChangedCb;
	ScreenColorChangedCb m_screenColorChangedCb;

};

} // namespace capnzero::CalculatorSignalsOnly


namespace capnzero::CalculatorSignalsOnly
{

class CalculatorSignalsOnlyClient : public CalculatorSignalsOnlyClientSignalsThreaded
{
public:
    CalculatorSignalsOnlyClient(zmq::context_t& rZmqContext, const std::string& serverSignalAddr);
};



} // namespace capnzero::CalculatorSignalsOnly
#include "CalculatorSignalsOnly_Client.inl"
#endif
