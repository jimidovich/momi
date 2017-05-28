#ifndef DISPATCHER_H
#define DISPATCHER_H

//#include <QObject>
#include "myevent.h"
#include "datahub.h"
//#include "kdbconnector.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

//typedef void(msgHandlerClass::*MyEventHandler)(QEvent*);

//class QObject;
class KdbConnector;

class Dispatcher : public QObject {
	Q_OBJECT
public:
	Dispatcher();
	~Dispatcher();

	void registerHandler(const QObject *receiver, const char *signal, const char *handler);
	void setKdbConnector(KdbConnector *val) { kdbConnector = val; };

signals:
	void dispatchToKdbConnector(const MyEvent *feed);
	void dispatchFeed(QEvent *ev);
	void dispatchAccInfo(QEvent *ev);
	void dispatchContractInfo(QEvent *ev);
	void dispatchOrder(QEvent *ev);
	void dispatchTrade(QEvent *ev);
	void dispatchPos(QEvent *ev);
	void dispatchPosDetail(QEvent *ev);
	void dispatchAccUpdate(QEvent *ev);

protected:

	void customEvent(QEvent *ev) override;

private:
	int count{ 0 };
	KdbConnector *kdbConnector{ nullptr };
};

class Reader;
class Portfolio;

class Dispatcher1
{
public:
    Dispatcher1(){}
    Dispatcher1(std::string name):name(name){}
    ~Dispatcher1() {myThread.join();}
    void waitForTick();
    void runThread();
    Reader *r1;
    Reader *r2;
    Portfolio *pf;

    DataHub *dataHub;
private:
    std::string name;
    std::thread myThread;
};


#endif // !DISPATCHER_H
