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

#ifndef ASIDBWATCHMODEL_H
#define ASIDBWATCHMODEL_H

#include "asidbtreemodel.h"
#include "class/asdebugger.h"
#include <memory>

class AsIDBWatchModel : public AsIDBTreeModel {
    Q_OBJECT
private:
    struct WatchItem {
        std::string expression;
        asIDBExpected<asIDBVariable::WeakPtr> result;
        bool expanded = false;

        bool isValid() const {
            return result.has_value() && !result.value().expired();
        }
        bool hasError() const { return !result.has_value(); }
    };

public:
    explicit AsIDBWatchModel(QObject *parent = nullptr);

public:
    QStringList expressionList() const;

public slots:
    void attachDebugger(asDebugger *debugger);
    void addWatchExpression(const QString &expression);
    void removeWatchExpression(qsizetype index);
    void removeWatchExpressions(const QModelIndexList &indexes);
    bool editWatchExpression(qsizetype index, const QString &newExpression);
    void refresh();

    void reloadExpressionList(const QStringList &expressions);

public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role) override;

    void clearAll();

public:
    QVariant data(const QModelIndex &index, int role) const override;

    bool hasChildren(const QModelIndex &parent) const override;

    int rowCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent) const override;

    QModelIndex parent(const QModelIndex &child) const override;

private:
    // use shared_ptr so WatchItem* (shared_ptr.get()) remains stable across
    // QVector reallocs
    QVector<std::shared_ptr<WatchItem>> m_watchItems;
    asDebugger *_dbg = nullptr;

    // helper: consistent user role id
    QString makeTopLevelUserRole(const WatchItem &item) const;

    // helper: build roots vector from current m_watchItems
    QVector<asIDBVariable::Ptr> buildRootsFromWatchItems() const;
};

#endif // ASIDBWATCHMODEL_H
