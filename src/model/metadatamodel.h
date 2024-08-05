#ifndef METADATAMODEL_H
#define METADATAMODEL_H

#include <QAbstractTableModel>
#include <QObject>

class MetaDataModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit MetaDataModel(QObject *parent = nullptr);
};

#endif // METADATAMODEL_H
