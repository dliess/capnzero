#ifndef TEMPERATUREQT_SERVER_H
#define TEMPERATUREQT_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "TemperatureQt.capnp.h"

#include "TemperatureQtRpcIf.h"


namespace capnzero::TemperatureQt
{

class TemperatureQtServer
{
public:
	TemperatureQtServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<RpcIf> _);
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
		void explicitSignalUUID(const Data<16>& uuid);
		void sensorNameChanged(const Text& val);
		void actualTemperatureChanged(Float32 val);
		void commandedTemperatureChanged(Float32 val);
		void enabledChanged(Bool val);
		void uuidChanged(const Data<16>& val);

        using SubscriptionCb = std::function<void(Signals&)>;
			void registerExplicitSignalUUIDSubscrCb(SubscriptionCb cb);
			void registerSensorNameChangedSubscrCb(SubscriptionCb cb);
			void registerActualTemperatureChangedSubscrCb(SubscriptionCb cb);
			void registerCommandedTemperatureChangedSubscrCb(SubscriptionCb cb);
			void registerEnabledChangedSubscrCb(SubscriptionCb cb);
			void registerUuidChangedSubscrCb(SubscriptionCb cb);

    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    };
    Signals &signals() { return m_signals; }

private:
	std::unique_ptr<RpcIf> m_p_RpcIf;
	zmq::socket_t m_zmqRepSocket;

	Signals m_signals;

};

} // namespace capnzero::TemperatureQt
#endif
