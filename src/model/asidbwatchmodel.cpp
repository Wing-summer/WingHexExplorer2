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
#include <QString>
#include <algorithm>

AsIDBWatchModel::AsIDBWatchModel(QObject *parent) : AsIDBTreeModel(parent) {}

void AsIDBWatchModel::attachDebugger(asDebugger *debugger) {
    _dbg = debugger;
    refresh();
}

QStringList AsIDBWatchModel::expressionList() const {
    QStringList ret;
    for (auto &item : m_watchItems) {
        if (item) {
            ret.append(QString::fromStdString(item->expression));
        }
    }
    return ret;
}

void AsIDBWatchModel::addWatchExpression(const QString &expression) {
    auto exp = expression.trimmed();
    if (exp.isEmpty()) {
        return;
    }
    auto p = std::make_shared<WatchItem>();
    p->expression = exp.toStdString();
    p->expanded = false;

    bool evaluated = false;
    if (_dbg) {
        auto &cache = _dbg->cache;
        if (cache) {
            p->result = cache->ResolveExpression(p->expression, 0);
            evaluated = true;
        }
    }

    if (!evaluated) {
        p->result = asIDBExpected<asIDBVariable::WeakPtr>{};
    }

    m_watchItems.append(std::move(p));

    // rebuild roots & reset model in one step
    auto newRoots = buildRootsFromWatchItems();
    beginResetModel();
    replaceRoots(newRoots);
    endResetModel();
}

void AsIDBWatchModel::removeWatchExpression(qsizetype index) {
    if (index < 0 || index >= m_watchItems.size())
        return;

    m_watchItems.removeAt(index);

    auto newRoots = buildRootsFromWatchItems();
    beginResetModel();
    replaceRoots(newRoots);
    endResetModel();
}

void AsIDBWatchModel::removeWatchExpressions(const QModelIndexList &indexes) {
    if (indexes.isEmpty()) {
        return;
    }

    std::set<int, std::greater<int>> rows;
    for (const QModelIndex &idx : indexes) {
        if (!idx.isValid())
            continue;
        QModelIndex top = idx;
        while (top.parent().isValid())
            top = top.parent();
        if (!top.isValid())
            continue;
        int r = top.row();
        if (r >= 0 && r < m_watchItems.size()) {
            rows.insert(r);
        }
    }

    if (rows.empty()) {
        return;
    }

    for (int r : rows) {
        if (r >= 0 && r < m_watchItems.size()) {
            m_watchItems.removeAt(r);
        }
    }

    auto newRoots = buildRootsFromWatchItems();
    beginResetModel();
    replaceRoots(newRoots);
    endResetModel();
}

bool AsIDBWatchModel::editWatchExpression(qsizetype index,
                                          const QString &newExpression) {
    if (index < 0 || index >= m_watchItems.size()) {
        return false;
    }

    auto ex = newExpression.toStdString();
    auto item = m_watchItems[index];
    item->expression = ex;

    if (_dbg) {
        auto &cache = _dbg->cache;
        if (cache) {
            item->result = cache->ResolveExpression(ex, 0);
        } else {
            item->result =
                asIDBExpected<asIDBVariable::WeakPtr>("error evaluated");
        }
    } else {
        item->result = asIDBExpected<asIDBVariable::WeakPtr>{};
    }

    // Replace roots in a single reset so view save/restore works
    auto newRoots = buildRootsFromWatchItems();
    beginResetModel();
    replaceRoots(newRoots);
    endResetModel();

    return true;
}

void AsIDBWatchModel::refresh() {
    if (_dbg) {
        auto &cache = _dbg->cache;
        if (cache) {
            QVector<asIDBExpected<asIDBVariable::WeakPtr>> newResults;
            newResults.reserve(m_watchItems.size());
            for (auto &itemPtr : m_watchItems) {
                newResults.append(
                    cache->ResolveExpression(itemPtr->expression, 0));
            }

            beginResetModel();

            for (int i = 0; i < m_watchItems.size(); ++i) {
                m_watchItems[i]->result = std::move(newResults[i]);
            }

            auto newRoots = buildRootsFromWatchItems();
            replaceRoots(newRoots);

            endResetModel();
        } else {
            beginResetModel();
            for (int i = 0; i < m_watchItems.size(); ++i) {
                m_watchItems[i]->result =
                    asIDBExpected<asIDBVariable::WeakPtr>("error evaluated");
            }
            auto newRoots = buildRootsFromWatchItems();
            replaceRoots(newRoots);
            endResetModel();
        }
    } else {
        beginResetModel();
        for (int i = 0; i < m_watchItems.size(); ++i) {
            m_watchItems[i]->result = asIDBExpected<asIDBVariable::WeakPtr>{};
        }
        auto newRoots = buildRootsFromWatchItems();
        replaceRoots(newRoots);
        endResetModel();
    }
}

void AsIDBWatchModel::reloadExpressionList(const QStringList &expressions) {
    for (auto &expression : expressions) {
        auto exp = expression.trimmed();
        if (exp.isEmpty()) {
            return;
        }
        auto p = std::make_shared<WatchItem>();
        p->expression = exp.toStdString();
        p->result = asIDBExpected<asIDBVariable::WeakPtr>{};
        p->expanded = false;

        m_watchItems.append(std::move(p));
    }

    // rebuild roots & reset model in one step
    auto newRoots = buildRootsFromWatchItems();
    beginResetModel();
    replaceRoots(newRoots);
    endResetModel();
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
    m_watchItems.clear();
    beginResetModel();
    replaceRoots({});
    endResetModel();
}

QString AsIDBWatchModel::makeTopLevelUserRole(const WatchItem &item) const {
    QString idPart;
    if (item.isValid()) {
        auto wp = item.result.value();
        auto sp = wp.lock();
        if (sp) {
            idPart = QString::fromStdString(sp->identifier.Combine());
        } else {
            idPart = QStringLiteral("EXPIRED");
        }
    } else if (item.hasError()) {
        idPart = QStringLiteral("!ERR");
    } else {
        idPart = QStringLiteral("UNEVALUATED");
    }
    // include expression so we have stable names even if var id absent
    return QStringLiteral("watch_%1_%2")
        .arg(QString::fromStdString(item.expression), idPart);
}

QVector<asIDBVariable::Ptr> AsIDBWatchModel::buildRootsFromWatchItems() const {
    QVector<asIDBVariable::Ptr> roots;
    roots.reserve(m_watchItems.size());
    for (const auto &p : m_watchItems) {
        if (p->isValid()) {
            auto sp = p->result.value().lock();
            if (sp) {
                roots.append(sp);
            }
        }
    }
    return roots;
}

QVariant AsIDBWatchModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    // top-level watch row
    if (!index.parent().isValid()) {
        int row = index.row();
        if (row < 0 || row >= m_watchItems.size()) {
            return {};
        }
        const auto &item = *m_watchItems[row];

        if (role == Qt::EditRole) {
            if (index.column() == 0) {
                return QString::fromStdString(item.expression);
            }
        }

        if (role == Qt::UserRole) {
            return makeTopLevelUserRole(item);
        }

        if (role == Qt::DisplayRole) {
            if (index.column() == 0) {
                // Always show expression in column 0
                return QString::fromStdString(item.expression);
            } else {
                if (item.isValid()) {
                    auto sp = item.result.value().lock();
                    if (sp) {
                        if (!sp->evaluated) {
                            const_cast<asIDBVariable *>(sp.get())->Evaluate();
                        }
                        return QString::fromStdString(sp->value);
                    }
                    return QStringLiteral("<expired>");
                } else if (item.hasError()) {
                    auto str = QString::fromStdString(
                        std::string(item.result.error()));
                    if (!str.isEmpty()) {
                        str.prepend('<').append('>');
                    }
                    return str;
                } else {
                    return QStringLiteral("<not evaluated>");
                }
            }
        }

        return {};
    }

    // non top-level: delegate to base class (variables/proxy/paging)
    return AsIDBTreeModel::data(index, role);
}

bool AsIDBWatchModel::hasChildren(const QModelIndex &parent) const {
    if (!parent.isValid())
        return !m_watchItems.empty();

    if (!parent.parent().isValid()) {
        int row = parent.row();
        if (row < 0 || row >= m_watchItems.size())
            return false;
        const auto &item = *m_watchItems[row];
        if (item.hasError()) {
            return false;
        }
        if (item.isValid()) {
            auto sp = item.result.value().lock();
            if (!sp) {
                return false;
            }
            if (sp->expandable) {
                return true;
            }
            return !sp->namedProps.empty() || !sp->indexedProps.empty();
        }
        return false;
    }

    return AsIDBTreeModel::hasChildren(parent);
}

int AsIDBWatchModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid())
        return static_cast<int>(m_watchItems.size());
    if (!parent.parent().isValid()) {
        int row = parent.row();
        if (row < 0 || row >= m_watchItems.size()) {
            return 0;
        }
        const auto &item = *m_watchItems[row];
        if (item.hasError() || !item.isValid()) {
            return 0;
        }
        auto sp = item.result.value().lock();
        if (!sp) {
            return 0;
        }
        if (sp->expandable && !sp->expanded) {
            const_cast<asIDBVariable *>(sp.get())->Expand();
        }
        auto totalNamed = static_cast<int>(sp->namedProps.size());
        auto totalIndexed = static_cast<int>(sp->indexedProps.size());
        auto show = std::min(totalIndexed, getPageSize());
        return totalNamed + show + (totalIndexed > show ? 1 : 0);
    }
    return AsIDBTreeModel::rowCount(parent);
}

QModelIndex AsIDBWatchModel::index(int row, int column,
                                   const QModelIndex &parent) const {
    if (row < 0 || column < 0) {
        return {};
    }

    if (!parent.isValid()) {
        if (row >= m_watchItems.size()) {
            return {};
        }
        return createIndex(row, column,
                           const_cast<WatchItem *>(m_watchItems[row].get()));
    }

    void *pip = parent.internalPointer();
    if (pip) {
        for (int i = 0; i < m_watchItems.size(); ++i) {
            if (m_watchItems[i].get() == pip) {
                const auto &item = *m_watchItems[i];
                if (item.isValid()) {
                    auto sp = item.result.value().lock();
                    if (sp) {
                        auto fakeParent =
                            createIndex(0, 0, encodeVar(sp.get()));
                        return AsIDBTreeModel::index(row, column, fakeParent);
                    }
                }
                // sentinel but unevaluated / error => no children
                return {};
            }
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

    asIDBVariable *childVar = decodeVar(ip);
    if (childVar) {
        for (int i = 0; i < m_watchItems.size(); ++i) {
            const auto &item = *m_watchItems[i];
            if (!item.isValid()) {
                continue;
            }
            auto sp = item.result.value().lock();
            if (!sp) {
                continue;
            }
            if (sp.get() == childVar) {
                return createIndex(
                    i, 0, const_cast<WatchItem *>(m_watchItems[i].get()));
            }
        }
    }

    auto baseParent = AsIDBTreeModel::parent(child);
    if (!baseParent.isValid()) {
        return {};
    }

    void *bpip = baseParent.internalPointer();
    asIDBVariable *bpv = decodeVar(bpip);
    if (bpv) {
        for (int i = 0; i < m_watchItems.size(); ++i) {
            const auto &item = *m_watchItems[i];
            if (!item.isValid()) {
                continue;
            }
            auto sp = item.result.value().lock();
            if (!sp) {
                continue;
            }
            if (sp.get() == bpv) {
                return createIndex(
                    i, 0, const_cast<WatchItem *>(m_watchItems[i].get()));
            }
        }
    }
    return baseParent;
}
