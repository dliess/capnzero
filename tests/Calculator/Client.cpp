#include "Calculator_Client.h"
#include <iostream>
#include <chrono>

int main()
{
    zmq::context_t context;
    capnzero::CalculatorClient client(context, "tcp://localhost:5555", "tcp://localhost:5556");
    std::cout << client.Calculator__add(47, 333).ret << "\n";
    ::capnp::MallocMessageBuilder msg;
    auto builder = msg.initRoot<TwoIntParams>();
    builder.setA(3);
    builder.setB(4);
    std::cout << client.Calculator__multiply2(msg).ret << "\n";
    

    client.onScreenBrightnessChanged([](capnzero::UInt32 brightness){
        std::cout << "brightness changed received: " << brightness << "\n";
    });
    for(int i = 0; i  < 20; ++i)
    {
        client.Screen__setBrightness(i);
    }
    client.Screen__setColor(EColor::GREEN);
    std::this_thread::sleep_for (std::chrono::seconds(1));
    return 0;
}