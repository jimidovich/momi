#include <QDebug>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTimeZone>

#include <iostream>
#include "include/dispatcher.h"

using namespace std;

Dispatcher::Dispatcher()
{
}

Dispatcher::~Dispatcher()
{
}

void Dispatcher::registerHandler(const QObject *receiver, const char *signal, const char *handler)
{
	QObject::connect(this, signal, receiver, handler);
}

void Dispatcher::customEvent(QEvent *ev)
{
	QElapsedTimer t;
	t.start();
	//qDebug() << "receive EVENT" << ++count << QTime::currentTime();

	//TODO: compare performance for two ways when delay not negligible
	//emit dispatch(ev);
	//dbc->onEvent(ev);

	//TODO: Create base class MsgHandlerClass, virtual function void onEvent(MyEvent*)
	//sendToHandler(ev, (msgHandlerClass::*)(MyEvent*) h);
	switch (((MyEvent*)ev)->myType)
	{
	case FeedEvent:
		emit dispatchFeed(ev);
		break;
	case AccountInfoEvent:
		emit dispatchAccInfo(ev);
		break;
	case ContractInfoEvent:
		emit dispatchContractInfo(ev);
		break;
	case OrderEvent:
		emit dispatchOrder(ev);
		break;
	case TradeEvent:
		emit dispatchTrade(ev);
		break;
	case PositionEvent:
		emit dispatchPos(ev);
		break;
	case PositionDetailEvent:
		emit dispatchPosDetail(ev);
		break;
	case AccountUpdateEvent:
		emit dispatchAccUpdate(ev);
		break;
	default:

		break;
	}

	//qDebug() << "dispatcher" << t.elapsed() << "ms";
}



class Reader
{
public:
    Reader(string name):name(name){}
    Reader(){}
    ~Reader(){}
    void onTick(double data, string color);
    void waitForTick();
    void runThread();

    DataHub* dh;
private:
    string name;
    thread myThread;
    unique_lock<mutex> locker;
};


void Dispatcher1::onTick(int data) {
    this_thread::sleep_for(chrono::milliseconds(15));
}


void Dispatcher1::waitForTick() {
    while(1) {
        locker = unique_lock<mutex>(dh->mu);
        if (dh->q.empty()) {
//            cout_lk.lock();
            cout << name << " waiting..." << endl;
//            cout_lk.unlock();
            dh->newTickPosted.wait(locker);
        }
//        cout_lk.lock();
        cout << name <<  " fetching data from thread id: " << this_thread::get_id() << endl;
        cout << name <<  " got data: " << dh->q.front() << endl;
//        cout_lk.unlock();
        auto data = dh->q.front();
        dh->q.pop();
        locker.unlock();

        //dispatching
        auto t1 = thread(&Reader::onTick, r1, data, "");
        auto t2 = thread(&Reader::onTick, r2, data, "");
        t1.detach();
        t2.detach();

//        r1->onTick(data, "");
//        r2->onTick(data, "");


        //onTick(data);
    }
}

void Dispatcher1::runThread() {
    myThread = thread(&Dispatcher1::waitForTick, this);
}
