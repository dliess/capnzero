#ifndef TEMPERATUREQTWITHSERVICENAME_CLIENT_H
#define TEMPERATUREQTWITHSERVICENAME_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "TemperatureQtWithServiceName.capnp.h"

#include "TemperatureQtWithServiceName_ClientTransport.h"
namespace capnzero::TemperatureQtWithServiceName
{

class TemperatureQtWithServiceNameClientRpc : public TemperatureQtWithServiceNameClientRpcTransport
{
public:
    using Super = TemperatureQtWithServiceNameClientRpcTransport;
    TemperatureQtWithServiceNameClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void TemperatureService__aTestRpc(const Text& title);

	void TemperatureService__setCommandedTemperature(Float32 val);


};

} // namespace capnzero::TemperatureQtWithServiceName

#include <functional>
#include <capnp/message.h>

namespace capnzero::TemperatureQtWithServiceName
{

class TemperatureQtWithServiceNameClientSignals
{
public:
    TemperatureQtWithServiceNameClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;

	using TemperatureServiceSensorNameChangedCb = std::function<void(const TextView& val)>;
	void onTemperatureServiceSensorNameChanged(TemperatureServiceSensorNameChangedCb cb);
	using TemperatureServiceActualTemperatureChangedCb = std::function<void(Float32 val)>;
	void onTemperatureServiceActualTemperatureChanged(TemperatureServiceActualTemperatureChangedCb cb);
	using TemperatureServiceCommandedTemperatureChangedCb = std::function<void(Float32 val)>;
	void onTemperatureServiceCommandedTemperatureChanged(TemperatureServiceCommandedTemperatureChangedCb cb);

private:
    zmq::socket_t m_zmqSubSocket;
	TemperatureServiceSensorNameChangedCb m_temperatureServiceSensorNameChangedCb;
	TemperatureServiceActualTemperatureChangedCb m_temperatureServiceActualTemperatureChangedCb;
	TemperatureServiceCommandedTemperatureChangedCb m_temperatureServiceCommandedTemperatureChangedCb;

};

} // namespace capnzero::TemperatureQtWithServiceName

#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::TemperatureQtWithServiceName
{

class TemperatureQtWithServiceNameClientSignalsThreaded
{
public:
    TemperatureQtWithServiceNameClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~TemperatureQtWithServiceNameClientSignalsThreaded();
    void StartThread();
	void signalReceiverThreadFn();
	using TemperatureServiceSensorNameChangedCb = std::function<void(const TextView& val)>;
	void onTemperatureServiceSensorNameChanged(TemperatureServiceSensorNameChangedCb cb);
	using TemperatureServiceActualTemperatureChangedCb = std::function<void(Float32 val)>;
	void onTemperatureServiceActualTemperatureChanged(TemperatureServiceActualTemperatureChangedCb cb);
	using TemperatureServiceCommandedTemperatureChangedCb = std::function<void(Float32 val)>;
	void onTemperatureServiceCommandedTemperatureChanged(TemperatureServiceCommandedTemperatureChangedCb cb);

private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{false};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
	TemperatureServiceSensorNameChangedCb m_temperatureServiceSensorNameChangedCb;
	TemperatureServiceActualTemperatureChangedCb m_temperatureServiceActualTemperatureChangedCb;
	TemperatureServiceCommandedTemperatureChangedCb m_temperatureServiceCommandedTemperatureChangedCb;

};

} // namespace capnzero::TemperatureQtWithServiceName


namespace capnzero::TemperatureQtWithServiceName
{

class TemperatureQtWithServiceNameClient : public TemperatureQtWithServiceNameClientRpc, public TemperatureQtWithServiceNameClientSignalsThreaded
{
public:
    TemperatureQtWithServiceNameClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr);
};



} // namespace capnzero::TemperatureQtWithServiceName
#include "TemperatureQtWithServiceName_Client.inl"
#endif
