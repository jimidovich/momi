#ifndef CTPMONITOR_H
#define CTPMONITOR_H

#include "ui_ctpmonitor.h"
#include "datahub.h"
#include "tablemodel.h"

class CtpMonitor : public QMainWindow {
	Q_OBJECT

public:
	CtpMonitor(QWidget *parent = 0);
	~CtpMonitor();

	Ui::CtpMonitorClass getui();

    DataHub *dataHub;

    PosTableModel *posTableModel;
    AccTableModel *accTableModel;


signals:
	void sendCmdLineToTrader(QString cmdLine);
	void sendCmdLineToMdspi(QString cmdLine);
	void sendCmdLineToOms(QString cmdLine);

public slots:
    void recCmdLine();
    void printTraderMsg(QString msg, QColor clr);
    void echoToTraderCmdMonitor();
    void printToTraderCmdMonitor(QString msg, QColor clr = Qt::white);

    void updatePosTable();
    void updateAccTable();


private slots:

    void on_stlinfoButton_clicked();

private:
    // CtpMonotorClass is sub-class of UI_CtpMonitorClass, which creates
    // mainwindow components and sets ui
	Ui::CtpMonitorClass ui;
};

#endif // CTPMONITOR_H
