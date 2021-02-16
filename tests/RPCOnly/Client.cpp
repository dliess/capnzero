#include "CalculatorRpcOnly_Client.h"
#include <iostream>
#include <chrono>

int main()
{
    zmq::context_t context;
    capnzero::CalculatorRpcOnlyClient client(context, "tcp://localhost:5555");
    std:: cout << client.Calculator__add(47, 333).ret << "\n";
    for(int i = 0; i  < 20; ++i)
    {
        client.Screen__setBrightness(i);
    }
    return 0;
}