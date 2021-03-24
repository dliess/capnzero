#include "Calculator_Client.h"
#include <iostream>
#include <chrono>

using namespace capnzero::Calculator;

int main()
{
    zmq::context_t context;
    CalculatorClient client(context, "tcp://localhost:5555", "tcp://localhost:5556");
    std::cout << client.add(47, 333).ret << "\n";
    ::capnp::MallocMessageBuilder msg;
    auto builder = msg.initRoot<TwoIntParams>();
    builder.setA(3);
    builder.setB(4);
    std::cout << client.multiply2(msg).ret << "\n";
    

    client.onScreenBrightnessChanged([](capnzero::UInt32 brightness){
        std::cout << "brightness changed received: " << brightness << "\n";
    });
    std::this_thread::sleep_for (std::chrono::milliseconds(1));
    for(int i = 0; i  < 20; ++i)
    {
        client.Screen__setBrightness(i);
    }
    client.Screen__setColor(EColor::GREEN);
    std::this_thread::sleep_for (std::chrono::seconds(1));
    return 0;
}