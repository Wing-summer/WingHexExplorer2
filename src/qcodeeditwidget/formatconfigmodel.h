#ifndef FORMATCONFIGMODEL_H
#define FORMATCONFIGMODEL_H

#include "qformatscheme.h"
#include <QAbstractTableModel>

class FormatConfigModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit FormatConfigModel(QObject *parent = nullptr);

    void setScheme(QFormatScheme *scheme);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QFormatScheme *_scheme = nullptr;
};

#endif // FORMATCONFIGMODEL_H
