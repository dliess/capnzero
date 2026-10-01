#include "Calculator_QObjectClient.h"
#include "Calculator.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::Calculator;


QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    CalculatorClientRpcTransport(rZmqContext, rpcAddr)
{
}    
        
Return_Add QClientRpc::add(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Return_Add retVal;

    _add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnAdd>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

Return_Sub QClientRpc::sub(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	Return_Sub retVal;

    _sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnSub>();
			retVal.ret = reader.getRet();

	}
    );
	return retVal;
}

qint32 QClientRpc::addDirectRet(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterAddDirectRet>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	qint32 retVal;

    _addDirectRet(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnAddDirectRet>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

qint32 QClientRpc::subDirectRet(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSubDirectRet>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	qint32 retVal;

    _subDirectRet(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnSubDirectRet>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

qint32 QClientRpc::directRet__add(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterDirectRetAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	qint32 retVal;

    _DirectRet__add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnDirectRetAdd>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

qint32 QClientRpc::directRet__sub(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterDirectRetSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	qint32 retVal;

    _DirectRet__sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnDirectRetSub>();
			retVal = reader.getRetParam();

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

void QClientRpc::screen__setColor(EColor color)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenSetColor>();
	paramBuilder.setColor(color);


    _Screen__setColor(
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
	paramBuilder.setTitle(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(title.data()), title.size()));


    _Screen__setId(
		message

    );

}



QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(signalAddr);
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "justASignal");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__listChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__colorChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__brightnessChanged");
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
	if(key == "justASignal"){
		emit justASignal();
	}
	else if(key == "Screen__listChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));
		emit screen__listChanged(reader);
	}
	else if(key == "Screen__colorChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenColorChanged>();
		emit screen__colorChanged(paramReader.getColor());
	}
	else if(key == "Screen__brightnessChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenBrightnessChanged>();
		emit screen__brightnessChanged(paramReader.getBrightness());
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
	QObject::connect(&m_qclientSignals, &QClientSignals::justASignal, [this](){
			emit justASignal();
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::screen__listChanged, [this](::capnp::MessageBuilder& sndData){
			emit screen__listChanged(sndData);
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::screen__colorChanged, [this](capnzero::Calculator::EColor color){
			emit screen__colorChanged(static_cast<QClient::EColor>(color));
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::screen__brightnessChanged, [this](quint32 brightness){
			emit screen__brightnessChanged(brightness);
	});

}


Return_Add QClient::add(qint32 a, qint32 b)
{
	return m_qclientRpc.add(a, b);
}

Return_Sub QClient::sub(qint32 a, qint32 b)
{
	return m_qclientRpc.sub(a, b);
}

qint32 QClient::addDirectRet(qint32 a, qint32 b)
{
	return m_qclientRpc.addDirectRet(a, b);
}

qint32 QClient::subDirectRet(qint32 a, qint32 b)
{
	return m_qclientRpc.subDirectRet(a, b);
}

qint32 QClient::directRet__add(qint32 a, qint32 b)
{
	return m_qclientRpc.directRet__add(a, b);
}

qint32 QClient::directRet__sub(qint32 a, qint32 b)
{
	return m_qclientRpc.directRet__sub(a, b);
}

void QClient::screen__setBrightness(quint32 brightness)
{
	m_qclientRpc.screen__setBrightness(brightness);
}

void QClient::screen__setColor(EColor color)
{
	m_qclientRpc.screen__setColor(static_cast<capnzero::Calculator::EColor>(color));
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
	qRegisterMetaType<Return_Add>();
	qRegisterMetaType<Return_Sub>();
	qRegisterMetaType<Return_Multiply2>();
	qRegisterMetaType<EColor>("EColor");
	qRegisterMetaType<EMode>("EMode");
}


