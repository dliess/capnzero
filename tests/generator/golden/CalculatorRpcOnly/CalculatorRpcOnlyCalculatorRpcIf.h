#ifndef CALCULATOR_RPC_ONLY_CALCULATOR_RPC_IF_H
#define CALCULATOR_RPC_ONLY_CALCULATOR_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "CalculatorRpcOnly.capnp.h"
#include "capnp/message.h"

namespace capnzero::CalculatorRpcOnly
{

class CalculatorRpcIf
{
public:
    virtual ~CalculatorRpcIf() = default;
	struct ReturnAdd
	{
		Int32 ret;
	};
	virtual ReturnAdd add(Int32 a, Int32 b) = 0;
	struct ReturnSub
	{
		Int32 ret;
	};
	virtual ReturnSub sub(Int32 a, Int32 b) = 0;
};

}
#endif
