#include <iostream>
#include "include/dispatcher.h"

using namespace std;

class Reader
{
public:
    Reader(string name):name(name){}
    Reader(){}
    ~Reader(){}
    void onTick(double data, string color);
    void onEvent(CtpEvent ev);
    void waitForTick();
    void runThread();

    DataQueue<double>* dh;
private:
    string name;
    thread myThread;
    unique_lock<mutex> locker;
};

void Dispatcher1::waitForTick() {
    while(1) {
//        auto data = dataHub->feedQueue.fetch();
        auto ev = dataHub->eventQueue.fetch();

//        auto us1 = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count()%1000000;
        //dispatching
//        auto t1 = thread(&Reader::onTick, r1, data, "");
//        auto t2 = thread(&Reader::onEvent, r2, ev);
//        t1.detach();
//        t2.detach();
//        auto t3 = thread(&Reader::onTick, r1, data, "");
//        auto t4 = thread(&Reader::onTick, r2, data, "");
//        auto t5 = thread(&Reader::onTick, r1, data, "");
//        auto t6 = thread(&Reader::onTick, r2, data, "");
//        t3.detach();
//        t4.detach();
//        t5.detach();
//        t6.detach();

//        r1->onTick(data, "");
//        r2->onEvent(ev);
//        pf->onCtpEvent(ev);
//        dataHub->onCtpEvent(ev);

        for (auto sub : subscribers) {
            sub(ev);
//            auto t = thread(sub, ev);
//            t.detach();
        }
//        auto us2 = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count()%1000000;
//        cout << "o " << us1 << " elapsed " << us2-us1 << endl;


        //onTick(data);
    }
}

void Dispatcher1::runThread() {
    myThread = thread(&Dispatcher1::waitForTick, this);
}
