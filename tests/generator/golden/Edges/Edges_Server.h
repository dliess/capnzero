#ifndef EDGES_SERVER_H
#define EDGES_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "Edges.capnp.h"

#include "EdgesZebraRpcIf.h"
#include "EdgesRpcIf.h"


namespace capnzero::Edges
{

class EdgesServer
{
public:
	EdgesServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<ZebraRpcIf> zebra, std::unique_ptr<RpcIf> _);
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
		void Zebra__activeChanged(Bool val);
		void empty();
		void tick();

        using SubscriptionCb = std::function<void(Signals&)>;
			void registerZebraActiveChangedSubscrCb(SubscriptionCb cb);
			void registerEmptySubscrCb(SubscriptionCb cb);
			void registerTickSubscrCb(SubscriptionCb cb);

    private:
        zmq::socket_t m_rZmqPubSocket;
        std::unordered_map<std::string, SubscriptionCb> m_subscrCbMap;
    };
    Signals &signals() { return m_signals; }

private:
	std::unique_ptr<ZebraRpcIf> m_pZebraRpcIf;
	std::unique_ptr<RpcIf> m_p_RpcIf;
	zmq::socket_t m_zmqRepSocket;

	Signals m_signals;

};

} // namespace capnzero::Edges
#endif
