#ifndef NUMSHOWMODEL_H
#define NUMSHOWMODEL_H

#include <QAbstractTableModel>

class NumShowModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum NumTableIndex {
        Byte,
        Char,
        Ushort,
        Short,
        Uint32,
        Int32,
        Uint64,
        Int64
    };

public:
    explicit NumShowModel(QObject *parent = nullptr);

    void setNumData(NumTableIndex index, const QString &data);
    QString numData(NumTableIndex index) const;
    void clear();

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QStringList m_num;
};

#endif // NUMSHOWMODEL_H
