#ifndef CALCULATOR_SERVER_H
#define CALCULATOR_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "Calculator.capnp.h"

#include "CalculatorRpcIf.h"
#include "CalculatorDirectRetRpcIf.h"
#include "CalculatorScreenRpcIf.h"


namespace capnzero::Calculator
{

class CalculatorServer
{
public:
	CalculatorServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<RpcIf> _, std::unique_ptr<DirectRetRpcIf> directRet, std::unique_ptr<ScreenRpcIf> screen);
	void unbind(const std::string& rpcBindAddr);
    enum class WaitMode {
        Blocking,
        NonBlocking
    };
    void processNextRequest(WaitMode waitMode = WaitMode::Blocking);
    void processNextRequestAllNonBlock();
  	int getFd() const;

    class Signals
    {
    public:
        Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr);
        void unbind(const std::string& signalBindAddr);
        int getFd() const;
        void handleAllSubscriptions();
		void justASignal();
		void Screen__listChanged(::capnp::MessageBuilder& sndData);
		void Screen__colorChanged(EColor color);
		void Screen__brightnessChanged(UInt32 brightness);

        using SubscriptionCb = std::function<void(Signals&)>;
			void registerJustASignalSubscrCb(SubscriptionCb cb);
			void registerScreenListChangedSubscrCb(SubscriptionCb cb);
			void registerScreenColorChangedSubscrCb(SubscriptionCb cb);
			void registerScreenBrightnessChangedSubscrCb(SubscriptionCb cb);

    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    };
    Signals &signals() { return m_signals; }

private:
	std::unique_ptr<RpcIf> m_p_RpcIf;
	std::unique_ptr<DirectRetRpcIf> m_pDirectRetRpcIf;
	std::unique_ptr<ScreenRpcIf> m_pScreenRpcIf;
	zmq::socket_t m_zmqRepSocket;

	Signals m_signals;

};

} // namespace capnzero::Calculator
#endif
