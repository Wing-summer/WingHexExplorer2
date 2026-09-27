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

#include "idbwatchmodel.h"

#include <QModelIndexList>
#include <QVariant>

IDBWatchModel::IDBWatchModel(QObject *parent)
    : IDBTreeModel(parent),
      m_watchScope(LuauScope::createLocal(
          nullptr,
          QStringLiteral("__watch__") + QString::number(quintptr(this), 16))) {}

QStringList IDBWatchModel::expressionList() const {
    QStringList result;
    result.reserve(m_watchItems.size());

    for (const auto &item : m_watchItems) {
        result.append(item.expression);
    }

    return result;
}

void IDBWatchModel::attachDebugger(LuauDebugger *debugger) {
    if (m_dbg == debugger) {
        refresh();
        return;
    }

    if (m_dbg) {
        disconnect(m_dbg, &LuauDebugger::onPullVariables, this,
                   &IDBWatchModel::refresh);
    }

    m_dbg = debugger;
    m_watchRegistry = nullptr;

    if (m_dbg) {
        connect(m_dbg, &LuauDebugger::onPullVariables, this,
                &IDBWatchModel::refresh);
    }

    refresh();
}

void IDBWatchModel::addWatchExpression(const QString &expression) {
    const QString expr = expression.trimmed();
    if (expr.isEmpty()) {
        return;
    }

    WatchItem item;
    item.expression = expr;
    m_watchItems.append(std::move(item));

    refresh();
}

void IDBWatchModel::removeWatchExpression(qsizetype index) {
    if (index < 0 || index >= m_watchItems.size()) {
        return;
    }

    m_watchItems.removeAt(index);
    refresh();
}

void IDBWatchModel::removeWatchExpressions(const QModelIndexList &indexes) {
    std::set<int, std::greater<int>> rows;

    for (const QModelIndex &index : indexes) {
        if (!index.isValid() || index.parent().isValid() ||
            IDBTreeModel::isProxyNode(index)) {
            continue;
        }

        const int row = index.row();
        if (row >= 0 && row < m_watchItems.size()) {
            rows.insert(row);
        }
    }

    if (rows.empty()) {
        return;
    }

    for (const int row : rows) {
        m_watchItems.removeAt(row);
    }

    refresh();
}

bool IDBWatchModel::editWatchExpression(qsizetype index,
                                        const QString &newExpression) {
    if (index < 0 || index >= m_watchItems.size()) {
        return false;
    }

    const QString expression = newExpression.trimmed();
    if (expression.isEmpty()) {
        return false;
    }

    auto &item = m_watchItems[index];
    if (item.expression == expression) {
        return true;
    }

    item.expression = expression;
    item.result.reset();

    refresh();
    return true;
}

void IDBWatchModel::refresh() {
    const bool canEvaluate = m_dbg && m_dbg->isDebugBreak();

    if (canEvaluate) {
        for (auto &item : m_watchItems) {
            item.result = m_dbg->evaluateExpression(item.expression);
        }
    }

    refreshTree();
}

void IDBWatchModel::reloadExpressionList(const QStringList &expressions) {
    m_watchItems.clear();
    m_watchItems.reserve(expressions.size());

    for (const QString &expression : expressions) {
        const auto expr = expression.trimmed();
        if (expr.isEmpty()) {
            continue;
        }

        WatchItem item;
        item.expression = expr;
        m_watchItems.append(std::move(item));
    }

    refresh();
}

Qt::ItemFlags IDBWatchModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags result = IDBTreeModel::flags(index);

    if (index.isValid() && !index.parent().isValid() &&
        !IDBTreeModel::isProxyNode(index) && index.column() == 0) {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

bool IDBWatchModel::setData(const QModelIndex &index, const QVariant &value,
                            int role) {
    if (role == Qt::EditRole && index.isValid() && !index.parent().isValid() &&
        !IDBTreeModel::isProxyNode(index) && index.column() == 0) {
        return editWatchExpression(index.row(), value.toString());
    }

    return IDBTreeModel::setData(index, value, role);
}

void IDBWatchModel::clearAll() {
    if (m_watchItems.isEmpty()) {
        return;
    }

    m_watchItems.clear();
    refresh();
}

QVariant IDBWatchModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    /*
     * Children/proxy nodes are completely handled by IDBTreeModel.
     */
    if (index.parent().isValid() || IDBTreeModel::isProxyNode(index)) {
        return IDBTreeModel::data(index, role);
    }

    const int row = index.row();
    if (row < 0 || row >= m_watchItems.size()) {
        return {};
    }

    const auto &item = m_watchItems.at(row);
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return item.expression;
        }

        // Don't show stale debugger values while the VM is running.
        if (!m_dbg || !m_dbg->isDebugBreak()) {
            return {};
        }

        if (!item.result) {
            return QStringLiteral("<not evaluated>");
        }
    }

    if (role == Qt::EditRole && index.column() == 0) {
        return item.expression;
    }

    if (role == Qt::UserRole) {
        return makeTopLevelUserRole(row);
    }

    // No real result means there is nothing that IDBTreeModel can display
    if (!item.result) {
        return {};
    }

    if (!m_dbg || !m_dbg->isDebugBreak()) {
        return {};
    }

    /*
     * For evaluated watches, use the original IDBTreeModel logic.
     * The index carries item.result as internalPointer, therefore
     * IDBTreeModel can resolve its real fields from debugger's registry.
     */
    return IDBTreeModel::data(index, role);
}

bool IDBWatchModel::hasChildren(const QModelIndex &parent) const {
    if (!parent.isValid())
        return !m_watchItems.isEmpty();

    if (!parent.parent().isValid() && !IDBTreeModel::isProxyNode(parent)) {

        const int row = parent.row();

        if (row < 0 || row >= m_watchItems.size())
            return false;

        const auto &item = m_watchItems.at(row);

        if (!item.result)
            return false;

        /*
         * The result is retained internally after leaving the debug
         * break, but its child tree is no longer considered current.
         */
        if (!m_dbg || !m_dbg->isDebugBreak())
            return false;
    }

    return IDBTreeModel::hasChildren(parent);
}

int IDBWatchModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid())
        return m_watchItems.size();

    /*
     * Unevaluated top-level watch:
     * no children.
     */
    if (!parent.parent().isValid() && !IDBTreeModel::isProxyNode(parent)) {

        const int row = parent.row();

        if (row < 0 || row >= m_watchItems.size())
            return 0;

        const auto &item = m_watchItems.at(row);

        if (!item.result)
            return 0;

        /*
         * When debugger is not stopped, don't expose stale children.
         */
        if (!m_dbg || !m_dbg->isDebugBreak())
            return 0;
    }

    return IDBTreeModel::rowCount(parent);
}

QModelIndex IDBWatchModel::index(int row, int column,
                                 const QModelIndex &parent) const {
    if (!parent.isValid()) {
        if (row < 0 || row >= m_watchItems.size())
            return {};

        const auto &item = m_watchItems.at(row);

        if (item.result) {
            /*
             * This MUST use the same internalPointer representation
             * as IDBTreeModel.
             */
            return createIndex(row, column,
                               IDBTreeModel::encodeVar(item.result.get()));
        }

        /*
         * Unevaluated watch item.
         * There is no LuauVariable yet.
         */
        return createIndex(row, column, nullptr);
    }

    /*
     * An unevaluated top-level watch item cannot have children.
     */
    if (!parent.parent().isValid() && !IDBTreeModel::isProxyNode(parent)) {

        const int row = parent.row();

        if (row < 0 || row >= m_watchItems.size())
            return {};

        if (!m_watchItems.at(row).result)
            return {};
    }

    /*
     * For all real children, let IDBTreeModel use its original
     * variable/proxy machinery.
     */
    return IDBTreeModel::index(row, column, parent);
}

QModelIndex IDBWatchModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return {};
    }

    QModelIndex parentIndex = IDBTreeModel::parent(child);
    if (!parentIndex.isValid()) {
        return {};
    }

    /*
     * If the returned parent already has a parent, it is a normal
     * nested node/proxy. Keep the original IDBTreeModel index.
     */
    if (parentIndex.parent().isValid()) {
        return parentIndex;
    }

    if (IDBTreeModel::isProxyNode(parentIndex)) {
        return parentIndex;
    }

    /*
     * The parent should be a real LuauVariable representing one of
     * our evaluated watch expressions.
     */
    auto *raw = IDBTreeModel::decodeVar(parentIndex.internalPointer());

    if (!raw) {
        return parentIndex;
    }

    for (int row = 0; row < m_watchItems.size(); ++row) {
        const auto &item = m_watchItems.at(row);

        if (item.result && item.result.get() == raw) {

            return createIndex(row, parentIndex.column(),
                               IDBTreeModel::encodeVar(raw));
        }
    }

    return parentIndex;
}

void IDBWatchModel::rebuildWatchRegistry() {
    if (!m_dbg) {
        m_watchRegistry = nullptr;
        return;
    }

    auto *registry = m_dbg->variableRegistry();
    if (!registry) {
        m_watchRegistry = nullptr;
        return;
    }

    auto variables = LuauVariableList::create();
    variables->reserve(m_watchItems.size());

    /*
     * Only evaluated variables are put into the real registry's
     * synthetic watch scope.
     *
     * Unevaluated watch items still exist in m_watchItems and are
     * represented by a top-level QModelIndex with nullptr internalPointer.
     */
    for (const auto &item : m_watchItems) {
        if (item.result)
            variables->append(item.result);
    }

    /*
     * IMPORTANT:
     *
     * The watch scope belongs to the debugger's real registry.
     * Therefore result variables and all of their child scopes are
     * resolved from the same LuauVariableRegistry.
     */
    registry->registerOrUpdateVariables(m_watchScope, variables);

    if (m_watchRegistry != registry) {
        m_watchRegistry = registry;
        IDBTreeModel::setRoot(m_watchRegistry, m_watchScope);
    }
}

void IDBWatchModel::refreshTree() {
    rebuildWatchRegistry();
    IDBTreeModel::refresh();
}

QString IDBWatchModel::makeTopLevelUserRole(int row) const {
    return QStringLiteral("watch_") + QString::number(row);
}
