#include <QDebug>
#include <QCoreApplication>
#include <QThread>

#include <iostream>
#include "include/ThostFtdcMdApi.h"

#include "include/mdspi.h"

using namespace spdlog::level;
using namespace std;

MdSpi::MdSpi(QObject * parent) : QObject(parent)
{
    init();
}

MdSpi::MdSpi(const std::string &frontAddress,
             const std::string &brokerID,
             const std::string &userID,
             const std::string &password)
    : QObject(Q_NULLPTR),
      FrontAddress(frontAddress),
      BROKER_ID(brokerID),
      USER_ID(userID),
      PASSWORD(password)
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
    logger(info, "Md Front Connected.");
    emit sendToTraderMonitor("Md Front Connected.", Qt::darkGreen);

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

        // wait for trader req all contracts info, then can initialize symList
        QThread::sleep(5);
        std::string instruments = {
            "IF1705;IH1705;IC1705;TF1706;T1706;"
            "rb1710;ru1709;cu1706;zn1706;au1706;ag1706;sn1709;al1706;hc1710;bu1709;pb1706;sn1709;"
            "i1709;p1709;m1709;y1709;j1709;l1709;c1709;jm1709;cs1709;pp1709;jd1709;a1709;"
            "SR709;TA709;MA709;CF709;OI709;RM709;ZC709;FG709;SM709"
        };
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

//    QString msg;
//    msg.append(QString::number(++countTick)).append("\t");
//    msg.append(pDepthMarketData->InstrumentID).append("\t");
//    msg.append("LastPrice=").append(QString::number(pDepthMarketData->LastPrice)).append("\t");
//    msg.append("Turnover=").append(QString::number(pDepthMarketData->Turnover / 1e8)).append("\t");
//    msg.append(pDepthMarketData->UpdateTime).append(".");
////    msg.append(QString::number(pDepthMarketData->UpdateMillisec));
//    msg.append(QString("%1").arg(pDepthMarketData->UpdateMillisec, 3, 10, QChar('0')));
//    emit sendToMdMonitor(msg);
//    auto fcpy = new CThostFtdcDepthMarketDataField;
//    memcpy(fcpy, pDepthMarketData, sizeof(CThostFtdcDepthMarketDataField));
//    auto feedEvent = new MyEvent(FeedEvent, fcpy);
//    QCoreApplication::postEvent(dispatcher, feedEvent);

    cout << "incoming new data: " << pDepthMarketData->LastPrice << endl;
    cout << "post() called from thread id: " << this_thread::get_id() << endl;
    unique_lock<mutex> locker(dataHub->mu);
    double lastpx = pDepthMarketData->LastPrice;
    dataHub->q.push(lastpx);
    locker.unlock();
//    cout_lk.lock();
    cout << "MD posted data: " << dataHub->q.back() << endl;
//    cout_lk.unlock();
    dataHub->newTickPosted.notify_one();

}

void MdSpi::subscribeMd(std::string instruments)
{
    QStringList argv(QString(instruments.c_str()).split(";"));
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
            msg = outputIfSuccess.append(QString(" <Sent successfully>"));
            logger(info, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::darkGreen);
            break;
        case -1:
            msg = outputIfSuccess.append(QString(" <Failed, network problem>"));
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
            break;
        case -2:
            msg = outputIfSuccess.append(QString(" <Failed, number of unhandled request queues passes limit>"));
            logger(err, msg.toStdString().c_str());
            emit sendToTraderMonitor(msg, Qt::red);
            break;
        case -3:
            msg = outputIfSuccess.append(QString(" <Failed, requests per sec pass limit>"));
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
    console = spdlog::stdout_color_mt("mdspi ");
    console->set_pattern("[%H:%M:%S.%f] [%L] [%n] %v");
    g_logger = spdlog::get("file_logger");
    mdspi_logger = spdlog::rotating_logger_mt("mdspi_logger", "logs/mdspi_log", 1024 * 1024 * 5, 3);
    //trader_logger = spdlog::daily_logger_mt("trader_logger", "logs/trader_log", 5, 0);
    mdspi_logger->flush_on(spdlog::level::info);
}

Dispatcher * MdSpi::getDispatcher()
{
    return dispatcher;
}

void MdSpi::setDispatcher(Dispatcher *ee)
{
    dispatcher = ee;
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
