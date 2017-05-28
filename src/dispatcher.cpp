#include <QDebug>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTimeZone>

#include <iostream>
#include "include/dispatcher.h"
#include "include/portfolio.h"

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
    case MarketEvent:
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
        }
//        auto us2 = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count()%1000000;
//        cout << "o " << us1 << " elapsed " << us2-us1 << endl;


        //onTick(data);
    }
}

void Dispatcher1::runThread() {
    myThread = thread(&Dispatcher1::waitForTick, this);
}
