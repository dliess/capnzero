#include "TemperatureQtWithServiceName_Server.h"
#include <cstdint>
#include <iostream>
#include <chrono>
#include <cmath>
#include <sys/timerfd.h>
#include <unistd.h>
#include "FdSet.h"

namespace capnzero::TemperatureQtWithServiceName{

class RpcImpl : public TemperatureServiceRpcIf
{
public:
    RpcImpl(TemperatureQtWithServiceNameServer::Signals& rSignals, float& rActualTemperature, float& rCommandedTemperature) :
        m_rSignals(rSignals),
        m_rActualTemperature(rActualTemperature),
        m_rCommandedTemperature(rCommandedTemperature)
    {}
  	void aTestRpc(const TextView& title) override
    {
        
    }

	void setCommandedTemperature(Float32 val) override
    {
        if(m_rCommandedTemperature != val)
        {
            m_rCommandedTemperature = val;
            m_rSignals.TemperatureService__commandedTemperatureChanged(m_rCommandedTemperature);
        }
    }
private:
    TemperatureQtWithServiceNameServer::Signals& m_rSignals;
    float& m_rActualTemperature;
    float& m_rCommandedTemperature;
};

}

using namespace capnzero::TemperatureQtWithServiceName;

int main()
{
    zmq::context_t context;
    float actualTemperatureTmp = 0.3;
    float commandedTemperature = 0.7;
    TemperatureQtWithServiceNameServer server(context,
                               "tcp://*:5555",
                               "tcp://*:5556",
                                std::make_unique<RpcImpl>(server.signals(), actualTemperatureTmp, commandedTemperature));

    int timerFd = timerfd_create(CLOCK_MONOTONIC, 0);
    constexpr auto Period = std::chrono::milliseconds(30);
    constexpr auto PeriodNs = std::chrono::duration_cast<std::chrono::nanoseconds>(Period);
    itimerspec t({ .it_interval = {0, PeriodNs.count()}, .it_value = {0, 1000000}});
    timerfd_settime(timerFd, 0, &t, NULL);

    utils::FdSet fdSet;
    fdSet.AddFd(timerFd, [&](int fd){
        int64_t timersElapsed = 0;
        (void) read(fd, &timersElapsed, 8);
        float diff = commandedTemperature - actualTemperatureTmp;
        if(std::abs(diff) > 0.005)
        {
            actualTemperatureTmp += (diff / 100.0);
            server.signals().TemperatureService__actualTemperatureChanged(actualTemperatureTmp);
        }
    });
    fdSet.AddFd(server.getFd(), [&server](int fd){
        server.processNextRequestAllNonBlock();
    });
    fdSet.AddFd(server.signals().getFd(), [&server](int fd){
        server.signals().handleAllSubscriptions();
    });

    server.signals().registerTemperatureServiceSensorNameChangedSubscrCb([](TemperatureQtWithServiceNameServer::Signals& signals){
        signals.TemperatureService__sensorNameChanged("Living-Room");
    });
    server.signals().registerTemperatureServiceActualTemperatureChangedSubscrCb([&actualTemperatureTmp](TemperatureQtWithServiceNameServer::Signals& signals){
        signals.TemperatureService__actualTemperatureChanged(actualTemperatureTmp);
    });
    server.signals().registerTemperatureServiceCommandedTemperatureChangedSubscrCb([&commandedTemperature](TemperatureQtWithServiceNameServer::Signals& signals){
        signals.TemperatureService__commandedTemperatureChanged(commandedTemperature);
    });

    while(true)
    {
        fdSet.Select();
    }

    return 0;
}
