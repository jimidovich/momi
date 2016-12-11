#ifndef CTPMONITOR_H
#define CTPMONITOR_H

#include <QtWidgets/QMainWindow>
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
    void printTraderMsg(QString msg, QColor clr);
	void printMdSpiMsg(QString msg);
	void printPosMsg(QString msg);
	void printAccMsg(QString msg);
	void printToTraderCommandHist();
	void recCmdLine();

private:
	Ui::CtpMonitorClass ui;
};

#endif // CTPMONITOR_H
