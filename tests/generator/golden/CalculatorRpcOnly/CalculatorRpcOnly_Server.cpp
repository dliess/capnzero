#include "CalculatorRpcOnly_Server.h"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include "CalculatorRpcOnly.capnp.h"
#include "capnzero_utils.h"

using namespace capnzero;
using namespace capnzero::CalculatorRpcOnly;

CalculatorRpcOnlyServer::CalculatorRpcOnlyServer(zmq::context_t& rZmqContext, const std::string& rpcBindAddr , std::unique_ptr<CalculatorRpcIf> calculator, std::unique_ptr<ScreenRpcIf> screen) :
	m_pCalculatorRpcIf(std::move(calculator)),
	m_pScreenRpcIf(std::move(screen)),
    m_zmqRepSocket(rZmqContext, zmq::socket_type::router)
{
    m_zmqRepSocket.bind(rpcBindAddr);
}
void CalculatorRpcOnlyServer::unbind(const std::string& rpcBindAddr) { m_zmqRepSocket.unbind(rpcBindAddr); }   
void CalculatorRpcOnlyServer::processNextRequest(WaitMode waitMode) {
    zmq::message_t routerId;
    auto res0 = m_zmqRepSocket.recv(routerId, (waitMode == WaitMode::Blocking) ? zmq::recv_flags::none : zmq::recv_flags::dontwait);
    if(waitMode == WaitMode::NonBlocking && !res0) return;
    zmq::message_t rpcCoordBuf;
    auto res1 = m_zmqRepSocket.recv(rpcCoordBuf, zmq::recv_flags::none);
    ::capnp::FlatArrayMessageReader msgReader(asCapnpArr(rpcCoordBuf));
    auto coordReader = msgReader.getRoot<RpcCoord>();
    switch(coordReader.getServiceId())
    {
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
					builder.setRet(ret.ret);
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
					builder.setRet(ret.ret);
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

void CalculatorRpcOnlyServer::processNextRequestAllNonBlock() {
    while(m_zmqRepSocket.get(zmq::sockopt::events) & ZMQ_POLLIN)
    {
        processNextRequest(WaitMode::Blocking);
    }
}

int CalculatorRpcOnlyServer::getFd() const
{
    return m_zmqRepSocket.get(zmq::sockopt::fd);
}

