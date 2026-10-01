#include "TypeSpelling_QObjectClient.h"
#include "TypeSpelling.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TypeSpelling;


QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    TypeSpellingClientRpcTransport(rZmqContext, rpcAddr)
{
}    
        
QByteArray QClientRpc::bytes(const QByteArray& value)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterBytes>();
	auto value_converted = QByteArray::fromBase64(value, QByteArray::Base64Encoding);
	paramBuilder.setValue(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(value_converted.data()), value_converted.size()));

	QByteArray retVal;

    _bytes(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnBytes>();
			auto src = reader.getRetParam();
			assert(src.size() == retVal.size());
			std::copy(src.begin(), src.end(), retVal.begin());
			retVal = retVal.toBase64();

	}
    );
	return retVal;
}



QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(signalAddr);

    QSocketNotifier* socketNotifier = new QSocketNotifier(getFd(), QSocketNotifier::Read, this);
    QObject::connect(socketNotifier, &QSocketNotifier::activated, [this](int fd){
        handleIncomingSignalAllNonBlock();
    });
}

void QClientSignals::handleIncomingSignal()
{
    zmq::message_t keyBuf;
    auto res = m_zmqSubSocket.recv(keyBuf);
    if (!res) { throw std::runtime_error("No received msg"); }
    std::string_view key(static_cast<const char*>(keyBuf.data()), *res);

}

void QClientSignals::handleIncomingSignalAllNonBlock()
{
    while(m_zmqSubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        handleIncomingSignal();
    }
}

int QClientSignals::getFd() const
{
    return m_zmqSubSocket.get(zmq::sockopt::fd);
}

QClient::QClient(zmq::context_t& rZmqContext,
                 const std::string& rpcAddr, 
                 QObject* pParent) :
    QObject(pParent),
    m_qclientRpc(rZmqContext, rpcAddr)
{{
    registerMetaTypes();
}}


QByteArray QClient::bytes(const QByteArray& value)
{
	return m_qclientRpc.bytes(value);
}



void QClient::registerMetaTypes()
{

}


