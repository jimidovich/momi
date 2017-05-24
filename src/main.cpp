#include <QTextCodec>
#include <QDebug>
#include <QThread>
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
#include "include/dispatcher.h"
// include kdbconnector.h in last order for k.h polute reason
#include "include/kdbconnector.h"

//#pragma comment(lib,"thostmduserapi.lib")
//#pragma comment(lib,"thosttraderapi.lib")
//#pragma comment(lib, "c.lib")

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

using namespace std;

//string greencolor = "\033[32m";
//string yellowcolor = "\033[33m";
//string resetcolor = "\033[00m";

class Reader
{
public:
    Reader(string name):name(name){}
    Reader(){}
    ~Reader(){}
    void onTick(double data, string color);
    void waitForTick();
    void runThread();

    DataHub* dh;
private:
    string name;
    thread myThread;
    unique_lock<mutex> locker;
};

void Reader::onTick(double data, string color) {
//    cout_lk.lock();
//    cout << name << " onTick(), got data: " << data << endl;
//    cout_lk.unlock();
//    this_thread::sleep_for(chrono::microseconds(1));
//    cout_lk.lock();
//    cout << name << " work done for data: " << data << ", on thread id: " << this_thread::get_id() << endl;
//    cout_lk.unlock();
    int s=0;
    for (int i=0;i<=100;i++) s+=i;
}

void Reader::waitForTick() {
    //while(1) {
    //    locker = unique_lock<mutex>(mu);
    //    if (q.empty()) {
    //        cout << name << " waiting..." << endl;
    //        newTickPosted.wait(locker);
    //    }
    //    cout << name <<  " fetching data from thread id: " << this_thread::get_id() << endl;
    //    cout << name <<  " got data: " << q.front() << endl;
    //    int data = q.front();
    //    q.pop();
    //    locker.unlock();

        //onTick(data);
    //}
}

void Reader::runThread() {
    myThread = thread(&Reader::waitForTick, this);
}




int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("GB2312");
    QTextCodec::setCodecForLocale(codec);

    // logs directory for spdlog file
    auto qdir = new QDir();
    if (qdir == nullptr )
    {
        return 1;
    }

    if (!qdir->exists("./logs")) qdir->mkdir("./logs");

    auto console = spdlog::stdout_color_mt(" momi ");
    if (console == nullptr )
    {
        return 2;
    }

    console->set_pattern("[%H:%M:%S.%f] [%L] [%n] %v");
    auto file_logger = spdlog::rotating_logger_mt("file_logger", "logs/main_log", 1024 * 1024 * 5, 3);
    if (file_logger == nullptr )
    {
        return 3;
    }

    file_logger->flush_on(spdlog::level::info);
    console->info("Enter Program");
    file_logger->info("Enter Program");

    QApplication a(argc, argv);
    CtpMonitor *w = nullptr;
    if (argc == 1) {
        w = new CtpMonitor;
        w->show();
    }

    KdbConnector kdbConnector("kdbconn");
    Dispatcher dispatcher;
    dispatcher.setKdbConnector(&kdbConnector);

    Trader trader("tcp://180.168.146.187:10000", "9999", "063669", "1qaz2wsx");
    //Trader trader("tcp://222.66.235.70:21205", "66666", "00008218", "183488");
    MdSpi mdspi("tcp://180.168.146.187:10011", "9999", "063669", "1qaz2wsx");
    //MdSpi mdspi("tcp://222.66.235.70:21214", "66666", "00008218", "183488");

    //call timer from main thread can work for trader schedule
//    auto timer = new QTimer;
//    trader.timer = timer;
//    QObject::connect(timer, SIGNAL(timeout()), &trader, SLOT(ReqQrySettlementInfoConfirm()));
//    timer->setSingleShot(true);
//    timer->start(2000);

    DataHub dHub;
    mdspi.dataHub = &dHub;
    Dispatcher1 d("d1");
    d.dh = &dHub;

    Reader reader1("reader1");
    reader1.dh = &dHub;
    Reader reader2("reader2");
    reader2.dh = &dHub;
    d.r1 = &reader1;
    d.r2 = &reader2;

    d.runThread();

    Kalman kf;
    OMS oms;
    Portfolio pf(&trader, &oms, &kf);

    kf.setOMS(&oms);
    kf.setPortfolio(&pf);
    oms.setTrader(&trader);
    oms.setPortfolio(&pf);
    pf.setDispatcher(&dispatcher);
    trader.setDispatcher(&dispatcher);
    mdspi.setDispatcher(&dispatcher);

    dispatcher.registerHandler(&pf, SIGNAL(dispatchPos(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&pf, SIGNAL(dispatchPosDetail(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&pf, SIGNAL(dispatchAccInfo(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&pf, SIGNAL(dispatchContractInfo(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&pf, SIGNAL(dispatchFeed(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&pf, SIGNAL(dispatchTrade(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&pf, SIGNAL(dispatchOrder(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&kdbConnector, SIGNAL(dispatchFeed(QEvent*)), SLOT(onEvent(QEvent*)));
    dispatcher.registerHandler(&kdbConnector, SIGNAL(dispatchAccUpdate(QEvent*)), SLOT(onEvent(QEvent*)));

    QThread thread;
    //QThread thread1;
    // TODO: Check connector operating in other thread.
    // use direct call or func pointer of base class for other thread.
    kdbConnector.moveToThread(&thread);
    dispatcher.moveToThread(&thread);
    pf.moveToThread(&thread);

    TickSubscriber tickSub("kdbsub");
    //tickSub.moveToThread(&thread1);

    thread.start();

    //if (std::string(argv[1]) == "--nogui")
    if (argc == 1) {
//        CtpMonitor w;
        QObject::connect(&trader, &Trader::sendToTraderMonitor, w, &CtpMonitor::printTraderMsg);
        QObject::connect(&trader, &Trader::sendToTraderCmdMonitor, w, &CtpMonitor::printToTraderCmdMonitor);
        QObject::connect(w, &CtpMonitor::sendCmdLineToTrader, &trader, &Trader::execCmdLine);
        QObject::connect(&mdspi, &MdSpi::sendToTraderMonitor, w, &CtpMonitor::printTraderMsg);
        QObject::connect(&mdspi, &MdSpi::sendToMdMonitor, w, &CtpMonitor::printMdSpiMsg);
        QObject::connect(w, &CtpMonitor::sendCmdLineToMdspi, &mdspi, &MdSpi::execCmdLine);
        QObject::connect(&pf, &Portfolio::sendToPosMonitor, w, &CtpMonitor::printPosMsg);
        QObject::connect(&pf, &Portfolio::sendToAccMonitor, w, &CtpMonitor::printAccMsg);
        QObject::connect(&oms, &OMS::sendToTraderMonitor, w, &CtpMonitor::printTraderMsg);
        QObject::connect(w, &CtpMonitor::sendCmdLineToOms, &oms, &OMS::execCmdLine);
        //mythread.kdbConnector.setTradingDay(trader.getTradingDay().c_str());

        w->getui().posTableView->setModel(&pf);
        pf.setPosTableView(w->getui().posTableView);

        w->show();
        auto ret = a.exec();
        thread.exit();
        delete w;
        return ret;
    } else {
        return a.exec();
    }

}
