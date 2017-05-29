#include <QDebug>
#include <QList>

#include "include/portfolio.h"
#include "include/oms.h"
#include "include/myevent.h"
#include "include/trader.h"
//#include "struct.h"

OMS::OMS(QObject * parent) : QObject(parent)
{
    console = spdlog::get("console");
}

//OMS::OMS(Trader * trader, Portfolio * pf)
//{
//	this->trader = trader;
//	this->pf = pf;
//}

OMS::~OMS()
{
}

void OMS::onCtpEvent(CtpEvent ev)
{
    switch (ev.type)
    {
    case TradeEvent:
    {
        Trade td(&(ev.trade));
        tradeList.insert(td.tradeID, td);
        // updating targetPos, now after portfolio::onEvent position updated
        if (targetList.contains(td.tradeInfo->InstrumentID))
            updatePosTarget(targetList[td.tradeInfo->InstrumentID]);
        break;
    }
    case OrderEvent:
    {
        Order od(&(ev.order));
        bool isOrderWithTrade{ false };
        if (workingOrderList.contains(od.orderID)) {
            isOrderWithTrade = od.orderInfo.VolumeTraded > workingOrderList[od.orderID].lastVolumeTraded;
        }
        // then might overwrite old order
        orderList.insert(od.orderID, od);

        // logic: delete old working volume, then update new if isWorking.
        //TODO: add global working volume.
        if (workingOrderList.contains(od.orderID)) {
            if (targetList.contains(od.sym.c_str())) {
                if (od.longShortSide == 'L')
                    targetList[od.sym.c_str()].workingLong -= workingOrderList[od.orderID].workingVolume;
                if (od.longShortSide == 'S')
                    targetList[od.sym.c_str()].workingShort -= workingOrderList[od.orderID].workingVolume;
            }
        }
        workingOrderList.insert(od.orderID, od);
        if (od.isWorking) {
            if (targetList.contains(od.sym.c_str())) {
                if (od.longShortSide == 'L')
                    targetList[od.sym.c_str()].workingLong += od.workingVolume;
                if (od.longShortSide == 'S')
                    targetList[od.sym.c_str()].workingShort += od.workingVolume;
            }
        }
        else {
            workingOrderList.erase(workingOrderList.find(od.orderID));
        }

        // Notice: logic, only update target for "non-trading" order feedback
        if (!isOrderWithTrade)
            updatePosTarget(targetList[od.sym.c_str()]);

        break;
    }
    default:
        break;
    }

}

void OMS::setTrader(Trader *trader)
{
    this->trader = trader;
}

void OMS::setPortfolio(Portfolio *pf)
{
    this->pf = pf;
}

void OMS::addPosTarget(QString targetID)
{
    PosTarget pt;
    pt.sym = targetID.toStdString();
    targetList.insert(targetID, pt);
}

void OMS::updatePosTarget(PosTarget &pt)
{
    NetPosition& np = pf->netPosList[pt.sym.c_str()];
    pt.currNetPos = np.netPos;
    pt.currLong = np.longPos;
    pt.currShort = np.shortPos;

    pt.residualLong = pt.targetLong - pt.currLong;
    pt.residualShort = pt.targetShort - pt.currShort;
    pt.gapLong = pt.residualLong - pt.workingLong;
    pt.gapShort = pt.residualShort - pt.workingShort;

    pt.residualPos = pt.targetPos - pt.currNetPos;
}

void OMS::setPosTarget(QString targetID, int tgtpos, double price)
{
    if (!targetList.contains(targetID))
        addPosTarget(targetID);
    PosTarget& pt = targetList[targetID];
    pt.targetPos = tgtpos;
    pt.targetPrice = price;

    calcLongShortTarget(pt);
    updatePosTarget(pt);
}

void OMS::updatePairPosTarget(std::string yname, std::string xname, int ypos, double yprice, int xpos, double xprice)
{
    ppt.yTarget.sym = yname;
    ppt.xTarget.sym = xname;
    ppt.yTarget.targetPos = ypos;
    ppt.yTarget.targetPrice = yprice;
    ppt.xTarget.targetPos = xpos;
    ppt.xTarget.targetPrice = xprice;
}

void OMS::handleTargets()
{
    if (isWorking) {
        for (auto &i : targetList) {
            if ((i.gapLong != 0) || (i.gapShort != 0)) {
                //updatePosTarget(i.value());  // pense si besoin d'update
                sendOrderForTarget(i);
            }
        }
    }
}

void OMS::switchOn() 
{ 
    isWorking = true; 
    console->warn("OMS switched ON");
}

void OMS::switchOff() { 
    isWorking = false;
    console->warn("OMS switched OFF");
}

void OMS::cancelAllWorking()
{
   for (auto od : workingOrderList) {
       trader->ReqOrderAction(od.orderInfo.InstrumentID, 0, 0, "", od.orderInfo.ExchangeID, od.orderInfo.OrderSysID);
//       trader->ReqOrderAction(od.orderInfo.InstrumentID, od.orderInfo.FrontID, od.orderInfo.SessionID, od.orderInfo.OrderRef);
   }
}


// TODO: to implement today target, yestpos target; close today and no close today cases.
void OMS::calcLongShortTarget(PosTarget &pt)
{
    pt.targetLong = pt.targetPos >= 0 ? pt.targetPos : 0;
    pt.targetShort = pt.targetPos <= 0 ? -pt.targetPos : 0;
}

void OMS::sendOrderForTarget(PosTarget & pt)
{
    if (pt.gapLong > 0) {
        if (pt.workingLong < 0) {
            if (abs(pt.workingLong) >= pt.gapLong) {  // cancel -gaplong <==> add gaplong
                cancelWorkingOrder(pt.sym, EnumDirectionType::Buy, -pt.gapLong);
            }
            else {
                cancelWorkingOrder(pt.sym, EnumDirectionType::Buy, pt.workingLong);
                orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::OpenTrade, EnumDirectionType::Buy, pt.targetPrice, pt.gapLong + pt.workingLong);
            }
        }
        else {  // workingLong>=0, add new workinglong
            orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::OpenTrade, EnumDirectionType::Buy, pt.targetPrice, pt.gapLong);
        }
    }
    else if (pt.gapLong < 0) {
        if (pt.workingLong > 0) {
            if (pt.workingLong >= abs(pt.gapLong)) {
                cancelWorkingOrder(pt.sym, EnumDirectionType::Buy, pt.gapLong);
            }
            else {
                cancelWorkingOrder(pt.sym, EnumDirectionType::Buy, pt.workingLong);
                orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::CloseTrade, EnumDirectionType::Sell, pt.targetPrice, abs(pt.gapLong) - pt.workingLong);
            }
        }
        else {  // there is always enough pos to close, >= gap
            orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::CloseTrade, EnumDirectionType::Sell, pt.targetPrice, abs(pt.gapLong));
        }
    }

    if (pt.gapShort > 0) {
        if (pt.workingShort < 0) {
            if (abs(pt.workingShort) >= pt.gapShort) {  // cancel -gaplong <==> add gaplong
                cancelWorkingOrder(pt.sym, EnumDirectionType::Sell, -pt.gapShort);
            }
            else {
                cancelWorkingOrder(pt.sym, EnumDirectionType::Sell, pt.workingShort);
                orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::OpenTrade, EnumDirectionType::Sell, pt.targetPrice, pt.gapShort + pt.workingShort);
            }
        }
        else {  // workingLong>=0, add new workinglong
            orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::OpenTrade, EnumDirectionType::Sell, pt.targetPrice, pt.gapShort);
        }
    }
    else if (pt.gapShort < 0) {
        if (pt.workingShort > 0) {
            if (pt.workingShort >= abs(pt.gapShort)) {
                cancelWorkingOrder(pt.sym, EnumDirectionType::Sell, pt.gapShort);
            }
            else {
                cancelWorkingOrder(pt.sym, EnumDirectionType::Sell, pt.workingShort);
                orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::CloseTrade, EnumDirectionType::Buy, pt.targetPrice, abs(pt.gapShort) - pt.workingShort);
            }
        }
        else {
            orderInsertWithOffsetFlag(pt.sym, EnumOpenClose::CloseTrade, EnumDirectionType::Buy, pt.targetPrice, abs(pt.gapShort));
        }
    }
}


void OMS::orderInsertWithOffsetFlag(std::string &sym, EnumOpenClose o_c, EnumDirectionType direction, double price, int volume)
{
    // TODO: logic implementation based on commission rates.
    switch (o_c)
    {
    case OpenTrade:
    {
        trader->ReqOrderInsert(sym, EnumOffsetFlagType::Open, direction, price, volume);
    }
    break;
    case CloseTrade:
    {
        // Notice: Important! for api consistency, close posDirection position <==> insert close direction order
        auto posDirection = direction == Buy ? Sell : Buy;

        QString aggID_H = QString("%1-%2-%3").arg(sym.c_str()).arg(mymap::direction_char.at(posDirection)).arg('H');
        int pos_H{ 0 };
        if (pf->aggPosList.contains(aggID_H))
            pos_H = pf->aggPosList[aggID_H].pos;

        // Commented the following coz pos_T not used.
        //QString aggID_T = QString("%1-%2-%3").arg(sym.c_str()).arg(mymap::direction_char.at(posDirection)).arg('T');
        //int pos_T{ 0 };
        //if (pf->aggPosList.contains(aggID_T))
        //    pos_T = pf->aggPosList[aggID_T].pos;

        // simple logic case. yesterday pos close first.
        // also notice that pos_H + pos_T >= volume, where in this close case gap < 0

        /*Reminder: Old documentation
        for SHFE: close yesterday ==> OffsetFlagType = Close
                  close today     ==> OffsetFlagType = CloseToday
        for others:               ==> OffsetFlagType = Close
        */
        if (pos_H > 0 && pos_H <= volume) {
            trader->ReqOrderInsert(sym, EnumOffsetFlagType::Close, direction, price, pos_H);
            if (volume > pos_H)
                trader->ReqOrderInsert(sym, EnumOffsetFlagType::CloseToday, direction, price, volume - pos_H);
        }
        else if (pos_H > volume)  // pos_H > volume, close H volume
            trader->ReqOrderInsert(sym, EnumOffsetFlagType::Close, direction, price, volume);
        else  // pos_H = 0
            trader->ReqOrderInsert(sym, EnumOffsetFlagType::CloseToday, direction, price, volume);
    }
    break;
    default:
        break;
    }
}

bool priorInOrderQueue(const Order &od1, const Order &od2)
{
    if (od1.direction == EnumDirectionType::Buy) {
        if (od1.orderInfo.LimitPrice > od2.orderInfo.LimitPrice) { return true; }
        else if (od1.orderInfo.LimitPrice < od2.orderInfo.LimitPrice) { return false; }
        else {
            if (abs(od1.workingVolume) < abs(od2.workingVolume)) { return true; }
            else if (abs(od1.workingVolume) > abs(od2.workingVolume)) { return false; }
            else {
                // TODO: is there pitfall for OrderRef as char[]?
                if (od1.orderInfo.OrderRef < od2.orderInfo.OrderRef) return true;
                else return false;
            }
        }
    }
    else {
        if (od1.orderInfo.LimitPrice < od2.orderInfo.LimitPrice) { return true; }
        else if (od1.orderInfo.LimitPrice > od2.orderInfo.LimitPrice) { return false; }
        else {
            if (abs(od1.workingVolume) < abs(od2.workingVolume)) { return true; }
            else if (abs(od1.workingVolume) > abs(od2.workingVolume)) { return false; }
            else {
                // TODO: is there pitfall for OrderRef as char[]?
                if (od1.orderInfo.OrderRef < od2.orderInfo.OrderRef) return true;
                else return false;
            }
        }
    }
}

void OMS::cancelWorkingOrder(std::string & sym, EnumDirectionType direction, int volume)
{
    QList<Order> wkOrderQueue;
    for (auto ord : workingOrderList) {
        if (ord.sym == sym)
            wkOrderQueue.append(ord);
    }
    if (!wkOrderQueue.empty()) {
        qSort(wkOrderQueue.begin(), wkOrderQueue.end(), priorInOrderQueue);

        // Notice: Strong Assert: working order list is always of the same sign(aka pending open or close)
        int res_vol = abs(volume);
        for (auto wkod : wkOrderQueue) {
            if (abs(wkod.workingVolume) <= res_vol) {
                trader->ReqOrderAction(wkod.sym, 0, 0, "", wkod.orderInfo.ExchangeID, wkod.orderInfo.OrderSysID);
                res_vol -= abs(wkod.workingVolume);

                if (res_vol == 0)
                    break;
            }
            else {
                // Cancel larger order and re-insert the compensate.
                trader->ReqOrderAction(wkod.sym, 0, 0, "", wkod.orderInfo.ExchangeID, wkod.orderInfo.OrderSysID);
                if (wkod.workingVolume > 0)
                    trader->ReqOrderInsert(wkod.sym, EnumOffsetFlagType::Open, direction, wkod.workingVolume - res_vol);
                else
                    trader->ReqOrderInsert(wkod.sym, mymap::offsetFlag_enum.at(wkod.orderInfo.CombOffsetFlag[0]), direction, abs(wkod.workingVolume) - abs(res_vol));
                break;
            }
        }
    }

}


// premature version of making order for target
//void OMS::sendOrderForTarget(std::string sym, int tgtPos, double price)
//{
//	int currNetPos = pf->netPosList[sym.c_str()].netPos;
//	int posDiff = tgtPos - currNetPos;
//	qDebug() << QString("%1, target=%2, curr=%3, price=%4").arg(sym.c_str()).arg(tgtPos).arg(currNetPos).arg(price);
//	// if posDiff == 0 do nothing
//	if (currNetPos > 0) {
//		if (posDiff > 0)
//			trader->ReqOrderInsert(sym, EnumOffsetFlagType::Open, EnumDirectionType::Buy, price, posDiff);
//		else {
//			if (tgtPos >= 0)
//				trader->ReqOrderInsert(sym, EnumOffsetFlagType::CloseToday, EnumDirectionType::Sell, price, -posDiff);
//			else {
//				trader->ReqOrderInsert(sym, EnumOffsetFlagType::CloseToday, EnumDirectionType::Sell, price, currNetPos);
//				trader->ReqOrderInsert(sym, EnumOffsetFlagType::Open, EnumDirectionType::Sell, price, -tgtPos);
//			}
//		}
//	}
//	else if (currNetPos < 0) {
//		if (posDiff < 0)
//			trader->ReqOrderInsert(sym, EnumOffsetFlagType::Open, EnumDirectionType::Sell, price, -posDiff);
//		else {
//			if (tgtPos <= 0)
//				trader->ReqOrderInsert(sym, EnumOffsetFlagType::CloseToday, EnumDirectionType::Buy, price, posDiff);
//			else {
//				trader->ReqOrderInsert(sym, EnumOffsetFlagType::CloseToday, EnumDirectionType::Buy, price, -currNetPos);
//				trader->ReqOrderInsert(sym, EnumOffsetFlagType::Open, EnumDirectionType::Buy, price, tgtPos);
//			}
//		}
//	}
//	else {
//		if (tgtPos > 0)
//			trader->ReqOrderInsert(sym, EnumOffsetFlagType::Open, EnumDirectionType::Buy, price, tgtPos);
//		else if (tgtPos < 0)
//			trader->ReqOrderInsert(sym, EnumOffsetFlagType::Open, EnumDirectionType::Sell, price, -tgtPos);
//	}
//}

void OMS::handleWorkingOrder(Order &od)
{
}



void OMS::execCmdLine(QString cmdLine)
{
    QStringList argv(cmdLine.split(" "));
    int n = argv.count();
    if (n > 1)
    {
        if (argv.at(1) == "on")
            switchOn();
        else if (argv.at(1) == "off")
            switchOff();
        else if (argv.at(1) == "tgt")
        {
            if (n == 5) {
                QString sym = argv.at(2);
                int pos = argv.at(3).toInt();
                double px = argv.at(4).toDouble();
                setPosTarget(sym, pos, px);
            }
        }
        else if (argv.at(1) == "ca")
            cancelAllWorking();
        else {
            emit sendToTraderMonitor("Invalid cmd");
        }
    }
    else {
        emit sendToTraderMonitor("invalid cmd");
    }
}

Trade::Trade()
{
}

Trade::Trade(CThostFtdcTradeField *tf)
{
    tradeID = QString("%1+%2").arg(tf->ExchangeID).arg(tf->OrderSysID);
    tradeInfo = tf;
}

Order::Order()
{
}

Order::Order(CThostFtdcOrderField *of)
{
    sym = of->InstrumentID;
    direction = mymap::direction_char.at(of->Direction);
    if (of->CombOffsetFlag[0] == EnumOffsetFlagType::Open)
        longShortSide = direction;
    else
        longShortSide = direction == 'L' ? 'S' : 'L';

    status = mymap::orderStatus_enum.at(of->OrderStatus);
    if (status == PartTradedQueueing || status == NoTradeQueueing) {
        if (of->VolumeTotal == 0) {
            qDebug() << "************* Order Status Logic Error *************";
            exit(EXIT_FAILURE);
        }
        isWorking = true;
        workingVolume = of->VolumeTotal;
        if (of->CombOffsetFlag[0] != EnumOffsetFlagType::Open)
            workingVolume *= -1;
        lastVolumeTraded = of->VolumeTraded;
    }
    //orderID = QString("%1-%2").arg(of->ExchangeID).arg(of->OrderSysID);
    //orderID = QString("%1-%2").arg(of->BrokerID).arg(of->BrokerOrderSeq);
    orderID = QString("%1-%2-%3").arg(of->FrontID).arg(of->SessionID).arg(of->OrderRef);
    orderInfo = *of;
}
