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

#include "class/scriptmachine.h"
#include "debugger/luauvariableregistry.h"

#include <algorithm>
#include <functional>

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
    root = r;
    rootScope = scope;
    refresh();
}

void IDBTreeModel::updateScope(const LuauScope &scope) {
    rootScope = scope;
    refresh();
}

void IDBTreeModel::refresh() {
    beginResetModel();
    auto L = rootScope.getLuaState();
    if (root && L) {
        proxyMap.clear();
        root->update(ScriptMachine::instance().getThreadAncestors(L));
    }
    endResetModel();
}

void IDBTreeModel::setPageSize(int s) {
    s = std::max(1, s);
    if (pageSize == s) {
        return;
    }

    beginResetModel();
    pageSize = s;
    proxyMap.clear();
    endResetModel();
}

QModelIndex IDBTreeModel::index(int row, int column,
                                const QModelIndex &parent) const {
    if (row < 0 || column < 0 || column >= columnCount(parent) || !root ||
        rootScope.getType() == LuauScope::Type::Unknown) {
        return {};
    }

    LuauVariableList variables;
    LuauVariable::Ptr owner;
    int pageIndex = 0;

    if (!parent.isValid()) {
        variables = root->getVariables(rootScope, true);
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
            variables = root->getVariables(proxy->scope, true);
        } else {
            auto *rawOwner = decodeVar(ip);
            if (!rawOwner || !rawOwner->hasFields()) {
                return {};
            }
            owner = getPtrFromRaw(rawOwner);
            variables = root->getVariables(rawOwner->getScope(), true);
        }
    }

    if (!variables) {
        return {};
    }

    const int start = pageIndex * pageSize;
    const int total = static_cast<int>(variables->size());
    const int visibleCount = std::max(0, std::min(pageSize, total - start));

    if (row < visibleCount) {
        return createIndex(row, column,
                           encodeVar(variables->at(start + row).get()));
    }

    if (row == visibleCount && start + visibleCount < total) {
        auto proxy = owner ? getOrCreateProxy(owner, pageIndex + 1)
                           : getOrCreateProxy(rootScope, pageIndex + 1);
        return createIndex(row, column, encodeProxy(proxy.get()));
    }

    return {};
}

bool IDBTreeModel::hasChildren(const QModelIndex &parent) const {
    if (!root || rootScope.getType() == LuauScope::Type::Unknown) {
        return false;
    }

    if (!parent.isValid()) {
        auto variables = root->getVariables(rootScope, true);
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
        auto variables = root->getVariables(proxy->scope, false);
        if (!variables) {
            return false;
        }
        return proxy->pageIndex * pageSize <
               static_cast<int>(variables->size());
    }

    auto *variable = decodeVar(ip);
    return variable && variable->hasFields();
}

QModelIndex IDBTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid() || !root) {
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
                         : getOrCreateProxy(rootScope, proxy->pageIndex - 1);
        auto variables = root->getVariables(proxy->scope, false);
        if (!variables) {
            return {};
        }

        const int parentStart = (proxy->pageIndex - 1) * pageSize;
        const int parentShow =
            std::max(0, std::min(pageSize, static_cast<int>(variables->size()) -
                                               parentStart));
        return createIndex(parentShow, 0, encodeProxy(parentProxy.get()));
    }

    auto variable = getPtrFromRaw(decodeVar(ip));
    if (!variable) {
        return {};
    }

    auto rootVariables = root->getVariables(rootScope, false);
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

        const LuauScope scope = owner ? owner->getScope() : rootScope;
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
            auto children = root->getVariables(current->getScope(), false);
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

    const int pageIndex = location.row / pageSize;
    if (pageIndex == 0) {
        if (!location.owner) {
            return {};
        }
        return findIndexForOwner(location.owner);
    }

    auto proxy = location.owner ? getOrCreateProxy(location.owner, pageIndex)
                                : getOrCreateProxy(rootScope, pageIndex);
    auto variables = root->getVariables(location.scope, false);
    if (!variables) {
        return {};
    }

    const int parentStart = (pageIndex - 1) * pageSize;
    const int proxyRow =
        std::max(0, std::min(pageSize, static_cast<int>(variables->size()) -
                                           parentStart));
    return createIndex(proxyRow, 0, encodeProxy(proxy.get()));
}

int IDBTreeModel::rowCount(const QModelIndex &parent) const {
    if (!root || rootScope.getType() == LuauScope::Type::Unknown) {
        return 0;
    }

    LuauVariableList variables;
    int pageIndex = 0;

    if (!parent.isValid()) {
        variables = root->getVariables(rootScope, true);
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
            variables = root->getVariables(proxy->scope, true);
        } else {
            auto *variable = decodeVar(ip);
            if (!variable || !variable->hasFields()) {
                return 0;
            }
            variables = root->getVariables(variable->getScope(), true);
        }
    }

    if (!variables) {
        return 0;
    }

    const int start = pageIndex * pageSize;
    const int total = static_cast<int>(variables->size());
    if (start >= total) {
        return 0;
    }

    const int visibleCount = std::min(pageSize, total - start);
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
            return tr("LoadMore...");
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

int IDBTreeModel::getPageSize() const { return pageSize; }

IDBTreeModel::ProxyNodeInfo
IDBTreeModel::getProxyNodeInfo(const QModelIndex &index) const {
    if (!index.isValid() || !isProxyNode(index) || !root) {
        return {};
    }

    auto *proxy = decodeProxy(index.internalPointer());
    if (!proxy) {
        return {};
    }

    ProxyNodeInfo info;
    info.isValid = true;
    info.pageIndex = proxy->pageIndex;
    info.startIndex = proxy->pageIndex * pageSize;
    info.ownerIdentifier =
        proxy->owner ? proxy->owner->getName() : proxy->scope.getName();

    auto variables = root->getVariables(proxy->scope, false);
    if (!variables) {
        info.itemCount = 0;
        return info;
    }

    const int total = static_cast<int>(variables->size());
    info.itemCount = std::max(0, std::min(pageSize, total - info.startIndex));
    return info;
}

QModelIndex
IDBTreeModel::findIndexForOwner(const LuauVariable::Ptr &owner) const {
    if (!owner || !root) {
        return {};
    }

    auto rootVariables = root->getVariables(rootScope, false);
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

        const LuauScope scope = parent ? parent->getScope() : rootScope;
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
            auto children = root->getVariables(current->getScope(), false);
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

    return createIndex(location.row % pageSize, 0, encodeVar(owner.get()));
}

LuauVariable::Ptr IDBTreeModel::getPtrFromRaw(LuauVariable *raw) const {
    if (!raw || !root || rootScope.getType() == LuauScope::Type::Unknown) {
        return {};
    }

    auto rootVariables = root->getVariables(rootScope, false);
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
            auto children = root->getVariables(variable->getScope(), false);
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
    auto it = proxyMap.find(key);
    if (it != proxyMap.end()) {
        return it->second;
    }

    auto proxy = std::make_shared<ProxyNode>();
    proxy->scope = scope;
    proxy->pageIndex = pageIndex;
    proxyMap[key] = proxy;
    return proxy;
}

std::shared_ptr<IDBTreeModel::ProxyNode>
IDBTreeModel::getOrCreateProxy(const LuauVariable::Ptr &owner,
                               int pageIndex) const {
    if (!owner) {
        return getOrCreateProxy(rootScope, pageIndex);
    }

    ProxyKey key{owner.get(), owner->getScope().getKey(), pageIndex};
    auto it = proxyMap.find(key);
    if (it != proxyMap.end()) {
        return it->second;
    }

    auto proxy = std::make_shared<ProxyNode>();
    proxy->owner = owner;
    proxy->scope = owner->getScope();
    proxy->pageIndex = pageIndex;
    proxyMap[key] = proxy;
    return proxy;
}
