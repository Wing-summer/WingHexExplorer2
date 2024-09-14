#ifndef DBGCALLSTACKMODEL_H
#define DBGCALLSTACKMODEL_H

#include <QAbstractTableModel>

#include "class/asdebugger.h"

class DbgCallStackModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit DbgCallStackModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

public slots:
    void updateData(const QList<asDebugger::CallStackItem> &callstack);

private:
    QList<asDebugger::CallStackItem> _stack;
};

#endif // DBGCALLSTACKMODEL_H
