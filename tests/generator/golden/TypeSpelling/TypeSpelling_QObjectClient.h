#ifndef TYPE_SPELLING_QOBJECT_CLIENT_H
#define TYPE_SPELLING_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "TypeSpelling.capnp.h"

#include "TypeSpelling_ClientTransport.h"
namespace capnzero::TypeSpelling
{



class QClientRpc : public QObject, public TypeSpellingClientRpcTransport
{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
	Q_INVOKABLE QByteArray bytes(const QByteArray& value);

};

} // namespace capnzero::TypeSpelling

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::TypeSpelling
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

} // namespace capnzero::TypeSpelling

namespace capnzero::TypeSpelling
{

class QClient : public QObject
{
    Q_OBJECT

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject* pParent = nullptr);


	Q_INVOKABLE QByteArray bytes(const QByteArray& value);

signals:

private:
	QClientRpc m_qclientRpc;

    void registerMetaTypes();
};

} // namespace capnzero::TypeSpelling


#endif
