#ifndef TEMPERATURE_QT_RPC_IF_H
#define TEMPERATURE_QT_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "TemperatureQt.capnp.h"
#include "capnp/message.h"

namespace capnzero::TemperatureQt
{

class RpcIf
{
public:
    virtual ~RpcIf() = default;
	virtual void explicitSetUUID(const SpanCL<16>& uuid) = 0;
	virtual Data<16> explicitGetUUID() = 0;
	virtual void setCommandedTemperature(Float32 val) = 0;
	virtual void setEnabled(Bool val) = 0;
	virtual void toggleEnabled() = 0;
	virtual void setUuid(const SpanCL<16>& val) = 0;
};

}
#endif
