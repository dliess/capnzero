#include "CalculatorRpcOnly_Server.h"
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
	void setBrightness(UInt32 brightness) override
    {
        if(m_brightness != brightness)
        {
            m_brightness = brightness;
        }
    }
	void setTitle(const TextView& title) override
    {

    }
	void setId(const SpanCL<8>& title) override
    {

    }
private:
    UInt32 m_brightness{0};
};
}

int main()
{
    zmq::context_t context;
    capnzero::CalculatorRpcOnlyServer server(context,
                                      "tcp://*:5555",
                                      std::make_unique<capnzero::CalculatorImpl>(),
                                      std::make_unique<capnzero::ScreenImpl>());

    while(true)
    {
        server.processNextRequest();
    }
    return 0;
}