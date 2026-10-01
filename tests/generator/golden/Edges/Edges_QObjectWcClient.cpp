#include "Edges_QObjectClient.h"
#include "Edges.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::Edges;


QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    EdgesClientRpcTransport(rZmqContext, rpcAddr)
{
}    
        
ReturnZebraEmpty
QClientRpc::zebra__empty()
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterZebraEmpty>();

	ReturnZebraEmpty retVal;

    _Zebra__empty(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnZebraEmpty>();

	}
    );
	return retVal;
}

qint32 QClientRpc::zebra__noArgs()
{

	qint32 retVal;

    _Zebra__noArgs(

		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnZebraNoArgs>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

QByteArray QClientRpc::zebra__bytes(const QByteArray& dynamic, const QByteArray& fixed, const QString& text, Mode mode)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterZebraBytes>();
	auto dynamic_converted = QByteArray::fromBase64(dynamic, QByteArray::Base64Encoding);
	paramBuilder.setDynamic(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(dynamic_converted.data()), dynamic_converted.size()));
	auto fixed_converted = QByteArray::fromBase64(fixed, QByteArray::Base64Encoding);
	paramBuilder.setFixed(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(fixed_converted.data()), fixed_converted.size()));
	paramBuilder.setText(text.toUtf8().constData());
	paramBuilder.setMode(mode);

	QByteArray retVal;

    _Zebra__bytes(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnZebraBytes>();
			auto src = reader.getRetParam();
			assert(src.size() == retVal.size());
			std::copy(src.begin(), src.end(), retVal.begin());
			retVal = retVal.toBase64();

	}
    );
	return retVal;
}

void QClientRpc::zebra__setActive(bool val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterZebraSetActive>();
	paramBuilder.setVal(val);


    _Zebra__setActive(
		message

    );

}

void QClientRpc::zebra__toggleActive()
{


    _Zebra__toggleActive(


    );

}

void QClientRpc::fire()
{


    _fire(


    );

}



QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(signalAddr);
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "Zebra__activeChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "empty");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "tick");
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
	if(key == "Zebra__activeChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterZebraActiveChanged>();
		emit zebra__activeChanged(paramReader.getVal());
	}
	else if(key == "empty"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterEmpty>();
		emit empty();
	}
	else if(key == "tick"){
		emit tick();
	}

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
                 const std::string& signalAddr, 
                 QObject* pParent) :
    QObject(pParent),
    m_qclientRpc(rZmqContext, rpcAddr),
    m_qclientSignals(rZmqContext, signalAddr)
{
    registerMetaTypes();
	QObject::connect(&m_qclientSignals, &QClientSignals::zebra__activeChanged, [this](bool val){
		if(m_zebra__active != val){
			m_zebra__active = val;
			emit zebra__activeChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::empty, [this](){
			emit empty();
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::tick, [this](){
			emit tick();
	});

}


ReturnZebraEmpty
QClient::zebra__empty()
{
	return m_qclientRpc.zebra__empty();
}

qint32 QClient::zebra__noArgs()
{
	return m_qclientRpc.zebra__noArgs();
}

QByteArray QClient::zebra__bytes(const QByteArray& dynamic, const QByteArray& fixed, const QString& text, Mode mode)
{
	return m_qclientRpc.zebra__bytes(dynamic, fixed, text, static_cast<capnzero::Edges::Mode>(mode));
}

void QClient::zebra__setActive(bool val)
{
	m_qclientRpc.zebra__setActive(val);
}

void QClient::zebra__toggleActive()
{
	m_qclientRpc.zebra__toggleActive();
}

void QClient::fire()
{
	m_qclientRpc.fire();
}

bool QClient::zebra__active() const
{
	return m_zebra__active;
}



void QClient::registerMetaTypes()
{
	qRegisterMetaType<ReturnZebraEmpty>();
	qRegisterMetaType<Mode>("Mode");
}


