#include "CalculatorSignalsOnly_Client.h"
#include <iostream>
#include <chrono>

int main()
{
    zmq::context_t context;
//#define NORMAL_VARIANT
#ifdef NORMAL_VARIANT
    capnzero::CalculatorSignalsOnlyClient client(context, "tcp://localhost:5556");
    client.onScreenBrightnessChanged([](capnzero::UInt32 brightness){
        std::cout << "brightness changed received: " << brightness << "\n";
    });
    std::this_thread::sleep_for (std::chrono::seconds(4));
#else
    capnzero::CalculatorSignalsOnly::CalculatorSignalsOnlyClientSignals client(context, "tcp://localhost:5556");
    std::cout << "CLIENT HERE 1\n";
    client.onScreenBrightnessChanged([](capnzero::UInt32 brightness){
        std::cout << "brightness changed received: " << brightness << "\n";
    });
    std::cout << "CLIENT HERE 2\n";
    while(true)
    {
        client.handleIncomingSignal();
    }
#endif
    return 0;
}