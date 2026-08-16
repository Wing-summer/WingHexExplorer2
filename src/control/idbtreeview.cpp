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

#include "idbtreeview.h"

#include "utilities.h"

#include <QTimer>

IDBTreeView::IDBTreeView(QWidget *parent) : QTreeView(parent) {
    Utilities::applyTreeViewProperty(this);
    auto mod = new IDBTreeModel(this);
    connect(mod, &IDBTreeModel::modelAboutToBeReset, this,
            &IDBTreeView::saveExpansionState);
    connect(mod, &IDBTreeModel::modelReset, this, [this]() {
        QTimer::singleShot(0, this, &IDBTreeView::restoreExpansionState);
    });
    QTreeView::setModel(mod);
}

IDBTreeView::IDBTreeView(IDBTreeModel *model, QWidget *parent)
    : QTreeView(parent) {
    Utilities::applyTreeViewProperty(this);
    header()->setDefaultSectionSize(200);
    if (model) {
        connect(model, &IDBTreeModel::modelAboutToBeReset, this,
                &IDBTreeView::saveExpansionState);
        connect(model, &IDBTreeModel::modelReset, this, [this]() {
            QTimer::singleShot(0, this, &IDBTreeView::restoreExpansionState);
        });
        QTreeView::setModel(model);
    }
}

IDBTreeModel *IDBTreeView::dataModel() const {
    return qobject_cast<IDBTreeModel *>(this->model());
}

void IDBTreeView::setModel(QAbstractItemModel *model) {
    if (this->model()) {
        this->model()->disconnect(this, nullptr);
    }
    if (model) {
        model->setParent(this);
        connect(model, &IDBTreeModel::modelAboutToBeReset, this,
                &IDBTreeView::saveExpansionState);
        connect(model, &IDBTreeModel::modelReset, this, [this]() {
            QTimer::singleShot(0, this, &IDBTreeView::restoreExpansionState);
        });
        QTreeView::setModel(model);
    }
}

void IDBTreeView::refreshData() { dataModel()->refresh(); }

void IDBTreeView::saveExpansionState() {
    m_expansionState.clear();
    saveExpansionStateRecursive(QModelIndex());
}

void IDBTreeView::saveExpansionStateRecursive(const QModelIndex &parent) {
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

void IDBTreeView::restoreExpansionState() {
    if (!model())
        return;

    restoreExpansionStateRecursive(QModelIndex());
}

void IDBTreeView::restoreExpansionStateRecursive(const QModelIndex &parent) {
    for (int i = 0; i < model()->rowCount(parent); ++i) {
        QModelIndex index = model()->index(i, 0, parent);
        QString identifier = getIndexIdentifier(index);

        if (m_expansionState.contains(identifier)) {
            expand(index);
            restoreExpansionStateRecursive(index);
        }
    }
}

QString IDBTreeView::getIndexIdentifier(const QModelIndex &index) const {
    if (!index.isValid())
        return {};

    void *internalPtr = index.internalPointer();
    if (!internalPtr)
        return {};

    auto *idbModel = qobject_cast<IDBTreeModel *>(model());
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
