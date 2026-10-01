#include "Interop_Server.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

using namespace capnzero;
using namespace capnzero::Interop;

class Main : public RpcIf {
public:
    InteropServer::Signals* signals = nullptr;
    int calls = 0;
    ReturnAdd add(Int32 a, Int32 b) override { return {a + b}; }
    Int32 direct(Int32 value) override { return value * 2; }
    ReturnEcho echo(const TextView& text, const SpanCL<4>& data, Color color,
                    Float64 fraction, Bool flag) override {
        ReturnEcho result{Text(text), {}, color, fraction, flag};
        std::copy(data.begin(), data.end(), result.data.begin());
        return result;
    }
    void native(::capnp::MessageReader& message, NativeCapnpMsgWriter& writer) override {
        ::capnp::MallocMessageBuilder response;
        response.initRoot<NativeValue>().setValue(message.getRoot<NativeValue>().getValue() + 1);
        writer.write(response);
    }
    Int32 count() override { return calls; }
    void notify() override { ++calls; signals->changed(42); }
};
class Device : public DeviceRpcIf {
public:
    Int32 increment(Int32 value) override { return value + 1; }
};
int main(int argc, char** argv) {
    if (argc != 3) return 2;
    zmq::context_t context;
    auto handler = std::make_unique<Main>();
    auto* impl = handler.get();
    InteropServer server(context, argv[1], argv[2], std::move(handler), std::make_unique<Device>());
    impl->signals = &server.signals();
    server.signals().registerChangedSubscrCb([](auto& signals) { signals.changed(7); });
    std::cout << "READY" << std::endl;
    for (;;) {
        server.signals().handleAllSubscriptions();
        server.processNextRequestAllNonBlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
