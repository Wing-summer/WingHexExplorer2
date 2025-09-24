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

#ifndef ASIDBTREEMODEL_H
#define ASIDBTREEMODEL_H

#include "as-debugger/as_debugger.h"

#include <QAbstractItemModel>

class AsIDBTreeModel : public QAbstractItemModel {
    Q_OBJECT
protected:
    struct ProxyNode {
        asIDBVariable::Ptr owner;
        int pageIndex = 0;
        int count = 0;
    };

    // encode/decode internalPointer via tagging: Proxy encoded with LSB=1
    static inline void *encodeProxy(ProxyNode *p) {
        return (void *)(uintptr_t(((uintptr_t)p) | 1));
    }
    static inline void *encodeVar(asIDBVariable *v) { return (void *)v; }
    static inline bool isProxy(void *p) { return (((uintptr_t)p) & 1) != 0; }
    static inline ProxyNode *decodeProxy(void *p) {
        return (ProxyNode *)(uintptr_t(((uintptr_t)p) & ~uintptr_t(1)));
    }
    static inline asIDBVariable *decodeVar(void *p) {
        return (asIDBVariable *)p;
    }

public:
    explicit AsIDBTreeModel(QObject *parent = nullptr);

    void setRoot(const asIDBVariable::Ptr &r);
    void setRoots(const QVector<asIDBVariable::Ptr> &r);
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
    QVector<asIDBVariable::Ptr> roots;
    int pageSize = 100;

private:
    struct ProxyKey {
        asIDBVariable *owner;
        int pageIndex;
        inline bool operator==(const ProxyKey &o) const {
            return owner == o.owner && pageIndex == o.pageIndex;
        }
    };
    struct KeyHash {
        inline size_t operator()(const ProxyKey &k) const noexcept {
            return ((uintptr_t)k.owner) ^ (k.pageIndex * 0x9e3779b1);
        }
    };
    mutable std::unordered_map<ProxyKey, std::shared_ptr<ProxyNode>, KeyHash>
        proxyMap;

protected:
    std::shared_ptr<ProxyNode> getOrCreateProxy(const asIDBVariable::Ptr &owner,
                                                int pageIndex) const;

    // find owner's index recursively (best-effort)
    QModelIndex findIndexForOwner(const asIDBVariable::Ptr &owner) const;

    // map raw pointer to stored shared_ptr (recursive search)
    asIDBVariable::Ptr getPtrFromRaw(asIDBVariable *raw) const;
};

#endif // ASIDBTREEMODEL_H
