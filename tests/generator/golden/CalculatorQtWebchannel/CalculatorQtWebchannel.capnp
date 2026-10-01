@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::CalculatorQtWebchannel");

enum ServiceId {
	main @0;
	calculator @1;
	screen @2;
}
enum EColor {
	blue @0;
	red @1;
	green @2;
	yellow @3;
}
enum EMode {
	dimmed @0;
	bright @1;
}
enum RpcIds{
	setMode @0;
}
enum CalculatorRpcIds{
	add @0;
	sub @1;
}
enum ScreenRpcIds{
	setBrightness @0;
	setColor @1;
	getColor @2;
	setTitle @3;
	setId @4;
	alltypes @5;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPParameterSetMode{
	val @0 :EMode;
}
struct CAPNPSignalParameterModeChanged{
	val @0 :EMode;
}
struct CAPNPParameterCalculatorAdd{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnCalculatorAdd {
	retParam @0 :Int32;
}
struct CAPNPParameterCalculatorSub{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnCalculatorSub {
	retParam @0 :Int32;
}
struct CAPNPParameterScreenSetBrightness{
	brightness @0 :UInt32;
}
struct CAPNPParameterScreenSetColor{
	color @0 :EColor;
}
struct CAPNPReturnScreenGetColor {
	retParam @0 :EColor;
}
struct CAPNPParameterScreenSetTitle{
	title @0 :Text;
}
struct CAPNPParameterScreenSetId{
	title @0 :Data;
}
struct CAPNPParameterScreenAlltypes{
	a @0 :Int32;
	b @1 :Float32;
	c @2 :Float64;
	d @3 :Text;
	e @4 :Data;
	f @5 :Data;
	color @6 :EColor;
	mode @7 :EMode;
}
struct CAPNPReturnScreenAlltypes {
	a @0 :Int32;
	b @1 :Float32;
	c @2 :Float64;
	d @3 :Text;
	f @4 :Data;
	color @5 :EColor;
	mode @6 :EMode;
}
struct CAPNPSignalParameterScreenColorChanged{
	color @0 :EColor;
}
struct CAPNPSignalParameterScreenBrightnessChanged{
	brightness @0 :UInt32;
}
struct CAPNPSignalParameterScreenAlltypess{
	a @0 :Int32;
	b @1 :Float32;
	c @2 :Float64;
	d @3 :Text;
	e @4 :Data;
	f @5 :Data;
	color @6 :EColor;
	mode @7 :EMode;
}
