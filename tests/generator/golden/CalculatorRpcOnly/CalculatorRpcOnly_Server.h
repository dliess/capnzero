#ifndef CALCULATORRPCONLY_SERVER_H
#define CALCULATORRPCONLY_SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>
#include <functional>
#include <unordered_map>
#include "capnzero_typedefs.h"
#include "CalculatorRpcOnly.capnp.h"

#include "CalculatorRpcOnlyCalculatorRpcIf.h"
#include "CalculatorRpcOnlyScreenRpcIf.h"


namespace capnzero::CalculatorRpcOnly
{

class CalculatorRpcOnlyServer
{
public:
	CalculatorRpcOnlyServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr , std::unique_ptr<CalculatorRpcIf> calculator, std::unique_ptr<ScreenRpcIf> screen);
	void unbind(const std::string& rpcBindAddr);
    enum class WaitMode {
        Blocking,
        NonBlocking
    };
    void processNextRequest(WaitMode waitMode = WaitMode::Blocking);
    void processNextRequestAllNonBlock();
  	int getFd() const;


private:
	std::unique_ptr<CalculatorRpcIf> m_pCalculatorRpcIf;
	std::unique_ptr<ScreenRpcIf> m_pScreenRpcIf;
	zmq::socket_t m_zmqRepSocket;


};

} // namespace capnzero::CalculatorRpcOnly
#endif
