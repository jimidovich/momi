#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <mutex>

#include <QAbstractTableModel>
#include <QTableView>
#include "portfolio.h"
#include "oms.h"

class PosTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PosTableModel(QObject *parent, Portfolio *pf, DataHub *dataHub);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    void updateTable();

    Portfolio *pf;
    DataHub *dataHub;
};

class AccTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    AccTableModel(QObject *parent, Portfolio *pf);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    void updateTable();

    Portfolio *pf;
};

class OrderTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    OrderTableModel(QObject *parent, OMS *oms);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    void updateTable();

    OMS *oms;
};

class TradeTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TradeTableModel(QObject *parent, OMS *oms);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    void updateTable();

    OMS *oms;
};
#endif // TABLEMODEL_H
