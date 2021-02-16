#include "CalculatorSignalsOnly_Client.h"
#include <iostream>
#include <chrono>

int main()
{
    zmq::context_t context;
    capnzero::CalculatorSignalsOnlyClient client(context, "tcp://localhost:5556");
    client.onScreenBrightnessChanged([](capnzero::UInt32 brightness){
        std::cout << "brightness changed received: " << brightness << "\n";
    });
    std::this_thread::sleep_for (std::chrono::seconds(1));
    return 0;
}