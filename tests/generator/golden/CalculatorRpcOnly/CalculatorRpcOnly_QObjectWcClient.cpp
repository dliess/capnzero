#include "CalculatorRpcOnly_QObjectClient.h"
#include "CalculatorRpcOnly.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorRpcOnly;


QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    CalculatorRpcOnlyClientRpcTransport(rZmqContext, rpcAddr)
{
}    
        
ReturnCalculatorAdd
QClientRpc::calculator__add(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	ReturnCalculatorAdd retVal;

    _Calculator__add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorAdd>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

ReturnCalculatorSub
QClientRpc::calculator__sub(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	ReturnCalculatorSub retVal;

    _Calculator__sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorSub>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

void QClientRpc::screen__setBrightness(quint32 brightness)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetBrightness>();
	paramBuilder.setBrightness(brightness);


    _Screen__setBrightness(
		message

    );

}

void QClientRpc::screen__setTitle(const QString& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetTitle>();
	paramBuilder.setTitle(title.toUtf8().constData());


    _Screen__setTitle(
		message

    );

}

void QClientRpc::screen__setId(const QByteArray& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetId>();
	auto title_converted = QByteArray::fromBase64(title, QByteArray::Base64Encoding);
	paramBuilder.setTitle(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(title_converted.data()), title_converted.size()));


    _Screen__setId(
		message

    );

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


ReturnCalculatorAdd
QClient::calculator__add(qint32 a, qint32 b)
{
	return m_qclientRpc.calculator__add(a, b);
}

ReturnCalculatorSub
QClient::calculator__sub(qint32 a, qint32 b)
{
	return m_qclientRpc.calculator__sub(a, b);
}

void QClient::screen__setBrightness(quint32 brightness)
{
	m_qclientRpc.screen__setBrightness(brightness);
}

void QClient::screen__setTitle(const QString& title)
{
	m_qclientRpc.screen__setTitle(title);
}

void QClient::screen__setId(const QByteArray& title)
{
	m_qclientRpc.screen__setId(title);
}



void QClient::registerMetaTypes()
{
	qRegisterMetaType<ReturnCalculatorAdd>();
	qRegisterMetaType<ReturnCalculatorSub>();
}


