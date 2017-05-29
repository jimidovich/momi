#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "myevent.h"
#include "datahub.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <functional>

//typedef void(msgHandlerClass::*MyEventHandler)(QEvent*);


class Dispatcher1
{
public:
    Dispatcher1(){}
    Dispatcher1(std::string name):name(name){}
    ~Dispatcher1() {myThread.join();}
    void waitForTick();
    void runThread();

    DataHub *dataHub;
    std::vector<std::function<void(CtpEvent)>> subscribers;

private:
    std::string name;
    std::thread myThread;
};


#endif // !DISPATCHER_H
