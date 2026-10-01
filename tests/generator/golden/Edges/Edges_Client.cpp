#include "Edges_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "Edges.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::Edges;
EdgesClientRpc::EdgesClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    Super(rZmqContext, serverRpcAddr)
{
}
EdgesClientSignals::EdgesClientSignals(zmq::context_t& rZmqContext, const std::string& serverSignalAddr):
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(serverSignalAddr);
}

int EdgesClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

EdgesClientSignalsThreaded::EdgesClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverSignalAddr):
    m_rZmqContext(rZmqContext),
    m_zmqSubHelperSocket(rZmqContext, zmq::socket_type::pair),
    m_serverSignalAddr(std::move(serverSignalAddr))
{
    m_zmqSubHelperSocket.bind("inproc://EdgesSubHelper");
}

EdgesClientSignalsThreaded::~EdgesClientSignalsThreaded()
{
	m_terminateRequest.store(true);
    // Abuse this socket to quit signal thread loop
	m_zmqSubHelperSocket.send(zmq::const_buffer("Gonna Quit", 10));
	if(m_pThread) m_pThread->join();
}

void EdgesClientSignalsThreaded::StartThread()
{
    if(m_pThread) return;
    m_pThread = std::make_unique<std::thread>([this]() { signalReceiverThreadFn(); });
}

EdgesClientRpc::ReturnZebraEmpty
EdgesClientRpc::Zebra__empty()
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterZebraEmpty>();

	ReturnZebraEmpty retVal;

    _Zebra__empty(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnZebraEmpty>();

	}
    );
	return retVal;
}

Int32 EdgesClientRpc::Zebra__noArgs()
{

	Int32 retVal;

    _Zebra__noArgs(

		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnZebraNoArgs>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

Data<4> EdgesClientRpc::Zebra__bytes(const Span& dynamic, const Data<4>& fixed, const Text& text, Mode mode)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterZebraBytes>();
	paramBuilder.setDynamic(capnp::Data::Reader(dynamic.data(), dynamic.size()));
	paramBuilder.setFixed(capnp::Data::Reader(fixed.data(), fixed.size()));
	paramBuilder.setText(text);
	paramBuilder.setMode(mode);

	Data<4> retVal;

    _Zebra__bytes(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnZebraBytes>();
			auto src = reader.getRetParam();
			assert(src.size() == retVal.size());
			std::copy(src.begin(), src.end(), retVal.begin());

	}
    );
	return retVal;
}

void EdgesClientRpc::Zebra__setActive(Bool val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterZebraSetActive>();
	paramBuilder.setVal(val);


    _Zebra__setActive(
		message

    );

}

void EdgesClientRpc::Zebra__toggleActive()
{


    _Zebra__toggleActive(


    );

}

void EdgesClientRpc::fire()
{


    _fire(


    );

}

void EdgesClientSignalsThreaded::signalReceiverThreadFn()
{
    zmq::socket_t zmqSubSocket(m_rZmqContext, zmq::socket_type::sub);
    zmqSubSocket.connect(m_serverSignalAddr);
    zmq::socket_t zmqSubHelperSocket(m_rZmqContext, zmq::socket_type::pair);
    zmqSubHelperSocket.connect("inproc://EdgesSubHelper");

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

void EdgesClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "Zebra__activeChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterZebraActiveChanged>();
		if(m_zebraActiveChangedCb) m_zebraActiveChangedCb(paramReader.getVal());
	}
	else if(key == "empty"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterEmpty>();
		if(m_emptyCb) m_emptyCb();
	}
	else if(key == "tick"){
		if(m_tickCb) m_tickCb();
	}

}

void EdgesClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

void EdgesClientSignalsThreaded::handleIncomingSignal(zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);
	if(key == "Zebra__activeChanged"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterZebraActiveChanged>();
		if(m_zebraActiveChangedCb) m_zebraActiveChangedCb(paramReader.getVal());
	}
	else if(key == "empty"){
		zmq::message_t paramBuf;
		auto res2 = zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterEmpty>();
		if(m_emptyCb) m_emptyCb();
	}
	else if(key == "tick"){
		if(m_tickCb) m_tickCb();
	}
}

void EdgesClientSignalsThreaded::handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket,
                                          zmq::socket_t& zmqSubSocket)
{
    zmq::message_t keyBuf;
    auto res = zmqSubHelperSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    zmqSubSocket.set(zmq::sockopt::subscribe, keyBuf.to_string_view());
}

void EdgesClientSignals::onZebraActiveChanged(ZebraActiveChangedCb cb)
{
    if(m_zebraActiveChangedCb) return; // set it only once
    m_zebraActiveChangedCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "Zebra__activeChanged");
}

void EdgesClientSignalsThreaded::onZebraActiveChanged(ZebraActiveChangedCb cb)
{
    if(m_zebraActiveChangedCb) return; // set it only once
    m_zebraActiveChangedCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("Zebra__activeChanged", 20));
}

void EdgesClientSignals::onEmpty(EmptyCb cb)
{
    if(m_emptyCb) return; // set it only once
    m_emptyCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "empty");
}

void EdgesClientSignalsThreaded::onEmpty(EmptyCb cb)
{
    if(m_emptyCb) return; // set it only once
    m_emptyCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("empty", 5));
}

void EdgesClientSignals::onTick(TickCb cb)
{
    if(m_tickCb) return; // set it only once
    m_tickCb = cb;
    m_zmqSubSocket.set(zmq::sockopt::subscribe, "tick");
}

void EdgesClientSignalsThreaded::onTick(TickCb cb)
{
    if(m_tickCb) return; // set it only once
    m_tickCb = cb;
    m_zmqSubHelperSocket.send(zmq::const_buffer("tick", 4));
}

EdgesClient::EdgesClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr) :
    EdgesClientRpc(rZmqContext, serverRpcAddr),
    EdgesClientSignalsThreaded(rZmqContext, serverSignalAddr)
{
    StartThread();
}
