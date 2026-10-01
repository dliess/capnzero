@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::TemperatureQtSignalsOnly");

enum ServiceId {
	main @0;
}
enum ETempRange {
	cold @0;
	hot @1;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPSignalParameterTempRangeChanged2{
	tempRange @0 :ETempRange;
}
struct CAPNPSignalParameterSensorNameChanged{
	val @0 :Text;
}
struct CAPNPSignalParameterActualTemperatureChanged{
	val @0 :Float32;
}
struct CAPNPSignalParameterTempRangeChanged{
	val @0 :ETempRange;
}
