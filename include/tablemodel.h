#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include "portfolio.h"

class PosTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PosTableModel(QObject *parent, Portfolio *pf, DataHub *dataHub);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    void updatePosTable();

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
    void updateAccTable();

    Portfolio *pf;

};

#endif // TABLEMODEL_H
