#ifndef TRADER_H
#define TRADER_H

#include <memory>
#include "spdlog/spdlog.h"

#include <QObject>
#include <QColor>
#include "ThostFtdcTraderApi.h"
#include "struct.h"
#include "dispatcher.h"


class Trader : public QObject, public CThostFtdcTraderSpi {
    Q_OBJECT

public:
    Trader(QObject *parent = Q_NULLPTR);

    Trader(const std::string &frontAddress, const std::string &brokerID, const std::string &userID, const std::string &password);

    ~Trader();

    void init();

    void reqConnect();

    int ReqQrySettlementInfoConfirm();

    int ReqQrySettlementInfo(std::string TradingDay = "");

    int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder);

    int ReqOrderInsert(std::string InstrumentID, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, double Price, int Volume);

    int ReqOrderInsert(std::string InstrumentID, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, int Volume);

    int ReqOrderInsert(std::string InstrumentID, EnumContingentConditionType ConditionType, double conditionPrice, EnumOffsetFlagType OffsetFlag, EnumDirectionType Direction, EnumOrderPriceTypeType PriceType, double Price, int Volume);

    int ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction);

    int ReqOrderAction(std::string InstrumentID, std::string OrderRef = "");

    int ReqOrderAction(std::string InstrumentID, int FrontID = 0, int SessionID = 0, std::string OrderRef = "", std::string ExchangeID = "", std::string OrderSysID = "");

    int ReqQryOrder(std::string InstrumentID = "", std::string ExchangeID = "", std::string timeStart = "", std::string timeEnd = "", std::string OrderSysID = "");

    int ReqQryTrade(std::string timeStart, std::string timeEnd, std::string InstrumentID, std::string ExchangeID, std::string TradeID);

    int ReqQryDepthMarketData(std::string InstrumentID);

    int ReqQryTradingAccount();

    int ReqQryInvestorPosition(std::string InstrumentID = "");

    int ReqQryInstrument();

    int ReqQryInstrumentMarginRate(std::string InstrumentID, EnumHedgeFlagType hedgeFlag);

    int ReqQryInstrumentCommissionRate(std::string InstrumentID = "");

    void showApiReturn(int ret, QString outputIfSuccess = "", QString outputIfError = "TraderApi sent Error.");

    Dispatcher* getDispatcher();

    void setDispatcher(Dispatcher *ee);

    std::string getTradingDay();

    int ReqQryInvestorPositionDetail(std::string InstrumentID = "");

    int login();

    int logout();

    int ReqSettlementInfoConfirm();

    void handleDispatch(int tt);

    public slots:
    void execCmdLine(QString cmdLine);

signals:
    void sendToTraderMonitor(QString msg, QColor clr = Qt::white);

private:
    bool isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, const char *msg = "");

    void OnFrontConnected();

    void OnFrontDisconnected(int nReason);

    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRtnOrder(CThostFtdcOrderField *pOrder);

    void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

    void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

    void OnRtnTrade(CThostFtdcTradeField *pTrade);

    void setLogger();

    //template <typename... Args>
    //void logger(const char* fmt, const Args&... args);

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

