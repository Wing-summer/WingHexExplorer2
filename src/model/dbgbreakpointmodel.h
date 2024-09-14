#ifndef DBGBREAKPOINTMODEL_H
#define DBGBREAKPOINTMODEL_H

#include <QAbstractTableModel>

#include "class/asdebugger.h"

class DbgBreakpointModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit DbgBreakpointModel(QObject *parent = nullptr);

signals:

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QList<asDebugger::BreakPoint> _bps;
};

#endif // DBGBREAKPOINTMODEL_H
