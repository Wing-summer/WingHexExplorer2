#ifndef BOOKMARKSMODEL_H
#define BOOKMARKSMODEL_H

#include "QHexView/document/qhexdocument.h"

#include <QAbstractTableModel>
#include <QObject>

class BookMarksModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit BookMarksModel(QHexDocument *doc, QObject *parent = nullptr);

    void beginRemove(int index);
    void endRemove();

    void beginReset();
    void endReset();

    void beginAdd(int index);
    void endAdd();

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

#endif // BOOKMARKSMODEL_H
