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

#ifndef IDBTREEMODEL_H
#define IDBTREEMODEL_H

#include "debugger/luauvariable.h"

#include <QAbstractItemModel>

#include <memory>
#include <unordered_map>

class IDBTreeModel : public QAbstractItemModel {
    Q_OBJECT
protected:
    struct ProxyNode {
        LuauVariable::Ptr owner;
        LuauScope scope;
        int pageIndex = 0;
        int count = 0;
    };

    // encode/decode internalPointer via tagging: Proxy encoded with LSB=1
    static inline void *encodeProxy(ProxyNode *p) {
        return (void *)(uintptr_t(((uintptr_t)p) | 1));
    }
    static inline void *encodeVar(LuauVariable *v) { return (void *)v; }
    static inline bool isProxy(void *p) { return (((uintptr_t)p) & 1) != 0; }
    static inline ProxyNode *decodeProxy(void *p) {
        return (ProxyNode *)(uintptr_t(((uintptr_t)p) & ~uintptr_t(1)));
    }
    static inline LuauVariable *decodeVar(void *p) { return (LuauVariable *)p; }

public:
    explicit IDBTreeModel(QObject *parent = nullptr);

    void setRoot(LuauVariableRegistry *r, const LuauScope &scope);
    void updateScope(const LuauScope &scope);
    void refresh();
    void setPageSize(int s);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent) const override;

    bool hasChildren(const QModelIndex &parent) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;

    int columnCount(const QModelIndex &p) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

public:
    QVariant getVariableIdentifier(const QModelIndex &index) const;

    bool isProxyNode(const QModelIndex &index) const;

    struct ProxyNodeInfo {
        bool isValid = false;
        QString ownerIdentifier;
        int pageIndex = -1;
        int startIndex = -1;
        int itemCount = 0;
    };

    ProxyNodeInfo getProxyNodeInfo(const QModelIndex &index) const;

    int getPageSize() const;

private:
    LuauVariableRegistry *root = nullptr;
    LuauScope rootScope;
    int pageSize = 100;

private:
    struct ProxyKey {
        LuauVariable *owner;
        int scopeKey;
        int pageIndex;
        inline bool operator==(const ProxyKey &o) const {
            return owner == o.owner && scopeKey == o.scopeKey &&
                   pageIndex == o.pageIndex;
        }
    };
    struct KeyHash {
        inline std::size_t operator()(const ProxyKey &k) const noexcept {
            return std::hash<LuauVariable *>{}(k.owner) ^
                   (std::hash<int>{}(k.scopeKey) << 1) ^
                   (std::hash<int>{}(k.pageIndex) << 2);
        }
    };
    mutable std::unordered_map<ProxyKey, std::shared_ptr<ProxyNode>, KeyHash>
        proxyMap;

protected:
    std::shared_ptr<ProxyNode> getOrCreateProxy(const LuauScope &scope,
                                                int pageIndex) const;
    std::shared_ptr<ProxyNode> getOrCreateProxy(const LuauVariable::Ptr &owner,
                                                int pageIndex) const;

    // find owner's index recursively (best-effort)
    QModelIndex findIndexForOwner(const LuauVariable::Ptr &owner) const;

    // map raw pointer to stored shared_ptr (recursive search)
    LuauVariable::Ptr getPtrFromRaw(LuauVariable *raw) const;
};

#endif // IDBTREEMODEL_H
