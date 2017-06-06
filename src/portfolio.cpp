//#include "include/ThostFtdcUserApiDataType.h"
#include "include/ThostFtdcUserApiStruct.h"

#include "include/portfolio.h"

using namespace std;

QString getTimeMsec(std::string time, int ms)
{
    QString msec(QString::number(ms));
    switch (msec.length())
    {
    case 1:
        msec = "00" + msec;
        break;
    case 2:
        msec = "0" + msec;
        break;
    default:
        break;
    }
    return QString(time.c_str()) + "." + msec;
}

Portfolio::Portfolio()
{
}

Portfolio::~Portfolio()
{
}

void Portfolio::onCtpEvent(CtpEvent ev)
{
    switch (ev.type)
    {
    case PositionEvent:
    {
        break;
    }
    case PositionDetailEvent:
    {
        if (pfValue.brokerID == ev.posDetail.BrokerID)  // or more pricise condition
        {
//            isInPosStream = true;
            if (beginUpdate)
            {
                Position p(ev.posDetail, dataHub->symInfoTable);
                posList.insert(p.positionID, p);
            }
        }
        if (ev.isLast)
        {
            aggPosList = constructAggPosList(posList);
            netPosList = constructNetPosList(aggPosList);
//            isInPosStream = false;
            beginUpdate = true;
            numNetPosRows = netPosList.size();  // just for posTableView behavior use
            emit updatePosTable();
        }
        break;
    }
    case AccountInfoEvent:
    {
        pfValue = PortfolioValue(&(ev.accInfo));
        break;
    }
    case ContractInfoEvent:
    {
        break;
    }
    case MarketEvent:
    {
        string sym = ev.mkt.InstrumentID;

        for (auto i : aggPosList) {
            if (i.sym == sym) {
                evalAccount(pfValue, aggPosList, dataHub->symMktTable);	// Choose which price to MTM
                time = ev.mkt.UpdateTime;
                millisec = ev.mkt.UpdateMillisec;

//                auto accEvent = new MyEvent(AccountUpdateEvent, &acc);
//                QCoreApplication::postEvent(dispatcher, accEvent);

                emit updateAccTable();
                emit updatePosTable();
            }
        }
        break;
    }
    case TradeEvent:
    {
        updatePosOnTrade(aggPosList, posList, ev.trade, dataHub->symInfoTable);
        netPosList.clear();
        netPosList = constructNetPosList(aggPosList);
        break;
    }
    case OrderEvent:
    {
        break;
    }
    default:
        break;
    }
}

AggPosList Portfolio::constructAggPosList(PosList pList)
{
    AggPosList apList;
    for (auto &pos : pList) {
        auto id = pos.aggPositionID;
        if (!apList.keys().contains(id))
            apList.insert(id, AggPosition(pos));
        else
            apList[id].addPosition(pos);
    }
    return apList;
}

NetPosList Portfolio::constructNetPosList(AggPosList apList)
{
    NetPosList npList;
    for (auto &pos : apList) {
        auto id = QString(pos.sym.c_str());
        if (!npList.keys().contains(id))
            npList.insert(id, NetPosition(pos));
        else
            npList[id].addAggPosition(pos);
    }
    return npList;
}

void Portfolio::updatePosOnTrade(AggPosList &al, PosList &pl, CThostFtdcTradeField &td, SymInfoTable &info)
{
    switch (td.OffsetFlag)
    {
    case THOST_FTDC_OF_Open:
    {
        auto p = Position(td, info);
        pl.insert(p.positionID, p);
        auto id = p.aggPositionID;
        if (!al.keys().contains(id))
            al.insert(id, AggPosition(p));
        else
            al[id].addPosition(p);
        break;
    }
    case THOST_FTDC_OF_Close:
    case THOST_FTDC_OF_CloseToday:
    case THOST_FTDC_OF_CloseYesterday:
    {
        auto tdcpy = td;
        // QMap is sorted by key
        for (auto &p : pl) {
            //Position& p = pos;
            if (p.sym == string(tdcpy.InstrumentID) &&
                p.direction != mymap::direction_char.at(tdcpy.Direction) &&
                p.pos > 0 &&
                (td.OffsetFlag == THOST_FTDC_OF_Close ||
                    (td.OffsetFlag == THOST_FTDC_OF_CloseToday && p.positionDateCategory == 'T') ||
                    (td.OffsetFlag == THOST_FTDC_OF_CloseYesterday && p.positionDateCategory == 'H')))
            {
                auto deltaPos = min(p.pos, tdcpy.Volume);  //check whether seperated to more than 1 trade? no. OnRtnTrade once.
                if (deltaPos != 0)
                {
                    al[p.aggPositionID].delPosition(p);
                    p.updateOnTrade(tdcpy);
                    al[p.aggPositionID].addPosition(p);
                }
                tdcpy.Volume -= deltaPos;  //can add: if Volume == 0, break
            }
        }
        break;
    }
    //case THOST_FTDC_OF_CloseYesterday:
    //{
    //	auto tdcpy = new CThostFtdcTradeField;
    //	memcpy(tdcpy, td, sizeof(CThostFtdcTradeField));
    //	// QMap is sorted by key
    //	auto i = pl.begin();
    //	while (i != pl.end())
    //	{
    //		Position& p = i.value();
    //		if (p.sym == string(tdcpy->InstrumentID) && p.positionDate == 'H'
    //			&& p.direction != direction2char(tdcpy->Direction))
    //		{
    //			auto deltaPos = min(p.pos, tdcpy->Volume);
    //			if (deltaPos != 0)
    //			{
    //				al[p.aggPositionID].delPosition(p);
    //				p.updateOnTrade(tdcpy);
    //				al[p.aggPositionID].addPosition(p);
    //			}
    //			tdcpy->Volume -= deltaPos;
    //		}
    //		i++;
    //	}
    //	break;
    //}
    default:
        break;
    }
}

void Portfolio::evalAccount(PortfolioValue &pfValue, AggPosList &aplist, SymMktTable &smkt)
{
//    std::mutex mu;
//    mu.lock();
    pfValue.prevBalance = pfValue.balance;
    pfValue.positionProfit = 0;
    pfValue.closeProfit = 0;
    pfValue.grossPnl = 0;
    pfValue.netPnl = 0;
    for (auto &ap : aplist)
    {
        if (smkt.find(ap.sym) != smkt.end())
        {
            if (smkt[ap.sym].Volume == 0)	// TODO: get proper way for night session
                ap.mtm(smkt[ap.sym].PreSettlementPrice);
            else
                ap.mtm(smkt[ap.sym].LastPrice);
        }
        pfValue.positionProfit += ap.positionProfit;
        pfValue.closeProfit += ap.dailyCloseProfit;
        pfValue.grossPnl += ap.grossPnl;
        pfValue.netPnl += ap.netPnl;
    }
    netPosList.clear();
    netPosList = constructNetPosList(aggPosList);
    //acc.balance = acc.cashBalance + acc.netPnl;
    pfValue.balance = pfValue.cashBalance + pfValue.grossPnl - pfValue.commission;
    pfValue.available = pfValue.balance - pfValue.margin; //TODO: add frozen margin etc.
    pfValue.lastUpdateTime = getTimeMsec(time, millisec).toStdString();
//    mu.unlock();
}

void Portfolio::evalOnTick(PortfolioValue &acc, AggPosList &aplist, CThostFtdcDepthMarketDataField &mkt)
{
    for (auto &ap : aplist) {
        if (ap.sym == mkt.InstrumentID)
        {
            acc.positionProfit -= ap.positionProfit;
            acc.closeProfit -= ap.dailyCloseProfit;
            acc.grossPnl -= ap.grossPnl;
            acc.netPnl -= ap.netPnl;

            if (mkt.Volume == 0)	// TODO: get proper way for night session
                ap.mtm(mkt.PreSettlementPrice);
            else
                ap.mtm(mkt.LastPrice);

            acc.positionProfit += ap.positionProfit;
            acc.closeProfit += ap.dailyCloseProfit;
            acc.grossPnl += ap.grossPnl;
            acc.netPnl += ap.netPnl;
        }
    }

    netPosList.clear();
    netPosList = constructNetPosList(aggPosList);
    //acc.balance = acc.cashBalance + acc.netPnl;
    acc.balance = acc.cashBalance + acc.grossPnl - acc.commission;
    acc.available = acc.balance - acc.margin; //TODO: add frozen margin etc.
}

PortfolioValue::PortfolioValue()
{
}

PortfolioValue::PortfolioValue(CThostFtdcTradingAccountField *af)
{
    brokerID = af->BrokerID;
    accountID = af->AccountID;
    tradingDay = af->TradingDay;
    preDeposit = af->PreDeposit;
    preBalance = af->PreBalance;
    preMargin = af->PreMargin;
    balance = af->Balance;
    available = af->Available;
    deposit = af->Deposit;
    withdraw = af->Withdraw;
    margin = af->CurrMargin;
    commission = af->Commission;
    closeProfit = af->CloseProfit;
    positionProfit = af->PositionProfit;

    cashBalance = preBalance - withdraw + deposit;
    grossPnl = closeProfit + positionProfit;
    netPnl = grossPnl - commission;  // balance = cashBalance + netPnl
}


void Portfolio::printNetPos()
{
//    QString msg;
//    int fw = -12; // field width left-aligned
//    msg = QString("%1%2%3%4%5%6%7\n")
//            .arg("Symbol", fw)
//            .arg("LastPx", fw)
//            .arg("NetPos", fw)
//            .arg("AvgCost", fw)
//            .arg("PosPnL", fw)
//            .arg("NetPnL", fw)
//            .arg("Time");
//    for (auto sym : netPosList.keys()) {
//        if (dataHub->symMktTable.find(sym.toStdString()) != dataHub->symMktTable.end()) {
//            auto pos = netPosList[sym];
//            msg += QString("%1%2%3%4%5%6%7\n")
//                    .arg(sym, fw)
//                    .arg(dataHub->symMktTable.at(sym.toStdString()).LastPrice, fw) //todo: to Subscribe if not in MD..
//                    .arg(pos.netPos, fw)
//                    .arg(pos.avgCostPrice, fw)
//                    .arg(pos.positionProfit, fw)
//                    .arg(pos.netPnl, fw)
//                    .arg(getTimeMsec(time, millisec));
//        }
//    }
//    emit sendToPosMonitor();
}

void Portfolio::printAcc()
{
//    QString msg;
//    int fw = -12; // field width left-aligned
//    msg = QString("%1%2%3%4%5%6%7\n")
//            .arg("Balance", fw)
//            .arg("Grs.PnL", fw)
//            .arg("R.PnL", fw)
//            .arg("Unr.PnL", fw)
//            .arg("Margin", fw)
//            .arg("Comm", fw)
//            .arg("Time");
//    msg += QString("%1%2%3%4%5%6%7\n")
//            .arg(pfValue.balance, fw, 'f', 0)
//            .arg(pfValue.netPnl, fw)
//            .arg(pfValue.closeProfit, fw)
//            .arg(pfValue.positionProfit, fw)
//            .arg(pfValue.margin, fw)
//            .arg(pfValue.commission, fw)
//            .arg(getTimeMsec(time, millisec));
//    emit sendToAccMonitor();
}
