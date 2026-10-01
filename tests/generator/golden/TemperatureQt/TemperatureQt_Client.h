#ifndef TEMPERATUREQT_CLIENT_H
#define TEMPERATUREQT_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "TemperatureQt.capnp.h"

#include "TemperatureQt_ClientTransport.h"
namespace capnzero::TemperatureQt
{

class TemperatureQtClientRpc : public TemperatureQtClientRpcTransport
{
public:
    using Super = TemperatureQtClientRpcTransport;
    TemperatureQtClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void explicitSetUUID(const Data<16>& uuid);

	Data<16>
	explicitGetUUID();

	void setCommandedTemperature(Float32 val);

	void setEnabled(Bool val);

	void toggleEnabled();

	void setUuid(const Data<16>& val);


};

} // namespace capnzero::TemperatureQt

#include <functional>
#include <capnp/message.h>

namespace capnzero::TemperatureQt
{

class TemperatureQtClientSignals
{
public:
    TemperatureQtClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;

	using ExplicitSignalUUIDCb = std::function<void(const SpanCL<16>& uuid)>;
	void onExplicitSignalUUID(ExplicitSignalUUIDCb cb);
	using SensorNameChangedCb = std::function<void(const TextView& val)>;
	void onSensorNameChanged(SensorNameChangedCb cb);
	using ActualTemperatureChangedCb = std::function<void(Float32 val)>;
	void onActualTemperatureChanged(ActualTemperatureChangedCb cb);
	using CommandedTemperatureChangedCb = std::function<void(Float32 val)>;
	void onCommandedTemperatureChanged(CommandedTemperatureChangedCb cb);
	using EnabledChangedCb = std::function<void(Bool val)>;
	void onEnabledChanged(EnabledChangedCb cb);
	using UuidChangedCb = std::function<void(const SpanCL<16>& val)>;
	void onUuidChanged(UuidChangedCb cb);

private:
    zmq::socket_t m_zmqSubSocket;
	ExplicitSignalUUIDCb m_explicitSignalUUIDCb;
	SensorNameChangedCb m_sensorNameChangedCb;
	ActualTemperatureChangedCb m_actualTemperatureChangedCb;
	CommandedTemperatureChangedCb m_commandedTemperatureChangedCb;
	EnabledChangedCb m_enabledChangedCb;
	UuidChangedCb m_uuidChangedCb;

};

} // namespace capnzero::TemperatureQt

#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::TemperatureQt
{

class TemperatureQtClientSignalsThreaded
{
public:
    TemperatureQtClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~TemperatureQtClientSignalsThreaded();
    void StartThread();
	void signalReceiverThreadFn();
	using ExplicitSignalUUIDCb = std::function<void(const SpanCL<16>& uuid)>;
	void onExplicitSignalUUID(ExplicitSignalUUIDCb cb);
	using SensorNameChangedCb = std::function<void(const TextView& val)>;
	void onSensorNameChanged(SensorNameChangedCb cb);
	using ActualTemperatureChangedCb = std::function<void(Float32 val)>;
	void onActualTemperatureChanged(ActualTemperatureChangedCb cb);
	using CommandedTemperatureChangedCb = std::function<void(Float32 val)>;
	void onCommandedTemperatureChanged(CommandedTemperatureChangedCb cb);
	using EnabledChangedCb = std::function<void(Bool val)>;
	void onEnabledChanged(EnabledChangedCb cb);
	using UuidChangedCb = std::function<void(const SpanCL<16>& val)>;
	void onUuidChanged(UuidChangedCb cb);

private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{false};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
	ExplicitSignalUUIDCb m_explicitSignalUUIDCb;
	SensorNameChangedCb m_sensorNameChangedCb;
	ActualTemperatureChangedCb m_actualTemperatureChangedCb;
	CommandedTemperatureChangedCb m_commandedTemperatureChangedCb;
	EnabledChangedCb m_enabledChangedCb;
	UuidChangedCb m_uuidChangedCb;

};

} // namespace capnzero::TemperatureQt


namespace capnzero::TemperatureQt
{

class TemperatureQtClient : public TemperatureQtClientRpc, public TemperatureQtClientSignalsThreaded
{
public:
    TemperatureQtClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr);
};



} // namespace capnzero::TemperatureQt
#include "TemperatureQt_Client.inl"
#endif
