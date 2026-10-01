#ifndef CALCULATOR_QT_WEBCHANNEL_CALCULATOR_RPC_IF_H
#define CALCULATOR_QT_WEBCHANNEL_CALCULATOR_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "CalculatorQtWebchannel.capnp.h"
#include "capnp/message.h"

namespace capnzero::CalculatorQtWebchannel
{

class CalculatorRpcIf
{
public:
    virtual ~CalculatorRpcIf() = default;
	virtual Int32 add(Int32 a, Int32 b) = 0;
	virtual Int32 sub(Int32 a, Int32 b) = 0;
};

}
#endif
