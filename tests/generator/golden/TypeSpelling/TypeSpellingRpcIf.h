#ifndef TYPE_SPELLING_RPC_IF_H
#define TYPE_SPELLING_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "TypeSpelling.capnp.h"
#include "capnp/message.h"

namespace capnzero::TypeSpelling
{

class RpcIf
{
public:
    virtual ~RpcIf() = default;
	virtual Data<04> bytes(const SpanCL<04>& value) = 0;
};

}
#endif
