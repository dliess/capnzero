#ifndef CALCULATOR_QT_WEBCHANNEL_QOBJECT_CLIENT_H
#define CALCULATOR_QT_WEBCHANNEL_QOBJECT_CLIENT_H

#include <QObject>
#include <QString>
#include "CalculatorQtWebchannel.capnp.h"

#include "CalculatorQtWebchannel_ClientTransport.h"
namespace capnzero::CalculatorQtWebchannel
{

class ReturnScreenAlltypes
{
    Q_GADGET
	Q_PROPERTY(qint32 a MEMBER a)
	Q_PROPERTY(qreal b MEMBER b)
	Q_PROPERTY(qreal c MEMBER c)
	Q_PROPERTY(QString d MEMBER d)
	Q_PROPERTY(QByteArray f MEMBER f)
	Q_PROPERTY(EColor color MEMBER color)
	Q_PROPERTY(EMode mode MEMBER mode)
public:
	qint32 a;
	qreal b;
	qreal c;
	QString d;
	QByteArray f;
	EColor color;
	EMode mode;
};



class QClientRpc : public QObject, public CalculatorQtWebchannelClientRpcTransport
{
    Q_OBJECT
public:
    QClientRpc(zmq::context_t& rZmqContext, const std::string& rpcAddr, QObject *pParent = Q_NULLPTR);
	Q_INVOKABLE void setMode(EMode val);
	Q_INVOKABLE qint32 calculator__add(qint32 a, qint32 b);
	Q_INVOKABLE qint32 calculator__sub(qint32 a, qint32 b);
	Q_INVOKABLE void screen__setBrightness(quint32 brightness);
	Q_INVOKABLE void screen__setColor(EColor color);
	Q_INVOKABLE EColor screen__getColor();
	Q_INVOKABLE void screen__setTitle(const QString& title);
	Q_INVOKABLE void screen__setId(const QByteArray& title);
	Q_INVOKABLE ReturnScreenAlltypes screen__alltypes(qint32 a, qreal b, qreal c, const QString& d, const QByteArray& e, const QByteArray& f, EColor color, EMode mode);

};

} // namespace capnzero::CalculatorQtWebchannel

#include <zmq.hpp>
#include <capnp/message.h>

namespace capnzero::CalculatorQtWebchannel
{

class QClientSignals : public QObject
{
    Q_OBJECT
public:
    QClientSignals(zmq::context_t& rZmqContext, const std::string& signalAddr, QObject *pParent = Q_NULLPTR);
signals:
	void modeChanged(EMode val);
	void screen__colorChanged(EColor color);
	void screen__brightnessChanged(quint32 brightness);
	void screen__alltypess(qint32 a, qreal b, qreal c, const QString& d, const QByteArray& e, const QByteArray& f, EColor color, EMode mode);

private:
    zmq::socket_t m_zmqSubSocket;
	void handleIncomingSignal();
	void handleIncomingSignalAllNonBlock();
 	int getFd() const;
};

} // namespace capnzero::CalculatorQtWebchannel

namespace capnzero::CalculatorQtWebchannel
{

class QClient : public QObject
{
    Q_OBJECT
	Q_PROPERTY(EMode mode READ mode WRITE setMode NOTIFY modeChanged)

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

	Q_INVOKABLE void setMode(EMode val);
	Q_INVOKABLE qint32 calculator__add(qint32 a, qint32 b);
	Q_INVOKABLE qint32 calculator__sub(qint32 a, qint32 b);
	Q_INVOKABLE void screen__setBrightness(quint32 brightness);
	Q_INVOKABLE void screen__setColor(EColor color);
	Q_INVOKABLE EColor screen__getColor();
	Q_INVOKABLE void screen__setTitle(const QString& title);
	Q_INVOKABLE void screen__setId(const QByteArray& title);
	Q_INVOKABLE ReturnScreenAlltypes screen__alltypes(qint32 a, qreal b, qreal c, const QString& d, const QByteArray& e, const QByteArray& f, EColor color, EMode mode);
	Q_INVOKABLE EMode mode() const;

signals:
	void modeChanged();
	void screen__colorChanged(EColor color);
	void screen__brightnessChanged(quint32 brightness);
	void screen__alltypess(qint32 a, qreal b, qreal c, const QString& d, const QByteArray& e, const QByteArray& f, EColor color, EMode mode);

private:
	QClientRpc m_qclientRpc;
	QClientSignals m_qclientSignals;
	EMode m_mode;

    void registerMetaTypes();
};

} // namespace capnzero::CalculatorQtWebchannel


#endif
