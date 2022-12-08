#include <chrono>
#include <iostream>

#include "CalculatorQtWebchannel_Server.h"

namespace capnzero::CalculatorQtWebchannel
{
class CalculatorImpl : public CalculatorRpcIf
{
public:
   ReturnAdd add(Int32 a, Int32 b) override { return {a + b}; }

   ReturnSub sub(Int32 a, Int32 b) override { return {a - b}; }
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
      if (m_brightness != brightness)
      {
         m_brightness = brightness;
         m_rSignals.Screen__brightnessChanged(brightness);
      }
   }
   void setColor(EColor color) override
   {
      switch (color)
      {
         case EColor::RED: break;
         case EColor::GREEN: break;
         case EColor::BLUE: break;
         case EColor::YELLOW: break;
      }
   }
   void setTitle(const TextView& title) override {}
   void setId(const SpanCL<8>& title) override {}
   ReturnAlltypes alltypes(Int32 a, Float32 b, Float64 c, const TextView& d,
                           const Span& e, const SpanCL<8>& f) override
   {
      m_rSignals.Screen__alltypess(1, 1.1, 2.2, "Hello", Data<8>(), Data<8>());
      return {1, 1.1, 2.2, "Hello", Data<8>()};
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
       std::make_unique<CalculatorImpl>(),
       std::make_unique<ScreenImpl>(server.signals()));

   while (true) { server.processNextRequest(); }
   return 0;
}