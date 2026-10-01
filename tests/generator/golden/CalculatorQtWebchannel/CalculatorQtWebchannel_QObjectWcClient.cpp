#include "CalculatorQtWebchannel_QObjectClient.h"
#include "CalculatorQtWebchannel.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorQtWebchannel;


QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    CalculatorQtWebchannelClientRpcTransport(rZmqContext, rpcAddr)
{
}    
        
void QClientRpc::setMode(EMode val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetMode>();
	paramBuilder.setVal(val);


    _setMode(
		message

    );

}

qint32 QClientRpc::calculator__add(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorAdd>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	qint32 retVal;

    _Calculator__add(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorAdd>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
}

qint32 QClientRpc::calculator__sub(qint32 a, qint32 b)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterCalculatorSub>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);

	qint32 retVal;

    _Calculator__sub(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnCalculatorSub>();
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

EColor QClientRpc::screen__getColor()
{

	EColor retVal;

    _Screen__getColor(

		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnScreenGetColor>();
			retVal = reader.getRetParam();

	}
    );
	return retVal;
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

ReturnScreenAlltypes
QClientRpc::screen__alltypes(qint32 a, qreal b, qreal c, const QString& d, const QByteArray& e, const QByteArray& f, EColor color, EMode mode)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterScreenAlltypes>();
	paramBuilder.setA(a);
	paramBuilder.setB(b);
	paramBuilder.setC(c);
	paramBuilder.setD(d.toUtf8().constData());
	auto e_converted = QByteArray::fromBase64(e, QByteArray::Base64Encoding);
	paramBuilder.setE(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(e_converted.data()), e_converted.size()));
	auto f_converted = QByteArray::fromBase64(f, QByteArray::Base64Encoding);
	paramBuilder.setF(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(f_converted.data()), f_converted.size()));
	paramBuilder.setColor(color);
	paramBuilder.setMode(mode);

	ReturnScreenAlltypes retVal;

    _Screen__alltypes(
		message, 
		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnScreenAlltypes>();
			retVal.a = reader.getA();
			retVal.b = reader.getB();
			retVal.c = reader.getC();
			retVal.d = reader.getD().cStr();
			auto src = reader.getF();
			assert(src.size() == retVal.f.size());
			std::copy(src.begin(), src.end(), retVal.f.begin());
			retVal.color = reader.getColor();
			retVal.mode = reader.getMode();

	}
    );
	return retVal;
}



QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(signalAddr);
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "modeChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__colorChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__brightnessChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "Screen__alltypess");
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
	if(key == "modeChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterModeChanged>();
		emit modeChanged(paramReader.getVal());
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
	else if(key == "Screen__alltypess"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterScreenAlltypess>();
		emit screen__alltypess(paramReader.getA(), paramReader.getB(), paramReader.getC(), QByteArray(reinterpret_cast<const char *>(paramReader.getD().begin()), int(paramReader.getD().size())), QByteArray(reinterpret_cast<const char *>(paramReader.getE().begin()), int(paramReader.getE().size())).toBase64(), QByteArray(reinterpret_cast<const char *>(paramReader.getF().begin()), int(paramReader.getF().size())).toBase64(), paramReader.getColor(), paramReader.getMode());
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
	QObject::connect(&m_qclientSignals, &QClientSignals::modeChanged, [this](capnzero::CalculatorQtWebchannel::EMode val){
		if(m_mode != static_cast<decltype(m_mode)>(val)){
			m_mode = static_cast<decltype(m_mode)>(val);
			emit modeChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::screen__colorChanged, [this](capnzero::CalculatorQtWebchannel::EColor color){
			emit screen__colorChanged(static_cast<QClient::EColor>(color));
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::screen__brightnessChanged, [this](quint32 brightness){
			emit screen__brightnessChanged(brightness);
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::screen__alltypess, [this](qint32 a,qreal b,qreal c,const QString& d,const QByteArray& e,const QByteArray& f,capnzero::CalculatorQtWebchannel::EColor color,capnzero::CalculatorQtWebchannel::EMode mode){
			emit screen__alltypess(a,b,c,d,e,f,static_cast<QClient::EColor>(color),static_cast<QClient::EMode>(mode));
	});

}


void QClient::setMode(EMode val)
{
	m_qclientRpc.setMode(static_cast<capnzero::CalculatorQtWebchannel::EMode>(val));
}

qint32 QClient::calculator__add(qint32 a, qint32 b)
{
	return m_qclientRpc.calculator__add(a, b);
}

qint32 QClient::calculator__sub(qint32 a, qint32 b)
{
	return m_qclientRpc.calculator__sub(a, b);
}

void QClient::screen__setBrightness(quint32 brightness)
{
	m_qclientRpc.screen__setBrightness(brightness);
}

void QClient::screen__setColor(EColor color)
{
	m_qclientRpc.screen__setColor(static_cast<capnzero::CalculatorQtWebchannel::EColor>(color));
}

QClient::EColor
QClient::screen__getColor()
{
	return static_cast<QClient::EColor>(m_qclientRpc.screen__getColor());
}

void QClient::screen__setTitle(const QString& title)
{
	m_qclientRpc.screen__setTitle(title);
}

void QClient::screen__setId(const QByteArray& title)
{
	m_qclientRpc.screen__setId(title);
}

ReturnScreenAlltypes
QClient::screen__alltypes(qint32 a, qreal b, qreal c, const QString& d, const QByteArray& e, const QByteArray& f, EColor color, EMode mode)
{
	return m_qclientRpc.screen__alltypes(a, b, c, d, e, f, static_cast<capnzero::CalculatorQtWebchannel::EColor>(color), static_cast<capnzero::CalculatorQtWebchannel::EMode>(mode));
}

QClient::EMode QClient::mode() const
{
	return m_mode;
}



void QClient::registerMetaTypes()
{
	qRegisterMetaType<ReturnScreenAlltypes>();
	qRegisterMetaType<EColor>("EColor");
	qRegisterMetaType<EMode>("EMode");
}


