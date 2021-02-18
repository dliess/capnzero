#include "Calculator_Server.h"
#include <iostream>
#include <chrono>

namespace capnzero{
class CalculatorImpl : public capnzero::CalculatorIf
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
};

class ScreenImpl : public capnzero::ScreenIf
{
public:
    ScreenImpl(capnzero::CalculatorServer::Signals& rSignals) : m_rSignals(rSignals) {}
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
    capnzero::CalculatorServer::Signals& m_rSignals;
    UInt32 m_brightness{0};
};
}

int main()
{
    zmq::context_t context;
    capnzero::CalculatorServer server(context,
                                      "tcp://*:5555",
                                      "tcp://*:5556",
                                      std::make_unique<capnzero::CalculatorImpl>(),
                                      std::make_unique<capnzero::ScreenImpl>(server.signals()));

    while(true)
    {
        server.processNextRequest();
    }
    return 0;
}