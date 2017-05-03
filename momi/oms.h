#ifndef OMS_H
#define OMS_H

#include <QObject>
#include <QColor>
//#include <QMap>

#include "spdlog/spdlog.h"
#include "ThostFtdcUserApiStruct.h"

#include "struct.h"

//class QObject;

template <class Key, class T>
class QMap;

class Portfolio;
class Trader;


class Trade {
public:
    Trade();
    Trade(CThostFtdcTradeField *tf);

    QString tradeID;
    CThostFtdcTradeField *tradeInfo;
};
typedef QMap<QString, Trade> TradeList;

class Order {
public:
    Order();
    Order(CThostFtdcOrderField *of);

    QString orderID;
    std::string sym;
    bool isWorking{ false };
    EnumOrderStatusType status;
    char direction{ 0 };
    char longShortSide{ 0 };
    int workingVolume{ 0 };
    int lastVolumeTraded{ 0 };
    CThostFtdcOrderField *orderInfo;
};
typedef QMap<QString, Order> OrderList;

struct PosTarget {
    std::string sym{ "" };
    double targetPrice{ 0 };
    int targetPos{ 0 };
    int currNetPos{ 0 };

    int currLong{ 0 };
    int currShort{ 0 };
    int targetLong{ 0 };
    int targetShort{ 0 };
    int workingLong{ 0 };
    int workingShort{ 0 };
    int gapLong{ 0 };
    int gapShort{ 0 };
    int residualLong{ 0 };
    int residualShort{ 0 };

    int residualPos{ 0 };
    int workingPos{ 0 };
    //QVector<Order> workingOrders;
};
typedef QMap<QString, PosTarget> TargetList;

struct PairPosTarget {
    PosTarget yTarget;
    PosTarget xTarget;
};

class OMS : public QObject {
    Q_OBJECT
public:
    OMS(QObject *parent = Q_NULLPTR);
    //OMS(Trader* trader, Portfolio* pf);
    ~OMS();

    void onEvent(QEvent *ev);
    void setTrader(Trader *trader);
    void setPortfolio(Portfolio *pf);
    void addPosTarget(QString targetID);
    void setPosTarget(QString targetID, int tgtpos, double price);
    void updatePosTarget(PosTarget &pt);
    void updatePairPosTarget(std::string yname, std::string xname, int ypos, double yprice, int xpos, double xprice);
    void handleTargets();
    void switchOn();
    void switchOff();
    //void sendOrderForTarget(std::string sym, int tgtPos, double price);

    TradeList tradeList;
    OrderList orderList;
    OrderList workingOrderList;
    TargetList targetList;

public slots:
    void execCmdLine(QString cmdLine);

signals:
    void sendToTraderMonitor(QString msg, QColor clr = Qt::white);

private:
    void calcLongShortTarget(PosTarget &pt);
    void sendOrderForTarget(PosTarget &pt);
    void orderInsertWithOffsetFlag(std::string &sym, EnumOpenClose o_c, EnumDirectionType direction, double price, int volume);
    void cancelWorkingOrder(std::string &sym, EnumDirectionType direction, int volume);
    void handleWorkingOrder(Order &od);

    Trader *trader{ nullptr };
    Portfolio *pf{ nullptr };
    PairPosTarget ppt;
    bool isWorking{ false };

    std::shared_ptr<spdlog::logger> console;
};
#endif // OMS_H
