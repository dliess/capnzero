#ifndef CALCULATORSIGNALSONLY_SERVER_H
#define CALCULATORSIGNALSONLY_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "CalculatorSignalsOnly.capnp.h"



namespace capnzero::CalculatorSignalsOnly
{

class CalculatorSignalsOnlyServer
{
public:
	CalculatorSignalsOnlyServer(zmq::context_t& rZmqContext, const std::string& signalBindAddr);


    class Signals
    {
    public:
        Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr);
        void unbind(const std::string& signalBindAddr);
        int getFd() const;
        void handleAllSubscriptions();
		void Screen__brightnessChanged(UInt32 brightness);
		void Screen__colorChanged(EColor color);

        using SubscriptionCb = std::function<void(Signals&)>;
			void registerScreenBrightnessChangedSubscrCb(SubscriptionCb cb);
			void registerScreenColorChangedSubscrCb(SubscriptionCb cb);

    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    };
    Signals &signals() { return m_signals; }

private:

	Signals m_signals;

};

} // namespace capnzero::CalculatorSignalsOnly
#endif
