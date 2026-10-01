#include "CalculatorQtWebchannel_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "CalculatorQtWebchannel.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorQtWebchannel;

CalculatorQtWebchannelServer::CalculatorQtWebchannelServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<RpcIf> _, std::unique_ptr<CalculatorRpcIf> calculator, std::unique_ptr<ScreenRpcIf> screen) :
	m_p_RpcIf(std::move(_)),
	m_pCalculatorRpcIf(std::move(calculator)),
	m_pScreenRpcIf(std::move(screen)),
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router),
    m_signals(rZmqContext, signalBindAddr)
{
    m_zmqRepSocket.bind(rpcBindAddr);
}
void CalculatorQtWebchannelServer::unbind(const std::string& rpcBindAddr) { m_zmqRepSocket.unbind(rpcBindAddr); }   
void CalculatorQtWebchannelServer::processNextRequest(WaitMode waitMode) {
    zmq::message_t routerId;
    auto res0 = m_zmqRepSocket.recv(routerId, (waitMode == WaitMode::Blocking) ? zmq::recv_flags::none : zmq::recv_flags::dontwait);
    if(waitMode == WaitMode::NonBlocking && !res0) return;
    zmq::message_t rpcCoordBuf;
    auto res1 = m_zmqRepSocket.recv(rpcCoordBuf, zmq::recv_flags::none);
    ::capnp::FlatArrayMessageReader msgReader(asCapnpArr(rpcCoordBuf));
    auto coordReader = msgReader.getRoot<RpcCoord>();
    switch(coordReader.getServiceId())
    {
		case to_underlying(ServiceId::MAIN):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(RpcIds::SET_MODE):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterSetMode>();
					m_p_RpcIf->setMode(paramReader.getVal());
					break;
				}
			}
			break;
		}
		case to_underlying(ServiceId::CALCULATOR):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(CalculatorRpcIds::ADD):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterCalculatorAdd>();
					auto ret = m_pCalculatorRpcIf->add(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnCalculatorAdd>();
					builder.setRetParam(ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(CalculatorRpcIds::SUB):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterCalculatorSub>();
					auto ret = m_pCalculatorRpcIf->sub(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnCalculatorSub>();
					builder.setRetParam(ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
			}
			break;
		}
		case to_underlying(ServiceId::SCREEN):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(ScreenRpcIds::SET_BRIGHTNESS):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterScreenSetBrightness>();
					m_pScreenRpcIf->setBrightness(paramReader.getBrightness());
					break;
				}
				case to_underlying(ScreenRpcIds::SET_COLOR):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterScreenSetColor>();
					m_pScreenRpcIf->setColor(paramReader.getColor());
					break;
				}
				case to_underlying(ScreenRpcIds::GET_COLOR):
				{
					auto ret = m_pScreenRpcIf->getColor();
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnScreenGetColor>();
					builder.setRetParam(ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(ScreenRpcIds::SET_TITLE):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterScreenSetTitle>();
					m_pScreenRpcIf->setTitle(TextView(paramReader.getTitle().begin(), paramReader.getTitle().size()));
					break;
				}
				case to_underlying(ScreenRpcIds::SET_ID):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterScreenSetId>();
					m_pScreenRpcIf->setId(SpanCL<8>(paramReader.getTitle().begin(), paramReader.getTitle().end()));
					break;
				}
				case to_underlying(ScreenRpcIds::ALLTYPES):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterScreenAlltypes>();
					auto ret = m_pScreenRpcIf->alltypes(paramReader.getA(), paramReader.getB(), paramReader.getC(), TextView(paramReader.getD().begin(), paramReader.getD().size()), Span(paramReader.getE().begin(), paramReader.getE().end()), SpanCL<8>(paramReader.getF().begin(), paramReader.getF().end()), paramReader.getColor(), paramReader.getMode());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnScreenAlltypes>();
					builder.setA(ret.a);
					builder.setB(ret.b);
					builder.setC(ret.c);
					builder.setD(ret.d);
					builder.setF(capnp::Data::Reader(ret.f.data(), ret.f.size()));
					builder.setColor(ret.color);
					builder.setMode(ret.mode);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
			}
			break;
		}

    }
}

void CalculatorQtWebchannelServer::processNextRequestAllNonBlock() {
    while(m_zmqRepSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        processNextRequest(WaitMode::Blocking);
    }
}

int CalculatorQtWebchannelServer::getFd() const
{
    return m_zmqRepSocket.get(zmq::sockopt::fd);
}

CalculatorQtWebchannelServer::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::xpub)
{
    m_rZmqPubSocket.bind(signalBindAddr);
    m_rZmqPubSocket.set(zmq::sockopt::xpub_verbose, true);
}

void CalculatorQtWebchannelServer::Signals::unbind(const std::string& signalBindAddr) { m_rZmqPubSocket.unbind(signalBindAddr); }

int CalculatorQtWebchannelServer::Signals::getFd() const
{
    return m_rZmqPubSocket.get(zmq::sockopt::fd);
}

void CalculatorQtWebchannelServer::Signals::handleAllSubscriptions()
{
    while(m_rZmqPubSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
	{
		zmq::message_t msg;
		auto res = m_rZmqPubSocket.recv(msg, zmq::recv_flags::none);
		bool subscribe = static_cast<const char*>(msg.data())[0];
		std::string key(&(static_cast<const char*>(msg.data())[1]), msg.size() - 1);
		if(subscribe)
		{
            auto it = m_subscrCbMap.find(key);
            if(it != m_subscrCbMap.end())
            {
                (it->second)(*this);
            }
		}
	}
}

void CalculatorQtWebchannelServer::Signals::registerModeChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["modeChanged"] = cb;
}
void CalculatorQtWebchannelServer::Signals::registerScreenColorChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__colorChanged"] = cb;
}
void CalculatorQtWebchannelServer::Signals::registerScreenBrightnessChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__brightnessChanged"] = cb;
}
void CalculatorQtWebchannelServer::Signals::registerScreenAlltypessSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__alltypess"] = cb;
}



void CalculatorQtWebchannelServer::Signals::modeChanged(EMode val)
{
	m_rZmqPubSocket.send(zmq::const_buffer("modeChanged", 11), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterModeChanged>();
	builder.setVal(val);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void CalculatorQtWebchannelServer::Signals::Screen__colorChanged(EColor color)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__colorChanged", 20), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterScreenColorChanged>();
	builder.setColor(color);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void CalculatorQtWebchannelServer::Signals::Screen__brightnessChanged(UInt32 brightness)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__brightnessChanged", 25), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterScreenBrightnessChanged>();
	builder.setBrightness(brightness);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void CalculatorQtWebchannelServer::Signals::Screen__alltypess(Int32 a, Float32 b, Float64 c, const Text& d, const Span& e, const Data<8>& f, EColor color, EMode mode)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__alltypess", 17), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterScreenAlltypess>();
	builder.setA(a);
	builder.setB(b);
	builder.setC(c);
	builder.setD(d);
	builder.setE(capnp::Data::Reader(e.data(), e.size()));
	builder.setF(capnp::Data::Reader(f.data(), f.size()));
	builder.setColor(color);
	builder.setMode(mode);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}
