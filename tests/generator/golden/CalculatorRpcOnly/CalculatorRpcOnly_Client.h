#ifndef CALCULATORRPCONLY_CLIENT_H
#define CALCULATORRPCONLY_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "CalculatorRpcOnly.capnp.h"

#include "CalculatorRpcOnly_ClientTransport.h"
namespace capnzero::CalculatorRpcOnly
{

class CalculatorRpcOnlyClientRpc : public CalculatorRpcOnlyClientRpcTransport
{
public:
    using Super = CalculatorRpcOnlyClientRpcTransport;
    CalculatorRpcOnlyClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	struct ReturnCalculatorAdd {
		Int32 ret;
	};
	ReturnCalculatorAdd
	Calculator__add(Int32 a, Int32 b);

	struct ReturnCalculatorSub {
		Int32 ret;
	};
	ReturnCalculatorSub
	Calculator__sub(Int32 a, Int32 b);

	void Screen__setBrightness(UInt32 brightness);

	void Screen__setTitle(const Text& title);

	void Screen__setId(const Data<8>& title);


};

} // namespace capnzero::CalculatorRpcOnly




namespace capnzero::CalculatorRpcOnly
{

class CalculatorRpcOnlyClient : public CalculatorRpcOnlyClientRpc
{
public:
    CalculatorRpcOnlyClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
};



} // namespace capnzero::CalculatorRpcOnly
#include "CalculatorRpcOnly_Client.inl"
#endif
