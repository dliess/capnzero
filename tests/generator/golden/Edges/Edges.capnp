@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::Edges");

enum ServiceId {
	zebra @0;
	main @1;
}
enum Mode {
	off @0;
	on @1;
}
enum ZebraRpcIds{
	empty @0;
	noArgs @1;
	bytes @2;
	setActive @3;
	toggleActive @4;
}
enum RpcIds{
	fire @0;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPParameterZebraEmpty{
}
struct CAPNPReturnZebraEmpty {
}
struct CAPNPReturnZebraNoArgs {
	retParam @0 :Int32;
}
struct CAPNPParameterZebraBytes{
	dynamic @0 :Data;
	fixed @1 :Data;
	text @2 :Text;
	mode @3 :Mode;
}
struct CAPNPReturnZebraBytes {
	retParam @0 :Data;
}
struct CAPNPParameterZebraSetActive{
	val @0 :Bool;
}
struct CAPNPSignalParameterZebraActiveChanged{
	val @0 :Bool;
}
struct CAPNPSignalParameterEmpty{
}
