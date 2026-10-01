#ifndef CALCULATOR_QT_WEBCHANNEL_SCREEN_RPC_IF_H
#define CALCULATOR_QT_WEBCHANNEL_SCREEN_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "CalculatorQtWebchannel.capnp.h"
#include "capnp/message.h"

namespace capnzero::CalculatorQtWebchannel
{

class ScreenRpcIf
{
public:
    virtual ~ScreenRpcIf() = default;
	virtual void setBrightness(UInt32 brightness) = 0;
	virtual void setColor(EColor color) = 0;
	virtual EColor getColor() = 0;
	virtual void setTitle(const TextView& title) = 0;
	virtual void setId(const SpanCL<8>& title) = 0;
	struct ReturnAlltypes
	{
		Int32 a;
		Float32 b;
		Float64 c;
		Text d;
		Data<8> f;
		EColor color;
		EMode mode;
	};
	virtual ReturnAlltypes alltypes(Int32 a, Float32 b, Float64 c, const TextView& d, const Span& e, const SpanCL<8>& f, EColor color, EMode mode) = 0;
};

}
#endif
