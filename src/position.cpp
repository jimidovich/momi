#include <QMap>
#include <QString>

#include "ThostFtdcUserApiStruct.h"

#include "include/position.h"

Position::Position()
{
}

Position::Position(CThostFtdcInvestorPositionDetailField *df, const SymbolList &sl)
{
	sym = df->InstrumentID;
	brokerID = df->BrokerID;
	investorID = df->InvestorID;
	hedgeFlag = mymap::hedgeFlag_char.at(df->HedgeFlag);
	direction = mymap::direction_char.at(df->Direction);
	//direction = direction_char['0'];
	openDate = df->OpenDate;
	tradeID = df->TradeID;
	pos = df->Volume;
	entryPrice = df->OpenPrice;
	tradingDay = df->TradingDay;
	exchangeID = df->ExchangeID;
	//dailyCloseProfit = df->CloseProfitByDate;
	tradeCloseProfit = df->CloseProfitByTrade;	// CHECK when need, not sure correct
	dailyPositionProfit = df->PositionProfitByDate;
	tradePositionProfit = df->PositionProfitByTrade;
	margin = df->Margin;
	marginRate = df->MarginRateByMoney;
	lastSttlPrice = df->LastSettlementPrice;
	sttlPrice = df->SettlementPrice;
	closeVolume = df->CloseVolume;

	multiple = sl[sym].info->VolumeMultiple;
	positionDate = (openDate == tradingDay ? 'T' : 'H');

	aggPositionID = QString("%1-%2-%3").arg(sym.c_str()).arg(direction).arg(positionDate);
	positionID = QString("%1-%2-%3-%4-%5").arg(sym.c_str()).arg(direction).arg(positionDate)
		.arg(openDate.c_str()).arg(tradeID.c_str());
	side = (direction == 'L' ? 1 : -1);
	dailyCloseProfit = side*(df->CloseAmount - closeVolume*multiple*(positionDate == 'H' ? lastSttlPrice : entryPrice));
	grossPnl = dailyCloseProfit + dailyPositionProfit;
	commission = 0; // TODO: calc
	netPnl = grossPnl - commission;
}

Position::Position(CThostFtdcTradeField *td, const SymbolList &sl)
{
	sym = td->InstrumentID;
	brokerID = td->BrokerID;
	investorID = td->InvestorID;
	hedgeFlag = mymap::hedgeFlag_char.at(td->HedgeFlag);
	direction = mymap::direction_char.at(td->Direction);
	openDate = td->TradingDay;
	tradeID = td->TradeID;
	pos = td->Volume;
	entryPrice = td->Price;
	tradingDay = td->TradingDay;
	exchangeID = td->ExchangeID;
	dailyCloseProfit = 0;
	tradeCloseProfit = 0;
	dailyPositionProfit = 0;
	tradePositionProfit = 0;
	margin = 0;
	marginRate = 0;
	lastSttlPrice = 0;
	sttlPrice = 0;
	closeVolume = 0;

	multiple = sl[sym].info->VolumeMultiple;
	positionDate = (openDate == tradingDay ? 'T' : 'H');
	aggPositionID = QString("%1-%2-%3").arg(sym.c_str()).arg(direction).arg(positionDate);
	positionID = QString("%1-%2-%3-%4-%5").arg(sym.c_str()).arg(direction).arg(positionDate)
		.arg(openDate.c_str()).arg(tradeID.c_str());
	side = (direction == 'L' ? 1 : -1);
	grossPnl = dailyCloseProfit + dailyPositionProfit;
	commission = 0; // TODO: calc
	netPnl = grossPnl - commission;
}

Position::~Position()
{
}

void Position::updateOnTrade(CThostFtdcTradeField *td)
{
	if (sym == std::string(td->InstrumentID))
	{
		switch (td->OffsetFlag)
		{
		case '0': // TODO:can trade add to new position??
			break;
		default: // close old
			auto deltaPos = std::min(pos, td->Volume);
			pos -= deltaPos;
			closeVolume += deltaPos;
			tradeCloseProfit += deltaPos*multiple*(direction == 'L' ? 1 : -1) * (td->Price - entryPrice);
			dailyCloseProfit += deltaPos*multiple*(direction == 'L' ? 1 : -1) * (td->Price - (positionDate == 'T' ? entryPrice : lastSttlPrice));
			break;
		}
	}
}

void Position::mtm(double price)
{
	dailyPositionProfit = side*pos*multiple*(price - (positionDate == 'H' ? lastSttlPrice : entryPrice));
	tradePositionProfit = side*pos*multiple*(price - entryPrice);
	margin = pos*multiple*price*marginRate;
	grossPnl = dailyCloseProfit + dailyPositionProfit;
	netPnl = grossPnl - commission;
}

AggPosition::AggPosition()
{
}

AggPosition::AggPosition(CThostFtdcInvestorPositionField *pf, const SymbolList &sl)
{
	sym = pf->InstrumentID;
	brokerID = pf->BrokerID;
	investorID = pf->InvestorID;
	direction = mymap::posiDirection_char.at(pf->PosiDirection);
	hedgeFlag = mymap::hedgeFlag_char.at(pf->HedgeFlag);
	positionDate = mymap::positionDate_char.at(pf->PositionDate);
	ydPos = pf->YdPosition;
	tdPos = pf->TodayPosition;
	pos = pf->Position;
	openVolume = pf->OpenVolume;
	closVolume = pf->CloseVolume;
	openAmount = pf->OpenAmount;
	closeAmount = pf->CloseAmount;
	positionCost = pf->PositionCost;
	margin = pf->UseMargin;
	marginRate = pf->MarginRateByMoney;
	commission = pf->Commission;
	closeProfit = pf->CloseProfit;
	positionProfit = pf->PositionProfit;
	preSttlPrice = pf->PreSettlementPrice;
	sttlPrice = pf->SettlementPrice;
	tradingDay = pf->TradingDay;
	dailyCloseProfit = pf->CloseProfitByDate;
	tradeCloseProfit = pf->CloseProfitByTrade;

	multiple = sl[sym].info->VolumeMultiple;
	avgCostPrice = (pos == 0 ? 0 : positionCost / pos / multiple);
	aggPositionID = QString("%1-%2-%3").arg(sym.c_str()).arg(direction).arg(positionDate);
	side = (direction == 'L' ? 1 : -1);
	grossPnl = closeProfit + positionProfit;
	netPnl = grossPnl - commission;
}

AggPosition::AggPosition(const Position &p)
{
	sym = p.sym;
	brokerID = p.brokerID;
	investorID = p.investorID;
	direction = p.direction;
	hedgeFlag = p.hedgeFlag;
	positionDate = p.positionDate;
	pos = p.pos;
	ydPos = (positionDate == 'H' ? pos : 0);
	tdPos = (positionDate == 'T' ? pos : 0);
	openVolume = p.pos + p.closeVolume;
	closVolume = p.closeVolume;
	openAmount = 0;
	closeAmount = 0;
	margin = p.margin;
	marginRate = p.marginRate;
	commission = p.commission; // TODO: commission calc
	closeProfit = p.dailyCloseProfit;
	positionProfit = 0;
	preSttlPrice = p.lastSttlPrice;
	sttlPrice = p.sttlPrice;
	tradingDay = p.tradingDay;
	dailyCloseProfit = p.dailyCloseProfit;
	tradeCloseProfit = p.tradeCloseProfit;

	multiple = p.multiple;
	positionCost = p.pos * multiple * (positionDate == 'H' ? p.lastSttlPrice : p.entryPrice);
	avgCostPrice = (pos == 0 ? 0 : positionCost / pos / multiple);
	aggPositionID = QString("%1-%2-%3").arg(sym.c_str()).arg(direction).arg(positionDate);
	side = (direction == 'L' ? 1 : -1);
	grossPnl = closeProfit + positionProfit;
	netPnl = grossPnl - commission;
}

AggPosition::~AggPosition()
{

}

void AggPosition::addPosition(const Position &p, int addsub /*= 1*/)
{
	if (p.sym == sym && p.direction == direction && p.positionDate == positionDate)
	{
		pos += addsub * p.pos;
		closeProfit += addsub * p.dailyCloseProfit;  // bydate or bytrade??
		dailyCloseProfit += addsub * p.dailyCloseProfit;
		tradeCloseProfit += addsub * p.tradeCloseProfit;

		if (positionDate == 'H')
		{
			ydPos += addsub * p.pos;
			positionCost += addsub * p.pos * p.lastSttlPrice * multiple;

		}
		else
		{
			tdPos += addsub * p.pos;
			positionCost += addsub * p.pos * p.entryPrice * multiple;
		}
		avgCostPrice = (pos == 0 ? 0 : positionCost / pos / multiple);
	}
}

void AggPosition::delPosition(const Position &p)
{
	addPosition(p, -1);
}

void AggPosition::mtm(double price)
{
	positionProfit = side*pos*multiple*(price - avgCostPrice);
	margin = pos*multiple*price*marginRate;
	grossPnl = closeProfit + positionProfit;
	netPnl = grossPnl - commission;
}

NetPosition::NetPosition()
{
}

NetPosition::NetPosition(const AggPosition &ap)
{
	sym = ap.sym;
	brokerID = ap.brokerID;
	investorID = ap.investorID;
	addAggPosition(ap);
}

NetPosition::~NetPosition()
{
}

void NetPosition::addAggPosition(const AggPosition &ap)
{
	netPos += ap.pos*ap.side;
	longPos += (ap.side == 1 ? ap.pos : 0);
	shortPos += (ap.side == -1 ? ap.pos : 0);
	closeProfit += ap.closeProfit;
	positionProfit += ap.positionProfit;
	commission += ap.commission;
	grossPnl += ap.grossPnl;
	netPnl += ap.netPnl;
	multiple = ap.multiple;
	positionCost += ap.positionCost*ap.side;
	avgCostPrice = (netPos == 0 ? 0 : positionCost / netPos / multiple);
	//avgEntryPrice
}
