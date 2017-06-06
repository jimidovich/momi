#include <chrono>
#include <set>
#include <thread>

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QDialog>
#include <QTextEdit>
#include <QTimer>
//#include <QColor>
#include <QtConcurrent/QtConcurrent>

#include "spdlog/spdlog.h"
//#include "fmt/format.h"

#include "include/trader.h"
#include "include/myevent.h"
#include "include/position.h"
#include "include/struct.h"
#include "include/datahub.h"

using namespace std;
using namespace spdlog::level;

Trader::Trader(QObject *parent) : QObject(parent)
{
    init();
    //tdapi->Join();
}

Trader::Trader(const string &frontAddress,
               const string &brokerID,
               const string &userID,
               const string &password)
    : QObject(Q_NULLPTR),
      FrontAddress(frontAddress),
      BROKER_ID(brokerID),
      USER_ID(userID),
      PASSWORD(password)
{
    init();
}

Trader::~Trader() {

}

void Trader::init()
{
    setLogger();

    logger(info, "Initializing Trader");
    reqConnect();
    logger(info, "Initializing Trader Finished");
}

void Trader::reqConnect()
{
    tdapi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    tdapi->RegisterSpi(this);
    tdapi->SubscribePublicTopic(THOST_TERT_RESTART);
    tdapi->SubscribePrivateTopic(THOST_TERT_QUICK);  //Shit RESUME mode cannot get OnRtnOrder and OnRtnTrade
    char *front = new char[FrontAddress.length() + 1];
    strcpy(front, FrontAddress.c_str());
    tdapi->RegisterFront(front);
    delete[] front;
    tdapi->Init();
}

int Trader::login()
{
//    logger(info, "--> Trader ReqLogin...");
//    emit sendToTraderMonitor("--> Trader Req Login...");
    auto loginField = new CThostFtdcReqUserLoginField();
    strcpy(loginField->BrokerID, BROKER_ID.c_str());
    strcpy(loginField->UserID, USER_ID.c_str());
    strcpy(loginField->Password, PASSWORD.c_str());
    int ret = tdapi->ReqUserLogin(loginField, ++nRequestID);
    showApiReturn(ret, "--> Trader ReqLogin", "Trader ReqLogin Failed: ");
    return ret;
}

int Trader::logout()
{
//    logger(info, "Trader Req Logout...");
//    emit sendToTraderMonitor("--> Trader Req Logout...");
    auto logoutField = new CThostFtdcUserLogoutField();
    strcpy(logoutField->BrokerID, BROKER_ID.c_str());
    strcpy(logoutField->UserID, USER_ID.c_str());
    int ret = tdapi->ReqUserLogout(logoutField, ++nRequestID);
    showApiReturn(ret, "--> Trader ReqLogout:", "Trader ReqLogout Failed: ");
    return ret;
}

void Trader::OnFrontConnected()
{
    /*chrono::seconds sleepDuration(1);
    this_thread::sleep_for(sleepDuration);*/
    logger(info, "Trader Front Connected.");
    emit sendToTraderMonitor("Trader Front Connected.", Qt::darkGreen);
    this->login();
}

void Trader::OnFrontDisconnected(int nReason)
{
    string msg = "Front Disconnected. Reason: ";
    switch (nReason)
    {
    case 0x1001:
        msg += u8"网络读失败";
        break;
    case 0x1002:
        msg += u8"网络写失败";
        break;
    case 0x2001:
        msg += u8"接收心跳超时";
        break;
    case 0x2002:
        msg += u8"发送心跳失败";
        break;
    case 0x2003:
        msg += u8"收到错误报文";
        break;
    default:
        break;
    }
//    logger(err, msg.toLocal8Bit());
    logger(err, msg.c_str());
    emit sendToTraderMonitor(msg.c_str(), Qt::red);
}

void Trader::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "ReqUserLogin Failed: ")) {
        //if ((pRspInfo != nullptr && pRspInfo->ErrorID == 0) && pRspUserLogin != nullptr) {
        // TODO: watch MaxOrderRef returned in diff sessions.
        nMaxOrderRef = atoi(pRspUserLogin->MaxOrderRef);
        FrontID = pRspUserLogin->FrontID;
        SessionID = pRspUserLogin->SessionID;
        tradingDay = pRspUserLogin->TradingDay;

//        string preSpaces = "\n" + string(" ").repeated(31);
        string preSpaces = fmt::format("\n{:>31}", ' ');
        string msg = "Trader Login Successful.";
        msg += preSpaces + "TradingDay = " + pRspUserLogin->TradingDay;
        msg += preSpaces + "LoginTime  = " + pRspUserLogin->LoginTime;
        msg += preSpaces + "SystemName = " + pRspUserLogin->SystemName;
        msg += preSpaces + "UserID     = " + pRspUserLogin->UserID;
        msg += preSpaces + "BrokerID   = " + pRspUserLogin->BrokerID;
        msg += preSpaces + "SessionID  = " + to_string(pRspUserLogin->SessionID);
        msg += preSpaces + "FrontID    = " + to_string(pRspUserLogin->FrontID);
        msg += preSpaces + "INETime    = " + pRspUserLogin->INETime;
        msg += preSpaces + "SHFE Time  = " + pRspUserLogin->SHFETime;
        msg += preSpaces + "DCE  Time  = " + pRspUserLogin->DCETime;
        msg += preSpaces + "CZCE Time  = " + pRspUserLogin->CZCETime;
        msg += preSpaces + "FFEX Time  = " + pRspUserLogin->FFEXTime;
        emit sendToTraderMonitor(msg.c_str(), Qt::green);
        logger(info, msg.c_str());

        // login workflow #1
        isLoginWorkflow = true;
        if (isLoginWorkflow)
            QtConcurrent::run(timerReq, this, SLOT(ReqQrySettlementInfo()));
    }
}

void Trader::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "Trader Logout Failed: ")) {
        logger(info, "Trader Logout Success");
        emit sendToTraderMonitor("Trader Logout Success", Qt::darkCyan);
    }
}

void Trader::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspSettlementInfoConfirm: ")) {
        string msg = fmt::format("Settlement Info Confirmed. ConfirmDate={} ConfirmTime={}", pSettlementInfoConfirm->ConfirmDate, pSettlementInfoConfirm->ConfirmTime);
        logger(info, msg.c_str());
        emit sendToTraderMonitor(msg.c_str());
    }
}

void Trader::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQrySettlementInfo: ")) {
        if (isNewSettlementInfo) {
            strSettlementInfo = "";
            isNewSettlementInfo = false;
        }
        if (pSettlementInfo != nullptr)
            strSettlementInfo += pSettlementInfo->Content;
        if (bIsLast) {
            isNewSettlementInfo = true;
            if (strSettlementInfo == "") {
                logger(critical, "No Settlement Info Retrieved.");
                emit sendToTraderMonitor("No Settlement Info Retrieved.");
            }
            else {
                logger(info, "Settlement Info Retrieved.");
                emit sendToTraderMonitor("Settlement Info Retrieved.");
            }

            // login workflow #2
            if (isLoginWorkflow)
                QtConcurrent::run(timerReq, this, SLOT(ReqQrySettlementInfoConfirm()));
        }
    }
}

void Trader::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    //ReqSettlementInfoConfirm();
    if (!isErrorRspInfo(pRspInfo, "RspQrySettlementInfoConfirm: ")) {
		if (pSettlementInfoConfirm != nullptr) {
            string msg = fmt::format("RspQrySettlementInfoConfirm: ConfirmDate={} ConfirmTime={}", pSettlementInfoConfirm->ConfirmDate, pSettlementInfoConfirm->ConfirmTime);
            logger(info, msg.c_str());
            emit sendToTraderMonitor(msg.c_str());
//			if (std::string(pSettlementInfoConfirm->ConfirmDate) != tradingDay) {
        }
        else {
            ReqSettlementInfoConfirm();  // need not to wait 1 sec?
        }
    }
    // login workflow #3
    if (isLoginWorkflow)
        QtConcurrent::run(timerReq, this, SLOT(ReqQryInstrument()));
}

void Trader::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "Thost RspOrderInsert Failed: "))
        logger(info, "Thost RspOrderInsert Success: OrderRef={}", pInputOrder->OrderRef);
}

void Trader::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "Thost RspOrderAction Failed: "))
        logger(info, "Thost RspOrderAction Success: OrderRef={}", pInputOrderAction->OrderRef);
}

void Trader::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryOrder: ")) {
        if (pOrder != nullptr) {
            string msg = fmt::format("OrdRef={}, {:<6}, St={}, Of={}, Dir={}, LmtPx={}, Ttl={}, Org={}, {}, exch={}, sysID={}, sessID={}, frntID={}",
                                     pOrder->OrderRef,
                                     pOrder->InstrumentID,
                                     mymap::orderStatus_string.at(pOrder->OrderStatus),
                                     mymap::offsetFlag_string.at(pOrder->CombOffsetFlag[0]),
                                     mymap::direction_char.at(pOrder->Direction),
                                     pOrder->LimitPrice,
                                     pOrder->VolumeTotal,
                                     pOrder->VolumeTotalOriginal,
                                     QString::fromLocal8Bit(pOrder->StatusMsg).toStdString(),
                                     pOrder->ExchangeID,
                                     pOrder->OrderSysID,
                                     pOrder->SessionID,
                                     pOrder->FrontID);
            emit sendToTraderMonitor(msg.c_str());

            dataHub->eventQueue.post(CtpEvent(pOrder));  //???
        }
        if (bIsLast) {
            logger(info, "Qry Order Finished.");
            emit sendToTraderMonitor("Query Order Finished.");
        }
    }
}

void Trader::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryTrade: ")) {
        if (pTrade != nullptr) {
            string msg = fmt::format("{:<6} TradeID={}, Dir={}, Offset={}, Px={}, Vol={}, Time={}",
                                     pTrade->InstrumentID,
                                     pTrade->TradeID,
                                     mymap::direction_char.at(pTrade->Direction),
                                     mymap::offsetFlag_string.at(pTrade->OffsetFlag),
                                     pTrade->Price,
                                     pTrade->Volume,
                                     pTrade->TradeTime);
            emit sendToTraderMonitor(msg.c_str());
        }
        if (bIsLast) {
            logger(info, "Qry Trade Finished.");
            emit sendToTraderMonitor("Qry Trade Finished.");
        }
    }
}

void Trader::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryPosition: ")) {
        if (pInvestorPosition != nullptr) {
            string msg = fmt::format("{:<6} Dir={}, Pos={}, PnL={}, CloseAmt={}, Margin={}",
                                     pInvestorPosition->InstrumentID,
                                     mymap::direction_char.at(pInvestorPosition->PosiDirection),
                                     pInvestorPosition->Position,
                                     pInvestorPosition->PositionProfit,
                                     pInvestorPosition->CloseAmount,
                                     pInvestorPosition->UseMargin);
            emit sendToTraderMonitor(msg.c_str());

            CtpEvent ev(pInvestorPosition);
            ev.isLast = bIsLast;
            dataHub->eventQueue.post(ev);
        }
        if (bIsLast) {
            logger(info, "Qry InvestorPosition Finished");
            emit sendToTraderMonitor("Qry InvestorPosition Finished.");
        }
    }
}

void Trader::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryPositionDetail: ")) {
        if (pInvestorPositionDetail != nullptr) {
            string msg = fmt::format("{:<6} Dir={}, Pos={}, PnL={}, CloseAmt={}, Margin={}",
                                     pInvestorPositionDetail->InstrumentID,
                                     mymap::direction_char.at(pInvestorPositionDetail->Direction),
                                     pInvestorPositionDetail->Volume,
                                     pInvestorPositionDetail->PositionProfitByDate,
                                     pInvestorPositionDetail->CloseVolume,
                                     pInvestorPositionDetail->Margin);
            emit sendToTraderMonitor(msg.c_str());

            CtpEvent ev(pInvestorPositionDetail);
            ev.isLast = bIsLast;
            dataHub->eventQueue.post(ev);
        }
        if (bIsLast) {
            logger(info, "Qry InvestorPositionDetail Finished");
            emit sendToTraderMonitor("Qry InvestorPositionDetail Finished.");
        }
    }
}

void Trader::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryTradingAccount: ")) {
        if (bIsLast) {
            string msg("Trading Account");
            string preSpaces = fmt::format("\n{:>31}", ' ');
            msg += preSpaces + "Balance    = " + to_string(pTradingAccount->Balance);
            msg += preSpaces + "Available  = " + to_string(pTradingAccount->Available);
            msg += preSpaces + "CurrMargin = " + to_string(pTradingAccount->CurrMargin);
            msg += preSpaces + "Reserve    = " + to_string(pTradingAccount->Reserve);
            logger(info, msg.c_str());
            emit sendToTraderMonitor(msg.c_str());

            dataHub->eventQueue.post(CtpEvent(pTradingAccount));

            // login workflow #5
            if (isLoginWorkflow) {
                QtConcurrent::run(timerReq, this, SLOT(ReqQryInvestorPositionDetail()));
                isLoginWorkflow = false;
            }
        }
    }
}

void Trader::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryInstrument: ")) {
        if (pInstrument != nullptr) {
            dataHub->eventQueue.post(CtpEvent(pInstrument));

            if (bIsLast) {
                // login workflow #4
                if (isLoginWorkflow)
                    QtConcurrent::run(timerReq, this, SLOT(ReqQryTradingAccount()));
            }
        }
    }
}

void Trader::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // TODO: RETURN NULL
}

void Trader::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryDepthMarketData: ")) {
        dataHub->eventQueue.post(CtpEvent(pDepthMarketData));
    }
}

void Trader::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if (pOrder != nullptr) {
        string msg = fmt::format("OnRtnOrder: OrdRef={}, {:<6}, St={}, Of={}, Dir={}, LmtPx={}, Ttl={}, Org={}, {}, exch={}, sysID={}, sessID={}, frntID={}",
                                 pOrder->OrderRef,
                                 pOrder->InstrumentID,
                                 mymap::orderStatus_string.at(pOrder->OrderStatus),
                                 mymap::offsetFlag_string.at(pOrder->CombOffsetFlag[0]),
                                 mymap::direction_char.at(pOrder->Direction),
                                 pOrder->LimitPrice,
                                 pOrder->VolumeTotal,
                                 pOrder->VolumeTotalOriginal,
                                 QString::fromLocal8Bit(pOrder->StatusMsg).toStdString(),
                                 pOrder->ExchangeID,
                                 pOrder->OrderSysID,
                                 pOrder->SessionID,
                                 pOrder->FrontID);
        logger(info, msg.c_str());
        emit sendToTraderMonitor(msg.c_str());

        dataHub->eventQueue.post(CtpEvent(pOrder));
    } else
        logger(err, "OnRtnOrder nullptr or null data");
}

void Trader::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    if (pTrade != nullptr) {
        string msg = fmt::format("{:<6} TradeID={}, Dir={}, Offset={}, Px={}, Vol={}, Time={}",
                                 pTrade->InstrumentID,
                                 pTrade->TradeID,
                                 mymap::direction_char.at(pTrade->Direction),
                                 mymap::offsetFlag_string.at(pTrade->OffsetFlag),
                                 pTrade->Price,
                                 pTrade->Volume,
                                 pTrade->TradeTime);
        logger(info, msg.c_str());
        emit sendToTraderMonitor(msg.c_str());

        dataHub->eventQueue.post(CtpEvent(pTrade));
    } else
        logger(err, "OnRtnTrade nullptr or null data");
}

void Trader::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    string msg = fmt::format("ErrRtnOrderInsert: OrderRef={} ", pInputOrder->OrderRef);
    isErrorRspInfo(pRspInfo, msg.c_str());
}

void Trader::OnErrRtnOrderAction(CThostFtdcOrderActionField * pOrderAction, CThostFtdcRspInfoField * pRspInfo)
{
    string msg = fmt::format("ErrRtnOrderAction: OrderRef={} ", pOrderAction->OrderRef);
    isErrorRspInfo(pRspInfo, msg.c_str());
}

int Trader::ReqSettlementInfoConfirm()
{
    auto info = new CThostFtdcSettlementInfoConfirmField();
    strcpy(info->BrokerID, BROKER_ID.c_str());
    strcpy(info->InvestorID, USER_ID.c_str());
    int ret = tdapi->ReqSettlementInfoConfirm(info, ++nRequestID);
    showApiReturn(ret, "--> ReqSettlementInfoConfirm", "--x ReqSettlementInfoConfirm Failed");
    return ret;
}

int Trader::ReqQrySettlementInfoConfirm()
{
    auto info = new CThostFtdcQrySettlementInfoConfirmField();
    strcpy(info->BrokerID, BROKER_ID.c_str());
    strcpy(info->InvestorID, USER_ID.c_str());
    int ret = tdapi->ReqQrySettlementInfoConfirm(info, ++nRequestID);
    showApiReturn(ret, "--> ReqQrySettlementInfoConfirm", "--x ReqQrySettlementInfoConfirm Failed");
    return ret;
}

int Trader::ReqQrySettlementInfo(string TradingDay)
{
    auto info = new CThostFtdcQrySettlementInfoField();
    strcpy(info->BrokerID, BROKER_ID.c_str());
    strcpy(info->InvestorID, USER_ID.c_str());
    strcpy(info->TradingDay, TradingDay.c_str());
    int ret = tdapi->ReqQrySettlementInfo(info, ++nRequestID);
    showApiReturn(ret, "--> ReqQrySettlementInfo", "--x ReqQrySettlementInfo Failed");
    return ret;
}

int Trader::ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder)
{
    int ret = tdapi->ReqOrderInsert(pInputOrder, ++nRequestID);
    showApiReturn(ret, "--> ReqOrderInsert", "--x ReqOrderInsert Sent Error");
    return ret;
}

// Limit Order
int Trader::ReqOrderInsert(string InstrumentID, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, double Price, int Volume)
{
    auto order = new CThostFtdcInputOrderField();
    strcpy(order->BrokerID, BROKER_ID.c_str());
    strcpy(order->UserID, USER_ID.c_str());
    strcpy(order->InvestorID, USER_ID.c_str());
    strcpy(order->OrderRef, QString::number(++nMaxOrderRef).toStdString().c_str());
    order->ContingentCondition = Immediately;
    order->ForceCloseReason = NotForceClose;
    order->IsAutoSuspend = false;
    order->MinVolume = 1;
    order->OrderPriceType = LimitPrice;
    order->TimeCondition = GFD;
    order->UserForceClose = false;
    order->VolumeCondition = AV;
    order->CombHedgeFlag[0] = Speculation;

    strcpy(order->InstrumentID, InstrumentID.c_str());
    order->CombOffsetFlag[0] = OffsetFlag;
    order->Direction = Direction;
    order->LimitPrice = Price;
    order->VolumeTotalOriginal = Volume;

    int ret = tdapi->ReqOrderInsert(order, ++nRequestID);
    showApiReturn(ret, "--> LimitOrderInsert", "--x LimitOrderInsert Sent Error");
    return ret;
}

// Market Order
int Trader::ReqOrderInsert(string InstrumentID, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, int Volume)
{
    auto order = new CThostFtdcInputOrderField();
    strcpy(order->BrokerID, BROKER_ID.c_str());
    strcpy(order->UserID, USER_ID.c_str());
    strcpy(order->InvestorID, USER_ID.c_str());
    //strcpy(order->OrderRef, QString::number(++nMaxOrderRef).toStdString().c_str());
    order->ContingentCondition = Immediately;
    order->ForceCloseReason = NotForceClose;
    order->IsAutoSuspend = false;
    order->MinVolume = 1;
    order->OrderPriceType = AnyPrice;
    order->TimeCondition = IOC; //立即完成，否则撤销
    order->UserForceClose = false;
    order->VolumeCondition = AV;
    order->CombHedgeFlag[0] = Speculation;

    strcpy(order->InstrumentID, InstrumentID.c_str());
    order->CombOffsetFlag[0] = OffsetFlag;
    order->Direction = Direction;
    order->LimitPrice = 0;
    order->VolumeTotalOriginal = Volume;

    int ret = tdapi->ReqOrderInsert(order, ++nRequestID);
    showApiReturn(ret, "--> MarketOrderInsert", "--x MarketOrderInsert Sent Error");
    return ret;
}

// Condition Order
int Trader::ReqOrderInsert(string InstrumentID, EnumContingentConditionType ConditionType, double conditionPrice,
    EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, EnumOrderPriceTypeType PriceType, double Price, int Volume)
{
    auto order = new CThostFtdcInputOrderField();
    strcpy(order->BrokerID, BROKER_ID.c_str());
    strcpy(order->UserID, USER_ID.c_str());
    strcpy(order->InvestorID, USER_ID.c_str());
    //strcpy(order->OrderRef, QString::number(++nMaxOrderRef).toStdString().c_str());
    order->ForceCloseReason = NotForceClose;
    order->IsAutoSuspend = false;
    order->MinVolume = 1;
    order->OrderPriceType = AnyPrice;
    order->TimeCondition = IOC; //立即完成，否则撤销
    order->UserForceClose = false;
    order->VolumeCondition = AV;
    order->CombHedgeFlag[0] = Speculation;

    strcpy(order->InstrumentID, InstrumentID.c_str());
    order->ContingentCondition = ConditionType;
    order->StopPrice = conditionPrice;
    order->CombOffsetFlag[0] = OffsetFlag;
    order->Direction = Direction;
    order->OrderPriceType = PriceType;
    order->LimitPrice = Price;  // Effective only if PriceType == LimitPrice
    order->VolumeTotalOriginal = Volume;

    int ret = tdapi->ReqOrderInsert(order, ++nRequestID);
    showApiReturn(ret, "--> MarketOrderInsert", "--x MarketOrderInsert Sent Error");
    return ret;
}

int Trader::ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction)
{
    int ret = tdapi->ReqOrderAction(pInputOrderAction, ++nRequestID);
    showApiReturn(ret, "--> ReqOrderAction", "--x ReqOrderAction Sent Error");
    return ret;
}

int Trader::ReqOrderAction(string InstrumentID, string OrderRef)
{
    auto action = new CThostFtdcInputOrderActionField();
    action->ActionFlag = THOST_FTDC_AF_Delete;
    strcpy(action->BrokerID, BROKER_ID.c_str());
    strcpy(action->InvestorID, USER_ID.c_str());
    action->FrontID = FrontID;
    action->SessionID = SessionID;
    strcpy(action->InstrumentID, InstrumentID.c_str());
    //strcpy(action->OrderRef, OrderRef.c_str());
    // TODO: check self set orderRef's format!
    if (OrderRef != "")
    {
        // OrderRef has format "       xxxxx", length set 12 here.
        // Shit this turns to  "xxx         " char[12]
        if (OrderRef.length() > 12) return -100;
//        strcpy(action->OrderRef, QString::fromStdString(OrderRef).rightJustified(12, ' ').toStdString().c_str());
        strcpy(action->OrderRef, OrderRef.c_str());
    }
    return ReqOrderAction(action);
}

int Trader::ReqOrderAction(string InstrumentID, int FrontID, int SessionID, string OrderRef, string ExchangeID, string OrderSysID)
{
    auto action = new CThostFtdcInputOrderActionField();
    action->ActionFlag = THOST_FTDC_AF_Delete;
    strcpy(action->BrokerID, BROKER_ID.c_str());
    strcpy(action->InstrumentID, InstrumentID.c_str());
    strcpy(action->InvestorID, USER_ID.c_str());

    if (FrontID != 0)
        action->FrontID = FrontID;
    if (SessionID != 0)
        action->SessionID = SessionID;
    if (OrderRef != "")
    {
        // OrderRef has format "       xxxxx", length set 12 here.
        // Shit this turns to  "xxx         " char[12]
        if (OrderRef.length() > 12) return -100;
//        strcpy(action->OrderRef, QString::fromStdString(OrderRef).rightJustified(12, ' ').toStdString().c_str());
        strcpy(action->OrderRef, OrderRef.c_str());
    }
    if (ExchangeID != "")
        strcpy(action->ExchangeID, ExchangeID.c_str());
    if (OrderSysID != "")
    {
        // OrderSysID has format "     xxxxx", length set 20 here.  //Holy shit, 12 or 20? sample feedback shows "     xxxx" 12 length in char[20]
        if (OrderRef.length() > 20) return -101;
//        strcpy(action->OrderRef, QString::fromStdString(OrderRef).rightJustified(20, ' ').toStdString().c_str());
        strcpy(action->OrderSysID, OrderSysID.c_str());
    }
    return ReqOrderAction(action);
}

int Trader::ReqQryOrder(string InstrumentID, string ExchangeID, string timeStart, string timeEnd, string OrderSysID)
{
    auto order = new CThostFtdcQryOrderField();
    strcpy(order->BrokerID, BROKER_ID.c_str());
    strcpy(order->InstrumentID, InstrumentID.c_str());
    strcpy(order->InvestorID, USER_ID.c_str());
    strcpy(order->ExchangeID, ExchangeID.c_str());
    strcpy(order->OrderSysID, OrderSysID.c_str());
    strcpy(order->InsertTimeStart, timeStart.c_str());
    strcpy(order->InsertTimeEnd, timeEnd.c_str());
    int ret = tdapi->ReqQryOrder(order, ++nRequestID);
    showApiReturn(ret, "--> ReqQryOrder", "--x ReqQryOrder Failed");
    return ret;
}

int Trader::ReqQryTrade(string timeStart = "", string timeEnd = "", string InstrumentID = "", string ExchangeID = "", string TradeID = "")
{
    auto trade = new CThostFtdcQryTradeField();
    strcpy(trade->BrokerID, BROKER_ID.c_str());
    strcpy(trade->InvestorID, USER_ID.c_str());
    strcpy(trade->InstrumentID, InstrumentID.c_str());
    strcpy(trade->ExchangeID, ExchangeID.c_str());
    strcpy(trade->TradeID, TradeID.c_str());
    strcpy(trade->TradeTimeStart, timeStart.c_str());
    strcpy(trade->TradeTimeEnd, timeEnd.c_str());
    int ret = tdapi->ReqQryTrade(trade, ++nRequestID);
    showApiReturn(ret, "--> ReqQryTrade", "--x ReqQryTrade Failed");
    return ret;
}

int Trader::ReqQryDepthMarketData(string InstrumentID)
{
    auto f = new CThostFtdcQryDepthMarketDataField();
    strcpy(f->InstrumentID, InstrumentID.c_str());
    int ret = tdapi->ReqQryDepthMarketData(f, ++nRequestID);
    showApiReturn(ret, "--> ReqQryDepthMarketData", "--x ReqQryDepthMarketData Failed");
    return ret;
}

int Trader::ReqQryTradingAccount()
{
    auto acc = new CThostFtdcQryTradingAccountField();
    strcpy(acc->BrokerID, BROKER_ID.c_str());
    strcpy(acc->InvestorID, USER_ID.c_str());
    int ret = tdapi->ReqQryTradingAccount(acc, ++nRequestID);
    showApiReturn(ret, "--> ReqQryTradingAccount", "--x ReqQryTradingAccount Failed");
    return ret;
}

int Trader::ReqQryInvestorPosition(string InstrumentID)
{
    auto pos = new CThostFtdcQryInvestorPositionField();
    strcpy(pos->BrokerID, BROKER_ID.c_str());
    strcpy(pos->InvestorID, USER_ID.c_str());
    strcpy(pos->InstrumentID, InstrumentID.c_str());
    int ret = tdapi->ReqQryInvestorPosition(pos, ++nRequestID);
    showApiReturn(ret, "--> ReqQryInvestorPosition", "--x ReqQryInvestorPosition Failed");
    return ret;
}

int Trader::ReqQryInvestorPositionDetail(string InstrumentID)
{
    auto pos = new CThostFtdcQryInvestorPositionDetailField();
    strcpy(pos->BrokerID, BROKER_ID.c_str());
    strcpy(pos->InvestorID, USER_ID.c_str());
    strcpy(pos->InstrumentID, InstrumentID.c_str());
    int ret = tdapi->ReqQryInvestorPositionDetail(pos, ++nRequestID);
    showApiReturn(ret, "--> ReqQryInvestorPositionDetail", "--x ReqQryInvestorPositionDetail Failed");
    return ret;
}

int Trader::ReqQryInstrument()
{
    auto field = new CThostFtdcQryInstrumentField();
    int ret = tdapi->ReqQryInstrument(field, ++nRequestID);
    showApiReturn(ret, "--> ReqQryInstrument", "--x ReqQryInstrument Failed");
    return ret;
}

int Trader::ReqQryInstrumentMarginRate(string InstrumentID, EnumHedgeFlagType hedgeFlag)
{
    auto field = new CThostFtdcQryInstrumentMarginRateField();
    strcpy(field->BrokerID, BROKER_ID.c_str());
    strcpy(field->InvestorID, USER_ID.c_str());
    strcpy(field->InstrumentID, InstrumentID.c_str());
    field->HedgeFlag = hedgeFlag;
    int ret = tdapi->ReqQryInstrumentMarginRate(field, ++nRequestID);
    showApiReturn(ret, "--> ReqQryInstrumentMarginRate", "--x ReqQryInstrumentMarginRate Failed");
    return ret;
}

int Trader::ReqQryInstrumentCommissionRate(string InstrumentID)
{
    auto field = new CThostFtdcQryInstrumentCommissionRateField();
    strcpy(field->BrokerID, BROKER_ID.c_str());
    strcpy(field->InvestorID, USER_ID.c_str());
    strcpy(field->InstrumentID, InstrumentID.c_str());
    int ret = tdapi->ReqQryInstrumentCommissionRate(field, ++nRequestID);
    showApiReturn(ret, "--> ReqQryInstrumentCommisionRate", "--x ReqQryInstrumentCommisionRate Failed");
    return ret;
}

void Trader::handleDispatch(int tt)
{
    qDebug() << "RECEIVED EVENT signal****************";
}

void Trader::setLogger()
{
    //console = spdlog::get("console");
    console = spdlog::stdout_color_mt("trader");
    console->set_pattern("[%H:%M:%S.%f] [%L] [%n] %v");
    g_logger = spdlog::get("file_logger");
    trader_logger = spdlog::rotating_logger_mt("trader_logger", "logs/trader_log", 1024 * 1024 * 5, 3);
    //trader_logger = spdlog::daily_logger_mt("trader_logger", "logs/trader_log", 5, 0);
    trader_logger->flush_on(spdlog::level::info);
}

void Trader::timerReq(Trader *trader, const char *req)
{
    QEventLoop el;
    auto timer = new QTimer;
    QObject::connect(timer, SIGNAL(timeout()), trader, req);
    QObject::connect(timer, SIGNAL(timeout()), &el, SLOT(quit()));
    timer->setSingleShot(true);
    timer->start(1000);
    el.exec();
}

string Trader::getTradingDay()
{
    return tradingDay;
}

// Note: not restoring and showing corresponding nRequestID. can be implemented if need.
bool Trader::isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, const char *msg)
{
    bool isError = (pRspInfo) && (pRspInfo->ErrorID != 0);
    if (isError) {
        string errMsg = fmt::format("ErrorID={}, ErrorMsg={}", pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg).toStdString());
        logger(err, errMsg.c_str());
        emit sendToTraderMonitor(errMsg.c_str(), Qt::red);
    }
    return isError;
}


void Trader::showApiReturn(int ret, string outputIfSuccess, string outputIfError)
{
    const string green = "\033[32m";
    const string reset = "\033[00m";

    if (outputIfSuccess != "" || outputIfError != "") {
        string msg;
        string msg_t;
        switch (ret) {
        case 0:
            //msg = outputIfSuccess.append("0: Sent successfully ").append(QString("ReqID=%1").arg(QString::number(nRequestID)));
            msg = outputIfSuccess + fmt::format(" <Sent successfully. ReqID={}>", nRequestID);
            msg_t = green + msg + reset;
            logger(info, msg_t.c_str());
            emit sendToTraderMonitor(msg.c_str(), Qt::darkGreen);
            break;
        case -1:
            msg = outputIfSuccess + fmt::format(" <Failed, network problem. ReqID={}>", nRequestID);
            logger(err, msg.c_str());
            emit sendToTraderMonitor(msg.c_str(), Qt::red);
            break;
        case -2:
            msg = outputIfSuccess + fmt::format(" <Failed, number of unhandled request queues passes limit. ReqID={}>", nRequestID);
            logger(err, msg.c_str());
            emit sendToTraderMonitor(msg.c_str(), Qt::red);
            break;
        case -3:
            msg = outputIfSuccess + fmt::format(" <Failed, requests per sec pass limit. ReqID={}>", nRequestID);
            logger(err, msg.c_str());
            emit sendToTraderMonitor(msg.c_str(), Qt::red);
            break;
        default:
            break;
        }
    }
}

//EnumOffsetFlagType str2OffsetFlagType(string str);
//EnumDirectionType str2DirectionType(string str);

void Trader::execCmdLine(QString cmdLine)
{
    QStringList argv(cmdLine.split(" "));
    int n = argv.count();
    if (n > 0) {
        if (argv.at(0) == "i" || argv.at(0) == "ins") {
            if (n == 6) {
                string InstrumentID{ argv.at(1).toStdString() };
                //EnumOffsetFlagType OffsetFlag{ str2OffsetFlagType(argv.at(2).toStdString()) };
                std::set<string> ofStrSet = { "open", "o", "close", "c", "ct", "cy" };
                std::set<string> dirStrSet = { "buy", "b", "sell", "s" };
                EnumOffsetFlagType OffsetFlag;
                EnumDirectionType Direction;
                if (ofStrSet.find(argv.at(2).toStdString()) != ofStrSet.end())
                    OffsetFlag = mymap::string_offsetFlag.at(argv.at(2).toStdString());
                else {
                    emit sendToTraderMonitor("invalid cmd");
                    return;
                }
                if (dirStrSet.find(argv.at(3).toStdString()) != dirStrSet.end())
                    Direction = mymap::string_directionFlag.at(argv.at(3).toStdString());
                else {
                    emit sendToTraderMonitor("invalid cmd");
                    return;
                }
                bool okp, okv;
                double Price{ argv.at(4).toDouble(&okp) };
                int Volume{ argv.at(5).toInt(&okv) };
                if (okp && okv) {
                    ReqOrderInsert(InstrumentID, OffsetFlag, Direction, Price, Volume);
                }
                else {
                    emit sendToTraderMonitor("invalid cmd");
                    return;
                }
            }
        }
        else if (argv.at(0) == "infoconfirm") {
            ReqSettlementInfoConfirm();
        }
        else if (argv.at(0) == "c" || argv.at(0) == "x") {
            if (n == 4 && argv.at(1) == "sys") {
                string ExchangeID{ argv.at(2).toStdString() };
                string OrderSysID{ argv.at(3).toStdString() };

                ReqOrderAction("", 0, 0, "", ExchangeID, OrderSysID);
            }
            if (n == 4 && argv.at(1) == "ref") {
                string InstrumentID{ argv.at(2).toStdString() };
                string OrderRef{ argv.at(3).toStdString() };
                ReqOrderAction(InstrumentID, OrderRef);
            }
        }
        else if (argv.at(0) == "qorder" || argv.at(0) == "qod") {
            ReqQryOrder();
        }
        else if (argv.at(0) == "qtrade" || argv.at(0) == "qtd") {
            ReqQryTrade();
        }
        else if (argv.at(0) == "qp") {
            ReqQryInvestorPosition();
        }
        else if (argv.at(0) == "qpd") {
            ReqQryInvestorPositionDetail();
        }
        else if (argv.at(0) == "qmkt") {
            ReqQryDepthMarketData(argv.at(1).toStdString());
        }
        else if (argv.at(0) == "qcomm") {
            ReqQryInstrumentCommissionRate(argv.at(1).toStdString());
        }
        else if (argv.at(0) == "qa" || argv.at(0) == "qacc") {
            ReqQryTradingAccount();
        }
        else if (argv.at(0) == "qc" || argv.at(0) == "qinst") {
            ReqQryInstrument();
        }
        else if (argv.at(0) == "login") {
            login();
        }
        else if (argv.at(0) == "logout") {
            logout();
        }
        else if (argv.at(0) == "showstlinfo") {
            QTextEdit *text = new QTextEdit;
            text->resize(1250, 750);
            text->setAttribute(Qt::WA_DeleteOnClose);
//            text->setFont(QFont("Noto Sans Mono CJK SC", 9));
            text->setFont(QFont("xos4 Terminess Powerline", 9));
            text->append(QString::fromLocal8Bit(strSettlementInfo.c_str()));
            text->show();
        }
        else {
            emit sendToTraderCmdMonitor("Invalid cmd.", Qt::yellow);
        }
    }
}

//void Trader::showRspInfo(CThostFtdcRspInfoField *pRspInfo, const char *outputIfSuccess /*= ""*/, const char *outputIfError /*= ""*/)
//{
//    if (pRspInfo != nullptr) {
//        if (pRspInfo->ErrorID == 0 && outputIfSuccess != "") {
//            //logger(spdlog::level::info, outputIfSuccess.toStdString().c_str());
//            logger(info, "{} | Thost RspInfo: ErrorID={}, ErrorMsg={}", outputIfSuccess, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
//            emit sendToTraderMonitor(outputIfSuccess);
//        }
//        else {
//            QString(outputIfError).append("\nErrorID=").append(QString::number(pRspInfo->ErrorID)).append("\tErrorMsg=").append(QString::fromLocal8Bit(pRspInfo->ErrorMsg));
//            //qDebug() << outputIfError.toStdString().c_str();
//            logger(warn, "{} | Thost RspInfo: ErrorID={}, ErrorMsg={}", outputIfError, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
//            emit sendToTraderMonitor(outputIfError);
//        }
//    }
//    else
//        //qDebug() << "pRspInfo is nullptr";
//        logger(warn, "pRspInfo is nullptr, {}", outputIfError);
//}

//EnumOffsetFlagType str2OffsetFlagType(string str)
//{
//    if (str == "open" || str == "o") { return Open; }
//    else if (str == "close" || str == "c") { return Close; }
//    else if (str == "forceclose") { return ForceClose; }
//    else if (str == "closetoday") { return CloseToday; }
//    else if (str == "closeyesterday") { return CloseYesterday; }
//    else if (str == "forceoff") { return ForceOff; }
//    else if (str == "localforceclose") { return LocalForceClose; }
//    else return Open;  // not good
//}

//EnumDirectionType str2DirectionType(string str)
//{
//    if (str == "b" || str == "buy") { return Buy; }
//    else if (str == "s" || str == "sell") { return Sell; }
//    else return Buy;  // not good
//}
