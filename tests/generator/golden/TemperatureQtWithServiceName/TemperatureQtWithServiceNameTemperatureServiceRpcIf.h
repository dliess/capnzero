#ifndef TEMPERATURE_QT_WITH_SERVICE_NAME_TEMPERATURE_SERVICE_RPC_IF_H
#define TEMPERATURE_QT_WITH_SERVICE_NAME_TEMPERATURE_SERVICE_RPC_IF_H

#include "capnzero_typedefs.h"
#include "capnzero_NativeCapnpMsgWriter.h"
#include "TemperatureQtWithServiceName.capnp.h"
#include "capnp/message.h"

namespace capnzero::TemperatureQtWithServiceName
{

class TemperatureServiceRpcIf
{
public:
    virtual ~TemperatureServiceRpcIf() = default;
	virtual void aTestRpc(const TextView& title) = 0;
	virtual void setCommandedTemperature(Float32 val) = 0;
};

}
#endif
