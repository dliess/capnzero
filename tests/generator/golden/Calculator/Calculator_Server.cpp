#include "Calculator_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "Calculator.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::Calculator;

CalculatorServer::CalculatorServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr, const std::string& signalBindAddr , std::unique_ptr<RpcIf> _, std::unique_ptr<DirectRetRpcIf> directRet, std::unique_ptr<ScreenRpcIf> screen) :
	m_p_RpcIf(std::move(_)),
	m_pDirectRetRpcIf(std::move(directRet)),
	m_pScreenRpcIf(std::move(screen)),
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router),
    m_signals(rZmqContext, signalBindAddr)
{
    m_zmqRepSocket.bind(rpcBindAddr);
}
void CalculatorServer::unbind(const std::string& rpcBindAddr) { m_zmqRepSocket.unbind(rpcBindAddr); }   
void CalculatorServer::processNextRequest(WaitMode waitMode) {
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
				case to_underlying(RpcIds::ADD):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterAdd>();
					auto ret = m_p_RpcIf->add(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnAdd>();
					builder.setRet(ret.ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(RpcIds::SUB):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterSub>();
					auto ret = m_p_RpcIf->sub(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnSub>();
					builder.setRet(ret.ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(RpcIds::ADD_DIRECT_RET):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterAddDirectRet>();
					auto ret = m_p_RpcIf->addDirectRet(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnAddDirectRet>();
					builder.setRetParam(ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(RpcIds::SUB_DIRECT_RET):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterSubDirectRet>();
					auto ret = m_p_RpcIf->subDirectRet(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnSubDirectRet>();
					builder.setRetParam(ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(RpcIds::MULTIPLY):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));
					m_p_RpcIf->multiply(reader);
					break;
				}
				case to_underlying(RpcIds::MULTIPLY2):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));
					auto ret = m_p_RpcIf->multiply2(reader);
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnMultiply2>();
					builder.setRet(ret.ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(RpcIds::MULTIPLY3):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterMultiply3>();
					NativeCapnpMsgWriter writer(routerId, m_zmqRepSocket);
					m_p_RpcIf->multiply3(paramReader.getA(), paramReader.getB(), writer);
					break;
				}
				case to_underlying(RpcIds::MULTIPLY4):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader reader(asCapnpArr(paramBuf));
					NativeCapnpMsgWriter writer(routerId, m_zmqRepSocket);
					m_p_RpcIf->multiply4(reader, writer);
					break;
				}
			}
			break;
		}
		case to_underlying(ServiceId::DIRECT_RET):
		{
			switch(coordReader.getRpcId())
			{
				case to_underlying(DirectRetRpcIds::ADD):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterDirectRetAdd>();
					auto ret = m_pDirectRetRpcIf->add(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnDirectRetAdd>();
					builder.setRetParam(ret);
					m_zmqRepSocket.send(routerId, zmq::send_flags::sndmore);
					sendOverZmq(retMessage, m_zmqRepSocket, zmq::send_flags::none);
					break;
				}
				case to_underlying(DirectRetRpcIds::SUB):
				{
					zmq::message_t paramBuf;
					auto res2 = m_zmqRepSocket.recv(paramBuf, zmq::recv_flags::none);
					if (!res2) { throw std::runtime_error("No received msg"); }
					::capnp::FlatArrayMessageReader msgReader(asCapnpArr(paramBuf));
					auto paramReader = msgReader.getRoot<CAPNPParameterDirectRetSub>();
					auto ret = m_pDirectRetRpcIf->sub(paramReader.getA(), paramReader.getB());
					::capnp::MallocMessageBuilder retMessage;
					auto builder = retMessage.initRoot<CAPNPReturnDirectRetSub>();
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
			}
			break;
		}

    }
}

void CalculatorServer::processNextRequestAllNonBlock() {
    while(m_zmqRepSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        processNextRequest(WaitMode::Blocking);
    }
}

int CalculatorServer::getFd() const
{
    return m_zmqRepSocket.get(zmq::sockopt::fd);
}

CalculatorServer::Signals::Signals(zmq::context_t& rZmqContext, const std::string& signalBindAddr) :
    m_rZmqPubSocket(rZmqContext, zmq::socket_type::xpub)
{
    m_rZmqPubSocket.bind(signalBindAddr);
    m_rZmqPubSocket.set(zmq::sockopt::xpub_verbose, true);
}

void CalculatorServer::Signals::unbind(const std::string& signalBindAddr) { m_rZmqPubSocket.unbind(signalBindAddr); }

int CalculatorServer::Signals::getFd() const
{
    return m_rZmqPubSocket.get(zmq::sockopt::fd);
}

void CalculatorServer::Signals::handleAllSubscriptions()
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

void CalculatorServer::Signals::registerJustASignalSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["justASignal"] = cb;
}
void CalculatorServer::Signals::registerScreenListChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__listChanged"] = cb;
}
void CalculatorServer::Signals::registerScreenColorChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__colorChanged"] = cb;
}
void CalculatorServer::Signals::registerScreenBrightnessChangedSubscrCb(SubscriptionCb cb)
{
    m_subscrCbMap["Screen__brightnessChanged"] = cb;
}



void CalculatorServer::Signals::justASignal()
{
	m_rZmqPubSocket.send(zmq::const_buffer("justASignal", 11), zmq::send_flags::none);
}

void CalculatorServer::Signals::Screen__listChanged(::capnp::MessageBuilder& sndData)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__listChanged", 19), zmq::send_flags::sndmore);
	sendOverZmq(sndData, m_rZmqPubSocket, zmq::send_flags::none);
}

void CalculatorServer::Signals::Screen__colorChanged(EColor color)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__colorChanged", 20), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterScreenColorChanged>();
	builder.setColor(color);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}

void CalculatorServer::Signals::Screen__brightnessChanged(UInt32 brightness)
{
	m_rZmqPubSocket.send(zmq::const_buffer("Screen__brightnessChanged", 25), zmq::send_flags::sndmore);
	::capnp::MallocMessageBuilder message;
	auto builder = message.initRoot<CAPNPSignalParameterScreenBrightnessChanged>();
	builder.setBrightness(brightness);
	sendOverZmq(message, m_rZmqPubSocket, zmq::send_flags::none);
}
