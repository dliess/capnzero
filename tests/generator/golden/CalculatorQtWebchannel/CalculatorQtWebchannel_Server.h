#ifndef CALCULATORQTWEBCHANNEL_SERVER_H
#define CALCULATORQTWEBCHANNEL_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "CalculatorQtWebchannel.capnp.h"

#include "CalculatorQtWebchannelRpcIf.h"
#include "CalculatorQtWebchannelCalculatorRpcIf.h"
#include "CalculatorQtWebchannelScreenRpcIf.h"


namespace capnzero::CalculatorQtWebchannel
{

class CalculatorQtWebchannelServer
{
public:
	CalculatorQtWebchannelServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<RpcIf> _, std::unique_ptr<CalculatorRpcIf> calculator, std::unique_ptr<ScreenRpcIf> screen);
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
		void modeChanged(EMode val);
		void Screen__colorChanged(EColor color);
		void Screen__brightnessChanged(UInt32 brightness);
		void Screen__alltypess(Int32 a, Float32 b, Float64 c, const Text& d, const Span& e, const Data<8>& f, EColor color, EMode mode);

        using SubscriptionCb = std::function<void(Signals&)>;
			void registerModeChangedSubscrCb(SubscriptionCb cb);
			void registerScreenColorChangedSubscrCb(SubscriptionCb cb);
			void registerScreenBrightnessChangedSubscrCb(SubscriptionCb cb);
			void registerScreenAlltypessSubscrCb(SubscriptionCb cb);

    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    };
    Signals &signals() { return m_signals; }

private:
	std::unique_ptr<RpcIf> m_p_RpcIf;
	std::unique_ptr<CalculatorRpcIf> m_pCalculatorRpcIf;
	std::unique_ptr<ScreenRpcIf> m_pScreenRpcIf;
	zmq::socket_t m_zmqRepSocket;

	Signals m_signals;

};

} // namespace capnzero::CalculatorQtWebchannel
#endif
