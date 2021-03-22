#include "CalculatorRpcOnly_Client.h"
#include <iostream>
#include <chrono>

using namespace capnzero::CalculatorRpcOnly;

int main()
{
    zmq::context_t context;
    CalculatorRpcOnlyClient client(context, "tcp://localhost:5555");
    std:: cout << client.Calculator__add(47, 333).ret << "\n";
    for(int i = 0; i  < 20; ++i)
    {
        client.Screen__setBrightness(i);
    }
    return 0;
}