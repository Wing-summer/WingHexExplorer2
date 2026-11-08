/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */

#include "idbtreemodel.h"

#include <algorithm>
#include <functional>

#include "luau/luauvariableregistry.h"
#include "luau/runtime/scriptmachine.h"

namespace {
constexpr int NameColumn = 0;
constexpr int ValueColumn = 1;

struct VariableLocation {
    bool found = false;
    LuauVariable::Ptr owner;
    LuauVariable::Ptr variable;
    LuauScope scope;
    int row = -1;
};
} // namespace

IDBTreeModel::IDBTreeModel(QObject *parent) : QAbstractItemModel(parent) {}

void IDBTreeModel::setRoot(LuauVariableRegistry *r, const LuauScope &scope) {
    m_root = r;
    m_rootScope = scope;
    refresh();
}

void IDBTreeModel::resetRoot() {
    m_root = nullptr;
    m_rootScope = LuauScope();
    refresh();
}

void IDBTreeModel::updateScope(const LuauScope &scope) {
    m_rootScope = scope;
    refresh();
}

void IDBTreeModel::refresh() {
    beginResetModel();
    m_proxyMap.clear();
    auto L = m_rootScope.getLuaState();
    if (m_root && L) {
        m_root->update(ScriptMachine::instance().getThreadAncestors(L));
    }
    endResetModel();
}

void IDBTreeModel::setPageSize(int s) {
    s = std::max(1, s);
    if (m_pageSize == s) {
        return;
    }

    beginResetModel();
    m_pageSize = s;
    m_proxyMap.clear();
    endResetModel();
}

QModelIndex IDBTreeModel::index(int row, int column,
                                const QModelIndex &parent) const {
    if (row < 0 || column < 0 || column >= columnCount(parent) || !m_root ||
        m_rootScope.getType() == LuauScope::Type::Unknown) {
        return {};
    }

    LuauVariableList variables;
    LuauVariable::Ptr owner;
    int pageIndex = 0;

    if (!parent.isValid()) {
        variables = m_root->getVariables(m_rootScope, true);
    } else {
        void *ip = parent.internalPointer();
        if (!ip) {
            return {};
        }

        if (isProxy(ip)) {
            auto *proxy = decodeProxy(ip);
            if (!proxy) {
                return {};
            }
            owner = proxy->owner;
            pageIndex = proxy->pageIndex;
            variables = m_root->getVariables(proxy->scope, true);
        } else {
            auto *rawOwner = decodeVar(ip);
            if (!rawOwner || !rawOwner->hasFields()) {
                return {};
            }
            owner = getPtrFromRaw(rawOwner);
            variables = m_root->getVariables(rawOwner->getScope(), true);
        }
    }

    if (!variables) {
        return {};
    }

    const int start = pageIndex * m_pageSize;
    const int total = static_cast<int>(variables->size());
    const int visibleCount = std::max(0, std::min(m_pageSize, total - start));

    if (row < visibleCount) {
        return createIndex(row, column,
                           encodeVar(variables->at(start + row).get()));
    }

    if (row == visibleCount && start + visibleCount < total) {
        auto proxy = owner ? getOrCreateProxy(owner, pageIndex + 1)
                           : getOrCreateProxy(m_rootScope, pageIndex + 1);
        return createIndex(row, column, encodeProxy(proxy.get()));
    }

    return {};
}

bool IDBTreeModel::hasChildren(const QModelIndex &parent) const {
    if (!m_root || m_rootScope.getType() == LuauScope::Type::Unknown) {
        return false;
    }

    if (!parent.isValid()) {
        auto variables = m_root->getVariables(m_rootScope, true);
        return variables && !variables->isEmpty();
    }

    void *ip = parent.internalPointer();
    if (!ip) {
        return false;
    }

    if (isProxy(ip)) {
        auto *proxy = decodeProxy(ip);
        if (!proxy) {
            return false;
        }
        auto variables = m_root->getVariables(proxy->scope, false);
        if (!variables) {
            return false;
        }
        return proxy->pageIndex * m_pageSize <
               static_cast<int>(variables->size());
    }

    auto *variable = decodeVar(ip);
    return variable && variable->hasContent();
}

QModelIndex IDBTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid() || !m_root) {
        return {};
    }

    void *ip = child.internalPointer();
    if (!ip) {
        return {};
    }

    if (isProxy(ip)) {
        auto *proxy = decodeProxy(ip);
        if (!proxy || proxy->pageIndex <= 0) {
            return {};
        }

        if (proxy->pageIndex == 1) {
            if (!proxy->owner) {
                return {};
            }
            return findIndexForOwner(proxy->owner);
        }

        auto parentProxy =
            proxy->owner ? getOrCreateProxy(proxy->owner, proxy->pageIndex - 1)
                         : getOrCreateProxy(m_rootScope, proxy->pageIndex - 1);
        auto variables = m_root->getVariables(proxy->scope, false);
        if (!variables) {
            return {};
        }

        const int parentStart = (proxy->pageIndex - 1) * m_pageSize;
        const int parentShow = std::max(
            0, std::min(m_pageSize,
                        static_cast<int>(variables->size()) - parentStart));
        return createIndex(parentShow, 0, encodeProxy(parentProxy.get()));
    }

    auto variable = getPtrFromRaw(decodeVar(ip));
    if (!variable) {
        return {};
    }

    auto rootVariables = m_root->getVariables(m_rootScope, false);
    if (!rootVariables) {
        return {};
    }

    std::function<VariableLocation(const LuauVariableList &,
                                   const LuauVariable::Ptr &)>
        locate;
    locate = [&](const LuauVariableList &variables,
                 const LuauVariable::Ptr &owner) -> VariableLocation {
        if (!variables) {
            return {};
        }

        const LuauScope scope = owner ? owner->getScope() : m_rootScope;
        for (int row = 0; row < variables->size(); ++row) {
            auto current = variables->at(row);
            if (current.get() == variable.get()) {
                return {true, owner, current, scope, row};
            }
        }

        for (auto &current : *variables) {
            if (!current || !current->hasFields()) {
                continue;
            }
            auto children = m_root->getVariables(current->getScope(), false);
            auto found = locate(children, current);
            if (found.found) {
                return found;
            }
        }

        return {};
    };

    auto location = locate(rootVariables, {});
    if (!location.found) {
        return {};
    }

    const int pageIndex = location.row / m_pageSize;
    if (pageIndex == 0) {
        if (!location.owner) {
            return {};
        }
        return findIndexForOwner(location.owner);
    }

    auto proxy = location.owner ? getOrCreateProxy(location.owner, pageIndex)
                                : getOrCreateProxy(m_rootScope, pageIndex);
    auto variables = m_root->getVariables(location.scope, false);
    if (!variables) {
        return {};
    }

    const int parentStart = (pageIndex - 1) * m_pageSize;
    const int proxyRow =
        std::max(0, std::min(m_pageSize, static_cast<int>(variables->size()) -
                                             parentStart));
    return createIndex(proxyRow, 0, encodeProxy(proxy.get()));
}

int IDBTreeModel::rowCount(const QModelIndex &parent) const {
    if (!m_root || m_rootScope.getType() == LuauScope::Type::Unknown) {
        return 0;
    }

    LuauVariableList variables;
    int pageIndex = 0;

    if (!parent.isValid()) {
        variables = m_root->getVariables(m_rootScope, true);
    } else {
        void *ip = parent.internalPointer();
        if (!ip) {
            return 0;
        }

        if (isProxy(ip)) {
            auto *proxy = decodeProxy(ip);
            if (!proxy) {
                return 0;
            }
            pageIndex = proxy->pageIndex;
            variables = m_root->getVariables(proxy->scope, true);
        } else {
            auto *variable = decodeVar(ip);
            if (!variable || !variable->hasFields()) {
                return 0;
            }
            variables = m_root->getVariables(variable->getScope(), true);
        }
    }

    if (!variables) {
        return 0;
    }

    const int start = pageIndex * m_pageSize;
    const int total = static_cast<int>(variables->size());
    if (start >= total) {
        return 0;
    }

    const int visibleCount = std::min(m_pageSize, total - start);
    return visibleCount + (start + visibleCount < total ? 1 : 0);
}

int IDBTreeModel::columnCount(const QModelIndex &p) const {
    Q_UNUSED(p);
    return 2;
}

QVariant IDBTreeModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
    case NameColumn:
        return tr("Name");
    case ValueColumn:
        return tr("Value");
    default:
        return {};
    }
}

QVariant IDBTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    void *ip = index.internalPointer();
    if (!ip) {
        return {};
    }

    if (role == Qt::UserRole) {
        return getVariableIdentifier(index);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    if (isProxy(ip)) {
        if (index.column() == NameColumn) {
            return tr("Load more...");
        }
        return {};
    }

    auto variable = getPtrFromRaw(decodeVar(ip));
    if (!variable) {
        return {};
    }

    if (index.column() == NameColumn) {
        return variable->getName();
    }
    if (index.column() == ValueColumn) {
        if (role == Qt::DecorationRole) {
            // TODO color type
        }
        return variable->getValue();
    }

    return {};
}

Qt::ItemFlags IDBTreeModel::flags(const QModelIndex &idx) const {
    if (!idx.isValid()) {
        return {};
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant IDBTreeModel::getVariableIdentifier(const QModelIndex &index) const {
    if (!index.isValid()) {
        return {};
    }

    void *ip = index.internalPointer();
    if (!ip) {
        return {};
    }

    if (isProxy(ip)) {
        auto info = getProxyNodeInfo(index);
        if (!info.isValid) {
            return {};
        }
        return QStringLiteral("proxy_%1_%2_%3")
            .arg(info.ownerIdentifier)
            .arg(info.startIndex)
            .arg(info.pageIndex);
    }

    auto variable = getPtrFromRaw(decodeVar(ip));
    if (!variable) {
        return {};
    }

    return QStringLiteral("var_%1_%2_%3")
        .arg(variable->getScope().getKey())
        .arg(variable->getName(), variable->getType());
}

bool IDBTreeModel::isProxyNode(const QModelIndex &index) const {
    return index.isValid() && isProxy(index.internalPointer());
}

int IDBTreeModel::getPageSize() const { return m_pageSize; }

IDBTreeModel::ProxyNodeInfo
IDBTreeModel::getProxyNodeInfo(const QModelIndex &index) const {
    if (!index.isValid() || !isProxyNode(index) || !m_root) {
        return {};
    }

    auto *proxy = decodeProxy(index.internalPointer());
    if (!proxy) {
        return {};
    }

    ProxyNodeInfo info;
    info.isValid = true;
    info.pageIndex = proxy->pageIndex;
    info.startIndex = proxy->pageIndex * m_pageSize;
    info.ownerIdentifier =
        proxy->owner ? proxy->owner->getName() : proxy->scope.getName();

    auto variables = m_root->getVariables(proxy->scope, false);
    if (!variables) {
        info.itemCount = 0;
        return info;
    }

    const int total = static_cast<int>(variables->size());
    info.itemCount = std::max(0, std::min(m_pageSize, total - info.startIndex));
    return info;
}

QModelIndex
IDBTreeModel::findIndexForOwner(const LuauVariable::Ptr &owner) const {
    if (!owner || !m_root) {
        return {};
    }

    auto rootVariables = m_root->getVariables(m_rootScope, false);
    if (!rootVariables) {
        return {};
    }

    std::function<VariableLocation(const LuauVariableList &,
                                   const LuauVariable::Ptr &)>
        locate;
    locate = [&](const LuauVariableList &variables,
                 const LuauVariable::Ptr &parent) -> VariableLocation {
        if (!variables) {
            return {};
        }

        const LuauScope scope = parent ? parent->getScope() : m_rootScope;
        for (int row = 0; row < variables->size(); ++row) {
            auto current = variables->at(row);
            if (current.get() == owner.get()) {
                return {true, parent, current, scope, row};
            }
        }

        for (auto &current : *variables) {
            if (!current || !current->hasFields()) {
                continue;
            }
            auto children = m_root->getVariables(current->getScope(), false);
            auto found = locate(children, current);
            if (found.found) {
                return found;
            }
        }

        return {};
    };

    auto location = locate(rootVariables, {});
    if (!location.found) {
        return {};
    }

    return createIndex(location.row % m_pageSize, 0, encodeVar(owner.get()));
}

LuauVariable::Ptr IDBTreeModel::getPtrFromRaw(LuauVariable *raw) const {
    if (!raw || !m_root || m_rootScope.getType() == LuauScope::Type::Unknown) {
        return {};
    }

    auto rootVariables = m_root->getVariables(m_rootScope, false);
    if (!rootVariables) {
        return {};
    }

    std::function<LuauVariable::Ptr(const LuauVariableList &)> find;
    find = [&](const LuauVariableList &variables) -> LuauVariable::Ptr {
        if (!variables) {
            return {};
        }

        for (auto &variable : *variables) {
            if (!variable) {
                continue;
            }
            if (variable.get() == raw) {
                return variable;
            }
        }

        for (auto &variable : *variables) {
            if (!variable || !variable->hasFields()) {
                continue;
            }
            auto children = m_root->getVariables(variable->getScope(), false);
            auto found = find(children);
            if (found) {
                return found;
            }
        }

        return {};
    };

    return find(rootVariables);
}

std::shared_ptr<IDBTreeModel::ProxyNode>
IDBTreeModel::getOrCreateProxy(const LuauScope &scope, int pageIndex) const {
    ProxyKey key{nullptr, scope.getKey(), pageIndex};
    auto it = m_proxyMap.find(key);
    if (it != m_proxyMap.end()) {
        return it->second;
    }

    auto proxy = std::make_shared<ProxyNode>();
    proxy->scope = scope;
    proxy->pageIndex = pageIndex;
    m_proxyMap[key] = proxy;
    return proxy;
}

std::shared_ptr<IDBTreeModel::ProxyNode>
IDBTreeModel::getOrCreateProxy(const LuauVariable::Ptr &owner,
                               int pageIndex) const {
    if (!owner) {
        return getOrCreateProxy(m_rootScope, pageIndex);
    }

    ProxyKey key{owner.get(), owner->getScope().getKey(), pageIndex};
    auto it = m_proxyMap.find(key);
    if (it != m_proxyMap.end()) {
        return it->second;
    }

    auto proxy = std::make_shared<ProxyNode>();
    proxy->owner = owner;
    proxy->scope = owner->getScope();
    proxy->pageIndex = pageIndex;
    m_proxyMap[key] = proxy;
    return proxy;
}
