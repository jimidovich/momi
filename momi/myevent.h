#ifndef MYEVENT_H
#define MYEVENT_H

#include <QObject>
#include <QEvent>

#include "ThostFtdcUserApiStruct.h"
#include "ThostFtdcUserApiDataType.h"

#include "struct.h"

class Account;

const QEvent::Type MY_CUSTOM_EVENT = static_cast<QEvent::Type>(QEvent::User + 100);

enum EnumMyEventType
{
	FeedEvent,
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
	CThostFtdcDepthMarketDataField *feed{ nullptr };
	CThostFtdcTradingAccountField *accInfo{ nullptr };
	CThostFtdcInstrumentField *contractInfo{ nullptr };
	CThostFtdcInvestorPositionField *pos{ nullptr };
	CThostFtdcInvestorPositionDetailField *posDetail{ nullptr };
	CThostFtdcTradeField *trade{ nullptr };
	CThostFtdcOrderField *order{ nullptr };
	Account *acc{ nullptr };
	bool isLast{ true };
};

#endif // MYEVENT_H
