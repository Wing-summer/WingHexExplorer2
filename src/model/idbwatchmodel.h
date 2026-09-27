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

#ifndef IDBWATCHMODEL_H
#define IDBWATCHMODEL_H

#include "idbtreemodel.h"
#include "luau/luaudebugger.h"

#include <QStringList>
#include <QVector>

class IDBWatchModel final : public IDBTreeModel {
    Q_OBJECT

private:
    struct WatchItem {
        QString expression;
        LuauVariable::Ptr result;
    };

public:
    explicit IDBWatchModel(QObject *parent = nullptr);
    ~IDBWatchModel() override = default;

public:
    QStringList expressionList() const;

public slots:
    void attachDebugger(LuauDebugger *debugger);
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

    int rowCount(const QModelIndex &parent = {}) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = {}) const override;

    QModelIndex parent(const QModelIndex &child = {}) const override;

private:
    void rebuildWatchRegistry();
    void refreshTree();
    QString makeTopLevelUserRole(int row) const;

private:
    QVector<WatchItem> m_watchItems;
    // The real registry owned by LuauDebugger.
    // IDBWatchModel does not own this object.
    LuauVariableRegistry *m_watchRegistry = nullptr;
    // Synthetic root scope registered into m_watchRegistry.
    LuauScope m_watchScope;
    LuauDebugger *m_dbg = nullptr;
};

#endif // IDBWATCHMODEL_H
