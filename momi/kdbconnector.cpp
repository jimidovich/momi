#include "qdebug.h"
#include "qthread.h"
#include "qapplication.h"
#include "qelapsedtimer.h"
#include "qtimezone.h"

#include "portfolio.h"
#include "kdbconnector.h"
#include "struct.h"
#include <string>

using namespace std;
using namespace spdlog::level;

KdbConnector::KdbConnector(QObject *parent)
    : QObject(parent)
{
    setLogger("kdbconn");
    handle = khp((S)hostname, (I)port);
    //checkTableExist();
}

KdbConnector::KdbConnector(std::string consoleName, QObject * parent)
    : QObject(parent)
{
    setLogger(consoleName);
    handle = khp((S)hostname, (I)port);
}

KdbConnector::~KdbConnector()
{
}

//void KdbConnector::onFeedEvent(CThostFtdcDepthMarketDataField * feed)
//{
//	insertFeed(feed);
//}

void KdbConnector::onEvent(QEvent *ev)
{
    auto myev = (MyEvent*)ev;
    switch (myev->myType)
    {
    case FeedEvent:
        //insertFeed(ev->getFeed());
        //qDebug() << QThread::currentThreadId() << "+++++++++++++kdb";
        insertFeed(myev->feed);
        break;
    case ContractInfoEvent:
        //writeContractInfo(ev->getContractInfo());
        insertContractInfo(myev->contractInfo);
        break;
    case AccountUpdateEvent:
        insertAccount(*myev->acc);
    default:
        break;
    }
}

void KdbConnector::setTradingDay(const char *tday)
{
    tradingDay = tday;
}

void KdbConnector::checkTableExist()
{
    string s{ "key `" };
    s += string(tableName);
    auto key = k(handle, (S)s.c_str(), (K)0);
    if (!key->t)
    {
        createMktTable();
    }
    s = "key `" + string(cinfoName);
    key = k(handle, (S)s.c_str(), (K)0);
    if (!key->t)
    {
        createInfoTable();
    }
}

void KdbConnector::createMktTable()
{
    k(-handle, (S)qStatementToCreateTable(tableName).c_str(), (K)0);
}

void KdbConnector::createInfoTable()
{
    k(-handle, (S)qStatementToCreateTable(cinfoName).c_str(), (K)0);
}

void KdbConnector::insertContractInfo(CThostFtdcInstrumentField *info)
{
    mutex.lock();
    //K Date = date2qDate("20160722");
    K Contract = ks(info->InstrumentID);
    K Exchange = ks(info->ExchangeID);
    K Name = ks(info->InstrumentName);
    K Product = ks(info->ProductID);
    K ProductClass = ks((S)mymap::productClass_string.at(info->ProductClass).c_str());
    K DelivYear = ki(info->DeliveryYear);
    K DelivMonth = ki(info->DeliveryMonth);
    K MaxMktOrderVolume = ki(info->MaxMarketOrderVolume);
    K MinMktOrderVolume = ki(info->MinMarketOrderVolume);
    K MaxLmtOrderVolume = ki(info->MaxLimitOrderVolume);
    K MinLmtOrderVolume = ki(info->MinLimitOrderVolume);
    K VolumeMultiples = ki(info->VolumeMultiple);
    K PriceTick = kf(info->PriceTick);
    K CreateDate = date2qDate(info->CreateDate);
    K OpenDate = date2qDate(info->OpenDate);
    K ExpireDate = date2qDate(info->ExpireDate);
    K StartDelivDate = date2qDate(info->StartDelivDate);
    K EndDelivDate = date2qDate(info->EndDelivDate);
    K LifePhase = ks((S)mymap::instLiftPhase_string.at(info->InstLifePhase).c_str());
    K IsTrading = ki(info->IsTrading);
    K PositionType = ks((S)mymap::positionType_string.at(info->PositionType).c_str());
    K PositionDateType = kc(mymap::positionDate_char.at(info->PositionDateType));
    K LongMarginRatio = kf(info->LongMarginRatio);
    K ShortMarginRatio = kf(info->ShortMarginRatio);
    K MMSA = kc(mymap::maxMarginSideAlgo_char.at(info->MaxMarginSideAlgorithm));

    K data = knk(25, Contract, Exchange, Name, Product, ProductClass, DelivYear, DelivMonth,
        MaxMktOrderVolume, MinMktOrderVolume, MaxLmtOrderVolume, MinLmtOrderVolume,
        VolumeMultiples, PriceTick, CreateDate, OpenDate, ExpireDate, StartDelivDate, EndDelivDate,
        LifePhase, IsTrading, PositionType, PositionDateType, LongMarginRatio, ShortMarginRatio, MMSA);

    //self-maintained table
    k(-handle, "insert", ks("ContractsInfo"), data, (K)0);

    //kdb+tick
    k(-handle, ".u.upd", ks((S)cinfoName), data, (K)0);

    //qDebug() << info->InstrumentID;
    mutex.unlock();
}

void KdbConnector::insertFeed(CThostFtdcDepthMarketDataField * feed)
{
    QElapsedTimer t;
    t.start();
    mutex.lock();
    K data, tspan, Contract, Exchange, Date, Time, Last, Bid1, BidSize1, \
        Ask1, AskSize1, Volume, Turnover, OpenInterest, AvgPrice, Open, High, Low, \
        UpperLimit, LowerLimit, PreSettlement, PreClose, PreOpenInterest, Close, Settlement;
    tspan = k(handle, ".z.N", (K)0);
    Contract = ks(feed->InstrumentID);
    Exchange = ks(feed->ExchangeID);
    Date = date2qDate(feed->TradingDay);
    Time = qMakeTime(feed->UpdateTime, feed->UpdateMillisec);
    Last = kf(feed->LastPrice);
    Bid1 = kf(feed->BidPrice1);
    BidSize1 = ki(feed->BidVolume1);
    Ask1 = kf(feed->AskPrice1);
    AskSize1 = ki(feed->AskVolume1);
    Volume = ki(feed->Volume);
    Turnover = kf(feed->Turnover);
    OpenInterest = kf(feed->OpenInterest);
    AvgPrice = kf(feed->AveragePrice);
    Open = kf(feed->OpenPrice);
    High = kf(feed->HighestPrice);
    Low = kf(feed->LowestPrice);
    /*UpperLimit = kf(feed->UpperLimitPrice);
    LowerLimit = kf(feed->LowerLimitPrice);
    PreSettlement = kf(feed->PreSettlementPrice);
    PreClose = kf(feed->PreClosePrice);
    PreOpenInterest = kf(feed->PreOpenInterest);
    Close = kf(feed->ClosePrice);
    Settlement = kf(feed->SettlementPrice);*/

    data = knk(15, Contract, Date, Time, Last, Bid1, BidSize1, Ask1, AskSize1,
        Volume, Turnover, OpenInterest, AvgPrice, Open, High, Low);
    k(-handle, ".u.upd", ks((S)tableName), data, (K)0);
    //k(handle, "", (K)0); // flush
    //qDebug() << ++countTick << QTime::currentTime();
    //QApplication::processEvents();
    //QApplication::sendPostedEvents(this);
    mutex.unlock();
    //qDebug() << "kdb" << t.elapsed() << "ms";
}

void KdbConnector::insertAccount(const Account &acc)
{
    K data, id, balance, pnl, rPnL, unrPnL, available, margin;
    id = ks((S)acc.accountID.c_str());
    balance = kf(acc.balance);
    pnl = kf(acc.netPnl);
    rPnL = kf(acc.closeProfit);
    unrPnL = kf(acc.positionProfit);
    available = kf(acc.available);
    margin = kf(acc.margin);

    data = knk(7, id, balance, pnl, rPnL, unrPnL, available, margin);
    k(-handle, ".u.upd", ks("account"), data, (K)0);
}

std::string KdbConnector::qStatementToCreateTable(const char *tbName)
{
    string qstr(tbName);
    if (qstr == tableName)
    {
        qstr = qstr + ":([] systime:`timespan$(); Contract:`symbol$(); Exchange:`symbol$(); Date:`date$(); Time:`time$(); Last:`float$(); \
            Bid1:`float$(); BidSize1:`int$(); Ask1:`float$(); AskSize1:`int$();	Volume:`int$(); Turnover:`float$(); OpenInterest:`float$(); \
            AvgPrice:`float$(); Open:`float$(); High:`float$(); Low:`float$())";
    }
    else if (qstr == cinfoName)
    {
        qstr = qstr + ":([Date:`date$(); Contract:`symbol$()] Exchange:`symbol$(); Name:`symbol$(); Product:`symbol$(); ProductClass:`symbol$(); DelivYear:`int$(); DelivMonth:`int$(); \
            MaxMktOrderVolume:`int$(); MinMktOrderVolume:`int$(); MaxLmtOrderVolume:`int$(); MinLmtOrderVolume:`int$(); \
            VolumeMultiples:`int$(); PriceTick:`float$(); CreateDate:`date$(); OpenDate:`date$(); ExpireDate:`date$(); StartDelivDate:`date$(); EndDelivDate:`date$(); \
            LifePhase:`symbol$(); IsTrading:`int$(); PositionType:`symbol$(); PositionDateType:`char$(); LongMarginRatio:`float$(); ShortMarginRatio:`float$(); MMSA:`char$())";
    }
    return qstr;
}

K KdbConnector::qMakeTime(char *time, int millisec)
{
    int h = atoi(time);
    int m = atoi(time + 3);
    int s = atoi(time + 6);
    int tq = ((h * 60 + m) * 60 + s) * 1000 + millisec;

    return kt(tq);
}

K KdbConnector::qDataList(K tspan, K time)
{
    return knk(5, tspan, ks("rb"), kd(0), time, kf(2055.3));
}

K KdbConnector::date2qDate(char *date)
{
    string dstr(date);
    int y = atoi(dstr.substr(0, 4).c_str());
    int m = atoi(dstr.substr(4, 2).c_str());
    int d = atoi(dstr.substr(6, 2).c_str());
    return kd(ymd(y, m, d));
}

void KdbConnector::setLogger(std::string consoleName)
{
    //console = spdlog::get("console");
    kdb_logger = spdlog::rotating_logger_mt(consoleName+"_logger", "logs/"+consoleName+"_log", 1024 * 1024 * 5, 3);
    kdb_logger->flush_on(spdlog::level::info);
    consoleName.resize(7, ' ');
    console = spdlog::stdout_color_mt(consoleName);
    console->set_pattern("[%H:%M:%S.%f] [%n] [%L] %v");
    g_logger = spdlog::get("file_logger");
}

TickSubscriber::TickSubscriber()
{
    port = 5010;
    handle = khp((S)hostname, (I)port);
    if (!handle || handle < 0)
    {
        logger(warn, "Cannot connect to kdb server.");
    }
}

TickSubscriber::TickSubscriber(std::string consoleName)
    :KdbConnector(consoleName)
{
    port = 5010;
    handle = khp((S)hostname, (I)port);
    if (!handle || handle < 0)
    {
        logger(warn, "Cannot connect to kdb server.");
    }
}

void TickSubscriber::subscribe()
{
    K r = k(handle, ".u.sub[`market;`]", (K)0);
    //if (!r)
        //qDebug() << "Network error";
        //logger(warn, "Network error");
    //else
        //qDebug() << "Subscribed";
        //logger(warn, "Subscribed");

    K tb, colname, coldata;
    while (1)
    {
        r = k(handle, (S)0);
        if (r)
        {
            if (r->t == 0)
            {
                qDebug() << QThread::currentThreadId() << "*****************" << kK(r)[1]->s;
                tb = kK(r)[2]->k;
                colname = kK(tb)[0];
                coldata = kK(tb)[1];
                qDebug() << kS(kK(coldata)[1])[0] << kF(kK(coldata)[4])[0] << kI(kK(coldata)[3])[0] << "-----" << ++countTick;
            }
            r0(r);
        }
    }
    kclose(handle);
}

class KdbSubscriberThread :public QThread {
private:
    void run()
    {
        TickSubscriber tickSub;
        qDebug() << "tickSub thread ID: " << currentThreadId();
        tickSub.subscribe();
    }
};
