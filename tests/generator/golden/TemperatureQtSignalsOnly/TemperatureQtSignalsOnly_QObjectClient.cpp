#include "TemperatureQtSignalsOnly_QObjectClient.h"
#include "TemperatureQtSignalsOnly.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQtSignalsOnly;



QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(signalAddr);
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "tempRangeChanged2");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "sensorNameChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "actualTemperatureChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "tempRangeChanged");
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
	if(key == "tempRangeChanged2"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTempRangeChanged2>();
		emit tempRangeChanged2(paramReader.getTempRange());
	}
	else if(key == "sensorNameChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterSensorNameChanged>();
		emit sensorNameChanged(QByteArray(reinterpret_cast<const char *>(paramReader.getVal().begin()), int(paramReader.getVal().size())));
	}
	else if(key == "actualTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterActualTemperatureChanged>();
		emit actualTemperatureChanged(paramReader.getVal());
	}
	else if(key == "tempRangeChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterTempRangeChanged>();
		emit tempRangeChanged(paramReader.getVal());
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
                 const std::string& signalAddr,
                 QObject* pParent) :
    QObject(pParent),
    m_qclientSignals(rZmqContext, signalAddr)
{
    registerMetaTypes();
	QObject::connect(&m_qclientSignals, &QClientSignals::tempRangeChanged2, [this](capnzero::TemperatureQtSignalsOnly::ETempRange tempRange){
			emit tempRangeChanged2(static_cast<QClient::ETempRange>(tempRange));
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::sensorNameChanged, [this](const QString& val){
		if(m_sensorName != val){
			m_sensorName = val;
			emit sensorNameChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::actualTemperatureChanged, [this](qreal val){
		if(m_actualTemperature != val){
			m_actualTemperature = val;
			emit actualTemperatureChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::tempRangeChanged, [this](capnzero::TemperatureQtSignalsOnly::ETempRange val){
		if(m_tempRange != static_cast<decltype(m_tempRange)>(val)){
			m_tempRange = static_cast<decltype(m_tempRange)>(val);
			emit tempRangeChanged();
		}
	});

}


QString QClient::sensorName() const
{
	return m_sensorName;
}

qreal QClient::actualTemperature() const
{
	return m_actualTemperature;
}

QClient::ETempRange QClient::tempRange() const
{
	return m_tempRange;
}



void QClient::registerMetaTypes()
{

	qRegisterMetaType<ETempRange>("ETempRange");
}


