/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QCODE_MODEL_H_
#define _QCODE_MODEL_H_

#include <QAbstractItemModel>
#include <QHash>
#include <QStack>

class QCodeNode;

class QCodeModel : public QAbstractItemModel {
    Q_OBJECT

    friend class QCodeNode;

public:
    enum ExtraRoles { TypeRole = Qt::UserRole, VisibilityRole };

    QCodeModel(QObject *p = nullptr);
    virtual ~QCodeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(QCodeNode *n) const;
    QCodeNode *node(const QModelIndex &idx) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QList<QCodeNode *> topLevelNodes() const;

    void appendTopLevelNode(QCodeNode *n);
    void removeTopLevelNode(QCodeNode *n);
    void clearTopLevelNodes();

    QList<QCodeNode *> findRootNodes(const QByteArray &name);
    QCodeNode *findNode(const QByteArray &language, const QByteArray &name);

    virtual bool isCachable(QCodeNode *n, QByteArray &cxt) const;

protected:
    void beginInsertRows(const QModelIndex idx, int beg, int end);
    void beginRemoveRows(const QModelIndex idx, int beg, int end);
    void endInsertRows();
    void endRemoveRows();

private:
    struct CacheOp {
        inline CacheOp() : parent(0), begin(-1), end(-1) {}
        inline CacheOp(QCodeNode *n, int b, int e)
            : parent(n), begin(b), end(e) {}

        QCodeNode *parent;
        int begin;
        int end;
    };

    void q_cache(QCodeNode *n, QByteArray cxt);
    void q_uncache(QCodeNode *n, QByteArray cxt);

    QList<QCodeNode *> m_topLevel;

    QStack<CacheOp> m_cache_ops;
    QHash<QByteArray, QCodeNode *> m_cache;
};

#endif // !_QCODE_MODEL_H_
