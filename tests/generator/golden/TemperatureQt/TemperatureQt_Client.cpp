#include "TemperatureQt_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "TemperatureQt.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQt;
TemperatureQtClientRpc::TemperatureQtClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    Super(rZmqContext, serverRpcAddr)
{
}
TemperatureQtClientSignals::TemperatureQtClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(serverSignalAddr);
}

int TemperatureQtClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

TemperatureQtClientSignalsThreaded::TemperatureQtClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{
    m_zmqSubHelperSocket.bind("inproc://TemperatureQtSubHelper");
}

TemperatureQtClientSignalsThreaded::~TemperatureQtClientSignalsThreaded()
{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}

void TemperatureQtClientSignalsThreaded::StartThread()
{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() { signalReceiverThreadFn(); });
}

void TemperatureQtClientRpc::explicitSetUUID(const Data<16>& uuid)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterExplicitSetUUID>();
	paramBuilder.setUuid(capnp::Data::Reader(uuid.data(), uuid.size()));


    _explicitSetUUID(
		message

    );

}

Data<16> TemperatureQtClientRpc::explicitGetUUID()
{

	Data<16> retVal;

    _explicitGetUUID(

		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnExplicitGetUUID>();
			auto src = reader.getRetParam();
			assert(src.size() == retVal.size());
			std::copy(src.begin(), src.end(), retVal.begin());

	}
    );
	return retVal;
}

void TemperatureQtClientRpc::setCommandedTemperature(Float32 val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetCommandedTemperature>();
	paramBuilder.setVal(val);


    _setCommandedTemperature(
		message

    );

}

void TemperatureQtClientRpc::setEnabled(Bool val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetEnabled>();
	paramBuilder.setVal(val);


    _setEnabled(
		message

    );

}

void TemperatureQtClientRpc::toggleEnabled()
{


    _toggleEnabled(


    );

}

void TemperatureQtClientRpc::setUuid(const Data<16>& val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetUuid>();
	paramBuilder.setVal(capnp::Data::Reader(val.data(), val.size()));


    _setUuid(
		message

    );

}

void TemperatureQtClientSignalsThreaded::signalReceiverThreadFn()
{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://TemperatureQtSubHelper");

    while(!m_terminateRequest.load())
    {
        zmq::pollitem_t items[] = {
            {zmqSubSocket.handle(), 0, ZMQ_POLLIN, 0},
			{zmqSubHelperSocket.handle(), 0, ZMQ_POLLIN, 0}
        };
        zmq::poll(&items[0], 2);
		if (items[0].revents & ZMQ_POLLIN)
		{
            handleIncomingSignal(zmqSubSocket);
		}
		if (items[1].revents & ZMQ_POLLIN)
		{
			handleSubscriptionRequest(zmqSubHelperSocket, zmqSubSocket);
		}
    }
}

void TemperatureQtClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "explicitSignalUUID"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterExplicitSignalUUID>();
		if(m_explicitSignalUUIDCb) m_explicitSignalUUIDCb(SpanCL<16>(paramReader.getUuid().begin(), paramReader.getUuid().end()));
	}
	else if(key == "sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterSensorNameChanged>();
		if(m_sensorNameChangedCb) m_sensorNameChangedCb(TextView(paramReader.getVal().begin(), paramReader.getVal().size()));
	}
	else if(key == "actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterActualTemperatureChanged>();
		if(m_actualTemperatureChangedCb) m_actualTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "commandedTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterCommandedTemperatureChanged>();
		if(m_commandedTemperatureChangedCb) m_commandedTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "enabledChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterEnabledChanged>();
		if(m_enabledChangedCb) m_enabledChangedCb(paramReader.getVal());
	}
	else if(key == "uuidChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterUuidChanged>();
		if(m_uuidChangedCb) m_uuidChangedCb(SpanCL<16>(paramReader.getVal().begin(), paramReader.getVal().end()));
	}

}

void TemperatureQtClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

void TemperatureQtClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "explicitSignalUUID"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterExplicitSignalUUID>();
		if(m_explicitSignalUUIDCb) m_explicitSignalUUIDCb(SpanCL<16>(paramReader.getUuid().begin(), paramReader.getUuid().end()));
	}
	else if(key == "sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterSensorNameChanged>();
		if(m_sensorNameChangedCb) m_sensorNameChangedCb(TextView(paramReader.getVal().begin(), paramReader.getVal().size()));
	}
	else if(key == "actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterActualTemperatureChanged>();
		if(m_actualTemperatureChangedCb) m_actualTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "commandedTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterCommandedTemperatureChanged>();
		if(m_commandedTemperatureChangedCb) m_commandedTemperatureChangedCb(paramReader.getVal());
	}
	else if(key == "enabledChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterEnabledChanged>();
		if(m_enabledChangedCb) m_enabledChangedCb(paramReader.getVal());
	}
	else if(key == "uuidChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterUuidChanged>();
		if(m_uuidChangedCb) m_uuidChangedCb(SpanCL<16>(paramReader.getVal().begin(), paramReader.getVal().end()));
	}
}

void TemperatureQtClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}

void TemperatureQtClientSignals::onExplicitSignalUUID(ExplicitSignalUUIDCb cb)
{
    if(m_explicitSignalUUIDCb) return; // set it only once
    m_explicitSignalUUIDCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "explicitSignalUUID");
}

void TemperatureQtClientSignalsThreaded::onExplicitSignalUUID(ExplicitSignalUUIDCb cb)
{
    if(m_explicitSignalUUIDCb) return; // set it only once
    m_explicitSignalUUIDCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("explicitSignalUUID", 18));
}

void TemperatureQtClientSignals::onSensorNameChanged(SensorNameChangedCb cb)
{
    if(m_sensorNameChangedCb) return; // set it only once
    m_sensorNameChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "sensorNameChanged");
}

void TemperatureQtClientSignalsThreaded::onSensorNameChanged(SensorNameChangedCb cb)
{
    if(m_sensorNameChangedCb) return; // set it only once
    m_sensorNameChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("sensorNameChanged", 17));
}

void TemperatureQtClientSignals::onActualTemperatureChanged(ActualTemperatureChangedCb cb)
{
    if(m_actualTemperatureChangedCb) return; // set it only once
    m_actualTemperatureChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "actualTemperatureChanged");
}

void TemperatureQtClientSignalsThreaded::onActualTemperatureChanged(ActualTemperatureChangedCb cb)
{
    if(m_actualTemperatureChangedCb) return; // set it only once
    m_actualTemperatureChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("actualTemperatureChanged", 24));
}

void TemperatureQtClientSignals::onCommandedTemperatureChanged(CommandedTemperatureChangedCb cb)
{
    if(m_commandedTemperatureChangedCb) return; // set it only once
    m_commandedTemperatureChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "commandedTemperatureChanged");
}

void TemperatureQtClientSignalsThreaded::onCommandedTemperatureChanged(CommandedTemperatureChangedCb cb)
{
    if(m_commandedTemperatureChangedCb) return; // set it only once
    m_commandedTemperatureChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("commandedTemperatureChanged", 27));
}

void TemperatureQtClientSignals::onEnabledChanged(EnabledChangedCb cb)
{
    if(m_enabledChangedCb) return; // set it only once
    m_enabledChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "enabledChanged");
}

void TemperatureQtClientSignalsThreaded::onEnabledChanged(EnabledChangedCb cb)
{
    if(m_enabledChangedCb) return; // set it only once
    m_enabledChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("enabledChanged", 14));
}

void TemperatureQtClientSignals::onUuidChanged(UuidChangedCb cb)
{
    if(m_uuidChangedCb) return; // set it only once
    m_uuidChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "uuidChanged");
}

void TemperatureQtClientSignalsThreaded::onUuidChanged(UuidChangedCb cb)
{
    if(m_uuidChangedCb) return; // set it only once
    m_uuidChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("uuidChanged", 11));
}

TemperatureQtClient::TemperatureQtClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr) :
    TemperatureQtClientRpc(rZmqContext, serverRpcAddr),
    TemperatureQtClientSignalsThreaded(rZmqContext, serverSignalAddr)
{
    StartThread();
}
