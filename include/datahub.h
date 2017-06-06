#ifndef DATAHUB_H
#define DATAHUB_H

#include <queue>
#include <condition_variable>
#include <mutex>

#include "myevent.h"
#include "struct.h"
#include "dispatcher.h"

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

class DataHub : public EventSubscriber
{
public:
//    DataQueue<double> feedQueue;  // for testing
    DataQueue<CtpEvent> eventQueue;

    SymInfoTable symInfoTable;
    SymMktTable symMktTable;
    SymMktTable symPrevMktTable;

    void onCtpEvent(CtpEvent ev);
};

#endif // DATAHUB_H
