#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <QObject>
#include <QAbstractTableModel>
#include <QTableView>

#include "position.h"
#include "datahub.h"

class position;
class RM;
class OMS;
class Trader;
class Dispatcher;
class Kalman;

struct CtpEvent;

struct PortfolioValue {
public:
    PortfolioValue();
    PortfolioValue(CThostFtdcTradingAccountField *af);

	std::string brokerID;
	std::string accountID;
	std::string tradingDay;
    std::string lastUpdateTime;
	double preDeposit{ 0 };
	double preBalance{ 0 };
	double preMargin{ 0 };
	double balance{ 0 };
	double available{ 0 };
	double deposit{ 0 };
	double withdraw{ 0 };
	double margin{ 0 };
	double commission{ 0 };
	double cashBalance{ 0 };
	double closeProfit{ 0 };
	double positionProfit{ 0 };
	double grossPnl{ 0 };
	double netPnl{ 0 };
};

class Portfolio : public QObject
{
	Q_OBJECT

public:
	Portfolio();
	~Portfolio();

    void onCtpEvent(CtpEvent ev);

	PosList posList;
	AggPosList aggPosList;
	NetPosList netPosList;
    PortfolioValue pfValue;

    DataHub *dataHub;

    std::string tradingDay;
    std::string time;
    int millisec{ 0 };
    int numNetPosRows = 0;


signals:
    void updatePosTable();
    void updateAccTable();

private:
	//QMap<string, double> commRateList;
	//QMap<string, NetPos> netPosList;
	AggPosList constructAggPosList(PosList pList);
	NetPosList constructNetPosList(AggPosList apList);
    void updatePosOnTrade(AggPosList &al, PosList &pl, CThostFtdcTradeField &td, SymInfoTable &sinfo);
    void evalAccount(PortfolioValue &pfValue, AggPosList &aplist, SymMktTable &smkt);
    void evalOnTick(PortfolioValue &pfValue, AggPosList &aplist, CThostFtdcDepthMarketDataField &mkt);

    void printNetPos();
	void printAcc();

    QTime initTime();
    QTime updateTime();


	int lastRowCount{ 0 };  // for tableview
	QTableView *postableview;

	//CThostFtdcTradingAccountField accInfo;
	bool beginUpdate{ true };
    bool isInPosStream{ false };

	//RM rm;

	/*void updateOnFeed(string sym, double price);
	void updatePosList(const Position& apos);
	void updateNetPosList(const Position& apos);*/
	//void updateNetPosList(const PosDetail& pd);
	//void addPnl(double rp, double rcp, double p, double cp);
    //void addPnl(NetPos& np, double rp, double rcp, double p, double cp);

};
#endif // !PORTFOLIO_H
