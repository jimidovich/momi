#include <qdebug.h>
#include <qcoreapplication.h>

#include "ThostFtdcMdApi.h"
#include "mdspi.h"
#include "myevent.h"

using namespace spdlog::level;

MdSpi::MdSpi(QObject * parent) : QObject(parent)
{
    init();
}

MdSpi::MdSpi(const std::string &frontAddress, const std::string &brokerID, const std::string &userID, const std::string &password)
    : QObject(Q_NULLPTR), FrontAddress(frontAddress), BROKER_ID(brokerID), USER_ID(userID), PASSWORD(password)
{
    init();
}

MdSpi::~MdSpi()
{
}

void MdSpi::init()
{
    setLogger();

    logger(info, "Initializing MdSpi");
    reqConnect();
    logger(info, "Initializing MdSpi Finished");
}


void MdSpi::reqConnect()
{
    mdapi = CThostFtdcMdApi::CreateFtdcMdApi();
    mdapi->RegisterSpi(this);

    //char *front = new char[30];
    //strcpy(front, FrontAddress.c_str());
    char *front = new char[FrontAddress.length() + 1];
    strcpy(front, FrontAddress.c_str());
    mdapi->RegisterFront(front);
    delete[] front;
    mdapi->Init();
}

void MdSpi::OnFrontConnected()
{
    logger(info, "MdSpi Front Connected. Req Login...");
    auto loginField = new CThostFtdcReqUserLoginField();
    strcpy(loginField->BrokerID, BROKER_ID.c_str());
    strcpy(loginField->UserID, USER_ID.c_str());
    strcpy(loginField->Password, PASSWORD.c_str());
    int ret = mdapi->ReqUserLogin(loginField, 2);
    showApiReturn(ret, "--> Md ReqLogin", "Md ReqLogin Failed");
}

void MdSpi::OnFrontDisconnected(int nReason)
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
    logger(err, msg.toLocal8Bit());
    emit sendToTraderMonitor(msg, Qt::red);
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "Md RspUserLogin: Failed. ")) {
        QString msg = QString("Md Login Successful. TradingDay=%1").arg(pRspUserLogin->TradingDay);
//        logger(info, "Md Login Successful. TradingDay={}", mdapi->GetTradingDay());
        logger(info, msg.toStdString().c_str());
        emit sendToTraderMonitor(msg, Qt::green);
//        logger(info, "Subscribe market data:");
        std::string instruments = { "au1612 ag1612 au1706 ag1706 cu1612 IF1611 i1701" };
        subscribeMd(instruments);
    }
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "Md RspUserLogout: Failed. "))
        logger(info, "Md Logout Successful");
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "Md RspSubMarketData: ")) {
        //logger(info, "\n....MarketData Subscirbe InstrumentID=", pSpecificInstrument->InstrumentID);
        if (bIsLast)
            logger(info, "MarketData Subscribe Finished.");
    }
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isErrorRspInfo(pRspInfo, "Md RspUnSubMarketData: "))
        //logger(info, "\n....MarketData UnSubscirbe InstrumentID=", pSpecificInstrument->InstrumentID);
        if (bIsLast)
            logger(info, "MarketData UnSubscribe Finished.");
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    //qDebug() << "Depth Market Data:" << endl;
    //qDebug() << this->thread();

    QString msg;
    msg.append(QString::number(++countTick)).append("\t");
    msg.append(pDepthMarketData->InstrumentID).append("\t");
    msg.append("LastPrice=").append(QString::number(pDepthMarketData->LastPrice)).append("\t");
    msg.append("Turnover=").append(QString::number(pDepthMarketData->Turnover / 1e8)).append("\t");
    msg.append(pDepthMarketData->UpdateTime).append(" ");
    msg.append(QString::number(pDepthMarketData->UpdateMillisec)).append("ms");
    emit sendToMdMonitor(msg);
    auto fcpy = new CThostFtdcDepthMarketDataField;
    memcpy(fcpy, pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));
    auto feedEvent = new MyEvent(FeedEvent, fcpy);
    QCoreApplication::postEvent(eventEngine, feedEvent);
}

void MdSpi::subscribeMd(std::string instruments)
{
    QStringList argv(QString(instruments.c_str()).split(" "));
    int n = argv.count();
    if (n > 1)
    {
        auto namelist = new char*[n];
        for (int i = 0; i < n; ++i) {
            namelist[i] = new char[7];
            strcpy(namelist[i], argv.at(i).toStdString().c_str());
        }
        int ret = mdapi->SubscribeMarketData(namelist, n);
        showApiReturn(ret, ("--> Md Subscribe: " + instruments).c_str(), "SubscribeMarketData Failed");
    }
}

void MdSpi::showApiReturn(int ret, QString outputIfSuccess, QString outputIfError)
{
    if (outputIfSuccess != "" || outputIfError != "") {
        QString msg;
        switch (ret) {
        case 0:
            //msg = outputIfSuccess.append("0: Sent successfully ").append(QString("ReqID=%1").arg(QString::number(nRequestID)));
            msg = outputIfSuccess.append(" | Api return 0: Sent successfully. ");
            logger(info, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::darkGreen);
            break;
        case -1:
            msg = outputIfError.append(" | Api return 1: Failed, network problem. ");
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
            break;
        case -2:
            msg = outputIfError.append(" | Api return 2: waiting request queue pass limit. ");
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
            break;
        case -3:
            msg = outputIfError.append(" | Api return 3: request/sec pass limit. ");
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
            break;
        default:
            break;
        }
    }
}

bool MdSpi::isErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, const char *msg)
{
    bool isError = (pRspInfo) && (pRspInfo->ErrorID != 0);
    if (isError) {
        QString errMsg = QString(msg).append("ErrorID=").append(QString::number(pRspInfo->ErrorID)).append(" ErrorMsg=").append(QString::fromLocal8Bit(pRspInfo->ErrorMsg));
        logger(err, "{}ErrorID={}, ErrorMsg={}", msg, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
        emit sendToTraderMonitor(errMsg, Qt::red);
    }
    return isError;
}

void MdSpi::setLogger()
{
    //console = spdlog::get("console");
    console = spdlog::stdout_color_mt("mdspi  ");
    console->set_pattern("[%H:%M:%S.%f] [%n] [%L] %v");
    g_logger = spdlog::get("file_logger");
    mdspi_logger = spdlog::rotating_logger_mt("mdspi_logger", "logs/mdspi_log", 1024 * 1024 * 5, 3);
    //trader_logger = spdlog::daily_logger_mt("trader_logger", "logs/trader_log", 5, 0);
    mdspi_logger->flush_on(spdlog::level::info);
}

Dispatcher * MdSpi::getEventEngine()
{
    return eventEngine;
}

void MdSpi::setDispatcher(Dispatcher *ee)
{
    eventEngine = ee;
}

void MdSpi::execCmdLine(QString cmdLine)
{
    QStringList argv(cmdLine.split(" "));
    int n = argv.count();
    if (n > 1)
    {
        if (argv.at(1) == "sub" || argv.at(1) == "unsub")
        {
            int num = n - 2;
            auto namelist = new char*[num];
            for (int i = 2; i < n; ++i) {
                namelist[i - 2] = new char[7];
                strcpy(namelist[i - 2], argv.at(i).toStdString().c_str());
            }
            if (argv.at(1) == "sub")
                mdapi->SubscribeMarketData(namelist, num);
            else
                mdapi->UnSubscribeMarketData(namelist, num);
        }
        else if (argv.at(1) == "-help" || argv.at(1) == "-h") {
            QString msg = "usage: md {sub, unsub} instrumentID";
            msg.append("\n").append("example: md sub ag1612").append("\n").append("md unsub IF1703");
            emit sendToTraderMonitor(msg);
        }
        else {
            emit sendToTraderMonitor("Invalid cmd.");
        }
    }
}
