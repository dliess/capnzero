#ifndef CAPNZERO_UTILS_H
#define CAPNZERO_UTILS_H

#include <type_traits>
#include <capnp/serialize.h>

namespace capnzero
{

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e)
{
   return static_cast<typename std::underlying_type<E>::type>(e);
}

template<typename MsgBuf>
kj::ArrayPtr<const capnp::word> asCapnpArr(MsgBuf&& msgBuf)
{
	return kj::ArrayPtr<const capnp::word>(
			reinterpret_cast<const capnp::word*>( msgBuf.data() ), 
			msgBuf.size() / sizeof(capnp::word));
}

template<typename T, typename MsgBuf>
typename T::Reader getReader(MsgBuf& msgBuf)
{
	::capnp::FlatArrayMessageReader msgReader(asCapnpArr(msgBuf));
	return msgReader.getRoot<T>();
}

inline
void sendOverZmq(::capnp::MessageBuilder& message,
                  zmq::socket_t& zmqSocket,
                  const zmq::send_flags& sendFlags){
    kj::Array<capnp::word> words = messageToFlatArray(message);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    zmqSocket.send(
        zmq::const_buffer(bytes.begin(), bytes.size()),
        sendFlags);
}

} //namespace capnzero
#endif