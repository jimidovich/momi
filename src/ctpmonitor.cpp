#include <mutex>

#include <QTime>
#include <QSplitter>
#include <QtWebEngineWidgets>

#include "include/ctpmonitor.h"
#include "include/portfolio.h"


void stringToHtml(QString &str, QColor crl)
{
     QByteArray array;
     array.append(crl.red());
     array.append(crl.green());
     array.append(crl.blue());
     QString strC(array.toHex());
     str = QString("<span style=\" color:#%1;\">%2</span>").arg(strC).arg(str);
}

void stringToHtmlFilter(QString &str)
{
    //the order below should not be changed
    str.replace("&","&amp;");
    str.replace(">","&gt;");
    str.replace("<","&lt;");
    str.replace("\"","&quot;");
    str.replace("\'","&#39;");
    str.replace(" ","&nbsp;");
    str.replace("\n","<br>");
    str.replace("\r","<br>");
}

CtpMonitor::CtpMonitor(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    QSplitter *splitter = new QSplitter(parent);
    QSplitter *splitter2 = new QSplitter(parent);
    QSplitter *splitter3 = new QSplitter(parent);
    splitter2->addWidget(ui.traderOutput);
    splitter2->addWidget(ui.frame_Command);
    splitter2->setOrientation(Qt::Vertical);
//    splitter3->addWidget(ui.mdOutput);
//    splitter3->addWidget(ui.posOutput);
//    splitter3->addWidget(ui.accOutput);
//    splitter3->addWidget(ui.webView);
    QWebEngineView *webView = new QWebEngineView(parent);
    splitter3->addWidget(webView);
    splitter3->addWidget(ui.accTableView);
    splitter3->addWidget(ui.posTableView);
    splitter3->addWidget(ui.orderTableView);
    splitter3->addWidget(ui.tradeTableView);
    splitter3->setOrientation(Qt::Vertical);
    splitter->addWidget(splitter2);
    splitter->addWidget(splitter3);
    auto hLayout = new QHBoxLayout(ui.centralWidget);
    hLayout->addWidget(splitter);

//    this->resize(1500, 800);
//    splitter3->resize(300, 0);
//    ui.traderOutput->resize(0, 300);
//    ui.accTableView->resize(20, 0);


    webView->load(QUrl("file:///home/yiju/tmp/qws/hichart2.html"));

    connect(ui.sendButton, &QPushButton::clicked, this, &CtpMonitor::echoToTraderCmdMonitor);
    connect(ui.sendButton, &QPushButton::clicked, this, &CtpMonitor::recCmdLine);
    connect(ui.traderCommandLine, &QLineEdit::returnPressed, this, &CtpMonitor::echoToTraderCmdMonitor);
    connect(ui.traderCommandLine, &QLineEdit::returnPressed, this, &CtpMonitor::recCmdLine);
}

CtpMonitor::~CtpMonitor()
{
}

Ui::CtpMonitorClass CtpMonitor::getui()
{
    return ui;
}

void CtpMonitor::updatePosTable()
{
    posTableModel->updateTable();
}

void CtpMonitor::updateAccTable()
{
    accTableModel->updateTable();
}

void CtpMonitor::updateOrderTable()
{
    orderTableModel->updateTable();
}

void CtpMonitor::updateTradeTable()
{
    tradeTableModel->updateTable();
}

void CtpMonitor::echoToTraderCmdMonitor()
{
    QString cmd(ui.traderCommandLine->text());
    cmd = QString("[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "]$") + cmd;
    printToTraderCmdMonitor(cmd, Qt::white);
}

void CtpMonitor::printToTraderCmdMonitor(QString msg, QColor clr)
{
    //QColor clr(255,0,0);
    //auto clr = Qt::yellow;
    stringToHtmlFilter(msg);
    stringToHtml(msg, clr);
    ui.traderCommandHist->moveCursor(QTextCursor::End);
    ui.traderCommandHist->insertHtml(msg);
    ui.traderCommandHist->append("");
}

void CtpMonitor::recCmdLine()
{
    QString cmdLine(ui.traderCommandLine->text());
    QStringList argv(cmdLine.split(" "));
    if (argv.at(0) == "?") {
        QString usage{
            "Trader commands:\n"
            "q[?]                queries\n"
            "i[?]                insert orders\n"
            "c[?]                cancel orders\n"
            "login               trader login\n"
            "logout              trader logout\n"
            "\n"
            "MD(Market Data) commands:\n"
            "md [?]\n"
            "\n"
            "OMS(Order Management System) commands:\n"
            "oms [?]"
        };
        printToTraderCmdMonitor(usage, Qt::cyan);
    }
    else if (argv.at(0) == "q?") {
        QString usage{
            "Query commands:\n"
            "qa                  Query account\n"
            "qod                 Query orders\n"
            "qtd                 Query trades\n"
            "qp                  Query positions\n"
            "qpd                 Query positions in detail\n"
            "qcomm               Query instrument commission rate\n"
            "qmkt [contract]     Query depth market data"
        };
        printToTraderCmdMonitor(usage, Qt::cyan);
    }
    else if (argv.at(0) == "i?") {
        QString usage{
            "Insert order commands:\n"
            "i [symbol] [o/c] [b/s] [price] [volume]\n"
            "\n"
            "examples:\n"
            "i rb1710 o b 2900 1     rb1710 Open Buy bid=2900 vol=1"
        };
        printToTraderCmdMonitor(usage, Qt::cyan);
    }
    else if (argv.at(0) == "c?") {
        QString usage{
            "Cancel order commands:\n"
            "c [sys] [ExchangeID] [OrderSysID]\n"
            "c [ref] [InstrumentID] [OrderRef]\n"
            "\n"
            "use qod to get order info"
        };
        printToTraderCmdMonitor(usage, Qt::cyan);
    }
    else if (argv.at(0) == "oms?") {
        QString usage{
            "oms commands:\n"
            "oms on                              Turn on OMS\n"
            "oms off                             Turn off OMS\n"
            "oms tgt [symbol] [tgtpos] [price]   Use algo trying to get tgtpos\n"
        };
        printToTraderCmdMonitor(usage, Qt::cyan);
    }
    else if (argv.at(0) == "md")
        emit sendCmdLineToMdspi(cmdLine);
    else if (argv.at(0) == "oms")
        emit sendCmdLineToOms(cmdLine);
    else
        emit sendCmdLineToTrader(cmdLine);
    ui.traderCommandLine->clear();
}

void CtpMonitor::printTraderMsg(QString msg, QColor clr)
{
    msg = QString("[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "] ") + msg;

    //QColor clr(255,0,0);
    //auto clr = Qt::yellow;
    stringToHtmlFilter(msg);
    stringToHtml(msg, clr);
    ui.traderOutput->moveCursor(QTextCursor::End);
    ui.traderOutput->insertHtml(msg);
    ui.traderOutput->append("");
}

void CtpMonitor::on_stlinfoButton_clicked()
{
    emit sendCmdLineToTrader("showstlinfo");
    ui.stlinfoButton->clearFocus();
}
