#ifndef CALCULATOR_DIRECT_RET_RPC_IF_H
#define CALCULATOR_DIRECT_RET_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "Calculator.capnp.h"
#include "capnp/message.h"

namespace capnzero::Calculator
{

class DirectRetRpcIf
{
public:
    virtual ~DirectRetRpcIf() = default;
	virtual Int32 add(Int32 a, Int32 b) = 0;
	virtual Int32 sub(Int32 a, Int32 b) = 0;
};

}
#endif
