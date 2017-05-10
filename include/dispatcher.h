#ifndef DISPATCHER_H
#define DISPATCHER_H

//#include <QObject>
#include "myevent.h"
//#include "kdbconnector.h"

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

#endif // !DISPATCHER_H
