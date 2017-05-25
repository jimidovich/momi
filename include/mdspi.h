#ifndef MDSPI_H
#define MDSPI_H

#include <QColor>

#include "spdlog/spdlog.h"
#include "ThostFtdcMdApi.h"

#include "datahub.h"
#include "dispatcher.h"

class QObject;

class MdSpi : public QObject, public CThostFtdcMdSpi {
    Q_OBJECT

public:
    MdSpi(QObject *parent = Q_NULLPTR);
    MdSpi(const std::string &frontAddress,
          const std::string &brokerID,
          const std::string &userID,
          const std::string &password);
    ~MdSpi();

    void init();
    void reqConnect();
    void setDispatcher(Dispatcher *ee);
    void subscribeMd(std::string instruments);
    void showApiReturn(int ret, QString outputIfSuccess = "", QString outputIfError = "MdApi sent Error.");
    bool isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, const char *msg = "");

    // overridden CThostFtdcMdSpi callback functions
    void OnFrontConnected();
    void OnFrontDisconnected(int nReason);

    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    Dispatcher *getDispatcher();

    DataHub* dataHub;

    public slots:
    void execCmdLine(QString cmdLine);

signals:
    void sendToMdMonitor(QString msg);
    void sendToTraderMonitor(QString msg, QColor clr = Qt::white);

private:
    void setLogger();

    template<typename ...Args>
    inline void logger(spdlog::level::level_enum lvl, const char * fmt, const Args & ...args)
    {
        console->log(lvl, fmt, args...);
        g_logger->log(lvl, fmt, args...);
    }

    CThostFtdcMdApi *mdapi{ nullptr };
    Dispatcher *dispatcher{ nullptr };
    /*char *FrontAddress{ "tcp://122.224.98.87:27225" };
    const string BROKER_ID{ "3010" };
    const string USER_ID{ "10101847" };
    const string PASSWORD{ "0" };*/

    //char *FrontAddress{ "tcp://180.168.146.187:10031" };
    //const std::string BROKER_ID{ "9999" };
    //const std::string USER_ID{ "063669" };
    //const std::string PASSWORD{ "1qaz2wsx" };

    const std::string FrontAddress;
    const std::string BROKER_ID;
    const std::string USER_ID;
    const std::string PASSWORD;
    long countTick{ 0 };

    std::shared_ptr<spdlog::logger> console;
    std::shared_ptr<spdlog::logger> g_logger;
    std::shared_ptr<spdlog::logger> mdspi_logger;
};

#endif // !MDSPI_H
