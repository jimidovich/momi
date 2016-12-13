#include "ctpmonitor.h"
#include "qstring.h"
#include <QtWidgets>


QString getCurrentTimeMsec()
{
    QTime t(QTime::currentTime());
    QString currentTime(t.toString());
    QString msec(QString::number(t.msec()));
    switch (msec.length())
    {
    case 1:
        msec = "00" + msec;
        break;
    case 2:
        msec = "0" + msec;
        break;
    default:
        break;
    }
    return currentTime + "." + msec;
}

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
    splitter3->addWidget(ui.mdOutput);
    splitter3->addWidget(ui.posOutput);
    splitter3->addWidget(ui.accOutput);
    splitter3->addWidget(ui.posTableView);
    splitter3->setOrientation(Qt::Vertical);
    splitter->addWidget(splitter2);
    splitter->addWidget(splitter3);
    this->resize(1500, 800);
    splitter3->resize(300, 0);
    ui.traderOutput->resize(0, 300);
    auto hLayout = new QHBoxLayout(ui.centralWidget);
    hLayout->addWidget(splitter);


    connect(ui.sendButton, &QPushButton::clicked, this, &CtpMonitor::printToTraderCommandHist);
    connect(ui.sendButton, &QPushButton::clicked, this, &CtpMonitor::recCmdLine);
    connect(ui.traderCommandLine, &QLineEdit::returnPressed, this, &CtpMonitor::printToTraderCommandHist);
    connect(ui.traderCommandLine, &QLineEdit::returnPressed, this, &CtpMonitor::recCmdLine);
}

CtpMonitor::~CtpMonitor()
{
}

Ui::CtpMonitorClass CtpMonitor::getui()
{
    return ui;
}

void CtpMonitor::printMdSpiMsg(QString msg)
{
    ui.mdOutput->appendPlainText(msg);
}

void CtpMonitor::printPosMsg(QString msg)
{
    ui.posOutput->clear();
    ui.posOutput->appendPlainText(msg);
}

void CtpMonitor::printAccMsg(QString msg)
{
    ui.accOutput->clear();
    ui.accOutput->appendPlainText(msg);
}

void CtpMonitor::printToTraderCommandHist()
{
    QTime t(QTime::currentTime());
    QString currentTime(t.toString());
    QString msec(QString::number(t.msec()));
    QString cmd(ui.traderCommandLine->text());
    ui.traderCommandHist->appendPlainText("[" + getCurrentTimeMsec() + "]$" + cmd);
}

void CtpMonitor::recCmdLine()
{
    QString cmdLine(ui.traderCommandLine->text());
    QStringList argv(cmdLine.split(" "));
    if (argv.at(0) == "md")
        emit sendCmdLineToMdspi(cmdLine);
    else if (argv.at(0) == "oms")
        emit sendCmdLineToOms(cmdLine);
    else
        emit sendCmdLineToTrader(cmdLine);
    ui.traderCommandLine->clear();
}

void CtpMonitor::printTraderMsg(QString msg, QColor clr)
{
    msg = QString("[" + getCurrentTimeMsec() + "] ") + msg;

    //QColor clr(255,0,0);
    //auto clr = Qt::yellow;
    stringToHtmlFilter(msg);
    stringToHtml(msg, clr);
    ui.traderOutput->insertHtml(msg);
    ui.traderOutput->append("");
    ui.traderOutput->moveCursor(QTextCursor::End);
}
