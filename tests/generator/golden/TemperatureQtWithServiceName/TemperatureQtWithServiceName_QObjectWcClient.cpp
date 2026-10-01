#include "TemperatureQtWithServiceName_QObjectClient.h"
#include "TemperatureQtWithServiceName.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQtWithServiceName;


QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    TemperatureQtWithServiceNameClientRpcTransport(rZmqContext, rpcAddr)
{
}    
        
void QClientRpc::temperatureService__aTestRpc(const QString& title)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterTemperatureServiceATestRpc>();
	paramBuilder.setTitle(title.toUtf8().constData());


    _TemperatureService__aTestRpc(
		message

    );

}

void QClientRpc::temperatureService__setCommandedTemperature(qreal val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterTemperatureServiceSetCommandedTemperature>();
	paramBuilder.setVal(val);


    _TemperatureService__setCommandedTemperature(
		message

    );

}



QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(signalAddr);
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "TemperatureService__sensorNameChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "TemperatureService__actualTemperatureChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "TemperatureService__commandedTemperatureChanged");
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
	if(key == "TemperatureService__sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceSensorNameChanged>();
		emit temperatureService__sensorNameChanged(QByteArray(reinterpret_cast<const char *>(paramReader.getVal().begin()), int(paramReader.getVal().size())));
	}
	else if(key == "TemperatureService__actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceActualTemperatureChanged>();
		emit temperatureService__actualTemperatureChanged(paramReader.getVal());
	}
	else if(key == "TemperatureService__commandedTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTemperatureServiceCommandedTemperatureChanged>();
		emit temperatureService__commandedTemperatureChanged(paramReader.getVal());
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
	QObject::connect(&m_qclientSignals, &QClientSignals::temperatureService__sensorNameChanged, [this](const QString& val){
		if(m_temperatureService__sensorName != val){
			m_temperatureService__sensorName = val;
			emit temperatureService__sensorNameChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::temperatureService__actualTemperatureChanged, [this](qreal val){
		if(m_temperatureService__actualTemperature != val){
			m_temperatureService__actualTemperature = val;
			emit temperatureService__actualTemperatureChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::temperatureService__commandedTemperatureChanged, [this](qreal val){
		if(m_temperatureService__commandedTemperature != val){
			m_temperatureService__commandedTemperature = val;
			emit temperatureService__commandedTemperatureChanged();
		}
	});

}


void QClient::temperatureService__aTestRpc(const QString& title)
{
	m_qclientRpc.temperatureService__aTestRpc(title);
}

void QClient::temperatureService__setCommandedTemperature(qreal val)
{
	m_qclientRpc.temperatureService__setCommandedTemperature(val);
}

QString QClient::temperatureService__sensorName() const
{
	return m_temperatureService__sensorName;
}

qreal QClient::temperatureService__actualTemperature() const
{
	return m_temperatureService__actualTemperature;
}

qreal QClient::temperatureService__commandedTemperature() const
{
	return m_temperatureService__commandedTemperature;
}



void QClient::registerMetaTypes()
{

}


