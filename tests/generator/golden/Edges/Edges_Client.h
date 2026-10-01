#ifndef EDGES_CLIENT_H
#define EDGES_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "Edges.capnp.h"

#include "Edges_ClientTransport.h"
namespace capnzero::Edges
{

class EdgesClientRpc : public EdgesClientRpcTransport
{
public:
    using Super = EdgesClientRpcTransport;
    EdgesClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	struct ReturnZebraEmpty {
	};
	ReturnZebraEmpty
	Zebra__empty();

	Int32 Zebra__noArgs();

	Data<4> Zebra__bytes(const Span& dynamic, const Data<4>& fixed, const Text& text, Mode mode);

	void Zebra__setActive(Bool val);

	void Zebra__toggleActive();

	void fire();


};

} // namespace capnzero::Edges

#include <functional>
#include <capnp/message.h>

namespace capnzero::Edges
{

class EdgesClientSignals
{
public:
    EdgesClientSignals(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;

	using ZebraActiveChangedCb = std::function<void(Bool val)>;
	void onZebraActiveChanged(ZebraActiveChangedCb cb);
	using EmptyCb = std::function<void()>;
	void onEmpty(EmptyCb cb);
	using TickCb = std::function<void()>;
	void onTick(TickCb cb);

private:
    zmq::socket_t m_zmqSubSocket;
	ZebraActiveChangedCb m_zebraActiveChangedCb;
	EmptyCb m_emptyCb;
	TickCb m_tickCb;

};

} // namespace capnzero::Edges

#include <thread>
#include <functional>
#include <atomic>

namespace capnzero::Edges
{

class EdgesClientSignalsThreaded
{
public:
    EdgesClientSignalsThreaded(zmq::context_t& rZmqContext, std::string serverRpcAddr);
    ~EdgesClientSignalsThreaded();
    void StartThread();
	void signalReceiverThreadFn();
	using ZebraActiveChangedCb = std::function<void(Bool val)>;
	void onZebraActiveChanged(ZebraActiveChangedCb cb);
	using EmptyCb = std::function<void()>;
	void onEmpty(EmptyCb cb);
	using TickCb = std::function<void()>;
	void onTick(TickCb cb);

private:
    zmq::context_t& m_rZmqContext;
    std::string m_serverSignalAddr;
	std::atomic<bool> m_terminateRequest{false};
	zmq::socket_t m_zmqSubHelperSocket;
	std::unique_ptr<std::thread> m_pThread;

	void handleIncomingSignal(zmq::socket_t& zmqSubSocket);
	void handleSubscriptionRequest(zmq::socket_t& zmqSubHelperSocket, zmq::socket_t& zmqSubSocket);
	ZebraActiveChangedCb m_zebraActiveChangedCb;
	EmptyCb m_emptyCb;
	TickCb m_tickCb;

};

} // namespace capnzero::Edges


namespace capnzero::Edges
{

class EdgesClient : public EdgesClientRpc, public EdgesClientSignalsThreaded
{
public:
    EdgesClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr, const std::string& serverSignalAddr);
};



} // namespace capnzero::Edges
#include "Edges_Client.inl"
#endif
