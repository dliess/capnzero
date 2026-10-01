#ifndef CALCULATOR_SIGNALS_ONLY_QOBJECT_CLIENT_H
#define CALCULATOR_SIGNALS_ONLY_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "CalculatorSignalsOnly.capnp.h"


#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::CalculatorSignalsOnly
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
	void screen__brightnessChanged(quint32 brightness);
	void screen__colorChanged(EColor color);

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::CalculatorSignalsOnly

namespace capnzero::CalculatorSignalsOnly
{

class QClient : public QObject
{
    Q_OBJECT

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject* pParent = nullptr);

	enum class EColor {
		Blue = 0,
		Red = 1,
		Green = 2,
		Yellow = 3
	};
	Q_ENUM(EColor)


signals:
	void screen__brightnessChanged(quint32 brightness);
	void screen__colorChanged(EColor color);

private:
	QClientSignals m_qclientSignals;

    void registerMetaTypes();
};

} // namespace capnzero::CalculatorSignalsOnly


#endif
