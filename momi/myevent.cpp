#include "myevent.h"


MyEvent::MyEvent(EnumMyEventType type, CThostFtdcDepthMarketDataField *mkt)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	feed(mkt)
{
}

MyEvent::MyEvent(EnumMyEventType type, CThostFtdcTradingAccountField *accInfo)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	accInfo(accInfo)
{
}

MyEvent::MyEvent(EnumMyEventType type, CThostFtdcInstrumentField *contractInfo)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	contractInfo(contractInfo)
{
}

MyEvent::MyEvent(EnumMyEventType type, CThostFtdcInvestorPositionField *pos)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	pos(pos)
{
}

MyEvent::MyEvent(EnumMyEventType type, CThostFtdcInvestorPositionDetailField *posDetail)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	posDetail(posDetail)
{
}

MyEvent::MyEvent(EnumMyEventType type, CThostFtdcTradeField *trade)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	trade(trade)
{
}

MyEvent::MyEvent(EnumMyEventType type, CThostFtdcOrderField *order)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	order(order)
{
}

MyEvent::MyEvent(EnumMyEventType type, Account *acc)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
	acc(acc)
{
}

MyEvent::~MyEvent()
{

}
