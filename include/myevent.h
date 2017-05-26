#ifndef MYEVENT_H
#define MYEVENT_H

#include <QObject>
#include <QEvent>

#include "ThostFtdcUserApiStruct.h"
#include "ThostFtdcUserApiDataType.h"

#include "struct.h"
#include "portfolio.h"

const QEvent::Type MY_CUSTOM_EVENT = static_cast<QEvent::Type>(QEvent::User + 100);

enum EnumMyEventType
{
    MarketEvent,
	AccountInfoEvent,
	ContractInfoEvent,
	PositionEvent,
	PositionDetailEvent,
	OrderEvent,
	TradeEvent,
	AccountUpdateEvent
};

class MyEvent : public QEvent {
public:
	MyEvent(EnumMyEventType type, CThostFtdcDepthMarketDataField *mkt);
	MyEvent(EnumMyEventType type, CThostFtdcTradingAccountField *accInfo);
	MyEvent(EnumMyEventType type, CThostFtdcInstrumentField *contractInfo);
	MyEvent(EnumMyEventType type, CThostFtdcInvestorPositionField *pos);
	MyEvent(EnumMyEventType type, CThostFtdcInvestorPositionDetailField *posDetail);
	MyEvent(EnumMyEventType type, CThostFtdcTradeField *trade);
	MyEvent(EnumMyEventType type, CThostFtdcOrderField *order);
	MyEvent(EnumMyEventType type, Account *acc);
	~MyEvent();

	EnumMyEventType myType;
    CThostFtdcDepthMarketDataField *mkt{ nullptr };
	CThostFtdcTradingAccountField *accInfo{ nullptr };
	CThostFtdcInstrumentField *contractInfo{ nullptr };
	CThostFtdcInvestorPositionField *pos{ nullptr };
	CThostFtdcInvestorPositionDetailField *posDetail{ nullptr };
	CThostFtdcTradeField *trade{ nullptr };
	CThostFtdcOrderField *order{ nullptr };
	Account *acc{ nullptr };
	bool isLast{ true };
};


class MyEvent1 {
public:
    MyEvent1(EnumMyEventType type, CThostFtdcDepthMarketDataField *mkt);
    MyEvent1(EnumMyEventType type, CThostFtdcTradingAccountField *accInfo);
    MyEvent1(EnumMyEventType type, CThostFtdcInstrumentField *contractInfo);
    MyEvent1(EnumMyEventType type, CThostFtdcInvestorPositionField *pos);
    MyEvent1(EnumMyEventType type, CThostFtdcInvestorPositionDetailField *posDetail);
    MyEvent1(EnumMyEventType type, CThostFtdcTradeField *trade);
    MyEvent1(EnumMyEventType type, CThostFtdcOrderField *order);
    MyEvent1(EnumMyEventType type, Account *acc);
    ~MyEvent1();

    EnumMyEventType eventType;
    CThostFtdcDepthMarketDataField *mkt{ nullptr };
    CThostFtdcTradingAccountField *accInfo{ nullptr };
    CThostFtdcInstrumentField *contractInfo{ nullptr };
    CThostFtdcInvestorPositionField *pos{ nullptr };
    CThostFtdcInvestorPositionDetailField *posDetail{ nullptr };
    CThostFtdcTradeField *trade{ nullptr };
    CThostFtdcOrderField *order{ nullptr };
    Account *acc{ nullptr };
    bool isLast{ true };
};


struct CtpEvent {
    EnumMyEventType type;
    union {
        CThostFtdcDepthMarketDataField mkt;
        CThostFtdcTradingAccountField accInfo;
        CThostFtdcInstrumentField contractInfo;
        CThostFtdcInvestorPositionField pos;
        CThostFtdcInvestorPositionDetailField posDetail;
        CThostFtdcTradeField trade;
        CThostFtdcOrderField order;
//        Account acc;
    };
    bool isLast{ false };

    CtpEvent() {}
    CtpEvent(CThostFtdcDepthMarketDataField *p) {type = MarketEvent; mkt = *p;}
    CtpEvent(CThostFtdcTradingAccountField *p) {type = AccountInfoEvent; accInfo = *p;}
    CtpEvent(CThostFtdcInstrumentField *p) {type = ContractInfoEvent; contractInfo = *p;}
    CtpEvent(CThostFtdcInvestorPositionField *p) {type = PositionEvent; pos = *p;}
    CtpEvent(CThostFtdcInvestorPositionDetailField *p) {type = PositionDetailEvent; posDetail = *p;}
    CtpEvent(CThostFtdcTradeField *p) {type = TradeEvent; trade = *p;}
    CtpEvent(CThostFtdcOrderField *p) {type = OrderEvent; order = *p;}
};

#endif // MYEVENT_H
