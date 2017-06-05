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
    if (role == Qt::DisplayRole) {
        int row = index.row();
        int col = index.column();
        if (row < pf->netPosList.size()) {
            auto i = pf->netPosList.begin();
            auto curr_sym = (i + row).value().sym;
            switch (col)
            {
            case 0: return QString(curr_sym.c_str());
            case 1:
                if (dataHub->symMktTable.find(curr_sym) != dataHub->symMktTable.end())
                    return QString::number(dataHub->symMktTable.at(curr_sym).LastPrice);
                else
                    return "";
            case 2: return QString::number((i + row).value().avgCostPrice);
            case 3: return QString::number((i + row).value().netPos);
            case 4: return QString::number((i + row).value().positionProfit);
            case 5: return QString::number((i + row).value().netPnl);
            case 6:
                if (dataHub->symMktTable.find(curr_sym) != dataHub->symMktTable.end())
                    return QString("%1:%2").arg(dataHub->symMktTable.at(curr_sym).UpdateTime)
                            .arg(QString::number(dataHub->symMktTable.at(curr_sym).UpdateMillisec).rightJustified(3, '0'));
                else
                    return "";
            default:
                break;
            }
        }
        else
            return "test";
    }
    return QVariant();
}

void PosTableModel::updatePosTable()
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
    if (role == Qt::DisplayRole) {
        int col = index.column();
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
    }
    return QVariant();
}

void AccTableModel::updateAccTable()
{
    beginResetModel();
    endResetModel();
}
