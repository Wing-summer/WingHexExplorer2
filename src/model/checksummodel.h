#ifndef CHECKSUMMODEL_H
#define CHECKSUMMODEL_H

#include <QAbstractTableModel>
#include <QCryptographicHash>

class CheckSumModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit CheckSumModel(QObject *parent = nullptr);

    void setCheckSumData(QCryptographicHash::Algorithm algorithm,
                         const QString &data);
    QString checkSumData(QCryptographicHash::Algorithm algorithm) const;

    void clearData();

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QMap<QCryptographicHash::Algorithm, QString> _checkSumData;
};

#endif // CHECKSUMMODEL_H
