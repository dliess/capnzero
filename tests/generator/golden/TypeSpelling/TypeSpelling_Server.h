#ifndef TYPESPELLING_SERVER_H
#define TYPESPELLING_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "TypeSpelling.capnp.h"

#include "TypeSpellingRpcIf.h"


namespace capnzero::TypeSpelling
{

class TypeSpellingServer
{
public:
	TypeSpellingServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr , std::unique_ptr<RpcIf> _);
	void unbind(const std::string& rpcBindAddr);
    enum class WaitMode {
        Blocking,
        NonBlocking
    };
    void processNextRequest(WaitMode waitMode = WaitMode::Blocking);
    void processNextRequestAllNonBlock();
  	int getFd() const;


private:
	std::unique_ptr<RpcIf> m_p_RpcIf;
	zmq::socket_t m_zmqRepSocket;


};

} // namespace capnzero::TypeSpelling
#endif
