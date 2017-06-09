#ifndef MYEVENT_H
#define MYEVENT_H

#include <chrono>

#include "ThostFtdcUserApiStruct.h"
#include "ThostFtdcUserApiDataType.h"

#include "struct.h"
//#include "portfolio.h"

enum EnumMyEventType
{
    MarketEvent,
	AccountInfoEvent,
	ContractInfoEvent,
	PositionEvent,
	PositionDetailEvent,
	OrderEvent,
	TradeEvent,
    QryTradeEvent,
	AccountUpdateEvent
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
    std::chrono::steady_clock::time_point ts = std::chrono::steady_clock::now();


    CtpEvent() {}
    CtpEvent(CThostFtdcDepthMarketDataField *p) {type = MarketEvent; mkt = *p;}
    CtpEvent(CThostFtdcTradingAccountField *p) {type = AccountInfoEvent; accInfo = *p;}
    CtpEvent(CThostFtdcInstrumentField *p) {type = ContractInfoEvent; contractInfo = *p;}
    CtpEvent(CThostFtdcInvestorPositionField *p) {type = PositionEvent; pos = *p;}
    CtpEvent(CThostFtdcInvestorPositionDetailField *p) {type = PositionDetailEvent; posDetail = *p;}
    CtpEvent(CThostFtdcOrderField *p) {type = OrderEvent; order = *p;}
    CtpEvent(CThostFtdcTradeField *p) {type = TradeEvent; trade = *p;}
    CtpEvent(CThostFtdcTradeField *p, char qflag) {type = QryTradeEvent; trade = *p;}
};

#endif // MYEVENT_H
