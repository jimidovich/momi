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
        std::unique_lock<std::mutex> locker(this->feed_mu);
        this->q.push(data);
        this->count++;
        locker.unlock();
        this->newDataPosted.notify_one();
    }

    T fetch() {
        std::unique_lock<std::mutex> locker(this->feed_mu);
        if (this->q.empty())
            this->newDataPosted.wait(locker);
        auto data = this->q.front();
        this->q.pop();
        locker.unlock();
        return data;
    }

    std::queue<T> q;
    std::condition_variable newDataPosted;
    std::mutex feed_mu;
    int count = 0;
};

class DataHub
{
public:
    DataQueue<double> feedQueue;
    DataQueue<CtpDataEvent> eventQueue;
};

#endif // DATAHUB_H
