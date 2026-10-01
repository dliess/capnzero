#include "Interop_Client.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace capnzero;
using namespace capnzero::Interop;
void check(bool value) { if (!value) throw std::runtime_error("interoperability assertion failed"); }
int main(int argc, char** argv) {
    if (argc != 3) return 2;
    zmq::context_t context;
    InteropClientRpc client(context, argv[1]);
    InteropClientSignals signals(context, argv[2]);
    int last = 0;
    signals.onChanged([&](Int32 value) { last = value; });
    auto waitFor = [&](int value) {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (last != value && std::chrono::steady_clock::now() < deadline) {
            signals.handleIncomingSignalAllNonBlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        check(last == value);
    };
    waitFor(7);
    check(client.add(17, 25).value == 42);
    check(client.direct(21) == 42);
    check(client.Device__increment(41) == 42);
    Data<4> data{{0, 1, 2, 255}};
    auto echo = client.echo("hello", data, Color::GREEN, 1.25, true);
    check(echo.text == "hello" && echo.data == data && echo.color == Color::GREEN && echo.fraction == 1.25 && echo.flag);
    ::capnp::MallocMessageBuilder native;
    native.initRoot<NativeValue>().setValue(41);
    bool called = false;
    client.native(native, [&](::capnp::MessageReader& result) {
        check(result.getRoot<NativeValue>().getValue() == 42);
        called = true;
    });
    check(called);
    client.notify();
    check(client.count() == 1);
    waitFor(42);
    std::cout << "OK" << std::endl;
}
