#ifndef CAPNZERO_NATIVE_CAPNP_MSG_WRITER_H
#define CAPNZERO_NATIVE_CAPNP_MSG_WRITER_H

#include <capnp/message.h>
#include <zmq.hpp>
#include "capnzero_utils.h"

namespace capnzero{

class NativeCapnpMsgWriter
{
public:
    NativeCapnpMsgWriter(zmq::message_t& rRouterId, zmq::socket_t& rZmqRepSocket) :
        m_rRouterId(rRouterId),
        m_rZmqRepSocket(rZmqRepSocket)
    {}
    void write(::capnp::MessageBuilder& msgBuilder) 
    {
        m_rZmqRepSocket.send(m_rRouterId, zmq::send_flags::sndmore);
        sendOverZmq(msgBuilder, m_rZmqRepSocket, zmq::send_flags::none);
    }
private:
    zmq::message_t& m_rRouterId;
    zmq::socket_t& m_rZmqRepSocket;
};

} // namespace capnzero

#endif