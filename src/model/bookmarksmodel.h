#ifndef BOOKMARKSMODEL_H
#define BOOKMARKSMODEL_H

#include "QHexView/document/qhexdocument.h"

#include <QAbstractTableModel>
#include <QObject>

class BookMarksModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit BookMarksModel(QList<BookMarkStruct> &db,
                            QObject *parent = nullptr);

    void clear();
    void remove(int index);
    void insert(int index, const BookMarkStruct &bm);

    void updateAll();

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;

private:
    QList<BookMarkStruct> &_bookmarks;
};

#endif // BOOKMARKSMODEL_H
