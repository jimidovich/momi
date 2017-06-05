#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "myevent.h"
//#include "datahub.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <functional>

//typedef void(msgHandlerClass::*MyEventHandler)(QEvent*);

class DataHub;

class EventSubscriber
{
public:
    EventSubscriber(){}
    ~EventSubscriber(){}

    virtual void onCtpEvent(CtpEvent) = 0;
};

class Dispatcher
{
public:
    Dispatcher(){}
    ~Dispatcher() {myThread.join();}
    void waitForTick();
    void runThread();

    DataHub *dataHub;
    std::vector<std::function<void(CtpEvent)>> subscribers;
    std::vector<EventSubscriber*> subs;

private:
    std::thread myThread;
};


#endif // !DISPATCHER_H
