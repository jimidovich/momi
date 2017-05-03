#include <QCoreApplication>
#include <QDebug>
#include <QVector>
#include <QMap>
#include <QTimer>
#include <QThread>

#include "ThostFtdcUserApiDataType.h"

#include "kalman.h"
#include "portfolio.h"
#include "struct.h"
#include "rm.h"
#include "oms.h"
#include "trader.h"

using namespace std;

QString getTimeMsec(string time, int ms)
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

Portfolio::Portfolio(Trader *td, OMS *oms, Kalman *kf)
{
    this->trader = td;
    this->oms = oms;
    this->kf = kf;
    tradingDay = td->getTradingDay();
    //QTimer::singleShot(5000, this->trader, SLOT(Trader::ReqQryInvestorPositionDetail()));
    //TODO: need workflow to set commission rate
}

Portfolio::~Portfolio()
{
}

int Portfolio::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return netPosList.size();
    //return 5;
}

int Portfolio::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 4;
}

QVariant Portfolio::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0: return QString("Instrument");
            case 1: return QString("Price");
            case 2: return QString("Net Pos");
            case 3: return QString("Net PnL");
            }
        }
    }
    return QVariant();
}

QVariant Portfolio::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (role == Qt::DisplayRole) {
        int row = index.row();
        int col = index.column();
        if (row < netPosList.size()) {
            auto i = netPosList.begin();
            switch (col)
            {
            case 0: return QString((i + row).value().sym.c_str());
            case 1:
                if (symList.contains((i + row).value().sym))
                {
                    if (symList[(i + row).value().sym].mkt != nullptr)
                        return QString::number(symList[(i + row).value().sym].mkt->LastPrice);
                }
                else
                    return "";
                //case 1: return "test";
            case 2: return QString::number((i + row).value().netPos);
            case 3: return QString::number((i + row).value().netPnl, 'f', 2);
            default:
                break;
            }
        }
        else
            return "test";
    }
    return QVariant();
}

void Portfolio::updatePosTable()
{
    //auto i = netPosList.begin();
    //auto role = Qt::EditRole;
    //for (int row = 0; row < netPosList.size(); ++row) {
    //	for (int col = 0; col < 3; ++col) {
    //		switch (col)
    //		{
    //		case 0: setData(index(row, col), QString((i + row).value().sym.c_str()), role);
    //		case 1: setData(index(row, col), QString::number((i + row).value().netPos), role);
    //		case 2: setData(index(row, col), QString::number((i + row).value().netPos), role);
    //		}
    //	}
    //}
    beginResetModel();
    endResetModel();
    //auto topleft = createIndex(0, 0);
    //auto bottomright = createIndex(netPosList.size(), 2);
    //emit dataChanged(topleft, bottomright);
}

void Portfolio::setDispatcher(Dispatcher *ee)
{
    dispatcher = ee;
}

void Portfolio::setOMS(OMS *oms)
{
    this->oms = oms;
}

void Portfolio::setPosTableView(QTableView *ptv)
{
    postableview = ptv;
}

Trader * Portfolio::getTrader()
{
    return trader;
}

void Portfolio::onEvent(QEvent *ev)
{
    auto myev = (MyEvent*)ev;
    switch (myev->myType)
    {
    case PositionEvent:
    {
        /*if (myev->pos->BrokerID[0] != 0)
        {
            isInPosStream = true;
            if (beginUpdate)
            {
            }
        }
        else
        {
            isInPosStream = false;
            beginUpdate = true;
        }*/
        break;
    }
    case PositionDetailEvent:
    {
        if (myev->posDetail->BrokerID[0] != 0)
        {
            isInPosStream = true;
            if (beginUpdate)
            {
                Position p(myev->posDetail, symList);
                posList.insert(p.positionID, p);
            }
        }
        else
        {
            aggPosList = constructAggPosList(posList);
            netPosList = constructNetPosList(aggPosList);
            isInPosStream = false;
            beginUpdate = true;
            // Reset Tableview rows
            beginResetModel();
            endResetModel();
        }
        break;
    }
    case AccountInfoEvent:
    {
        acc = Account(myev->accInfo);
        break;
    }
    case ContractInfoEvent:
    {
        string sym = myev->contractInfo->InstrumentID;
        if (symList.keys().contains(sym))
        {
            symList[sym].info = myev->contractInfo;
        }
        else
        {
            Symbol s = { nullptr, myev->contractInfo };
            symList.insert(sym, s);
        }
        break;
    }
    case FeedEvent:
    {
        string sym = myev->feed->InstrumentID;
        symList[sym].mkt = myev->feed;

        evalAccount(acc, aggPosList, symList);	// Choose which price to MTM
        time = myev->feed->UpdateTime;
        millisec = myev->feed->UpdateMillisec;

        auto accEvent = new MyEvent(AccountUpdateEvent, &acc);
        QCoreApplication::postEvent(dispatcher, accEvent);
        printAcc();
        printNetPos();
        updatePosTable();
        //postableview->update();
        //qDebug() << QThread::currentThreadId() << "++++++++++++++++++++++ pf";

        kf->onFeed(myev);
        oms->handleTargets();

        break;
    }
    case TradeEvent:
    {
        updatePosOnTrade(aggPosList, posList, myev->trade, symList);
        netPosList.clear();
        netPosList = constructNetPosList(aggPosList);
        oms->onEvent(ev);
        break;
    }
    case OrderEvent:
    {
        oms->onEvent(ev);
        break;
    }
    default:
        break;
    }
}

void Portfolio::printNetPos()
{
    QString msg;
    msg.append("Symbol").append("\tNetPos").append("\tAvgCost").append("\tPosPnL").append("\tNetPnL").append("\ttime\n");
    for (auto sym : netPosList.keys()) {
        auto pos = netPosList[sym];
        msg.append(sym).append("\t").append(QString::number(pos.netPos)).append("\t");
        msg.append(QString::number(pos.avgCostPrice)).append("\t").append(QString::number(pos.positionProfit));
        msg.append("\t").append(QString("%1").arg(pos.netPnl));
        msg.append("\t").append(getTimeMsec(time, millisec)).append("\n");

    }
    emit sendToPosMonitor(msg);
}

void Portfolio::printAcc()
{
    QString msg;
    msg.append("Balance\t").append("Gross PnL\t").append("R.PnL\t").append("Unr.PnL\t").append("Margin\t").append("Comm\t").append("time\n");
    msg.append(QString("%1").arg(int(acc.balance)));
    msg.append("\t").append(QString("%1").arg(acc.netPnl));
    msg.append("\t").append(QString("%1").arg(acc.closeProfit));
    msg.append("\t").append(QString("%1").arg(acc.positionProfit));
    msg.append("\t").append(QString("%1").arg(acc.margin));
    msg.append("\t").append(QString("%1").arg(acc.commission));
    msg.append("\t").append(getTimeMsec(time, millisec));
    emit sendToAccMonitor(msg);
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

void Portfolio::updatePosOnTrade(AggPosList &al, PosList &pl, CThostFtdcTradeField *td, SymbolList &sl)
{
    switch (td->OffsetFlag)
    {
    case THOST_FTDC_OF_Open:
    {
        auto p = Position(td, sl);
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
        auto tdcpy = new CThostFtdcTradeField;
        memcpy(tdcpy, td, sizeof(CThostFtdcTradeField));
        // QMap is sorted by key
        for (auto &p : pl) {
            //Position& p = pos;
            if (p.sym == string(tdcpy->InstrumentID) &&
                p.direction != mymap::direction_char.at(tdcpy->Direction) &&
                p.pos > 0 &&
                (td->OffsetFlag == THOST_FTDC_OF_Close ||
                    (td->OffsetFlag == THOST_FTDC_OF_CloseToday && p.positionDate == 'T') ||
                    (td->OffsetFlag == THOST_FTDC_OF_CloseYesterday && p.positionDate == 'H')))
            {
                auto deltaPos = min(p.pos, tdcpy->Volume);
                if (deltaPos != 0)
                {
                    al[p.aggPositionID].delPosition(p);
                    p.updateOnTrade(tdcpy);
                    al[p.aggPositionID].addPosition(p);
                }
                tdcpy->Volume -= deltaPos;
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

void Portfolio::evalAccount(Account &acc, AggPosList &aplist, SymbolList &sl)
{
    acc.positionProfit = 0;
    acc.closeProfit = 0;
    acc.grossPnl = 0;
    acc.netPnl = 0;
    for (auto &ap : aplist)
    {
        if (sl[ap.sym].mkt != nullptr)
        {
            if (sl[ap.sym].mkt->Volume == 0)	// TODO: get proper way for night session
                ap.mtm(sl[ap.sym].mkt->PreSettlementPrice);
            else
                ap.mtm(sl[ap.sym].mkt->LastPrice);
        }
        acc.positionProfit += ap.positionProfit;
        acc.closeProfit += ap.dailyCloseProfit;
        acc.grossPnl += ap.grossPnl;
        acc.netPnl += ap.netPnl;
    }
    netPosList.clear();
    netPosList = constructNetPosList(aggPosList);
    //acc.balance = acc.cashBalance + acc.netPnl;
    acc.balance = acc.cashBalance + acc.grossPnl - acc.commission;
    acc.available = acc.balance - acc.margin; //TODO: add frozen margin etc.
}

Account::Account()
{
}

Account::Account(CThostFtdcTradingAccountField *af)
{
    brokerID = af->BrokerID;
    accountID = af->AccountID;
    preDeposit = af->PreDeposit;
    preBalance = af->PreBalance;
    preMargin = af->PreMargin;
    balance = af->Balance;
    available = af->Available;
    deposit = af->Deposit;
    withdraw = af->Withdraw;
    margin = af->CurrMargin;
    commission = af->Commission;
    cashBalance = preBalance - withdraw + deposit;
    closeProfit = af->CloseProfit;
    positionProfit = af->PositionProfit;
    grossPnl = closeProfit + positionProfit;
    netPnl = grossPnl - commission;
}
