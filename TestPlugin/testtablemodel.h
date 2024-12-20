#ifndef TESTTABLEMODEL_H
#define TESTTABLEMODEL_H

#include <QAbstractTableModel>

class TestTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit TestTableModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;
};

#endif // TESTTABLEMODEL_H
