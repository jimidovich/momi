#ifndef POSITION_H
#define POSITION_H

#include "ThostFtdcUserApiStruct.h"

#include "struct.h"

class Position {
public:
	Position();
    Position(CThostFtdcInvestorPositionDetailField *df, const SymInfoTable &info);
    Position(CThostFtdcTradeField *td, const SymInfoTable &info);
	~Position();

	void updateOnTrade(CThostFtdcTradeField *td);
	void mtm(double price);

	QString positionID;
	QString aggPositionID;

	std::string sym;
	std::string brokerID;
	std::string investorID;
	char hedgeFlag{ 0 };
	char direction{ 0 };
	std::string openDate;
	std::string tradeID;
    int pos{ 0 };
	double entryPrice{ 0 };
	std::string tradingDay;
	std::string exchangeID;
	double dailyCloseProfit{ 0 };
	double tradeCloseProfit{ 0 };
    double dailyUnrProfit{ 0 };
    double cumulUnrProfit{ 0 };
	double margin{ 0 };
	double marginRate{ 0 };
	double lastSttlPrice{ 0 };
	double sttlPrice{ 0 };
	int closeVolume{ 0 };

	int multiple{ 0 };
    char positionDateCategory{ 0 };
	int side{ 0 };
	double commission{ 0 };
	double grossPnl{ 0 };
	double netPnl{ 0 };

};

typedef QMap<QString, Position> PosList;

class AggPosition {
public:
	AggPosition();
    AggPosition(CThostFtdcInvestorPositionField *pf, const SymInfoTable &info);
	AggPosition(const Position &p);
	~AggPosition();

	void addPosition(const Position &p, int addsub = 1);
	void delPosition(const Position &p);
	void mtm(double price);

	QString aggPositionID;

	std::string sym;
	std::string brokerID;
	std::string investorID;
	char direction{ 0 };
	char hedgeFlag{ 0 };
    char positionDateCategory{ 0 };
	int ydPos{ 0 };
	int tdPos{ 0 };
	int pos{ 0 };
	int openVolume{ 0 };
	int closVolume{ 0 };
	double openAmount{ 0 };
	double closeAmount{ 0 };
	double positionCost{ 0 };
	double margin{ 0 };
	double marginRate{ 0 };
	double commission{ 0 };
	double closeProfit{ 0 };
	double positionProfit{ 0 };
	double preSttlPrice{ 0 };
	double sttlPrice{ 0 };
	std::string tradingDay;
	double dailyCloseProfit{ 0 };
	double tradeCloseProfit{ 0 };

	int multiple{ 0 };
	int side{ 0 };
	double avgCostPrice{ 0 };
	double grossPnl{ 0 };
	double netPnl{ 0 };

};

typedef QMap<QString, AggPosition> AggPosList;

class NetPosition {
public:
	NetPosition();
	NetPosition(const AggPosition &ap);
	~NetPosition();

	void addAggPosition(const AggPosition &ap);

	QString netPositionID;

	std::string sym;
	std::string brokerID;
	std::string investorID;
	int netPos{ 0 };
	int longPos{ 0 };
	int shortPos{ 0 };
	double closeProfit{ 0 };
	double positionProfit{ 0 };
	double commission{ 0 };
	int multiple;
	double positionCost{ 0 };
	double avgCostPrice{ 0 };
	double avgEntryPrice{ 0 };
	double grossPnl{ 0 };
	double netPnl{ 0 };

};
typedef QMap<QString, NetPosition> NetPosList;
#endif // POSITION_H
