#include "TemperatureQtSignalsOnly_Server.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <sys/timerfd.h>
#include <unistd.h>
#include "FdSet.h"

using namespace capnzero::TemperatureQtSignalsOnly;

int main()
{
    zmq::context_t context;
    float actualTemperatureTmp = 0.3;
    float commandedTemperature = 0.7;
    bool enabled = false;
    TemperatureQtSignalsOnlyServer server(context,"tcp://*:5556");

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
    fdSet.AddFd(server.signals().getFd(), [&server](int fd){
        server.signals().handleAllSubscriptions();
    });

    server.signals().registerSensorNameChangedSubscrCb([](TemperatureQtSignalsOnlyServer::Signals& signals){
        signals.sensorNameChanged("Living-Room");
    });
    server.signals().registerActualTemperatureChangedSubscrCb([&actualTemperatureTmp](TemperatureQtSignalsOnlyServer::Signals& signals){
        signals.actualTemperatureChanged(actualTemperatureTmp);
    });

    while(true)
    {
        fdSet.Select();
    }

    return 0;
}