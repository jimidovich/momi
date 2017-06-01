#ifndef CTPMONITOR_H
#define CTPMONITOR_H

#include "ui_ctpmonitor.h"
//#include "portfolio.h"
#include "datahub.h"

class Portfolio;

class CtpMonitor : public QMainWindow {
	Q_OBJECT

public:
	CtpMonitor(QWidget *parent = 0);
	~CtpMonitor();

	Ui::CtpMonitorClass getui();

    void printMdSpiMsg(QString msg);

    Portfolio *pf;
    DataHub *dataHub;

signals:
	void sendCmdLineToTrader(QString cmdLine);
	void sendCmdLineToMdspi(QString cmdLine);
	void sendCmdLineToOms(QString cmdLine);

public slots:
    void recCmdLine();
    void printTraderMsg(QString msg, QColor clr);
    void echoToTraderCmdMonitor();
    void printToTraderCmdMonitor(QString msg, QColor clr = Qt::white);

    void printPosMsg();
    void printAccMsg();


private slots:

    void on_stlinfoButton_clicked();

private:
    // CtpMonotorClass is sub-class of UI_CtpMonitorClass, which creates
    // mainwindow components and sets ui
	Ui::CtpMonitorClass ui;
};

#endif // CTPMONITOR_H
