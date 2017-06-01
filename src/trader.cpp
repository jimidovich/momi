#include <chrono>
#include <set>
#include <thread>
//#include <stdio.h>

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QDialog>
#include <QTextEdit>
#include <QTimer>
//#include <QColor>
#include <QtConcurrent/QtConcurrent>

#include "spdlog/spdlog.h"

#include "include/trader.h"
#include "include/myevent.h"
#include "include/position.h"
#include "include/struct.h"

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
    QString msg = "Front Disconnected. Reason: ";
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
    logger(err, msg.toStdString().c_str());
    emit sendToTraderMonitor(msg, Qt::red);
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

        QString msg;
        QString preSpaces = "\n" + QString(" ").repeated(31);
        msg.append("Trader Login Successful.");
        msg.append(preSpaces).append("TradingDay = ").append(pRspUserLogin->TradingDay);
        msg.append(preSpaces).append("LoginTime  = ").append(pRspUserLogin->LoginTime);
        msg.append(preSpaces).append("SystemName = ").append(pRspUserLogin->SystemName);
        msg.append(preSpaces).append("UserID     = ").append(pRspUserLogin->UserID);
        msg.append(preSpaces).append("BrokerID   = ").append(pRspUserLogin->BrokerID);
        msg.append(preSpaces).append("SessionID  = ").append(QString::number(pRspUserLogin->SessionID));
        msg.append(preSpaces).append("FrontID    = ").append(QString::number(pRspUserLogin->FrontID));
        msg.append(preSpaces).append("INETime    = ").append(pRspUserLogin->INETime);
        msg.append(preSpaces).append("SHFE Time  = ").append(pRspUserLogin->SHFETime);
        msg.append(preSpaces).append("DCE  Time  = ").append(pRspUserLogin->DCETime);
        msg.append(preSpaces).append("CZCE Time  = ").append(pRspUserLogin->CZCETime);
        msg.append(preSpaces).append("FFEX Time  = ").append(pRspUserLogin->FFEXTime);
        emit sendToTraderMonitor(msg, Qt::green);
        logger(info, msg.toStdString().c_str());

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
        QString msg;
        msg += QString("Settlement Info Confirmed. ConfirmDate=%1 ConfirmTime=%2").arg(pSettlementInfoConfirm->ConfirmDate, pSettlementInfoConfirm->ConfirmTime);
        logger(info, msg.toStdString().c_str());
        emit sendToTraderMonitor(msg);
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
                emit sendToTraderMonitor(QString("No Settlement Info Retrieved."));
                logger(critical, "No Settlement Info Retrieved.");
            }
            else {
                emit sendToTraderMonitor(QString("Settlement Info Retrieved."));
                logger(info, "Settlement Info Retrieved.");
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
			QString msg;
			msg += QString("RspQrySettlementInfoConfirm: ConfirmDate=%1 ConfirmTime=%2").arg(pSettlementInfoConfirm->ConfirmDate, pSettlementInfoConfirm->ConfirmTime);
			logger(info, msg.toStdString().c_str());
			emit sendToTraderMonitor(msg);
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
            QString msg;
            msg.append("OrderRef=").append(pOrder->OrderRef);
            msg.append(" sessID=").append(QString::number(pOrder->SessionID));
            msg.append(" frtID=").append(QString::number(pOrder->FrontID));
            msg.append(" exch=").append(pOrder->ExchangeID);
            msg.append(" sysID=").append(pOrder->OrderSysID);
            msg.append(" ").append(pOrder->InstrumentID);
            msg.append(" S=").append(mymap::orderStatus_string.at(pOrder->OrderStatus).c_str());
            msg.append(" O=").append(mymap::offsetFlag_string.at(pOrder->CombOffsetFlag[0]).c_str());
            msg.append(" D=").append(mymap::direction_char.at(pOrder->Direction));
            //msg.append(" PriceType=").append(pOrder->OrderPriceType);
            msg.append(" LmtPx=").append(QString::number(pOrder->LimitPrice));
            msg.append(" Total=").append(QString::number(pOrder->VolumeTotal));
            msg.append(" Orig=").append(QString::number(pOrder->VolumeTotalOriginal));
            msg.append(" ").append(QString::fromLocal8Bit(pOrder->StatusMsg));

            //msg.append(" SessionID=").append(QString::number(pOrder->SessionID));
            //msg.append(" OrderSysID=").append(pOrder->OrderSysID);
            //msg.append(" TraderID=").append(pOrder->TraderID);
            //msg.append(" OrderLocalID=").append(pOrder->OrderLocalID);
            //msg.append(" ActiveTime=").append(pOrder->ActiveTime);
            emit sendToTraderMonitor(msg);

            dataHub->eventQueue.post(CtpEvent(pOrder));
        }
        if (bIsLast)
        {
            logger(info, "Qry Order Finished.");
            emit sendToTraderMonitor("Query Order Finished.");
        }
    }
}

void Trader::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "RspQryTrade: ")) {
        if (pTrade != nullptr) {
            QString msg;
            msg.append(pTrade->InstrumentID);
            msg.append(" TradeID=").append(pTrade->TradeID);
            msg.append(" Dir=").append(mymap::direction_char.at(pTrade->Direction));
            msg.append(" Offset=").append(mymap::offsetFlag_string.at(pTrade->OffsetFlag).c_str());
            msg.append(" Price=").append(QString::number(pTrade->Price));
            msg.append(" Volume=").append(QString::number(pTrade->Volume));
            msg.append(" T=").append(pTrade->TradeTime);
            emit sendToTraderMonitor(msg);
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
            QString msg;
            msg.append(pInvestorPosition->InstrumentID);
            msg.append(" Direction=").append(pInvestorPosition->PosiDirection);
            msg.append(" Position=").append(QString::number(pInvestorPosition->Position));
            msg.append(" PL=").append(QString::number(pInvestorPosition->PositionProfit));
            msg.append(" ClsAmount=").append(QString::number(pInvestorPosition->CloseAmount));
            msg.append(" Margin=").append(QString::number(pInvestorPosition->UseMargin));
            emit sendToTraderMonitor(msg);

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
            QString msg;
            msg.append(pInvestorPositionDetail->InstrumentID);
            msg.append(" Direction=").append(pInvestorPositionDetail->Direction);
            msg.append(" Position=").append(QString::number(pInvestorPositionDetail->Volume));
            msg.append(" PL=").append(QString::number(pInvestorPositionDetail->CloseProfitByDate));
            msg.append(" ClsVolume=").append(QString::number(pInvestorPositionDetail->CloseVolume));
            msg.append(" Margin=").append(QString::number(pInvestorPositionDetail->Margin));
            emit sendToTraderMonitor(msg);

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
            QString msg = "Trading Account";
            QString preSpaces = "\n" + QString(" ").repeated(31);
            msg.append(preSpaces).append("Available  = ").append(QString::number(pTradingAccount->Available));
            msg.append(preSpaces).append("Balance    = ").append(QString::number(pTradingAccount->Balance, 'f', 0));
            msg.append(preSpaces).append("CurrMargin = ").append(QString::number(pTradingAccount->CurrMargin));
            msg.append(preSpaces).append("Reserve    = ").append(QString::number(pTradingAccount->Reserve));
            logger(info, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg);

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
            //QString msg;
            //msg.append(pInstrument->InstrumentID);
            //msg.append(" ").append(pInstrument->ExchangeID);
            //msg.append(" ").append(pInstrument->ExpireDate);
            //emit sendToTraderMonitor(msg);
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
        QString msg;
        msg += QString("OnRtnOrder: OrderRef=%1, %2, StatusMsg=%3").arg(pOrder->OrderRef, pOrder->InstrumentID, QString::fromLocal8Bit(pOrder->StatusMsg));
        emit sendToTraderMonitor(msg);

        //logger(info, "OnRtnOrder: OrderRef={}, Status={}, Status Msg={}", pOrder->OrderRef, pOrder->OrderStatus, pOrder->StatusMsg);
        logger(info, msg.toStdString().c_str());

        dataHub->eventQueue.post(CtpEvent(pOrder));
    }
    else
        logger(err, "OnRtnOrder nullptr or null data");
}

void Trader::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    if (pTrade != nullptr) {
        QString msg;
        msg += QString("OnRtnTrade: OrderRef=%1, %2").arg(pTrade->OrderRef, pTrade->InstrumentID);
//        msg.append("\nExchangeID=").append(pTrade->ExchangeID);
//        msg.append(" TraderID=").append(pTrade->TraderID);
//        msg.append(" OrderSysID=").append(pTrade->OrderSysID);
//        msg.append(" OrderRef=").append(pTrade->OrderRef);
//        msg.append("\n").append(pTrade->InstrumentID);

        msg.append(" Dir=").append(mymap::direction_char.at(pTrade->Direction));
        msg.append(" Offset=").append(mymap::offsetFlag_string.at(pTrade->OffsetFlag).c_str());
        msg.append(" Price=").append(QString::number(pTrade->Price));
        msg.append(" Volume=").append(QString::number(pTrade->Volume));
        msg.append(" T=").append(pTrade->TradeTime);
        logger(info, msg.toStdString().c_str());
        emit sendToTraderMonitor(msg);

        dataHub->eventQueue.post(CtpEvent(pTrade));

    }
    else
        logger(err, "OnRtnTrade nullptr or null data");
}

void Trader::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
    QString msg = QString("ErrRtnOrderInsert: OrderRef=%1 ").arg(pInputOrder->OrderRef);
    isErrorRspInfo(pRspInfo, msg.toStdString().c_str());
}

void Trader::OnErrRtnOrderAction(CThostFtdcOrderActionField * pOrderAction, CThostFtdcRspInfoField * pRspInfo)
{
    QString msg = QString("ErrRtnOrderAction: OrderRef=%1 ").arg(pOrderAction->OrderRef);
    isErrorRspInfo(pRspInfo, msg.toStdString().c_str());
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
        QString errMsg = QString(msg).append("ErrorID=").append(QString::number(pRspInfo->ErrorID)).append(", ErrorMsg=").append(QString::fromLocal8Bit(pRspInfo->ErrorMsg));
//        logger(err, "{}ErrorID={}, ErrorMsg={}", msg, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        logger(err, errMsg.toStdString().c_str());
        emit sendToTraderMonitor(errMsg, Qt::red);
    }
    return isError;
}


void Trader::showApiReturn(int ret, QString outputIfSuccess, QString outputIfError)
{
    const QString green      = "\033[32m";
    const QString reset      = "\033[00m";

    if (outputIfSuccess != "" || outputIfError != "") {
        QString msg;
        QString msg_t;
        switch (ret) {
        case 0:
            //msg = outputIfSuccess.append("0: Sent successfully ").append(QString("ReqID=%1").arg(QString::number(nRequestID)));
            msg = outputIfSuccess.append(QString(" <Sent successfully. ReqID=%1>").arg(nRequestID));
            msg_t = green + msg + reset;
            logger(info, msg_t.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::darkGreen);
            break;
        case -1:
            msg = outputIfSuccess.append(QString(" <Failed, network problem. ReqID=%1>").arg(nRequestID));
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
            break;
        case -2:
            msg = outputIfSuccess.append(QString(" <Failed, number of unhandled request queues passes limit. ReqID=%1>").arg(nRequestID));
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
            break;
        case -3:
            msg = outputIfSuccess.append(QString(" <Failed, requests per sec pass limit.  ReqID=%1>").arg(nRequestID));
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
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
    //qDebug() << "Trader thread" << QObject::thread();
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
