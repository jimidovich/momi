#include "fmt/format.h"
#include "include/tablemodel.h"

PosTableModel::PosTableModel(QObject *parent, Portfolio *pf, DataHub *dataHub)
    : QAbstractTableModel(parent), pf(pf), dataHub(dataHub)
{
}


int PosTableModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return pf->netPosList.size();
//    return pf->numNetPosRows;
}

int PosTableModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 7;
}

QVariant PosTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0: return QString("Instrument");
            case 1: return QString("Last Price");
            case 2: return QString("Avg Cost");
            case 3: return QString("Net Pos");
            case 4: return QString("Pos PnL");
            case 5: return QString("Net PnL");
            case 6: return QString("Last Time");
            }
        }
    }
    return QVariant();
}

QVariant PosTableModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    int row = index.row();
    int col = index.column();

    if (role == Qt::DisplayRole) {
        pf->mu.lock();
        auto npl = pf->netPosListCopy;
        pf->mu.unlock();
        if (row < npl.size()) {
            auto idxPos = npl.begin() + row;
            auto idxSym = idxPos.key().toStdString();
            switch (col)
            {
            case 0: return QString(idxSym.c_str());
            case 1:
//                if (dataHub->dictHasMkt.at(idxSym))
                    return QString::number(dataHub->symMktTable.at(idxSym).LastPrice);
//                else
//                    return "";
            case 2: return QString::number(idxPos.value().avgCostPrice);
            case 3: return QString::number(idxPos.value().netPos);
            case 4: return QString::number(idxPos.value().positionProfit);
            case 5: return QString::number(idxPos.value().netPnl);
            case 6:
//                if (dataHub->symMktTable.find(idxSym) != dataHub->symMktTable.end())
//                if (dataHub->dictHasMkt.at(idxSym))
//                    return QString("%1.%2").arg(dataHub->symMktTable.at(idxSym).UpdateTime)
//                            .arg(QString::number(dataHub->symMktTable.at(idxSym).UpdateMillisec).rightJustified(3, '0'));
                    return fmt::format("{}.{:0>3}", dataHub->symMktTable.at(idxSym).UpdateTime, dataHub->symMktTable.at(idxSym).UpdateMillisec).c_str();
//                else
//                    return "";
            default:
                break;
            }
        }
        else
            return "test";
    } else if (role == Qt::BackgroundRole) {
        if (col == 1) {
            pf->mu.lock();
            auto npl = pf->netPosListCopy;
            pf->mu.unlock();
            if (row < npl.size()) {
                auto idxPos = npl.begin() + row;
                auto idxSym = idxPos.key().toStdString();
                if (dataHub->symMktTable.at(idxSym).LastPrice > dataHub->symPrevMktTable.at(idxSym).LastPrice)
                    return QColor(Qt::red);  // need to explicitly create QColor obj for QVariant
                else if (dataHub->symMktTable.at(idxSym).LastPrice < dataHub->symPrevMktTable.at(idxSym).LastPrice)
                    return QColor(Qt::darkGreen);  // need to explicitly create QColor obj for QVariant
                else
                    return QVariant();
            }
        }
    }

    return QVariant();
}

void PosTableModel::updateTable()
{
    beginResetModel();
    endResetModel();
}




AccTableModel::AccTableModel(QObject *parent, Portfolio *pf)
    : QAbstractTableModel(parent), pf(pf)
{
}


int AccTableModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 1;
}

int AccTableModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 7;
}

QVariant AccTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0: return QString("Balance");
            case 1: return QString("Grs. PnL");
            case 2: return QString("R. PnL");
            case 3: return QString("Unr. PnL");
            case 4: return QString("Margin");
            case 5: return QString("Comm");
            case 6: return QString("UpdateTime");
            }
        }
    }
    return QVariant();
}

QVariant AccTableModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    int col = index.column();
    if (role == Qt::DisplayRole) {
        switch (col)
        {
        case 0: return QString::number(pf->pfValue.balance, 'f', 2);
        case 1: return QString::number(pf->pfValue.netPnl);
        case 2: return QString::number(pf->pfValue.closeProfit);
        case 3: return QString::number(pf->pfValue.positionProfit);
        case 4: return QString::number(pf->pfValue.margin);
        case 5: return QString::number(pf->pfValue.commission, 'f', 2);
        case 6: return QString(pf->pfValue.lastUpdateTime.c_str());
        default:
            break;
        }
    } else if (role == Qt::BackgroundRole) {
        if (col == 0) {
            if (pf->pfValue.balance > pf->pfValue.prevBalance)
                return QColor(Qt::red);
            else if (pf->pfValue.balance < pf->pfValue.prevBalance)
                return QColor(Qt::darkGreen);
            else
                return QVariant();
        }
    }

    return QVariant();
}


void AccTableModel::updateTable()
{
    beginResetModel();
    endResetModel();
}


OrderTableModel::OrderTableModel(QObject *parent, OMS *oms)
    : QAbstractTableModel(parent), oms(oms)
{
}


int OrderTableModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return oms->orderList.size();
}

int OrderTableModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 12;
}

QVariant OrderTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0: return QString("Instrument");
            case 1: return QString("Status");
            case 2: return QString("OffsetFlag");
            case 3: return QString("Direction");
            case 4: return QString("TradeSide");
            case 5: return QString("LimitPx");
            case 6: return QString("WorkingVol");
            case 7: return QString("TradedVol");
            case 8: return QString("OrigVol");
            case 9: return QString("InsertTime");
            case 10: return QString("Exch");
            case 11: return QString("OrderSysID");
            }
        }
    }
    return QVariant();
}

QVariant OrderTableModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    int row = index.row();
    int col = index.column();
    if (role == Qt::DisplayRole) {
        if (row < oms->orderList.size()) {
            auto iOrder = (oms->orderList.begin() + oms->orderList.size() - 1 - row).value();
            switch (col)
            {
            case 0: return iOrder.orderInfo.InstrumentID;
            case 1: return mymap::orderStatus_string.at(iOrder.orderInfo.OrderStatus).c_str();
            case 2: return mymap::offsetFlag_string.at(iOrder.orderInfo.CombOffsetFlag[0]).c_str();
            case 3: return QString(iOrder.direction);
            case 4: return QString(iOrder.longShortSide);
            case 5: return iOrder.orderInfo.LimitPrice;
            case 6: return iOrder.workingVolume;
            case 7: return iOrder.orderInfo.VolumeTraded;
            case 8: return iOrder.orderInfo.VolumeTotalOriginal;
            case 9: return QString(iOrder.orderInfo.InsertTime);
            case 10: return QString(iOrder.orderInfo.ExchangeID);
            case 11: return QString(iOrder.orderInfo.OrderSysID);
            default:
                break;
            }
        }
    } else if (role == Qt::BackgroundRole) {
        if (col == 1) {
            auto iOrder = (oms->orderList.begin() + oms->orderList.size() - 1 - row).value();
            switch (iOrder.status)
            {
            case AllTraded: return QColor(Qt::darkGreen);
            case PartTradedQueueing: return QColor(Qt::darkCyan);
            case PartTradedNotQueueing: return QColor(Qt::darkMagenta);
            case NoTradeQueueing: return QColor(Qt::darkBlue);
            case NoTradeNotQueueing: return QColor(Qt::darkGray);
            case Canceled: return QColor(Qt::darkRed);
            case Unknown: return QColor(Qt::darkGray);
            case NotTouched: return QColor(Qt::darkGray);
            case Touched: return QColor(Qt::darkGray);
            default:
                break;
            }
        }
    }

    return QVariant();
}

void OrderTableModel::updateTable()
{
    beginResetModel();
    endResetModel();
}


TradeTableModel::TradeTableModel(QObject *parent, OMS *oms)
    : QAbstractTableModel(parent), oms(oms)
{
}


int TradeTableModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return oms->tradeList.size();
}

int TradeTableModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 11;
}

QVariant TradeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0: return QString("Instrument");
            case 1: return QString("TradeID");
            case 2: return QString("OffsetFlag");
            case 3: return QString("Direction");
            case 4: return QString("TradeSide");
            case 5: return QString("TradePx");
            case 6: return QString("TradeVol");
            case 7: return QString("TradeTime");
            case 8: return QString("Exch");
            case 9: return QString("OrderSysID");
            case 10: return QString("TradeDate");
            }
        }
    }
    return QVariant();
}

QVariant TradeTableModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    int row = index.row();
    int col = index.column();
    if (role == Qt::DisplayRole) {
        if (row < oms->tradeList.size()) {
            auto iTrade = (oms->tradeList.begin() + oms->tradeList.size() -1 - row).value();
            switch (col)
            {
            case 0: return iTrade.tradeInfo.InstrumentID;
            case 1: return iTrade.tradeInfo.TradeID;
            case 2: return mymap::offsetFlag_string.at(iTrade.tradeInfo.OffsetFlag).c_str();
            case 3: return QString(mymap::direction_char.at(iTrade.tradeInfo.Direction));
            case 4: return QVariant();
            case 5: return iTrade.tradeInfo.Price;
            case 6: return iTrade.tradeInfo.Volume;
            case 7: return iTrade.tradeInfo.TradeTime;
            case 8: return iTrade.tradeInfo.ExchangeID;
            case 9: return iTrade.tradeInfo.OrderSysID;
            case 10: return iTrade.tradeInfo.TradeDate;
            default:
                break;
            }
        }
//    } else if (role == Qt::BackgroundRole) {
//        if (col == 1) {
//            auto iTrade = (oms->tradeList.begin() + oms->tradeList.size() -1 - row).value();
//            switch (iTrade.tradeInfo->)
//            {
//            case AllTraded: return QColor(Qt::darkGreen);
//            case PartTradedQueueing: return QColor(Qt::darkCyan);
//            case PartTradedNotQueueing: return QColor(Qt::darkMagenta);
//            case NoTradeQueueing: return QColor(Qt::darkBlue);
//            case NoTradeNotQueueing: return QColor(Qt::darkGray);
//            case Canceled: return QColor(Qt::darkRed);
//            case Unknown: return QColor(Qt::darkGray);
//            case NotTouched: return QColor(Qt::darkGray);
//            case Touched: return QColor(Qt::darkGray);
//            default:
//                break;
//            }
    }

    return QVariant();
}

void TradeTableModel::updateTable()
{
    beginResetModel();
    endResetModel();
}
