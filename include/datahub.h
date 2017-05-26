#ifndef DATAHUB_H
#define DATAHUB_H

#include <queue>
#include <condition_variable>
#include <mutex>

#include "myevent.h"

template <typename T>
class DataQueue
{
public:
    DataQueue(){}
    ~DataQueue(){}

    void post(T data) {
        std::unique_lock<std::mutex> locker(mu);
        q.push(data);
        count++;
        locker.unlock();
        newDataPosted.notify_one();
    }

    T fetch() {
        std::unique_lock<std::mutex> locker(mu);
        if (q.empty())
            newDataPosted.wait(locker);
        auto data = q.front();
        q.pop();
        locker.unlock();
        return data;
    }

    std::queue<T> q;
    std::condition_variable newDataPosted;
    std::mutex mu;
    int count = 0;
};

class DataHub
{
public:
    DataQueue<double> feedQueue;
    DataQueue<CtpEvent> eventQueue;
};

#endif // DATAHUB_H
