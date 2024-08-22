#ifndef METADATAMODEL_H
#define METADATAMODEL_H

#include "QHexView/document/qhexdocument.h"
#include <QAbstractTableModel>
#include <QObject>

class MetaDataModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit MetaDataModel(QHexDocument *doc, QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

public slots:
    void setDocument(QHexDocument *newDoc);

private:
    QHexDocument *_doc = nullptr;
};

#endif // METADATAMODEL_H
