@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::TemperatureQt");

enum ServiceId {
	main @0;
}
enum RpcIds{
	explicitSetUUID @0;
	explicitGetUUID @1;
	setCommandedTemperature @2;
	setEnabled @3;
	toggleEnabled @4;
	setUuid @5;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPParameterExplicitSetUUID{
	uuid @0 :Data;
}
struct CAPNPReturnExplicitGetUUID {
	retParam @0 :Data;
}
struct CAPNPParameterSetCommandedTemperature{
	val @0 :Float32;
}
struct CAPNPParameterSetEnabled{
	val @0 :Bool;
}
struct CAPNPParameterSetUuid{
	val @0 :Data;
}
struct CAPNPSignalParameterExplicitSignalUUID{
	uuid @0 :Data;
}
struct CAPNPSignalParameterSensorNameChanged{
	val @0 :Text;
}
struct CAPNPSignalParameterActualTemperatureChanged{
	val @0 :Float32;
}
struct CAPNPSignalParameterCommandedTemperatureChanged{
	val @0 :Float32;
}
struct CAPNPSignalParameterEnabledChanged{
	val @0 :Bool;
}
struct CAPNPSignalParameterUuidChanged{
	val @0 :Data;
}
