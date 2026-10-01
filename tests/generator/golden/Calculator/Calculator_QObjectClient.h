#ifndef CALCULATOR_QOBJECT_CLIENT_H
#define CALCULATOR_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "Calculator.capnp.h"

#include "Calculator_ClientTransport.h"
namespace capnzero::Calculator
{

class Return_Add
{
    Q_GADGET
	Q_PROPERTY(qint32 ret MEMBER ret)
public:
	qint32 ret;
};

class Return_Sub
{
    Q_GADGET
	Q_PROPERTY(qint32 ret MEMBER ret)
public:
	qint32 ret;
};

class Return_Multiply2
{
    Q_GADGET
	Q_PROPERTY(qint32 ret MEMBER ret)
public:
	qint32 ret;
};



class QClientRpc : public QObject, public CalculatorClientRpcTransport
{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
	Q_INVOKABLE Return_Add add(qint32 a, qint32 b);
	Q_INVOKABLE Return_Sub sub(qint32 a, qint32 b);
	Q_INVOKABLE qint32 addDirectRet(qint32 a, qint32 b);
	Q_INVOKABLE qint32 subDirectRet(qint32 a, qint32 b);
	Q_INVOKABLE qint32 directRet__add(qint32 a, qint32 b);
	Q_INVOKABLE qint32 directRet__sub(qint32 a, qint32 b);
	Q_INVOKABLE void screen__setBrightness(quint32 brightness);
	Q_INVOKABLE void screen__setColor(EColor color);
	Q_INVOKABLE void screen__setTitle(const QString& title);
	Q_INVOKABLE void screen__setId(const QByteArray& title);

};

} // namespace capnzero::Calculator

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::Calculator
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
	void justASignal();
	void screen__listChanged(::capnp::MessageBuilder& sndData);
	void screen__colorChanged(EColor color);
	void screen__brightnessChanged(quint32 brightness);

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::Calculator

namespace capnzero::Calculator
{

class QClient : public QObject
{
    Q_OBJECT

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent = nullptr);

	enum class EColor {
		Blue = 0,
		Red = 1,
		Green = 2,
		Yellow = 3
	};
	Q_ENUM(EColor)
	enum class EMode {
		Dimmed = 0,
		Bright = 1
	};
	Q_ENUM(EMode)

	Q_INVOKABLE Return_Add add(qint32 a, qint32 b);
	Q_INVOKABLE Return_Sub sub(qint32 a, qint32 b);
	Q_INVOKABLE qint32 addDirectRet(qint32 a, qint32 b);
	Q_INVOKABLE qint32 subDirectRet(qint32 a, qint32 b);
	Q_INVOKABLE qint32 directRet__add(qint32 a, qint32 b);
	Q_INVOKABLE qint32 directRet__sub(qint32 a, qint32 b);
	Q_INVOKABLE void screen__setBrightness(quint32 brightness);
	Q_INVOKABLE void screen__setColor(EColor color);
	Q_INVOKABLE void screen__setTitle(const QString& title);
	Q_INVOKABLE void screen__setId(const QByteArray& title);

signals:
	void justASignal();
	void screen__listChanged(::capnp::MessageBuilder& sndData);
	void screen__colorChanged(EColor color);
	void screen__brightnessChanged(quint32 brightness);

private:
	QClientRpc m_qclientRpc;
	QClientSignals m_qclientSignals;

    void registerMetaTypes();
};

} // namespace capnzero::Calculator


#endif
