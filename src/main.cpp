#include <QTextCodec>
#include <QDir>

#include "spdlog/spdlog.h"

#include "include/ctpmonitor.h"
#include "include/datahub.h"
#include "include/myevent.h"
#include "include/oms.h"
#include "include/trader.h"
#include "include/mdspi.h"
#include "include/portfolio.h"
#include "include/kalman.h"
#include "include/strategy.h"
// include kdbconnector.h in last order for k.h polute reason
#include "include/kdbconnector.h"

using namespace std;

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("GB2312");
    QTextCodec::setCodecForLocale(codec);

    // logs directory for spdlog file
    auto qdir = new QDir();
    if (qdir == nullptr )
        return 1;

    if (!qdir->exists("./logs")) qdir->mkdir("./logs");

    auto console = spdlog::stdout_color_mt(" momi ");
    if (console == nullptr )
        return 2;

    console->set_pattern("[%H:%M:%S.%f] [%L] [%n] %v");
    auto file_logger = spdlog::rotating_logger_mt("file_logger", "logs/main_log", 1024 * 1024 * 5, 3);
    if (file_logger == nullptr )
        return 3;

    file_logger->flush_on(spdlog::level::info);
    console->info("Enter Program");
    file_logger->info("Enter Program");

    // Begin making real shit
//    Trader trader("tcp://180.168.146.187:10000", "9999", "063669", "1qaz2wsx");
//    MdSpi mdspi("tcp://180.168.146.187:10010", "9999", "063669", "1qaz2wsx");
    Trader trader("tcp://180.168.146.187:10030", "9999", "063669", "1qaz2wsx");
    MdSpi mdspi("tcp://180.168.146.187:10031", "9999", "063669", "1qaz2wsx");
//    Trader trader("tcp://222.66.235.70:21205", "66666", "00008218", "183488");
//    MdSpi mdspi("tcp://222.66.235.70:21214", "66666", "00008218", "183488");

    mdspi.subInstruments =
        "IF1706;IH1706;IC1706;TF1712;T1712;"
        "rb1710;ru1709;cu1706;zn1706;au1706;ag1706;au1712;ag1712;sn1709;al1706;hc1710;bu1709;pb1706;sn1709;"
        "i1709;p1709;m1709;y1709;j1709;l1709;c1709;jm1709;cs1709;pp1709;jd1709;a1709;"
        "SR709;TA709;MA709;CF709;OI709;RM709;ZC709;FG709;SM709";

    Portfolio pf;
    OMS oms(&trader, &pf);
    KdbConnector kdbConnector("kdbconn");
    Kalman kf;
    kf.setOMS(&oms);

    DataHub dataHub;
    mdspi.dataHub = &dataHub;
    trader.dataHub = &dataHub;
    pf.dataHub = &dataHub;
    kf.dataHub = &dataHub;
    Dispatcher dispatcher;
    dispatcher.dataHub = &dataHub;

    dispatcher.subs.push_back(&kdbConnector);
    dispatcher.subs.push_back(&dataHub);
    dispatcher.subs.push_back(&pf);
    dispatcher.subs.push_back(&oms);
    dispatcher.subs.push_back(&kf);

    //functional way to register
//    using namespace std::placeholders;
//    d1.subscribers.push_back(std::bind(&DataHub::onCtpEvent, &dataHub, _1));
//    d1.subscribers.push_back(std::bind(&Portfolio::onCtpEvent, &pf, _1));
//    d1.subscribers.push_back(std::bind(&OMS::onCtpEvent, &oms, _1));
//    d1.subscribers.push_back(std::bind(&Kalman::onCtpEvent, &kf, _1));
//    d1.subscribers.push_back(std::bind(&KdbConnector::onCtpEvent, &kdbConnector, _1));
    dispatcher.runThread();

    //if later wrap all shit to strategy
//    Strategy strategy;
//    strategy.trader = &trader;
//    strategy.pf = &pf;
//    strategy.oms = &oms;
//    strategy.rm = &rm;


//    TickSubscriber tickSub("kdbsub");
    //tickSub.moveToThread(&thread1);

    QApplication a(argc, argv);
    if (argc == 1) {
        auto w = new CtpMonitor;
        w->dataHub = &dataHub;

        auto posTM = new PosTableModel(w, &pf, &dataHub);
        auto accTM = new AccTableModel(w, &pf);
        auto orderTM = new OrderTableModel(w, &oms);
        auto tradeTM = new TradeTableModel(w, &oms);
        w->posTableModel = posTM;
        w->accTableModel = accTM;
        w->orderTableModel = orderTM;
        w->tradeTableModel = tradeTM;
        w->getui().posTableView->setModel(w->posTableModel);
        w->getui().accTableView->setModel(w->accTableModel);
        w->getui().orderTableView->setModel(w->orderTableModel);
        w->getui().tradeTableView->setModel(w->tradeTableModel);

        w->getui().posTableView->setColumnWidth(6, 120);
        w->getui().accTableView->setColumnWidth(6, 120);
        w->getui().orderTableView->setColumnWidth(1, 120);
        w->getui().orderTableView->setColumnWidth(12, 120);
        w->getui().tradeTableView->setColumnWidth(1, 120);
        w->getui().tradeTableView->setColumnWidth(12, 120);

        QObject::connect(&trader, &Trader::sendToTraderMonitor, w, &CtpMonitor::printTraderMsg);
        QObject::connect(&trader, &Trader::sendToTraderCmdMonitor, w, &CtpMonitor::printToTraderCmdMonitor);
        QObject::connect(&mdspi, &MdSpi::sendToTraderMonitor, w, &CtpMonitor::printTraderMsg);
        QObject::connect(&oms, &OMS::sendToTraderMonitor, w, &CtpMonitor::printTraderMsg);
//        QObject::connect(&mdspi, &MdSpi::sendToMdMonitor, w, &CtpMonitor::printMdSpiMsg);
        QObject::connect(w, &CtpMonitor::sendCmdLineToTrader, &trader, &Trader::execCmdLine);
        QObject::connect(w, &CtpMonitor::sendCmdLineToMdspi, &mdspi, &MdSpi::execCmdLine);
        QObject::connect(w, &CtpMonitor::sendCmdLineToOms, &oms, &OMS::execCmdLine);
        QObject::connect(&pf, &Portfolio::updatePosTable, w, &CtpMonitor::updatePosTable);
        QObject::connect(&pf, &Portfolio::updateAccTable, w, &CtpMonitor::updateAccTable);
        QObject::connect(&oms, &OMS::updateOrderTable, w, &CtpMonitor::updateOrderTable);
        QObject::connect(&oms, &OMS::updateTradeTable, w, &CtpMonitor::updateTradeTable);

        //mythread.kdbConnector.setTradingDay(trader.getTradingDay().c_str());

        w->show();
        auto ret = a.exec();
        delete w;
        return ret;
    } else {
        return a.exec();
    }
}
