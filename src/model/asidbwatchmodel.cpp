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

#include "asidbwatchmodel.h"

AsIDBWatchModel::AsIDBWatchModel(QObject *parent) : AsIDBTreeModel(parent) {}

void AsIDBWatchModel::attachDebugger(asDebugger *debugger) { _dbg = debugger; }

bool AsIDBWatchModel::addWatchExpression(const QString &expression) {
    if (!_dbg) {
        return false;
    }

    auto &cache = _dbg->cache;
    if (!cache) {
        return false;
    }

    auto ex = expression.toStdString();

    WatchItem wi;
    wi.expression = ex;
    wi.result = cache->ResolveExpression(ex, 0);

    beginInsertRows(QModelIndex(), rowCount({}), rowCount({}));
    m_watchItems.append(wi);
    updateRootsFromWatchItems();
    endInsertRows();

    return true;
}

void AsIDBWatchModel::removeWatchExpression(qsizetype index) {
    if (index < 0 || index >= m_watchItems.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_watchItems.removeAt(index);
    updateRootsFromWatchItems();
    endRemoveRows();
}

bool AsIDBWatchModel::editWatchExpression(qsizetype index,
                                          const QString &newExpression) {
    if (index < 0 || index >= m_watchItems.size() || !_dbg) {
        return false;
    }

    auto &cache = _dbg->cache;
    if (!cache) {
        return false;
    }

    auto ex = newExpression.toStdString();
    WatchItem wi;
    wi.expression = ex;
    wi.result = cache->ResolveExpression(ex, 0);

    m_watchItems[index] = wi;
    updateRootsFromWatchItems();
    QModelIndex modelIndex = createIndex(index, 0);
    Q_EMIT dataChanged(modelIndex,
                       modelIndex.sibling(index, columnCount({}) - 1));

    return true;
}

void AsIDBWatchModel::refreshAll() {
    auto &cache = _dbg->cache;
    if (!cache) {
        return;
    }

    for (auto &item : m_watchItems) {
        item.result = cache->ResolveExpression(item.expression, 0);
    }

    updateRootsFromWatchItems();
    Q_EMIT layoutChanged();
}

Qt::ItemFlags AsIDBWatchModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags flags = AsIDBTreeModel::flags(index);

    if (!index.parent().isValid() && index.column() == 0) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool AsIDBWatchModel::setData(const QModelIndex &index, const QVariant &value,
                              int role) {
    if (role != Qt::EditRole || !index.isValid() || index.column() != 0) {
        return false;
    }

    if (index.parent().isValid()) {
        return false;
    }

    auto newExpression = value.toString();
    if (newExpression.isEmpty()) {
        return false;
    }

    return editWatchExpression(index.row(), newExpression);
}

void AsIDBWatchModel::clearAll() {
    beginResetModel();
    m_watchItems.clear();
    updateRootsFromWatchItems();
    endResetModel();
}

QVariant AsIDBWatchModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    if (!index.parent().isValid()) {
        int row = index.row();
        if (row < 0 || row >= m_watchItems.size()) {
            return {};
        }

        const auto &item = m_watchItems[row];

        if (role == Qt::DisplayRole) {
            if (index.column() == 0) {
                return QString::fromStdString(item.expression);
            } else if (index.column() == 1) {
                if (item.hasError()) {
                    return QString::fromUtf8(item.result.error().data())
                        .prepend('<')
                        .append('>');
                } else if (auto var = item.result.value().lock()) {
                    if (!var->evaluated) {
                        var->Evaluate();
                    }
                    return QString::fromStdString(var->value);
                } else {
                    return QStringLiteral("<expired>");
                }
            }
        }

        if (role == Qt::UserRole) {
            return QString::fromStdString(item.expression);
        }

        return {};
    }

    return AsIDBTreeModel::data(index, role);
}

bool AsIDBWatchModel::hasChildren(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return !m_watchItems.empty();
    }

    int row = parent.row();
    if (row >= 0 && row < m_watchItems.size()) {
        const auto &item = m_watchItems[row];
        if (item.hasError()) {
            return false;
        }

        if (auto var = item.result.value().lock()) {
            if (!var->evaluated) {
                const_cast<asIDBVariable *>(var.get())->Evaluate();
            }
            return var->expandable;
        }
    }

    return AsIDBTreeModel::hasChildren(parent);
}

int AsIDBWatchModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return static_cast<int>(m_watchItems.size());
    }

    if (!parent.parent().isValid()) {
        int row = parent.row();
        if (row >= 0 && row < m_watchItems.size()) {
            const auto &item = m_watchItems[row];
            if (item.hasError()) {
                return 0;
            }

            if (auto var = item.result.value().lock()) {
                if (!var->evaluated) {
                    const_cast<asIDBVariable *>(var.get())->Evaluate();
                }

                if (item.expanded) {
                    int totalNamed = static_cast<int>(var->namedProps.size());
                    int totalIndexed =
                        static_cast<int>(var->indexedProps.size());
                    int show = std::min(totalIndexed, getPageSize());
                    return totalNamed + show + (totalIndexed > show ? 1 : 0);
                }
            }
        }
        return 0;
    }

    return AsIDBTreeModel::rowCount(parent);
}

QModelIndex AsIDBWatchModel::index(int row, int column,
                                   const QModelIndex &parent) const {
    if (!parent.isValid()) {
        if (row < 0 || row >= static_cast<int>(m_watchItems.size())) {
            return {};
        }
        return createIndex(row, column, nullptr);
    }

    if (!parent.parent().isValid()) {
        int parentRow = parent.row();
        if (parentRow < 0 ||
            parentRow >= static_cast<int>(m_watchItems.size())) {
            return {};
        }

        const auto &item = m_watchItems[parentRow];
        if (item.hasError()) {
            return {};
        }

        if (auto var = item.result.value().lock()) {
            if (!item.expanded) {
                const_cast<WatchItem &>(item).expanded = true;
                if (var->expandable && !var->expanded) {
                    var->Expand();
                }
            }

            QModelIndex fakeParent =
                createIndex(parentRow, 0, encodeVar(var.get()));
            return AsIDBTreeModel::index(row, column, fakeParent);
        }
    }

    return AsIDBTreeModel::index(row, column, parent);
}

QModelIndex AsIDBWatchModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return {};
    }

    void *ip = child.internalPointer();
    if (!ip) {
        return {};
    }

    if (!isProxy(ip)) {
        asIDBVariable *var = decodeVar(ip);

        for (int i = 0; i < static_cast<int>(m_watchItems.size()); ++i) {
            const auto &item = m_watchItems[i];
            if (auto lockedVar = item.result.value().lock()) {
                if (lockedVar.get() == var || isChildOf(lockedVar, var)) {
                    return createIndex(i, 0, nullptr);
                }
            }
        }
    }

    return AsIDBTreeModel::parent(child);
}

void AsIDBWatchModel::updateRootsFromWatchItems() {
    QVector<asIDBVariable::Ptr> proxyRoots;

    for (auto &item : m_watchItems) {
        auto proxyVar = std::make_shared<asIDBVariable>(*_dbg);
        proxyVar->identifier = asIDBVarName(item.expression);

        if (item.hasError()) {
            proxyVar->expandable = false;
            proxyVar->evaluated = true;
            proxyVar->value = item.result.error();
        } else if (auto var = item.result.value().lock()) {
            proxyVar->typeName = var->typeName;
            proxyVar->stackValue = var->stackValue;
            proxyVar->expandable = var->expandable;
            proxyVar->evaluated = var->evaluated;
        } else {
            proxyVar->expandable = false;
            proxyVar->evaluated = true;
            proxyVar->value = "<expired>";
        }

        proxyVar->evaluated = true;
        proxyRoots.append(proxyVar);
    }

    AsIDBTreeModel::setRoots(proxyRoots);
}

bool AsIDBWatchModel::isChildOf(const asIDBVariable::Ptr &parent,
                                const asIDBVariable *child) const {
    if (!parent || !child)
        return false;

    for (const auto &prop : parent->namedProps) {
        if (prop.get() == child)
            return true;
        if (isChildOf(prop, child))
            return true;
    }

    for (const auto &prop : parent->indexedProps) {
        if (prop.get() == child)
            return true;
        if (isChildOf(prop, child))
            return true;
    }

    return false;
}
