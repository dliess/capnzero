#ifndef TEMPERATURE_QT_SIGNALS_ONLY_QOBJECT_CLIENT_H
#define TEMPERATURE_QT_SIGNALS_ONLY_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "TemperatureQtSignalsOnly.capnp.h"


#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TemperatureQtSignalsOnly
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
	void tempRangeChanged2(ETempRange tempRange);
	void sensorNameChanged(const QString& val);
	void actualTemperatureChanged(qreal val);
	void tempRangeChanged(ETempRange val);

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::TemperatureQtSignalsOnly

namespace capnzero::TemperatureQtSignalsOnly
{

class QClient : public QObject
{
    Q_OBJECT
	Q_PROPERTY(QString sensorName READ sensorName NOTIFY sensorNameChanged)
	Q_PROPERTY(qreal actualTemperature READ actualTemperature NOTIFY actualTemperatureChanged)
	Q_PROPERTY(ETempRange tempRange READ tempRange NOTIFY tempRangeChanged)

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject* pParent = nullptr);

	enum class ETempRange {
		Cold = 0,
		Hot = 1
	};
	Q_ENUM(ETempRange)

	Q_INVOKABLE QString sensorName() const;
	Q_INVOKABLE qreal actualTemperature() const;
	Q_INVOKABLE ETempRange tempRange() const;

signals:
	void tempRangeChanged2(ETempRange tempRange);
	void sensorNameChanged();
	void actualTemperatureChanged();
	void tempRangeChanged();

private:
	QClientSignals m_qclientSignals;
	QString m_sensorName;
	qreal m_actualTemperature;
	ETempRange m_tempRange;

    void registerMetaTypes();
};

} // namespace capnzero::TemperatureQtSignalsOnly


#endif
