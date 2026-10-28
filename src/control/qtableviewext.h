/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef QTABLEVIEWEXT_H
#define QTABLEVIEWEXT_H

#include "searchbar.h"

#include <QHash>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QRegularExpression>
#include <QSet>
#include <QTableView>
#include <QTimer>
#include <optional>

class QLabel;
class QMenu;
class AsyncTableProxyModel;
class TableColumnWidthController;

class QTableViewExt : public QTableView {
    Q_OBJECT
public:
    enum class WidthSource { Specified, Contents, Weight };
    enum class WidthUpdate { Initial, Continuous };
    enum class WidthUnit { Pixels, ViewportRatio };
    struct ColumnWidthPolicy {
        WidthSource source = WidthSource::Contents;
        WidthUpdate update = WidthUpdate::Initial;
        WidthUnit unit = WidthUnit::Pixels;
        qreal value = 100;
        qreal weight = 1;
        std::optional<int> minimumPixels;
        std::optional<int> maximumPixels;
        std::optional<qreal> minimumRatio;
        std::optional<qreal> maximumRatio;
    };

public:
    explicit QTableViewExt(QWidget *parent = nullptr);
    ~QTableViewExt() override;

public:
    virtual void setModel(QAbstractItemModel *model) override;
    QAbstractItemModel *sourceModel() const;
    QModelIndex mapToSource(const QModelIndex &index) const;
    QModelIndex mapFromSource(const QModelIndex &index) const;

    void setSearchEnabled(bool enabled);
    bool isSearchEnabled() const;
    // Unconfigured: all columns. An explicit empty list allows none.
    void setSearchableColumns(const QList<int> &columns);
    void resetSearchableColumns();
    void setColumnSearchRole(int column, int role);
    void setSearchText(const QString &text);

    void setAsyncSortingEnabled(bool enabled);
    bool isAsyncSortingEnabled() const;
    void setColumnSortable(int column, bool enabled);
    bool isColumnSortable(int column) const;
    void setColumnSortRole(int column, int role);
    bool isSorting() const;

    // Invalid policies are rejected without changing the existing
    // configuration.
    bool setColumnWidthPolicy(int column, const ColumnWidthPolicy &policy);
    std::optional<ColumnWidthPolicy> columnWidthPolicy(int column) const;
    void clearColumnWidthPolicy(int column);
    // Clears manual overrides and first-layout state for the current model.
    // A negative column resets all configured columns.
    void resetColumnWidths(int column = -1);
    void setItemDelegate(QAbstractItemDelegate *delegate);
    void setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate);
    void setItemDelegateForRow(int row, QAbstractItemDelegate *delegate);

public slots:
    void showSearch();
    void closeSearch();
    void findNext();
    void findPrevious();
    void sortByColumn(int column, Qt::SortOrder order);

signals:
    void modelChanged();
    void sortingChanged(bool busy);
    void searchFinished(const QModelIndex &match);
    void columnWidthsAdjusted();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
    void updateGeometries() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    using SearchState = SearchBar::State;
    void buildSearchBar();
    void setSearchState(SearchState state, const QString &message = {});
    void rebuildColumnMenu();
    void startSearch(bool backwards);
    void scanSearch();
    void cancelSearch();
    void bindDisplayModel();
    void saveState();
    void restoreState();
    void scheduleWidths(bool contentsChanged = false);
    void updateStatus();

private:
    struct ModelState {
        int scroll = 0;
        QList<QPersistentModelIndex> selected;
        QPersistentModelIndex current;
    };

private:
    friend class TableColumnWidthController;
    TableColumnWidthController *m_widths = nullptr;
    QPointer<QAbstractItemModel> m_source;
    AsyncTableProxyModel *m_proxy;
    struct SortState {
        bool enabled = false;
        int column = -1;
        Qt::SortOrder order = Qt::AscendingOrder;
        QLabel *status = nullptr;
        bool layoutPending = false;
    } m_sort;
    QList<QMetaObject::Connection> m_sourceConnections;
    QList<QMetaObject::Connection> m_displayConnections;
    QHash<QAbstractItemModel *, ModelState> m_states;

    // Long-lived configuration, transient scan progress, and child widgets
    // have separate lifetimes. Keep each group together.
    struct SearchConfiguration {
        bool enabled = false;
        std::optional<QSet<int>> allowedColumns;
        QSet<int> userColumns;
        QSet<int> knownColumns;
        QHash<int, int> roles;
    } m_search;
    struct SearchSession {
        QTimer timer;
        SearchBar::Query query;
        QList<int> columns;
        qint64 position = 0;
        qint64 remaining = 0;
        int step = 1;
    } m_scan;
    struct SearchControls {
        SearchBar *bar = nullptr;
        QMenu *columns = nullptr;
    } m_searchControls;

    bool m_updatingGeometries = false;
};
#endif // QTABLEVIEWEXT_H
