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

#include "qcodemodel.h"

/*!
        \file qcodemodel.cpp
        \brief Implementation of the QCodeModel class.
*/

#include "qcodeloader.h"
#include "qcodenode.h"
#include "qcodeparser.h"

#define Q_EXTRACT_INDEX(i, d)                                                  \
    QCodeNode *d = static_cast<QCodeNode *>(i.internalPointer());

#include <QStack>

void QCodeModel::q_cache(QCodeNode *n, QByteArray cxt = QByteArray()) {
    if (isCachable(n, cxt)) {
        m_cache.insert(cxt, n);

        // qDebug("Caching %s [0x%x] in 0x%x", cxt.constData(), n, this);
    }

    foreach (QCodeNode *child, n->children)
        q_cache(child, cxt);
}

void QCodeModel::q_uncache(QCodeNode *n, QByteArray cxt = QByteArray()) {
    if (isCachable(n, cxt)) {
        m_cache.remove(cxt);

        // qDebug("De-Caching %s", cxt.constData());
    }

    foreach (QCodeNode *child, n->children)
        q_uncache(child, cxt);
}

/*!
        \class QCodeModel
        \brief Class used to store code hierarchies and display them through
   model/view
*/

/*!
        \brief ctor
*/
QCodeModel::QCodeModel(QObject *p) : QAbstractItemModel(p), m_loader(0) {}

/*!
        \brief dtor
*/
QCodeModel::~QCodeModel() { clearTopLevelNodes(); }

/*!
        \return A list of code nodes occupying the top level of the model
   (unparented ones)
*/
QList<QCodeNode *> QCodeModel::topLevelNodes() const { return m_topLevel; }

/*!
        \return The current code loader used by this model
*/
QCodeLoader *QCodeModel::codeLoader() const { return m_loader; }

/*!
        \brief Set the code loader used by this model
*/
void QCodeModel::setCodeLoader(QCodeLoader *p) { m_loader = p; }

/*!
        \brief Update a file within a group
        \param group group of files to remove (as passed to addGroup())
        \param file file to update
*/
void QCodeModel::updateGroup(const QString &group, const QString &file) {
    QByteArray grp = group.toLocal8Bit();

    foreach (QCodeNode *n, m_topLevel) {
        // qDebug("group %s ?", n->context().constData());

        if (n->role(QCodeNode::Context) == grp) {
            m_loader->update(n, file);
            return;
        }
    }

    // qDebug("group %s not found", qPrintable(group));
}

/*!
        \brief Add a group of files of whathever type (typically from a project)
        \param group group of files to add
        \param files list of files to load to populate the tree
*/
void QCodeModel::addGroup(const QString &group, const QStringList &files) {
    if (!m_loader)
        return;

    foreach (QCodeNode *n, m_topLevel)
        if (n->context() == group)
            return;

    // qDebug("loading %i files into group %s", files.count(),
    // qPrintable(group));
    m_loader->load(group, files, this);
}

/*!
        \brief Remove a group of files from the model
        \param group group of files to remove (as passed to addGroup())
*/
void QCodeModel::removeGroup(const QString &group) {
    foreach (QCodeNode *n, m_topLevel) {
        if (n->role(QCodeNode::Context) == group) {
            removeTopLevelNode(n);
            delete n;
        }
    }
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
void QCodeModel::beginInsertRows(const QModelIndex idx, int beg, int end) {
    QAbstractItemModel::beginInsertRows(idx, beg, end);

    Q_EXTRACT_INDEX(idx, parent)

    m_cache_ops.push(CacheOp(parent, beg, end));
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
void QCodeModel::beginRemoveRows(const QModelIndex idx, int beg, int end) {
    QAbstractItemModel::beginRemoveRows(idx, beg, end);

    Q_EXTRACT_INDEX(idx, parent)

    const QList<QCodeNode *> &l = parent ? parent->children : m_topLevel;

    QByteArray cxt;

    if (parent)
        cxt = parent->qualifiedName();

    // qDebug("uncaching %i out of %i", l.count(), end - beg + 1);

    for (int i = beg; (i <= end) && (i < l.count()); ++i)
        q_uncache(l.at(i), cxt);
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
void QCodeModel::endInsertRows() {
    if (m_cache_ops.count()) {
        CacheOp op = m_cache_ops.pop();

        const QList<QCodeNode *> &l =
            op.parent ? op.parent->children : m_topLevel;

        QByteArray cxt;

        if (op.parent)
            cxt = op.parent->qualifiedName();

        for (int i = op.begin; i <= op.end; ++i)
            q_cache(l.at(i), cxt);
    } else {
        // qDebug("Odd things happenning over there...");
    }

    QAbstractItemModel::endInsertRows();
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
void QCodeModel::endRemoveRows() { QAbstractItemModel::endRemoveRows(); }

/*!
        \brief Append a top level code node to the model
*/
void QCodeModel::appendTopLevelNode(QCodeNode *n) {
    if (!n)
        return;

    int row = m_topLevel.count();

    beginInsertRows(QModelIndex(), row, row);

    m_topLevel.insert(row, n);

    QStack<QCodeNode *> nodes;
    nodes.push(n);

    while (nodes.count()) {
        n = nodes.pop();
        n->model = this;

        foreach (QCodeNode *c, n->children)
            nodes.push(c);
    }

    endInsertRows();
}

/*!
        \brief remove a top level code node from the model
*/
void QCodeModel::removeTopLevelNode(QCodeNode *n) {
    int row = n ? m_topLevel.indexOf(n) : -1;

    if (row == -1)
        return;

    beginRemoveRows(QModelIndex(), row, row);

    m_topLevel.removeAt(row);

    QStack<QCodeNode *> nodes;
    nodes.push(n);

    while (nodes.count()) {
        n = nodes.pop();
        n->model = 0;

        foreach (QCodeNode *c, n->children)
            nodes.push(c);
    }

    endRemoveRows();
}

/*!
        \brief remove all top level nodes from the model
        \warning All the nodes get DELETED.
*/
void QCodeModel::clearTopLevelNodes() {
    int row = m_topLevel.count() - 1;

    if (row == -1)
        return;

    beginRemoveRows(QModelIndex(), 0, row);

    qDeleteAll(m_topLevel);
    m_topLevel.clear();

    /*
    QCodeNode *n;
    QStack<QCodeNode*> nodes;

    foreach ( n, m_topLevel )
            nodes.push(n);

    while ( nodes.count() )
    {
            n = nodes.pop();
            n->model = 0;

            foreach ( QCodeNode *c, n->children )
                    nodes.push(c);
    }
    */

    endRemoveRows();
}

/*!
        \brief Find a node in the internal cache
        \param language concerned programming language
        \param name bare name of the node (e.g class name, typedef, function
   name, variable name, ...) \return the first node found or 0 if none matching
*/
QCodeNode *QCodeModel::findNode(const QByteArray &language,
                                const QByteArray &name) {
    QByteArray id = name;

    if (language.count())
        id.prepend("::").prepend(language);

    QHash<QByteArray, QCodeNode *>::const_iterator i = m_cache.constFind(id);

    if (i != m_cache.constEnd()) {
        // qDebug("%s found... [%s] : 0x%x", name.constData(), id.constData(),
        // *i);
        return *i;
    }

    // qDebug("%s not found... [%s]", name.constData(), id.constData());

    return 0;
}

/*!
        \brief Find nodes in the internal cache
        \param name prefix to match against the bare name of the nodes (e.g
   class name, typedef, function name, variable name, ...) \return the first
   node found or 0 if none matching
*/
QList<QCodeNode *> QCodeModel::findRootNodes(const QByteArray &name) {
    QList<QCodeNode *> l;

    foreach (QCodeNode *g, m_topLevel)
        foreach (QCodeNode *r, g->children)
            if (r->role(QCodeNode::Name) == name)
                l << r;

    return l;
}

/*!
        \return whether the given node is worth caching
        \param n node to test
        \param cxt cache context
*/
bool QCodeModel::isCachable(QCodeNode *n, QByteArray &cxt) const {
    int t = n->type();
    QByteArray qn = n->role(QCodeNode::Name);

    if (cxt.count())
        qn.prepend("::");

    if (cxt.isEmpty() && (t != QCodeNode::Group)) {
        cxt += qn;

        return true;
    } else if ((t == QCodeNode::Enum) || (t == QCodeNode::Union) ||
               (t == QCodeNode::Class) || (t == QCodeNode::Struct) ||
               (t == QCodeNode::Typedef)) {
        cxt += qn;

        return true;
    } else if (t == QCodeNode::Namespace) {

        cxt += qn;

        return true;

    } else if (t == QCodeNode::Language) {

        cxt += n->role(QCodeNode::Name); // + "/";
    }

    return false;
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
bool QCodeModel::hasChildren(const QModelIndex &parent) const {
    return rowCount(parent);
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
int QCodeModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid())
        return m_topLevel.count();

    Q_EXTRACT_INDEX(parent, item)

    return item ? item->children.count() : 0;
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
int QCodeModel::columnCount(const QModelIndex &) const { return 1; }

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
QVariant QCodeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.column())
        return QVariant();

    Q_EXTRACT_INDEX(index, item)

    return item ? item->data(role) : QVariant();
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
Qt::ItemFlags QCodeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
QVariant QCodeModel::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
QCodeNode *QCodeModel::node(const QModelIndex &idx) const {
    Q_EXTRACT_INDEX(idx, n)

    return idx.isValid() ? n : 0;
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
QModelIndex QCodeModel::index(QCodeNode *n) const {
    return n ? createIndex(n->parent ? n->parent->children.indexOf(n)
                                     : m_topLevel.indexOf(n),
                           0, n)
             : QModelIndex();
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
QModelIndex QCodeModel::index(int row, int column,
                              const QModelIndex &parent) const {
    if ((row < 0) || column)
        return QModelIndex();

    Q_EXTRACT_INDEX(parent, item)
    QCodeNode *abs = 0;

    // qDebug("asking index...");

    if (!parent.isValid() && (row < m_topLevel.count())) {
        abs = m_topLevel.at(row);
    } else if (item && (row < item->children.count())) {
        abs = item->children.at(row);
    }

#ifdef _TRACE_MODEL_

    qDebug("%s(%i, %i) : %s",
           item ? qPrintable(item->data(Qt::DisplayRole).toString()) : "root",
           row, column,
           abs ? qPrintable(abs->data(Qt::DisplayRole).toString()) : "!none!");

#endif

    return abs ? createIndex(row, column, abs) : QModelIndex();
}

/*!
        \brief Please read Qt docs on Model/View framework for more informations
*/
QModelIndex QCodeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }

    QCodeNode *parent = 0;
    Q_EXTRACT_INDEX(index, child)

    if (child)
        parent = child->parent;

#ifdef _TRACE_MODEL_

    qDebug("%s->parent() = %s",
           child ? qPrintable(child->data(Qt::DisplayRole).toString())
                 : "@invalid@",
           parent ? qPrintable(parent->data(Qt::DisplayRole).toString())
                  : "!none!");

#endif

    if (!parent)
        return QModelIndex();

    const int row = parent->parent ? parent->parent->children.indexOf(parent)
                                   : m_topLevel.indexOf(parent);

    return createIndex(row, 0, parent);
}
