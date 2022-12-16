#include <chrono>
#include <iostream>

#include "CalculatorQtWebchannel_Server.h"

namespace capnzero::CalculatorQtWebchannel
{

class CalculatorMainImpl : public RpcIf
{
public:
   CalculatorMainImpl(CalculatorQtWebchannelServer::Signals& rSignals) :
       m_rSignals(rSignals)
   {
   }
	void setMode(EMode val) override {
        std::cout << "CalculatorMainImpl::setMode("<<static_cast<int>(val)<<")\n";
        m_rSignals.modeChanged(val);
    };
private:
   CalculatorQtWebchannelServer::Signals& m_rSignals;
};

class CalculatorImpl : public CalculatorRpcIf
{
public:
   Int32 add(Int32 a, Int32 b) override { return {a + b}; }
   Int32 sub(Int32 a, Int32 b) override { return {a - b}; }
};

class ScreenImpl : public ScreenRpcIf
{
public:
   ScreenImpl(CalculatorQtWebchannelServer::Signals& rSignals) :
       m_rSignals(rSignals)
   {
   }
   void setBrightness(UInt32 brightness) override
   {
        m_brightness = brightness;
        m_rSignals.Screen__brightnessChanged(brightness);
   }
   void setColor(EColor color) override
   {
      switch (color)
      {
         case EColor::RED: std::cout << "RED" << "\n"; break;
         case EColor::GREEN: std::cout << "GREEN" << "\n"; break;
         case EColor::BLUE: std::cout << "BLUE" << "\n"; break;
         case EColor::YELLOW: std::cout << "YELLOW" << "\n"; break;
      }
    m_rSignals.Screen__colorChanged(color);
   }
   EColor getColor() override { return EColor::BLUE; }
   void setTitle(const TextView& title) override {}
   void setId(const SpanCL<8>& title) override {}
   ReturnAlltypes alltypes(Int32 a, Float32 b, Float64 c, const TextView& d,
                           const Span& e, const SpanCL<8>& f, EColor color, EMode mode) override
   {
      m_rSignals.Screen__alltypess(1, 1.1, 2.2, "Hello", Data<8>(), Data<8>(), EColor::GREEN, EMode::BRIGHT);
      return {1, 1.1, 2.2, "Hello", Data<8>(), EColor::GREEN, EMode::BRIGHT};
   }

private:
   CalculatorQtWebchannelServer::Signals& m_rSignals;
   UInt32 m_brightness{0};
};
}   // namespace capnzero::CalculatorQtWebchannel

using namespace capnzero::CalculatorQtWebchannel;
int main()
{
   zmq::context_t context;
   CalculatorQtWebchannelServer server(
       context, "tcp://*:5555", "tcp://*:5556",
       std::make_unique<CalculatorMainImpl>(server.signals()),
       std::make_unique<CalculatorImpl>(),
       std::make_unique<ScreenImpl>(server.signals()));

   while (true) { server.processNextRequest(); }
   return 0;
}