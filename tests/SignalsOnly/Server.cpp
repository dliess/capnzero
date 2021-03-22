#include "CalculatorSignalsOnly_Server.h"
#include <iostream>
#include <chrono>

using namespace capnzero::CalculatorSignalsOnly; 

int main()
{
    zmq::context_t context;
    CalculatorSignalsOnlyServer server(context, "tcp://*:5556");
    while(true)
    {
        server.signals().Screen__brightnessChanged(33);
        std::this_thread::sleep_for (std::chrono::seconds(1));
    }
    return 0;
}