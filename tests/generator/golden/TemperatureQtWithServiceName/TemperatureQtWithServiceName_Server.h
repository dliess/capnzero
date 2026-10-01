#ifndef TEMPERATUREQTWITHSERVICENAME_SERVER_H
#define TEMPERATUREQTWITHSERVICENAME_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "TemperatureQtWithServiceName.capnp.h"

#include "TemperatureQtWithServiceNameTemperatureServiceRpcIf.h"


namespace capnzero::TemperatureQtWithServiceName
{

class TemperatureQtWithServiceNameServer
{
public:
	TemperatureQtWithServiceNameServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<TemperatureServiceRpcIf> temperatureService);
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
		void TemperatureService__sensorNameChanged(const Text& val);
		void TemperatureService__actualTemperatureChanged(Float32 val);
		void TemperatureService__commandedTemperatureChanged(Float32 val);

        using SubscriptionCb = std::function<void(Signals&)>;
			void registerTemperatureServiceSensorNameChangedSubscrCb(SubscriptionCb cb);
			void registerTemperatureServiceActualTemperatureChangedSubscrCb(SubscriptionCb cb);
			void registerTemperatureServiceCommandedTemperatureChangedSubscrCb(SubscriptionCb cb);

    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    };
    Signals &signals() { return m_signals; }

private:
	std::unique_ptr<TemperatureServiceRpcIf> m_pTemperatureServiceRpcIf;
	zmq::socket_t m_zmqRepSocket;

	Signals m_signals;

};

} // namespace capnzero::TemperatureQtWithServiceName
#endif
