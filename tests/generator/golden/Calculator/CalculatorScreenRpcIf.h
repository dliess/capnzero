#ifndef CALCULATOR_SCREEN_RPC_IF_H
#define CALCULATOR_SCREEN_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "Calculator.capnp.h"
#include "capnp/message.h"

namespace capnzero::Calculator
{

class ScreenRpcIf
{
public:
    virtual ~ScreenRpcIf() = default;
	virtual void setBrightness(UInt32 brightness) = 0;
	virtual void setColor(EColor color) = 0;
	virtual void setTitle(const TextView& title) = 0;
	virtual void setId(const SpanCL<8>& title) = 0;
};

}
#endif
