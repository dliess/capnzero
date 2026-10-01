@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::TemperatureQtWithServiceName");

enum ServiceId {
	temperatureService @0;
}
enum TemperatureServiceRpcIds{
	aTestRpc @0;
	setCommandedTemperature @1;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPParameterTemperatureServiceATestRpc{
	title @0 :Text;
}
struct CAPNPParameterTemperatureServiceSetCommandedTemperature{
	val @0 :Float32;
}
struct CAPNPSignalParameterTemperatureServiceSensorNameChanged{
	val @0 :Text;
}
struct CAPNPSignalParameterTemperatureServiceActualTemperatureChanged{
	val @0 :Float32;
}
struct CAPNPSignalParameterTemperatureServiceCommandedTemperatureChanged{
	val @0 :Float32;
}
