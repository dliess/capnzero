#ifndef TEMPERATURE_QT_WITH_SERVICE_NAME_QOBJECT_CLIENT_H
#define TEMPERATURE_QT_WITH_SERVICE_NAME_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "TemperatureQtWithServiceName.capnp.h"

#include "TemperatureQtWithServiceName_ClientTransport.h"
namespace capnzero::TemperatureQtWithServiceName
{



class QClientRpc : public QObject, public TemperatureQtWithServiceNameClientRpcTransport
{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
	Q_INVOKABLE void temperatureService__aTestRpc(const QString& title);
	Q_INVOKABLE void temperatureService__setCommandedTemperature(qreal val);

};

} // namespace capnzero::TemperatureQtWithServiceName

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TemperatureQtWithServiceName
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
	void temperatureService__sensorNameChanged(const QString& val);
	void temperatureService__actualTemperatureChanged(qreal val);
	void temperatureService__commandedTemperatureChanged(qreal val);

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::TemperatureQtWithServiceName

namespace capnzero::TemperatureQtWithServiceName
{

class QClient : public QObject
{
    Q_OBJECT
	Q_PROPERTY(QString temperatureService__sensorName READ temperatureService__sensorName NOTIFY temperatureService__sensorNameChanged)
	Q_PROPERTY(qreal temperatureService__actualTemperature READ temperatureService__actualTemperature NOTIFY temperatureService__actualTemperatureChanged)
	Q_PROPERTY(qreal temperatureService__commandedTemperature READ temperatureService__commandedTemperature WRITE temperatureService__setCommandedTemperature NOTIFY temperatureService__commandedTemperatureChanged)

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent = nullptr);


	Q_INVOKABLE void temperatureService__aTestRpc(const QString& title);
	Q_INVOKABLE void temperatureService__setCommandedTemperature(qreal val);
	Q_INVOKABLE QString temperatureService__sensorName() const;
	Q_INVOKABLE qreal temperatureService__actualTemperature() const;
	Q_INVOKABLE qreal temperatureService__commandedTemperature() const;

signals:
	void temperatureService__sensorNameChanged();
	void temperatureService__actualTemperatureChanged();
	void temperatureService__commandedTemperatureChanged();

private:
	QClientRpc m_qclientRpc;
	QClientSignals m_qclientSignals;
	QString m_temperatureService__sensorName;
	qreal m_temperatureService__actualTemperature;
	qreal m_temperatureService__commandedTemperature;

    void registerMetaTypes();
};

} // namespace capnzero::TemperatureQtWithServiceName


#endif
