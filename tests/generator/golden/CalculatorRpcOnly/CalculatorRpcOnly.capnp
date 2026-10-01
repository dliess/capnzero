@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::CalculatorRpcOnly");

enum ServiceId {
	calculator @0;
	screen @1;
}
enum CalculatorRpcIds{
	add @0;
	sub @1;
}
enum ScreenRpcIds{
	setBrightness @0;
	setTitle @1;
	setId @2;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPParameterCalculatorAdd{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnCalculatorAdd {
	ret @0 :Int32;
}
struct CAPNPParameterCalculatorSub{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnCalculatorSub {
	ret @0 :Int32;
}
struct CAPNPParameterScreenSetBrightness{
	brightness @0 :UInt32;
}
struct CAPNPParameterScreenSetTitle{
	title @0 :Text;
}
struct CAPNPParameterScreenSetId{
	title @0 :Data;
}
