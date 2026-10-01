@0xdeadbeefdeadbeef;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("capnzero::TypeSpelling");

enum ServiceId {
	main @0;
}
enum RpcIds{
	bytes @0;
}
struct RpcCoord {
	serviceId @0 :UInt16;
	rpcId @1 :UInt16;
}
struct CAPNPParameterBytes{
	value @0 :Data;
}
struct CAPNPReturnBytes {
	retParam @0 :Data;
}
