#ifndef EDGES_ZEBRA_RPC_IF_H
#define EDGES_ZEBRA_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "Edges.capnp.h"
#include "capnp/message.h"

namespace capnzero::Edges
{

class ZebraRpcIf
{
public:
    virtual ~ZebraRpcIf() = default;
	struct ReturnEmpty
	{
	};
	virtual ReturnEmpty empty() = 0;
	virtual Int32 noArgs() = 0;
	virtual Data<4> bytes(const Span& dynamic, const SpanCL<4>& fixed, const TextView& text, Mode mode) = 0;
	virtual void setActive(Bool val) = 0;
	virtual void toggleActive() = 0;
};

}
#endif
