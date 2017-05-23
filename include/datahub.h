#ifndef DATAHUB_H
#define DATAHUB_H

#include <queue>
#include <condition_variable>
#include <mutex>

class DataHub
{
public:
    DataHub(){}
    ~DataHub(){}

    std::queue<double> q;
    std::condition_variable newTickPosted;
    std::mutex mu;
};

#endif // DATAHUB_H
