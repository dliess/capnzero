#ifndef TEMPERATURE_QT_QOBJECT_CLIENT_H
#define TEMPERATURE_QT_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "TemperatureQt.capnp.h"

#include "TemperatureQt_ClientTransport.h"
namespace capnzero::TemperatureQt
{



class QClientRpc : public QObject, public TemperatureQtClientRpcTransport
{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
	Q_INVOKABLE void explicitSetUUID(const QByteArray& uuid);
	Q_INVOKABLE QByteArray explicitGetUUID();
	Q_INVOKABLE void setCommandedTemperature(qreal val);
	Q_INVOKABLE void setEnabled(bool val);
	Q_INVOKABLE void toggleEnabled();
	Q_INVOKABLE void setUuid(const QByteArray& val);

};

} // namespace capnzero::TemperatureQt

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TemperatureQt
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
	void explicitSignalUUID(const QByteArray& uuid);
	void sensorNameChanged(const QString& val);
	void actualTemperatureChanged(qreal val);
	void commandedTemperatureChanged(qreal val);
	void enabledChanged(bool val);
	void uuidChanged(const QByteArray& val);

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::TemperatureQt

namespace capnzero::TemperatureQt
{

class QClient : public QObject
{
    Q_OBJECT
	Q_PROPERTY(QString sensorName READ sensorName NOTIFY sensorNameChanged)
	Q_PROPERTY(qreal actualTemperature READ actualTemperature NOTIFY actualTemperatureChanged)
	Q_PROPERTY(qreal commandedTemperature READ commandedTemperature WRITE setCommandedTemperature NOTIFY commandedTemperatureChanged)
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
	Q_PROPERTY(QByteArray uuid READ uuid WRITE setUuid NOTIFY uuidChanged)

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent = nullptr);


	Q_INVOKABLE void explicitSetUUID(const QByteArray& uuid);
	Q_INVOKABLE QByteArray explicitGetUUID();
	Q_INVOKABLE void setCommandedTemperature(qreal val);
	Q_INVOKABLE void setEnabled(bool val);
	Q_INVOKABLE void toggleEnabled();
	Q_INVOKABLE void setUuid(const QByteArray& val);
	Q_INVOKABLE QString sensorName() const;
	Q_INVOKABLE qreal actualTemperature() const;
	Q_INVOKABLE qreal commandedTemperature() const;
	Q_INVOKABLE bool enabled() const;
	Q_INVOKABLE QByteArray uuid() const;

signals:
	void explicitSignalUUID(const QByteArray& uuid);
	void sensorNameChanged();
	void actualTemperatureChanged();
	void commandedTemperatureChanged();
	void enabledChanged();
	void uuidChanged();

private:
	QClientRpc m_qclientRpc;
	QClientSignals m_qclientSignals;
	QString m_sensorName;
	qreal m_actualTemperature;
	qreal m_commandedTemperature;
	bool m_enabled;
	QByteArray m_uuid;

    void registerMetaTypes();
};

} // namespace capnzero::TemperatureQt


#endif
