#ifndef CALCULATOR_RPC_ONLY_QOBJECT_CLIENT_H
#define CALCULATOR_RPC_ONLY_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "CalculatorRpcOnly.capnp.h"

#include "CalculatorRpcOnly_ClientTransport.h"
namespace capnzero::CalculatorRpcOnly
{

class ReturnCalculatorAdd
{
    Q_GADGET
	Q_PROPERTY(qint32 ret MEMBER ret)
public:
	qint32 ret;
};

class ReturnCalculatorSub
{
    Q_GADGET
	Q_PROPERTY(qint32 ret MEMBER ret)
public:
	qint32 ret;
};



class QClientRpc : public QObject, public CalculatorRpcOnlyClientRpcTransport
{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
	Q_INVOKABLE ReturnCalculatorAdd calculator__add(qint32 a, qint32 b);
	Q_INVOKABLE ReturnCalculatorSub calculator__sub(qint32 a, qint32 b);
	Q_INVOKABLE void screen__setBrightness(quint32 brightness);
	Q_INVOKABLE void screen__setTitle(const QString& title);
	Q_INVOKABLE void screen__setId(const QByteArray& title);

};

} // namespace capnzero::CalculatorRpcOnly

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::CalculatorRpcOnly
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::CalculatorRpcOnly

namespace capnzero::CalculatorRpcOnly
{

class QClient : public QObject
{
    Q_OBJECT

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject* pParent = nullptr);


	Q_INVOKABLE ReturnCalculatorAdd calculator__add(qint32 a, qint32 b);
	Q_INVOKABLE ReturnCalculatorSub calculator__sub(qint32 a, qint32 b);
	Q_INVOKABLE void screen__setBrightness(quint32 brightness);
	Q_INVOKABLE void screen__setTitle(const QString& title);
	Q_INVOKABLE void screen__setId(const QByteArray& title);

signals:

private:
	QClientRpc m_qclientRpc;

    void registerMetaTypes();
};

} // namespace capnzero::CalculatorRpcOnly


#endif
