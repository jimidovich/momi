#ifndef CTPMONITOR_H
#define CTPMONITOR_H

#include "ui_ctpmonitor.h"

class CtpMonitor : public QMainWindow {
	Q_OBJECT

public:
	CtpMonitor(QWidget *parent = 0);
	~CtpMonitor();

	Ui::CtpMonitorClass getui();

signals:
	void sendCmdLineToTrader(QString cmdLine);
	void sendCmdLineToMdspi(QString cmdLine);
	void sendCmdLineToOms(QString cmdLine);

public slots:
    void recCmdLine();
    void printTraderMsg(QString msg, QColor clr);
	void printMdSpiMsg(QString msg);
	void printPosMsg(QString msg);
	void printAccMsg(QString msg);
    void echoToTraderCmdMonitor();
    void printToTraderCmdMonitor(QString msg, QColor clr = Qt::white);

private slots:

    void on_stlinfoButton_clicked();

private:
    // CtpMonotorClass is sub-class of UI_CtpMonitorClass, which creates
    // mainwindow components and sets ui
	Ui::CtpMonitorClass ui;
};

#endif // CTPMONITOR_H
