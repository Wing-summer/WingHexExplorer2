/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "asidbtreeview.h"

#include "utilities.h"

#include <QTimer>

asIDBTreeView::asIDBTreeView(QWidget *parent) : QTreeView(parent) {
    Utilities::applyTreeViewProperty(this);
    auto mod = new AsIDBTreeModel(this);
    connect(mod, &AsIDBTreeModel::modelAboutToBeReset, this,
            &asIDBTreeView::saveExpansionState);
    connect(mod, &AsIDBTreeModel::modelReset, this, [this]() {
        QTimer::singleShot(0, this, &asIDBTreeView::restoreExpansionState);
    });
    QTreeView::setModel(mod);
}

asIDBTreeView::asIDBTreeView(AsIDBTreeModel *model, QWidget *parent)
    : QTreeView(parent) {
    Utilities::applyTreeViewProperty(this);
    header()->setDefaultSectionSize(200);
    if (model) {
        connect(model, &AsIDBTreeModel::modelAboutToBeReset, this,
                &asIDBTreeView::saveExpansionState);
        connect(model, &AsIDBTreeModel::modelReset, this, [this]() {
            QTimer::singleShot(0, this, &asIDBTreeView::restoreExpansionState);
        });
        QTreeView::setModel(model);
    }
}

void asIDBTreeView::refreshWithNewRoots(
    const QVector<asIDBVariable::Ptr> &newRoots) {
    auto model = qobject_cast<AsIDBTreeModel *>(this->model());
    if (model) {
        model->setRoots(newRoots);
        _newRoot.reset();
    }
}

void asIDBTreeView::refreshWithNewRoot(const asIDBVariable::Ptr &newRoot) {
    auto model = qobject_cast<AsIDBTreeModel *>(this->model());
    if (model) {
        model->setRoot(newRoot);
        _newRoot = newRoot;
    }
}

void asIDBTreeView::setModel(QAbstractItemModel *model) {
    if (this->model()) {
        this->model()->disconnect(this, nullptr);
    }
    if (model) {
        model->setParent(this);
        connect(model, &AsIDBTreeModel::modelAboutToBeReset, this,
                &asIDBTreeView::saveExpansionState);
        connect(model, &AsIDBTreeModel::modelReset, this, [this]() {
            QTimer::singleShot(0, this, &asIDBTreeView::restoreExpansionState);
        });
        QTreeView::setModel(model);
    }
}

void asIDBTreeView::saveExpansionState() {
    m_expansionState.clear();
    saveExpansionStateRecursive(QModelIndex());
}

void asIDBTreeView::saveExpansionStateRecursive(const QModelIndex &parent) {
    for (int i = 0; i < model()->rowCount(parent); ++i) {
        QModelIndex index = model()->index(i, 0, parent);
        if (isExpanded(index)) {
            QString identifier = getIndexIdentifier(index);
            if (!identifier.isEmpty()) {
                m_expansionState.insert(identifier);
                saveExpansionStateRecursive(index);
            }
        }
    }
}

void asIDBTreeView::restoreExpansionState() {
    if (!model())
        return;

    restoreExpansionStateRecursive(QModelIndex());
}

void asIDBTreeView::restoreExpansionStateRecursive(const QModelIndex &parent) {
    for (int i = 0; i < model()->rowCount(parent); ++i) {
        QModelIndex index = model()->index(i, 0, parent);
        QString identifier = getIndexIdentifier(index);

        if (m_expansionState.contains(identifier)) {
            expand(index);
            restoreExpansionStateRecursive(index);
        }
    }
}

QString asIDBTreeView::getIndexIdentifier(const QModelIndex &index) const {
    if (!index.isValid())
        return {};

    void *internalPtr = index.internalPointer();
    if (!internalPtr)
        return {};

    AsIDBTreeModel *idbModel = qobject_cast<AsIDBTreeModel *>(model());
    if (!idbModel)
        return {};

    if (idbModel->isProxyNode(index)) {
        auto proxyInfo = idbModel->getProxyNodeInfo(index);
        if (proxyInfo.isValid) {
            return QStringLiteral("proxy_%1_%2_%3")
                .arg(proxyInfo.ownerIdentifier)
                .arg(proxyInfo.pageIndex)
                .arg(proxyInfo.startIndex);
        }
    }

    auto varData = model()->data(index, Qt::UserRole);
    if (varData.isValid()) {
        return QStringLiteral("var_%1").arg(varData.toString());
    }

    auto name = model()->data(index, Qt::DisplayRole).toString();
    auto value = model()
                     ->data(index.sibling(index.row(), 1), Qt::DisplayRole)
                     .toString();
    return QStringLiteral("%1|%2").arg(name, value);
}
