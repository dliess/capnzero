#ifndef TYPESPELLING_CLIENT_H
#define TYPESPELLING_CLIENT_H

#include <zmq.hpp>
#include "capnzero_typedefs.h"
#include "TypeSpelling.capnp.h"

#include "TypeSpelling_ClientTransport.h"
namespace capnzero::TypeSpelling
{

class TypeSpellingClientRpc : public TypeSpellingClientRpcTransport
{
public:
    using Super = TypeSpellingClientRpcTransport;
    TypeSpellingClientRpc(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
	Data<04>
	bytes(const Data<04>& value);


};

} // namespace capnzero::TypeSpelling




namespace capnzero::TypeSpelling
{

class TypeSpellingClient : public TypeSpellingClientRpc
{
public:
    TypeSpellingClient(zmq::context_t& rZmqContext, const std::string& serverRpcAddr);
};



} // namespace capnzero::TypeSpelling
#include "TypeSpelling_Client.inl"
#endif
