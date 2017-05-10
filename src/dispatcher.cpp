#include <QDebug>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTimeZone>

#include "include/dispatcher.h"

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
