#ifndef TEMPERATUREQTSIGNALSONLY_SERVER_H
#define TEMPERATUREQTSIGNALSONLY_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "TemperatureQtSignalsOnly.capnp.h"



namespace capnzero::TemperatureQtSignalsOnly
{

class TemperatureQtSignalsOnlyServer
{
public:
	TemperatureQtSignalsOnlyServer(zmq::context_t& rZmqContext, const std::string& signalBindAddr);


    class Signals
    {
    public:
        Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr);
        void unbind(const std::string& signalBindAddr);
        int getFd() const;
        void handleAllSubscriptions();
		void tempRangeChanged2(ETempRange tempRange);
		void sensorNameChanged(const Text& val);
		void actualTemperatureChanged(Float32 val);
		void tempRangeChanged(ETempRange val);

        using SubscriptionCb = std::function<void(Signals&)>;
			void registerTempRangeChanged2SubscrCb(SubscriptionCb cb);
			void registerSensorNameChangedSubscrCb(SubscriptionCb cb);
			void registerActualTemperatureChangedSubscrCb(SubscriptionCb cb);
			void registerTempRangeChangedSubscrCb(SubscriptionCb cb);

    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    };
    Signals &signals() { return m_signals; }

private:

	Signals m_signals;

};

} // namespace capnzero::TemperatureQtSignalsOnly
#endif
