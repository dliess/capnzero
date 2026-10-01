@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::CalculatorSignalsOnly");

enum ServiceId {
	screen @0;
}
enum EColor {
	blue @0;
	red @1;
	green @2;
	yellow @3;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPSignalParameterScreenBrightnessChanged{
	brightness @0 :UInt32;
}
struct CAPNPSignalParameterScreenColorChanged{
	color @0 :EColor;
}
