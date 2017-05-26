#ifndef TRADER_H
#define TRADER_H

#include <memory>

//#include <QCoreApplication>
//#include <QObject>
//#include <QTimer>
#include <QColor>

#include "spdlog/spdlog.h"
#include "ThostFtdcTraderApi.h"

#include "struct.h"
#include "dispatcher.h"

class QObject;
class QString;


class Trader : public QObject, public CThostFtdcTraderSpi {
    Q_OBJECT

public:
    Trader(QObject *parent = Q_NULLPTR);
    Trader(const std::string &frontAddress, const std::string &brokerID, const std::string &userID, const std::string &password);
    ~Trader();

    void init();
    void reqConnect();

    // my TraderApi
    int login();
    int logout();
    int ReqSettlementInfoConfirm();

    int ReqQryDepthMarketData(std::string InstrumentID);
    int ReqQryInstrumentMarginRate(std::string InstrumentID, EnumHedgeFlagType hedgeFlag);
    int ReqQryInstrumentCommissionRate(std::string InstrumentID = "");
    int ReqQryInvestorPosition(std::string InstrumentID = "");

    int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder);
    int ReqOrderInsert(std::string InstrumentID, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, double Price, int Volume);
    int ReqOrderInsert(std::string InstrumentID, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, int Volume);
    int ReqOrderInsert(std::string InstrumentID, EnumContingentConditionType ConditionType, double conditionPrice, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, EnumOrderPriceTypeType PriceType, double Price, int Volume);

    int ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction);
    int ReqOrderAction(std::string InstrumentID, std::string OrderRef = "");
    int ReqOrderAction(std::string InstrumentID, int FrontID = 0, int SessionID = 0, std::string OrderRef = "", std::string ExchangeID = "", std::string OrderSysID = "");

    int ReqQryOrder(std::string InstrumentID = "", std::string ExchangeID = "", std::string timeStart = "", std::string timeEnd = "", std::string OrderSysID = "");
    int ReqQryTrade(std::string timeStart, std::string timeEnd, std::string InstrumentID, std::string ExchangeID, std::string TradeID);


    void showApiReturn(int ret, QString outputIfSuccess = "", QString outputIfError = "TraderApi sent Error.");
    std::string getTradingDay();
    void setDispatcher(Dispatcher *ee);
    void handleDispatch(int tt);

    Dispatcher* getDispatcher();

    DataHub *dataHub;

public slots:
    int ReqQrySettlementInfo(std::string TradingDay = "");
    int ReqQrySettlementInfoConfirm();
    int ReqQryInstrument();
    int ReqQryTradingAccount();
    int ReqQryInvestorPositionDetail(std::string InstrumentID = "");
    void execCmdLine(QString cmdLine);

signals:
    void sendToTraderMonitor(QString msg, QColor clr = Qt::white);
    void sendToTraderCmdMonitor(QString msg, QColor clr = Qt::white);

public:
    // overridden ThostFtdcTraderSpi callback functions
    void OnFrontConnected();
    void OnFrontDisconnected(int nReason);

    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRtnOrder(CThostFtdcOrderField *pOrder);
    void OnRtnTrade(CThostFtdcTradeField *pTrade);
    void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
    void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

    void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);


    bool isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, const char *msg = "");

    static void timerReq(Trader *trader, const char *req);

    //template <typename... Args>
    //void logger(const char* fmt, const Args&... args);

    void setLogger();

    template<typename ...Args>
    inline void logger(spdlog::level::level_enum lvl, const char * fmt, const Args & ...args)
    {
        console->log(lvl, fmt, args...);
        g_logger->log(lvl, fmt, args...);
    }

    CThostFtdcTraderApi *tdapi;

    int nRequestID{ 0 };
    int nMaxOrderRef{ 0 };
    int FrontID{ 0 };
    int SessionID{ 0 };
    std::string tradingDay;
    std::string strSettlementInfo;
    bool isNewSettlementInfo{ false };
    bool isLoginWorkflow{false};

    //char *FrontAddress{ "tcp://122.224.98.87:27225" };
    //const std::string BROKER_ID{ "3010" };
    //const std::string USER_ID{ "10101847" };
    //const std::string PASSWORD{ "0" };

    //char *FrontAddress{ "tcp://180.168.146.187:10000" };
    //const std::string BROKER_ID{ "9999" };
    //const std::string USER_ID{ "063669" };
    //const std::string PASSWORD{ "1qaz2wsx" };

    const std::string FrontAddress;
    const std::string BROKER_ID;
    const std::string USER_ID;
    const std::string PASSWORD;

    Dispatcher *dispatcher;

    std::shared_ptr<spdlog::logger> console;
    std::shared_ptr<spdlog::logger> g_logger;
    std::shared_ptr<spdlog::logger> trader_logger;
};
#endif // !TRADER_H

