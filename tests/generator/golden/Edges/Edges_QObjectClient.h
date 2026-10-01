#ifndef EDGES_QOBJECT_CLIENT_H
#define EDGES_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "Edges.capnp.h"

#include "Edges_ClientTransport.h"
namespace capnzero::Edges
{

class ReturnZebraEmpty
{
    Q_GADGET

public:

};



class QClientRpc : public QObject, public EdgesClientRpcTransport
{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
	Q_INVOKABLE ReturnZebraEmpty zebra__empty();
	Q_INVOKABLE qint32 zebra__noArgs();
	Q_INVOKABLE QByteArray zebra__bytes(const QByteArray& dynamic, const QByteArray& fixed, const QString& text, Mode mode);
	Q_INVOKABLE void zebra__setActive(bool val);
	Q_INVOKABLE void zebra__toggleActive();
	Q_INVOKABLE void fire();

};

} // namespace capnzero::Edges

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::Edges
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
	void zebra__activeChanged(bool val);
	void empty();
	void tick();

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::Edges

namespace capnzero::Edges
{

class QClient : public QObject
{
    Q_OBJECT
	Q_PROPERTY(bool zebra__active READ zebra__active WRITE zebra__setActive NOTIFY zebra__activeChanged)

public:
	explicit QClient(zmq::context_t& rZmqContext, const std::string& rpcAddr, const std::string& signalAddr, QObject* pParent = nullptr);

	enum class Mode {
		Off = 0,
		On = 1
	};
	Q_ENUM(Mode)

	Q_INVOKABLE ReturnZebraEmpty zebra__empty();
	Q_INVOKABLE qint32 zebra__noArgs();
	Q_INVOKABLE QByteArray zebra__bytes(const QByteArray& dynamic, const QByteArray& fixed, const QString& text, Mode mode);
	Q_INVOKABLE void zebra__setActive(bool val);
	Q_INVOKABLE void zebra__toggleActive();
	Q_INVOKABLE void fire();
	Q_INVOKABLE bool zebra__active() const;

signals:
	void zebra__activeChanged();
	void empty();
	void tick();

private:
	QClientRpc m_qclientRpc;
	QClientSignals m_qclientSignals;
	bool m_zebra__active;

    void registerMetaTypes();
};

} // namespace capnzero::Edges


#endif
