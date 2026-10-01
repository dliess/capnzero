@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::Calculator");

struct OneIntParam{
	a @0 :Int32;
}

struct TwoIntParams{
	a @0 :Int32;
	b @1 :Int32;
}
enum ServiceId {
	main @0;
	directRet @1;
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
	add @0;
	sub @1;
	addDirectRet @2;
	subDirectRet @3;
	multiply @4;
	multiply2 @5;
	multiply3 @6;
	multiply4 @7;
}
enum DirectRetRpcIds{
	add @0;
	sub @1;
}
enum ScreenRpcIds{
	setBrightness @0;
	setColor @1;
	setTitle @2;
	setId @3;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPParameterAdd{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnAdd {
	ret @0 :Int32;
}
struct CAPNPParameterSub{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnSub {
	ret @0 :Int32;
}
struct CAPNPParameterAddDirectRet{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnAddDirectRet {
	retParam @0 :Int32;
}
struct CAPNPParameterSubDirectRet{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnSubDirectRet {
	retParam @0 :Int32;
}
struct CAPNPReturnMultiply2 {
	ret @0 :Int32;
}
struct CAPNPParameterMultiply3{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPParameterDirectRetAdd{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnDirectRetAdd {
	retParam @0 :Int32;
}
struct CAPNPParameterDirectRetSub{
	a @0 :Int32;
	b @1 :Int32;
}
struct CAPNPReturnDirectRetSub {
	retParam @0 :Int32;
}
struct CAPNPParameterScreenSetBrightness{
	brightness @0 :UInt32;
}
struct CAPNPParameterScreenSetColor{
	color @0 :EColor;
}
struct CAPNPParameterScreenSetTitle{
	title @0 :Text;
}
struct CAPNPParameterScreenSetId{
	title @0 :Data;
}
struct CAPNPSignalParameterScreenColorChanged{
	color @0 :EColor;
}
struct CAPNPSignalParameterScreenBrightnessChanged{
	brightness @0 :UInt32;
}
