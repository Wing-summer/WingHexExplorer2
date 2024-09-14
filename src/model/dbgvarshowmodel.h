#ifndef DBGVARSHOWMODEL_H
#define DBGVARSHOWMODEL_H

#include <QAbstractTableModel>

#include "class/asdebugger.h"

class DbgVarShowModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit DbgVarShowModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QList<asDebugger::VariablesInfo> _vars;
};

#endif // DBGVARSHOWMODEL_H
