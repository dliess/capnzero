#include "TypeSpelling_Client.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <string_view>
#include "TypeSpelling.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TypeSpelling;
TypeSpellingClientRpc::TypeSpellingClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr):
    Super(rZmqContext, serverRpcAddr)
{
}
Data<04> TypeSpellingClientRpc::bytes(const Data<04>& value)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterBytes>();
	paramBuilder.setValue(capnp::Data::Reader(value.data(), value.size()));

	Data<04> retVal;

    _bytes(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnBytes>();
			auto src = reader.getRetParam();
			assert(src.size() == retVal.size());
			std::copy(src.begin(), src.end(), retVal.begin());

	}
    );
	return retVal;
}


TypeSpellingClient::TypeSpellingClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr) :
    TypeSpellingClientRpc(rZmqContext, serverRpcAddr)
{
}
