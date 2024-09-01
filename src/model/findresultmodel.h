#ifndef FINDRESULTMODEL_H
#define FINDRESULTMODEL_H

#include "plugin/iwingplugin.h"
#include <QAbstractTableModel>

class FindResultModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit FindResultModel(QObject *parent = nullptr);

    QList<WingHex::FindResult> &results();
    QString &lastFindData();

    void beginUpdate();
    void endUpdate();

    WingHex::FindResult resultAt(qsizetype index) const;
    void clear();
    QList<WingHex::FindResult>::size_type size() const;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QList<WingHex::FindResult> m_results;
    QString m_lastFindData;
};

#endif // FINDRESULTMODEL_H
