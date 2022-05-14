#include "TemperatureQt_Server.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <sys/timerfd.h>
#include <unistd.h>
#include "FdSet.h"

namespace capnzero::TemperatureQt{

class RpcImpl : public RpcIf
{
public:
    RpcImpl(TemperatureQtServer::Signals& rSignals, float& rActualTemperature, float& rCommandedTemperature, bool& enabled) :
        m_rSignals(rSignals),
        m_rActualTemperature(rActualTemperature),
        m_rCommandedTemperature(rCommandedTemperature),
        m_rEnabled(enabled)
    {}
	void setCommandedTemperature(Float32 val) override
    {
        if(m_rCommandedTemperature != val)
        {
            m_rCommandedTemperature = val;
            m_rSignals.commandedTemperatureChanged(m_rCommandedTemperature);
        }
    }
	void setEnabled(Bool val) override
    {
        if(m_rEnabled != val)
        {
            m_rEnabled = val;
            m_rSignals.enabledChanged(m_rEnabled);
        }
    }

	void toggleEnabled() override
    {
        m_rEnabled != m_rEnabled;
        m_rSignals.enabledChanged(m_rEnabled);
    }

private:
    TemperatureQtServer::Signals& m_rSignals;
    float& m_rActualTemperature;
    float& m_rCommandedTemperature;
    bool &m_rEnabled;
};

}

using namespace capnzero::TemperatureQt;

int main()
{
    zmq::context_t context;
    float actualTemperatureTmp = 0.3;
    float commandedTemperature = 0.7;
    bool enabled = false;
    TemperatureQtServer server(context,
                               "tcp://*:5555",
                               "tcp://*:5556",
                                std::make_unique<RpcImpl>(server.signals(), actualTemperatureTmp, commandedTemperature, enabled));

    int timerFd = timerfd_create(CLOCK_MONOTONIC, 0);
    constexpr auto Period = std::chrono::milliseconds(30);
    constexpr auto PeriodNs = std::chrono::duration_cast<std::chrono::nanoseconds>(Period);
    itimerspec t({ .it_interval = {0, PeriodNs.count()}, .it_value = {0, 1000000}});
    timerfd_settime(timerFd, 0, &t, NULL);

    utils::FdSet fdSet;
    fdSet.AddFd(timerFd, [&](int fd){
        int timersElapsed = 0;
        (void) read(fd, &timersElapsed, 8);
        float diff = commandedTemperature - actualTemperatureTmp;
        if(std::abs(diff) > 0.005)
        {
            actualTemperatureTmp += (diff / 100.0);
            server.signals().actualTemperatureChanged(actualTemperatureTmp);
        }
    });
    fdSet.AddFd(server.getFd(), [&server](int fd){
        server.processNextRequestAllNonBlock();
    });
    fdSet.AddFd(server.signals().getFd(), [&server](int fd){
        server.signals().handleAllSubscriptions();
    });

    server.signals().registerSensorNameChangedSubscrCb([](TemperatureQtServer::Signals& signals){
        signals.sensorNameChanged("Living-Room");
    });
    server.signals().registerActualTemperatureChangedSubscrCb([&actualTemperatureTmp](TemperatureQtServer::Signals& signals){
        signals.actualTemperatureChanged(actualTemperatureTmp);
    });
    server.signals().registerCommandedTemperatureChangedSubscrCb([&commandedTemperature](TemperatureQtServer::Signals& signals){
        signals.commandedTemperatureChanged(commandedTemperature);
    });

    while(true)
    {
        fdSet.Select();
    }

    return 0;
}