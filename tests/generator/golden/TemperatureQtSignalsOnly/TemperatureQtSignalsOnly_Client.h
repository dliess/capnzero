#ifndef TEMPERATUREQTSIGNALSONLY_CLIENT_H
#define TEMPERATUREQTSIGNALSONLY_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "TemperatureQtSignalsOnly.capnp.h"


#include <functional>
#include <capnp/message.h>

namespace capnzero::TemperatureQtSignalsOnly
{

class TemperatureQtSignalsOnlyClientSignals
{
public:
    TemperatureQtSignalsOnlyClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;

	using TempRangeChanged2Cb = std::function<void(ETempRange tempRange)>;
	void onTempRangeChanged2(TempRangeChanged2Cb cb);
	using SensorNameChangedCb = std::function<void(const TextView& val)>;
	void onSensorNameChanged(SensorNameChangedCb cb);
	using ActualTemperatureChangedCb = std::function<void(Float32 val)>;
	void onActualTemperatureChanged(ActualTemperatureChangedCb cb);
	using TempRangeChangedCb = std::function<void(ETempRange val)>;
	void onTempRangeChanged(TempRangeChangedCb cb);

private:
    zmq::socket_t m_zmqSubSocket;
	TempRangeChanged2Cb m_tempRangeChanged2Cb;
	SensorNameChangedCb m_sensorNameChangedCb;
	ActualTemperatureChangedCb m_actualTemperatureChangedCb;
	TempRangeChangedCb m_tempRangeChangedCb;

};

} // namespace capnzero::TemperatureQtSignalsOnly

#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::TemperatureQtSignalsOnly
{

class TemperatureQtSignalsOnlyClientSignalsThreaded
{
public:
    TemperatureQtSignalsOnlyClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~TemperatureQtSignalsOnlyClientSignalsThreaded();
    void StartThread();
	void signalReceiverThreadFn();
	using TempRangeChanged2Cb = std::function<void(ETempRange tempRange)>;
	void onTempRangeChanged2(TempRangeChanged2Cb cb);
	using SensorNameChangedCb = std::function<void(const TextView& val)>;
	void onSensorNameChanged(SensorNameChangedCb cb);
	using ActualTemperatureChangedCb = std::function<void(Float32 val)>;
	void onActualTemperatureChanged(ActualTemperatureChangedCb cb);
	using TempRangeChangedCb = std::function<void(ETempRange val)>;
	void onTempRangeChanged(TempRangeChangedCb cb);

private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{false};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
	TempRangeChanged2Cb m_tempRangeChanged2Cb;
	SensorNameChangedCb m_sensorNameChangedCb;
	ActualTemperatureChangedCb m_actualTemperatureChangedCb;
	TempRangeChangedCb m_tempRangeChangedCb;

};

} // namespace capnzero::TemperatureQtSignalsOnly


namespace capnzero::TemperatureQtSignalsOnly
{

class TemperatureQtSignalsOnlyClient : public TemperatureQtSignalsOnlyClientSignalsThreaded
{
public:
    TemperatureQtSignalsOnlyClient(zmq::context_t& rZmqContext, const std::string& serverSignalAddr);
};



} // namespace capnzero::TemperatureQtSignalsOnly
#include "TemperatureQtSignalsOnly_Client.inl"
#endif
