#ifndef CALCULATOR_RPC_IF_H
#define CALCULATOR_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "Calculator.capnp.h"
#include "capnp/message.h"

namespace capnzero::Calculator
{

class RpcIf
{
public:
    virtual ~RpcIf() = default;
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
	virtual Int32 addDirectRet(Int32 a, Int32 b) = 0;
	virtual Int32 subDirectRet(Int32 a, Int32 b) = 0;
	virtual void multiply(::capnp::MessageReader& recvData) = 0;
	struct ReturnMultiply2
	{
		Int32 ret;
	};
	virtual ReturnMultiply2 multiply2(::capnp::MessageReader& recvData) = 0;
	virtual void multiply3(Int32 a, Int32 b, NativeCapnpMsgWriter &writer) = 0;
	virtual void multiply4(::capnp::MessageReader& recvData, NativeCapnpMsgWriter &writer) = 0;
};

}
#endif
