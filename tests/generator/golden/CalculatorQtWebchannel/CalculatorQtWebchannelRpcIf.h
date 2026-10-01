#ifndef CALCULATOR_QT_WEBCHANNEL_RPC_IF_H
#define CALCULATOR_QT_WEBCHANNEL_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "CalculatorQtWebchannel.capnp.h"
#include "capnp/message.h"

namespace capnzero::CalculatorQtWebchannel
{

class RpcIf
{
public:
    virtual ~RpcIf() = default;
	virtual void setMode(EMode val) = 0;
};

}
#endif
