#include "Calculator_Server.h"
#include <iostream>
#include <chrono>

namespace capnzero::Calculator{
class CalculatorImpl : public RpcIf
{
public:
    ReturnAdd add(Int32 a, Int32 b) override
    {
        return {a + b};
    }

    ReturnSub sub(Int32 a, Int32 b) override
    {
        return {a - b};
    }

    Int32 addDirectRet(Int32 a, Int32 b) override
    {
        return a + b;
    }

    Int32 subDirectRet(Int32 a, Int32 b) override
    {
        return a - b;
    }

	void multiply(::capnp::MessageReader& recvData) override
    {

    }

	ReturnMultiply2 multiply2(::capnp::MessageReader& recvData) override
    {
        auto reader = recvData.getRoot<TwoIntParams>();
        return { reader.getA() * reader.getB() };
    }

	void multiply3(Int32 a, Int32 b, NativeCapnpMsgWriter &writer) override
    {
        ::capnp::MallocMessageBuilder message;
        auto builder = message.initRoot<OneIntParam>();
        builder.setA(a * b);
        writer.write(message);
    }

	void multiply4(::capnp::MessageReader& recvData, NativeCapnpMsgWriter &writer) override
    {
        auto reader = recvData.getRoot<TwoIntParams>();
        ::capnp::MallocMessageBuilder message;
        auto builder = message.initRoot<OneIntParam>();
        builder.setA(reader.getA() * reader.getB());
        writer.write(message);
    }
};

class DirectRetImpl : public DirectRetRpcIf
{
public:
    Int32 add(Int32 a, Int32 b) override
    {
        return a + b;
    }

    Int32 sub(Int32 a, Int32 b) override
    {
        return a - b;
    }
};

class ScreenImpl : public ScreenRpcIf
{
public:
    ScreenImpl(CalculatorServer::Signals& rSignals) : m_rSignals(rSignals) {}
	void setBrightness(UInt32 brightness) override
    {
        if(m_brightness != brightness)
        {
            m_brightness = brightness;
            m_rSignals.Screen__brightnessChanged(brightness);
        }
    }
	void setColor(EColor color) override
    {
        switch(color)
        {
            case EColor::RED: break;
            case EColor::GREEN: break;
            case EColor::BLUE: break;
            case EColor::YELLOW: break;
        }
    }
	void setTitle(const TextView& title) override
    {

    }
	void setId(const SpanCL<8>& title) override
    {

    }
private:
    CalculatorServer::Signals& m_rSignals;
    UInt32 m_brightness{0};
};
}

using namespace capnzero::Calculator;

int main()
{
    zmq::context_t context;
    CalculatorServer server(context,
                            "tcp://*:5555",
                            "tcp://*:5556",
                            std::make_unique<CalculatorImpl>(),
                            std::make_unique<DirectRetImpl>(),
                            std::make_unique<ScreenImpl>(server.signals()));

    while(true)
    {
        server.processNextRequest();
    }
    return 0;
}