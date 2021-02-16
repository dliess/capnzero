#include "CalculatorSignalsOnly_Server.h"
#include <iostream>
#include <chrono>

int main()
{
    zmq::context_t context;
    capnzero::CalculatorSignalsOnlyServer server(context, "tcp://*:5556");
    server.signals().Screen__brightnessChanged(33);
    return 0;
}