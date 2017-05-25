#include "include/myevent.h"


MyEvent::MyEvent(EnumMyEventType type, CThostFtdcDepthMarketDataField *mkt)
	: QEvent(MY_CUSTOM_EVENT),
	myType(type),
    mkt(mkt)
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
    switch (myType) {
    case MarketEvent:
        delete mkt;
        break;
    case AccountInfoEvent:
        delete accInfo;
        break;
    case ContractInfoEvent:
        delete contractInfo;
        break;
    case PositionEvent:
        delete pos;
        break;
    case PositionDetailEvent:
        delete posDetail;
        break;
    case OrderEvent:
        delete order;
        break;
    case TradeEvent:
        delete trade;
        break;
    default:
        break;
    }

}

MyEvent1::MyEvent1(EnumMyEventType type, CThostFtdcDepthMarketDataField *mkt)
{
    this->mkt = new CThostFtdcDepthMarketDataField;
    memcpy(this->mkt, mkt, sizeof(CThostFtdcDepthMarketDataField));
}

MyEvent1::MyEvent1(EnumMyEventType type, CThostFtdcTradingAccountField *accInfo)
{
    this->accInfo = new CThostFtdcTradingAccountField;
    memcpy(this->accInfo, accInfo, sizeof(CThostFtdcDepthMarketDataField));
}

MyEvent1::MyEvent1(EnumMyEventType type, CThostFtdcInstrumentField *contractInfo)
{
    this->contractInfo = new CThostFtdcInstrumentField ;
    memcpy(this->contractInfo, contractInfo, sizeof(CThostFtdcInstrumentField ));
}

MyEvent1::MyEvent1(EnumMyEventType type, CThostFtdcInvestorPositionField *pos)
{
    this->pos = new CThostFtdcInvestorPositionField;
    memcpy(this->pos, pos, sizeof(CThostFtdcInvestorPositionField));
}

MyEvent1::MyEvent1(EnumMyEventType type, CThostFtdcInvestorPositionDetailField *posDetail)
{
    this->posDetail = new CThostFtdcInvestorPositionDetailField;
    memcpy(this->posDetail, posDetail, sizeof(CThostFtdcInvestorPositionDetailField));
}

MyEvent1::MyEvent1(EnumMyEventType type, CThostFtdcTradeField *trade)
{
    this->trade = new CThostFtdcTradeField;
    memcpy(this->trade, trade, sizeof(CThostFtdcTradeField));
}

MyEvent1::MyEvent1(EnumMyEventType type, CThostFtdcOrderField *order)
{
    this->order = new CThostFtdcOrderField;
    memcpy(this->order, order, sizeof(CThostFtdcOrderField));
}

MyEvent1::MyEvent1(EnumMyEventType type, Account *acc)
{
    this->acc = new Account;
    memcpy(this->acc, acc, sizeof(Account));
}

MyEvent1::~MyEvent1()
{
    switch (eventType) {
    case MarketEvent:
        delete mkt;
        break;
    case AccountInfoEvent:
        delete accInfo;
        break;
    case ContractInfoEvent:
        delete contractInfo;
        break;
    case PositionEvent:
        delete pos;
        break;
    case PositionDetailEvent:
        delete posDetail;
        break;
    case OrderEvent:
        delete order;
        break;
    case TradeEvent:
        delete trade;
        break;
    default:
        break;
    }
}
