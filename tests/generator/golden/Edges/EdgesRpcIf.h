#ifndef EDGES_RPC_IF_H
#define EDGES_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "Edges.capnp.h"
#include "capnp/message.h"

namespace capnzero::Edges
{

class RpcIf
{
public:
    virtual ~RpcIf() = default;
	virtual void fire() = 0;
};

}
#endif
