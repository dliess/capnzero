#include "CalculatorRpcOnly_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "CalculatorRpcOnly.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorRpcOnly;
CalculatorRpcOnlyClientRpc::CalculatorRpcOnlyClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    Super(rZmqContext, serverRpcAddr)
{
}
CalculatorRpcOnlyClientRpc::ReturnCalculatorAdd
CalculatorRpcOnlyClientRpc::Calculator__add(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	ReturnCalculatorAdd retVal;

    _Calculator__add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorAdd>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

CalculatorRpcOnlyClientRpc::ReturnCalculatorSub
CalculatorRpcOnlyClientRpc::Calculator__sub(Int32 a, Int32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	ReturnCalculatorSub retVal;

    _Calculator__sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorSub>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

void CalculatorRpcOnlyClientRpc::Screen__setBrightness(UInt32 brightness)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetBrightness>();
	paramBuilder.setBrightness(brightness);


    _Screen__setBrightness(
		message

    );

}

void CalculatorRpcOnlyClientRpc::Screen__setTitle(const Text& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetTitle>();
	paramBuilder.setTitle(title);


    _Screen__setTitle(
		message

    );

}

void CalculatorRpcOnlyClientRpc::Screen__setId(const Data<8>& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetId>();
	paramBuilder.setTitle(capnp::Data::Reader(title.data(), title.size()));


    _Screen__setId(
		message

    );

}


CalculatorRpcOnlyClient::CalculatorRpcOnlyClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    CalculatorRpcOnlyClientRpc(rZmqContext, serverRpcAddr)
{
}
