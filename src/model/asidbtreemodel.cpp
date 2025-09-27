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

#include "asidbtreemodel.h"

AsIDBTreeModel::AsIDBTreeModel(QObject *parent) : QAbstractItemModel(parent) {}

void AsIDBTreeModel::setRoot(const asIDBVariable::Ptr &r) {
    QVector<asIDBVariable::Ptr> roots;
    if (r) {
        for (auto &item : r->namedProps) {
            roots.append(item);
        }
        for (auto &item : r->indexedProps) {
            roots.append(item);
        }
    }
    setRoots(roots);
}

void AsIDBTreeModel::setRoots(const QVector<asIDBVariable::Ptr> &r) {
    beginResetModel();
    roots = r;
    proxyMap.clear();
    endResetModel();
}

void AsIDBTreeModel::setPageSize(int s) { pageSize = s; }

QModelIndex AsIDBTreeModel::index(int row, int column,
                                  const QModelIndex &parent) const {
    if (row < 0 || column < 0)
        return {};
    if (!parent.isValid()) {
        if (static_cast<size_t>(row) >= roots.size())
            return {};
        return createIndex(row, column, encodeVar(roots[row].get()));
    }

    void *ip = parent.internalPointer();
    if (isProxy(ip)) {
        ProxyNode *pp = decodeProxy(ip);
        if (!pp || !pp->owner)
            return {};
        auto owner = pp->owner;

        if (owner->expandable && !owner->expanded)
            const_cast<asIDBVariable *>(owner.get())->Expand();

        int total = static_cast<int>(owner->indexedProps.size());
        int start = pp->pageIndex * pageSize;
        int show = std::min(pageSize, total - start);

        QVector<void *> children;
        for (int i = 0; i < show; ++i) {
            if (start + i < total) {
                children.push_back(
                    encodeVar(owner->indexedProps[start + i].get()));
            }
        }

        if (start + show < total) {
            auto nested = getOrCreateProxy(owner, pp->pageIndex + 1);
            children.push_back(encodeProxy(nested.get()));
        }

        if (static_cast<size_t>(row) < children.size())
            return createIndex(row, column, children[row]);
        return {};
    } else {
        asIDBVariable *pv = decodeVar(ip);
        auto pptr = getPtrFromRaw(pv);
        if (!pptr)
            return {};

        if (pptr->expandable && !pptr->expanded)
            const_cast<asIDBVariable *>(pptr.get())->Expand();

        QVector<void *> children;
        for (const auto &n : pptr->namedProps)
            children.append(encodeVar(n.get()));

        int total = static_cast<int>(pptr->indexedProps.size());
        int show = std::min(total, pageSize);

        for (int i = 0; i < show; ++i) {
            children.append(encodeVar(pptr->indexedProps[i].get()));
        }

        if (total > show) {
            auto proxy = getOrCreateProxy(pptr, 1);
            children.append(encodeProxy(proxy.get()));
        }

        if (static_cast<size_t>(row) < children.size())
            return createIndex(row, column, children[row]);
        return {};
    }
}

bool AsIDBTreeModel::hasChildren(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return !roots.empty();
    }

    void *ip = parent.internalPointer();
    if (!ip) {
        return false;
    }

    if (isProxy(ip)) {
        ProxyNode *p = decodeProxy(ip);
        if (!p || !p->owner) {
            return false;
        }

        auto owner = p->owner;
        owner->Evaluate();
        if (owner->expandable && !owner->expanded) {
            return owner->expandable;
        }

        int total = static_cast<int>(owner->indexedProps.size());
        int start = p->pageIndex * pageSize;

        return (start < total) && (start + pageSize < total ||
                                   std::min(pageSize, total - start) > 0);
    } else {
        asIDBVariable *pv = decodeVar(ip);
        auto pptr = getPtrFromRaw(pv);
        if (!pptr) {
            return false;
        }

        pptr->Evaluate();
        if (pptr->expandable) {
            return true;
        }

        return pptr->expandable || !pptr->namedProps.empty() ||
               !pptr->indexedProps.empty();
    }
}

QModelIndex AsIDBTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid())
        return {};

    void *ip = child.internalPointer();
    if (isProxy(ip)) {
        ProxyNode *proxy = decodeProxy(ip);
        if (!proxy || !proxy->owner)
            return {};

        auto owner = proxy->owner;

        if (proxy->pageIndex == 1) {
            for (size_t i = 0; i < roots.size(); ++i) {
                if (roots[i].get() == owner.get()) {
                    return createIndex(static_cast<int>(i), 0,
                                       encodeVar(roots[i].get()));
                }
            }
            return findIndexForOwner(owner);
        }

        auto parentProxy = getOrCreateProxy(owner, proxy->pageIndex - 1);

        int parentStart = (proxy->pageIndex - 1) * pageSize;
        int total = static_cast<int>(owner->indexedProps.size());
        int parentShow = std::min(pageSize, total - parentStart);

        int row = parentShow;
        return createIndex(row, 0, encodeProxy(parentProxy.get()));
    } else {
        asIDBVariable *childVar = decodeVar(ip);
        auto childPtr = getPtrFromRaw(childVar);
        if (!childPtr)
            return {};

        auto owner = childPtr->owner.lock();
        if (!owner)
            return {};

        int namedIndex = 0;
        for (const auto &n : owner->namedProps) {
            if (n.get() == childPtr.get()) {
                for (size_t i = 0; i < roots.size(); ++i) {
                    if (roots[i].get() == owner.get()) {
                        return createIndex(static_cast<int>(i), 0,
                                           encodeVar(roots[i].get()));
                    }
                }
                return findIndexForOwner(owner);
            }
            namedIndex++;
        }

        int totalIndexed = static_cast<int>(owner->indexedProps.size());
        for (int i = 0; i < totalIndexed; ++i) {
            if (owner->indexedProps[i].get() == childPtr.get()) {
                int pageIndex = i / pageSize;

                if (pageIndex == 0) {
                    for (size_t j = 0; j < roots.size(); ++j) {
                        if (roots[j].get() == owner.get()) {
                            return createIndex(static_cast<int>(j), 0,
                                               encodeVar(roots[j].get()));
                        }
                    }
                    return findIndexForOwner(owner);
                } else {
                    auto proxy = getOrCreateProxy(owner, pageIndex);

                    int start = pageIndex * pageSize;
                    int indexInPage = i - start;

                    return createIndex(indexInPage, 0,
                                       encodeProxy(proxy.get()));
                }
            }
        }

        return {};
    }
}

int AsIDBTreeModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid())
        return static_cast<int>(roots.size());

    void *ip = parent.internalPointer();
    if (!ip)
        return 0;

    if (isProxy(ip)) {
        ProxyNode *p = decodeProxy(ip);
        if (!p || !p->owner)
            return 0;

        auto owner = p->owner;
        if (owner->expandable && !owner->expanded)
            const_cast<asIDBVariable *>(owner.get())->Expand();

        int total = static_cast<int>(owner->indexedProps.size());
        int start = p->pageIndex * pageSize;

        if (start >= total)
            return 0;

        int show = std::min(pageSize, total - start);
        bool hasMore = (start + show) < total;

        return show + (hasMore ? 1 : 0);
    } else {
        asIDBVariable *pv = decodeVar(ip);
        auto pptr = getPtrFromRaw(pv);
        if (!pptr)
            return 0;

        if (pptr->expandable && !pptr->expanded)
            const_cast<asIDBVariable *>(pptr.get())->Expand();

        int totalNamed = static_cast<int>(pptr->namedProps.size());
        int totalIndexed = static_cast<int>(pptr->indexedProps.size());
        int show = std::min(totalIndexed, pageSize);

        return totalNamed + show + (totalIndexed > show ? 1 : 0);
    }
}

int AsIDBTreeModel::columnCount(const QModelIndex &p) const {
    Q_UNUSED(p);
    return 2;
}

QVariant AsIDBTreeModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Value");
            }
        }
    }
    return QVariant();
}

QVariant AsIDBTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        break;
    case Qt::UserRole:
        return getVariableIdentifier(index);
    default:
        return {};
    }

    void *ip = index.internalPointer();
    if (!ip) {
        return {};
    }
    if (isProxy(ip)) {
        ProxyNode *p = decodeProxy(ip);
        if (!p || !p->owner) {
            return {};
        }
        if (index.column() == 0) {
            return QStringLiteral("Load more...");
        } else {
            return {};
        }
    }
    asIDBVariable *v = decodeVar(ip);
    auto vp = getPtrFromRaw(v);
    if (!vp) {
        return {};
    }
    if (index.column() == 0) {
        return QString::fromStdString(vp->identifier.Combine());
    }
    if (!vp->evaluated) {
        const_cast<asIDBVariable *>(vp.get())->Evaluate();
    }
    return QString::fromStdString(vp->value);
}

Qt::ItemFlags AsIDBTreeModel::flags(const QModelIndex &idx) const {
    if (!idx.isValid()) {
        return {};
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant AsIDBTreeModel::getVariableIdentifier(const QModelIndex &index) const {
    if (!index.isValid())
        return QVariant();

    void *ip = index.internalPointer();
    if (isProxy(ip)) {
        auto info = getProxyNodeInfo(index);
        if (info.isValid) {
            return QStringLiteral("proxy_%1_%2")
                .arg(info.ownerIdentifier)
                .arg(info.pageIndex);
        }
    } else {
        asIDBVariable *v = decodeVar(ip);
        auto vp = getPtrFromRaw(v);
        if (vp) {
            return QStringLiteral("var_%1_%2")
                .arg(QString::fromStdString(vp->identifier.Combine()),
                     QString::fromStdString(vp->typeName));
        }
    }

    return QVariant();
}

bool AsIDBTreeModel::isProxyNode(const QModelIndex &index) const {
    if (!index.isValid())
        return false;
    void *ip = index.internalPointer();
    return isProxy(ip);
}

int AsIDBTreeModel::getPageSize() const { return pageSize; }

AsIDBTreeModel::ProxyNodeInfo
AsIDBTreeModel::getProxyNodeInfo(const QModelIndex &index) const {
    ProxyNodeInfo info;
    if (!index.isValid() || !isProxyNode(index)) {
        return info;
    }

    void *ip = index.internalPointer();
    ProxyNode *proxy = decodeProxy(ip);
    if (!proxy || !proxy->owner) {
        return info;
    }

    info.isValid = true;
    info.pageIndex = proxy->pageIndex;
    info.startIndex = proxy->pageIndex * pageSize;

    auto owner = proxy->owner;
    info.ownerIdentifier = QString::fromStdString(owner->identifier.Combine());

    int total = static_cast<int>(owner->indexedProps.size());
    info.itemCount = std::min(pageSize, total - info.startIndex);

    return info;
}

QModelIndex
AsIDBTreeModel::findIndexForOwner(const asIDBVariable::Ptr &owner) const {
    for (size_t i = 0; i < roots.size(); ++i)
        if (roots[i].get() == owner.get())
            return createIndex((int)i, 0, encodeVar(roots[i].get()));
    std::function<QModelIndex(const asIDBVariable::Ptr &)> rec;
    rec = [&](const asIDBVariable::Ptr &node) -> QModelIndex {
        if (node->expandable && !node->expanded)
            const_cast<asIDBVariable *>(node.get())->Expand();
        int row = 0;
        for (const auto &n : node->namedProps) {
            if (n.get() == owner.get())
                return createIndex(row, 0, encodeVar(n.get()));
            auto s = rec(n);
            if (s.isValid())
                return s;
            ++row;
        }
        int total = (int)node->indexedProps.size();
        int show = std::min(total, pageSize);
        for (int i = 0; i < show; ++i) {
            auto c = node->indexedProps[i];
            if (c.get() == owner.get())
                return createIndex(row, 0, encodeVar(c.get()));
            auto s = rec(c);
            if (s.isValid())
                return s;
            ++row;
        }
        for (int i = show; i < total; ++i)
            if (node->indexedProps[i].get() == owner.get())
                return createIndex(row, 0, encodeVar(node.get()));
        return {};
    };
    for (const auto &r : roots) {
        auto s = rec(r);
        if (s.isValid())
            return s;
    }
    return {};
}

asIDBVariable::Ptr AsIDBTreeModel::getPtrFromRaw(asIDBVariable *raw) const {
    if (!raw)
        return {};
    std::function<asIDBVariable::Ptr(const asIDBVariable::Ptr &)> rec;
    rec = [&](const asIDBVariable::Ptr &n) -> asIDBVariable::Ptr {
        if (n.get() == raw)
            return n;
        for (const auto &p : n->namedProps) {
            auto r = rec(p);
            if (r)
                return r;
        }
        for (const auto &p : n->indexedProps) {
            auto r = rec(p);
            if (r)
                return r;
        }
        return {};
    };
    for (const auto &r : roots) {
        auto f = rec(r);
        if (f)
            return f;
    }
    return {};
}

void AsIDBTreeModel::replaceRoots(const QVector<asIDBVariable::Ptr> &newRoots) {
    roots = newRoots;
    proxyMap.clear();
}

std::shared_ptr<AsIDBTreeModel::ProxyNode>
AsIDBTreeModel::getOrCreateProxy(const asIDBVariable::Ptr &owner,
                                 int pageIndex) const {
    ProxyKey key{owner.get(), pageIndex};
    auto it = proxyMap.find(key);
    if (it != proxyMap.end())
        return it->second;

    auto p = std::make_shared<ProxyNode>();
    p->owner = owner;
    p->pageIndex = pageIndex;
    proxyMap[key] = p;
    return p;
}
