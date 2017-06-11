#ifndef DATAHUB_H
#define DATAHUB_H

#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "myevent.h"
#include "struct.h"
//#include "dispatcher.h"

template <typename T>
class DataQueue
{
public:
    DataQueue(){}
    ~DataQueue(){}

    void post(T const& data) {
        {
            std::unique_lock<std::mutex> locker(mu);
            q.push(data);
            count++;
        }
        newDataPosted.notify_one();
    }

    T fetch() {
        std::unique_lock<std::mutex> locker(mu);
        while (q.empty())
            newDataPosted.wait(locker);
        auto data = q.front();
        q.pop();
        return data;
    }

    std::queue<T> q;
    std::condition_variable newDataPosted;
    std::mutex mu;
    int count = 0;
};

class EventSubscriber
{
public:
    EventSubscriber(){}
    ~EventSubscriber(){}

    virtual void onCtpEvent(CtpEvent) = 0;
};

class DataHub : public EventSubscriber
{
public:
//    DataQueue<double> feedQueue;  // for testing
    void onCtpEvent(CtpEvent ev);

    DataQueue<CtpEvent> eventQueue;

    SymInfoTable symInfoTable;
    SymMktTable symMktTable;
    SymMktTable symPrevMktTable;
    std::map<std::string, bool> dictHasMkt;
};

class Dispatcher
{
public:
    Dispatcher(){}
    ~Dispatcher() {myThread.join();}
    void waitForTick() {
        while(1) {
            auto ev = dataHub->eventQueue.fetch();
            for (auto sub : subs) {
                sub->onCtpEvent(ev);
            }
        }
    }

    void runThread() {myThread = std::thread(&Dispatcher::waitForTick, this);}

    DataHub *dataHub;
//    std::vector<std::function<void(CtpEvent)>> subscribers;
    std::vector<EventSubscriber*> subs;

private:
    std::thread myThread;
};

#endif // DATAHUB_H
