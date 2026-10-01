
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "capnzero_utils.h"
namespace capnzero::Calculator
{

// public section rpc definitions

template <typename Callable>
void CalculatorClientRpc::multiply3(Int32 a, Int32 b, Callable&& cb)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterMultiply3>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);


    _multiply3(
		message, 
		cb
    );

}

template <typename Callable>
void CalculatorClientRpc::multiply4(::capnp::MessageBuilder& sndData, Callable&& cb)
{


    _multiply4(
		sndData, 
		cb
    );

}



} // namespace capnzero::Calculator
