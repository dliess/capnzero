#include "TemperatureQt_QObjectClient.h"
#include "TemperatureQt.capnp.h"
#include <QSocketNotifier>
#include <capnp/serialize.h>
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::TemperatureQt;


QClientRpc::QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent):
    QObject(pParent),
    TemperatureQtClientRpcTransport(rZmqContext, rpcAddr)
{
}    
        
void QClientRpc::explicitSetUUID(const QByteArray& uuid)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterExplicitSetUUID>();
	auto uuid_converted = QByteArray::fromBase64(uuid, QByteArray::Base64Encoding);
	paramBuilder.setUuid(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(uuid_converted.data()), uuid_converted.size()));


    _explicitSetUUID(
		message

    );

}

QByteArray QClientRpc::explicitGetUUID()
{

	QByteArray retVal;

    _explicitGetUUID(

		[&retVal](capnp::MessageReader& message){
			auto reader = message.getRoot<CAPNPReturnExplicitGetUUID>();
			auto src = reader.getRetParam();
			assert(src.size() == retVal.size());
			std::copy(src.begin(), src.end(), retVal.begin());
			retVal = retVal.toBase64();

	}
    );
	return retVal;
}

void QClientRpc::setCommandedTemperature(qreal val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetCommandedTemperature>();
	paramBuilder.setVal(val);


    _setCommandedTemperature(
		message

    );

}

void QClientRpc::setEnabled(bool val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetEnabled>();
	paramBuilder.setVal(val);


    _setEnabled(
		message

    );

}

void QClientRpc::toggleEnabled()
{


    _toggleEnabled(


    );

}

void QClientRpc::setUuid(const QByteArray& val)
{
	::capnp::MallocMessageBuilder message;
	auto paramBuilder = message.initRoot<CAPNPParameterSetUuid>();
	auto val_converted = QByteArray::fromBase64(val, QByteArray::Base64Encoding);
	paramBuilder.setVal(capnp::Data::Reader(reinterpret_cast<const ::capnp::byte*>(val_converted.data()), val_converted.size()));


    _setUuid(
		message

    );

}



QClientSignals::QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent):
    QObject(pParent),
    m_zmqSubSocket(rZmqContext, zmq::socket_type::sub)
{
    m_zmqSubSocket.connect(signalAddr);
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "explicitSignalUUID");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "sensorNameChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "actualTemperatureChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "commandedTemperatureChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "enabledChanged");
	m_zmqSubSocket.set(zmq::sockopt::subscribe, "uuidChanged");
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
	if(key == "explicitSignalUUID"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterExplicitSignalUUID>();
		emit explicitSignalUUID(QByteArray(reinterpret_cast<const char *>(paramReader.getUuid().begin()), int(paramReader.getUuid().size())).toBase64());
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
	else if(key == "commandedTemperatureChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterCommandedTemperatureChanged>();
		emit commandedTemperatureChanged(paramReader.getVal());
	}
	else if(key == "enabledChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterEnabledChanged>();
		emit enabledChanged(paramReader.getVal());
	}
	else if(key == "uuidChanged"){
		zmq::message_t paramBuf;
		auto res2 = m_zmqSubSocket.recv(paramBuf, zmq::recv_flags::none);
		if (!res2) { throw std::runtime_error("No received msg"); }
		::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
		auto paramReader = msgReader.getRoot<CAPNPSignalParameterUuidChanged>();
		emit uuidChanged(QByteArray(reinterpret_cast<const char *>(paramReader.getVal().begin()), int(paramReader.getVal().size())).toBase64());
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
	QObject::connect(&m_qclientSignals, &QClientSignals::explicitSignalUUID, [this](const QByteArray& uuid){
			emit explicitSignalUUID(uuid);
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
	QObject::connect(&m_qclientSignals, &QClientSignals::commandedTemperatureChanged, [this](qreal val){
		if(m_commandedTemperature != val){
			m_commandedTemperature = val;
			emit commandedTemperatureChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::enabledChanged, [this](bool val){
		if(m_enabled != val){
			m_enabled = val;
			emit enabledChanged();
		}
	});
	QObject::connect(&m_qclientSignals, &QClientSignals::uuidChanged, [this](const QByteArray& val){
		if(m_uuid != val){
			m_uuid = val;
			emit uuidChanged();
		}
	});

}


void QClient::explicitSetUUID(const QByteArray& uuid)
{
	m_qclientRpc.explicitSetUUID(uuid);
}

QByteArray QClient::explicitGetUUID()
{
	return m_qclientRpc.explicitGetUUID();
}

void QClient::setCommandedTemperature(qreal val)
{
	m_qclientRpc.setCommandedTemperature(val);
}

void QClient::setEnabled(bool val)
{
	m_qclientRpc.setEnabled(val);
}

void QClient::toggleEnabled()
{
	m_qclientRpc.toggleEnabled();
}

void QClient::setUuid(const QByteArray& val)
{
	m_qclientRpc.setUuid(val);
}

QString QClient::sensorName() const
{
	return m_sensorName;
}

qreal QClient::actualTemperature() const
{
	return m_actualTemperature;
}

qreal QClient::commandedTemperature() const
{
	return m_commandedTemperature;
}

bool QClient::enabled() const
{
	return m_enabled;
}

QByteArray QClient::uuid() const
{
	return m_uuid;
}



void QClient::registerMetaTypes()
{

}


