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

#include "qtableviewext.h"
#include "model/asynctableproxymodel.h"
#include "tablecolumnwidthcontroller.h"

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QScrollBar>
#include <QStyle>
#include <algorithm>

QTableViewExt::QTableViewExt(QWidget *parent)
    : QTableView(parent), m_proxy(new AsyncTableProxyModel(this)) {
    m_widths = new TableColumnWidthController(this);
    installEventFilter(this);
    viewport()->installEventFilter(this);
    m_scan.timer.setSingleShot(true);
    connect(&m_scan.timer, &QTimer::timeout, this, &QTableViewExt::scanSearch);
    connect(m_proxy, &AsyncTableProxyModel::sortingChanged, this,
            [this](bool busy) {
                updateStatus();
                Q_EMIT sortingChanged(busy);
            });
    connect(m_proxy, &AsyncTableProxyModel::sortCleared, this, [this] {
        m_sort.column = -1;
        auto header = horizontalHeader();
        header->setSortIndicator(-1, Qt::AscendingOrder);
        header->setSortIndicatorShown(false);
    });

    auto header = horizontalHeader();
    connect(header, &QHeaderView::sectionClicked, this, [this](int column) {
        if (!m_sort.enabled || !isColumnSortable(column)) {
            auto header = horizontalHeader();
            header->setSortIndicator(m_sort.column, m_sort.order);
            header->setSortIndicatorShown(m_sort.column >= 0);
            return;
        }
        sortByColumn(column, m_sort.column == column &&
                                     m_sort.order == Qt::AscendingOrder
                                 ? Qt::DescendingOrder
                                 : Qt::AscendingOrder);
    });
    connect(header, &QHeaderView::sectionMoved, this,
            &QTableViewExt::cancelSearch);
    connect(qApp, &QApplication::focusChanged, this,
            [this](QWidget *old, QWidget *now) {
                if (old && (old == this || isAncestorOf(old)) && now &&
                    now != this && !isAncestorOf(now)) {
                    clearSelection();
                }
            });
}

QTableViewExt::~QTableViewExt() {
    // QWidget teardown can emit header/focus signals after our members are
    // gone.
    disconnect(qApp, nullptr, this, nullptr);
    for (auto &child : findChildren<QObject *>()) {
        disconnect(child, nullptr, this, nullptr);
    }
    for (auto it = m_states.cbegin(); it != m_states.cend(); ++it) {
        disconnect(it.key(), nullptr, this, nullptr);
    }
    m_scan.timer.stop();
    if (m_searchControls.bar) {
        m_searchControls.bar->cancelPendingSearch();
    }
    delete m_widths;
    m_widths = nullptr;
    QTableView::setModel(nullptr);
}

QAbstractItemModel *QTableViewExt::sourceModel() const { return m_source; }

QModelIndex QTableViewExt::mapToSource(const QModelIndex &index) const {
    if (index.model() == m_source) {
        return index;
    }
    return m_proxy->mapToSource(index);
}

QModelIndex QTableViewExt::mapFromSource(const QModelIndex &index) const {
    return m_sort.enabled ? m_proxy->mapFromSource(index)
                          : (index.model() == m_source ? index : QModelIndex());
}

void QTableViewExt::saveState() {
    auto sel = selectionModel();
    if (!m_source || !sel) {
        return;
    }
    auto &state = m_states[m_source];
    state.scroll = verticalScrollBar()->value();
    state.selected.clear();
    for (auto &i : sel->selectedIndexes()) {
        state.selected.append(mapToSource(i));
    }
    state.current = mapToSource(currentIndex());
    m_widths->saveState();
}

void QTableViewExt::restoreState() {
    auto sel = selectionModel();
    if (!m_source || !sel) {
        return;
    }
    const auto &state = m_states[m_source];
    QItemSelection selection;
    for (const auto &i : state.selected) {
        const auto mapped = mapFromSource(i);
        if (mapped.isValid()) {
            selection.select(mapped, mapped);
        }
    }
    sel->select(selection, QItemSelectionModel::ClearAndSelect);
    sel->setCurrentIndex(mapFromSource(state.current),
                         QItemSelectionModel::NoUpdate);
    verticalScrollBar()->setValue(state.scroll);
}

void QTableViewExt::setModel(QAbstractItemModel *source) {
    if (source == m_proxy) {
        return;
    }
    saveState();
    for (const auto &c : std::as_const(m_sourceConnections)) {
        disconnect(c);
    }
    m_sourceConnections.clear();
    cancelSearch();
    m_source = source;
    if (source) {
        if (!m_states.contains(source)) {
            m_states.insert(source, {});
            connect(source, &QObject::destroyed, this, [this, source] {
                m_states.remove(source);
                cancelSearch();
                scheduleWidths();
            });
        }
        const auto changed = [this] {
            cancelSearch();
            rebuildColumnMenu();
            scheduleWidths(true);
        };
        m_sourceConnections << connect(
            source, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &first, const QModelIndex &last) {
                cancelSearch();
                m_widths->invalidateContents(first.column(), last.column());
            });
        m_sourceConnections
            << connect(source, &QAbstractItemModel::modelReset, this, changed);
        m_sourceConnections << connect(
            source, &QAbstractItemModel::layoutChanged, this, changed);
        m_sourceConnections << connect(
            source, &QAbstractItemModel::rowsInserted, this, changed);
        m_sourceConnections
            << connect(source, &QAbstractItemModel::rowsRemoved, this, changed);
        m_sourceConnections << connect(
            source, &QAbstractItemModel::columnsInserted, this, changed);
        m_sourceConnections << connect(
            source, &QAbstractItemModel::columnsRemoved, this, changed);
        m_sourceConnections << connect(
            source, &QAbstractItemModel::headerDataChanged, this, changed);
    }
    bindDisplayModel();
    m_widths->setSourceModel(m_source);
    restoreState();
    rebuildColumnMenu();
    scheduleWidths(true);
    Q_EMIT modelChanged();
}

void QTableViewExt::bindDisplayModel() {
    for (const auto &c : std::as_const(m_displayConnections)) {
        disconnect(c);
    }
    m_displayConnections.clear();
    QTableView::setModel(nullptr);
    m_proxy->setSourceModel(m_sort.enabled ? m_source.data() : nullptr);
    QTableView::setModel(m_sort.enabled ? m_proxy : m_source.data());
    if (model()) {
        m_displayConnections
            << connect(model(), &QAbstractItemModel::layoutAboutToBeChanged,
                       this, &QTableViewExt::cancelSearch);
        m_displayConnections
            << connect(model(), &QAbstractItemModel::modelAboutToBeReset, this,
                       &QTableViewExt::cancelSearch);
    }
}

void QTableViewExt::setAsyncSortingEnabled(bool enabled) {
    if (m_sort.enabled == enabled) {
        return;
    }
    saveState();
    m_sort.enabled = enabled;
    QTableView::setSortingEnabled(false);
    auto header = horizontalHeader();
    header->setSectionsClickable(enabled);
    header->setSortIndicatorShown(false);
    if (!enabled) {
        m_proxy->sort(-1);
    }
    bindDisplayModel();
    m_widths->setSourceModel(m_source);
    restoreState();
    scheduleWidths();
    Q_EMIT modelChanged();
}

bool QTableViewExt::isAsyncSortingEnabled() const { return m_sort.enabled; }

void QTableViewExt::sortByColumn(int column, Qt::SortOrder order) {
    if (!m_sort.enabled || (column >= 0 && !isColumnSortable(column))) {
        return;
    }
    m_sort.column = column;
    m_sort.order = order;
    auto header = horizontalHeader();
    header->setSortIndicator(column, order);
    header->setSortIndicatorShown(column >= 0);
    m_proxy->sort(column, order);
}

void QTableViewExt::setColumnSortable(int column, bool enabled) {
    m_proxy->setColumnSortable(column, enabled);
}

bool QTableViewExt::isColumnSortable(int column) const {
    return m_proxy->isColumnSortable(column);
}

void QTableViewExt::setColumnSortRole(int column, int role) {
    m_proxy->setColumnSortRole(column, role);
}

bool QTableViewExt::isSorting() const {
    return m_sort.enabled && m_proxy->isSorting();
}

void QTableViewExt::buildSearchBar() {
    if (m_searchControls.bar) {
        return;
    }
    m_searchControls.bar = new SearchBar(this, QStringLiteral("tableSearch"));
    m_searchControls.bar->lineEdit()->setAccessibleName(tr("Search table"));
    m_searchControls.columns =
        m_searchControls.bar->optionsMenu()->addMenu(tr("Search columns"));
    connect(m_searchControls.bar, &SearchBar::queryChanged, this,
            &QTableViewExt::cancelSearch);
    connect(m_searchControls.bar, &SearchBar::searchRequested, this,
            &QTableViewExt::startSearch);
    connect(m_searchControls.bar, &SearchBar::showRequested, this,
            &QTableViewExt::showSearch);
    connect(m_searchControls.bar, &SearchBar::closeRequested, this,
            &QTableViewExt::closeSearch);
    connect(m_searchControls.bar, &SearchBar::geometryChanged, this,
            &QTableViewExt::updateGeometries);
    rebuildColumnMenu();
}
void QTableViewExt::setSearchEnabled(bool enabled) {
    m_search.enabled = enabled;
    if (enabled) {
        buildSearchBar();
    } else {
        closeSearch();
    }
}

bool QTableViewExt::isSearchEnabled() const { return m_search.enabled; }

void QTableViewExt::setSearchableColumns(const QList<int> &columns) {
    m_search.allowedColumns = QSet<int>(columns.begin(), columns.end());
    m_search.knownColumns.clear();
    rebuildColumnMenu();
    cancelSearch();
}

void QTableViewExt::resetSearchableColumns() {
    m_search.allowedColumns.reset();
    m_search.knownColumns.clear();
    rebuildColumnMenu();
    cancelSearch();
}

void QTableViewExt::setColumnSearchRole(int column, int role) {
    m_search.roles[column] = role;
    cancelSearch();
}

void QTableViewExt::setSearchText(const QString &text) {
    buildSearchBar();
    m_searchControls.bar->setSearchText(text);
}

void QTableViewExt::rebuildColumnMenu() {
    if (!m_searchControls.columns) {
        return;
    }
    m_searchControls.columns->clear();
    if (!m_source) {
        return;
    }
    for (int c = 0; c < m_source->columnCount(); ++c) {
        if (m_search.allowedColumns && !m_search.allowedColumns->contains(c)) {
            continue;
        }
        if (!m_search.knownColumns.contains(c)) {
            m_search.userColumns.insert(c);
            m_search.knownColumns.insert(c);
        }
        auto a = m_searchControls.columns->addAction(
            m_source->headerData(c, Qt::Horizontal).toString());
        a->setCheckable(true);
        a->setChecked(m_search.userColumns.contains(c));
        connect(a, &QAction::toggled, this, [this, c](bool checked) {
            if (checked) {
                m_search.userColumns.insert(c);
            } else {
                m_search.userColumns.remove(c);
            }
            cancelSearch();
            m_searchControls.bar->scheduleSearch();
        });
    }
}

void QTableViewExt::showSearch() {
    if (!m_search.enabled) {
        return;
    }
    buildSearchBar();
    m_searchControls.bar->show();
    updateGeometries();
    m_searchControls.bar->activate();
}

void QTableViewExt::closeSearch() {
    cancelSearch();
    if (m_searchControls.bar) {
        const bool hadFocus =
            m_searchControls.bar->isAncestorOf(QApplication::focusWidget());
        m_searchControls.bar->hide();
        updateGeometries();
        if (hadFocus) {
            setFocus();
        }
    }
}

void QTableViewExt::findNext() { startSearch(false); }

void QTableViewExt::findPrevious() { startSearch(true); }

void QTableViewExt::cancelSearch() {
    if (m_searchControls.bar) {
        m_searchControls.bar->cancelPendingSearch();
    }
    m_scan.timer.stop();
    m_scan.remaining = 0;
    setSearchState(SearchState::Idle);
}

void QTableViewExt::startSearch(bool backwards) {
    cancelSearch();
    if (!m_search.enabled || !m_searchControls.bar ||
        !m_searchControls.bar->isVisible() || !model()) {
        return;
    }
    auto text = m_searchControls.bar->searchText();
    if (text.isEmpty()) {
        return;
    }
    m_scan.query = m_searchControls.bar->query();
    const auto &options = m_scan.query.options;
    if (!m_scan.query.pattern.isValid()) {
        setSearchState(SearchState::InvalidPattern,
                       m_scan.query.pattern.errorString());
        return;
    }
    m_scan.columns.clear();
    for (int visual = 0; visual < horizontalHeader()->count(); ++visual) {
        const int c = horizontalHeader()->logicalIndex(visual);
        if (!isColumnHidden(c) &&
            (!m_search.allowedColumns ||
             m_search.allowedColumns->contains(c)) &&
            m_search.userColumns.contains(c)) {
            m_scan.columns.append(c);
        }
    }
    const qint64 total = qint64(model()->rowCount()) * m_scan.columns.size();
    m_scan.remaining = total;
    m_scan.step = backwards ? -1 : 1;
    const auto current = currentIndex();
    m_scan.position = backwards ? total - 1 : 0;
    if (current.isValid()) {
        const int visual = horizontalHeader()->visualIndex(current.column());
        int slot = backwards ? -1 : m_scan.columns.size();
        for (int i = 0; i < m_scan.columns.size(); ++i) {
            const int candidate =
                horizontalHeader()->visualIndex(m_scan.columns.at(i));
            if (backwards && candidate < visual) {
                slot = i;
            }
            if (!backwards && candidate > visual) {
                slot = i;
                break;
            }
        }
        m_scan.position = qint64(current.row()) * m_scan.columns.size() + slot;
    }
    if (!options.wrap) {
        m_scan.remaining =
            backwards ? m_scan.position + 1 : total - m_scan.position;
    }
    setSearchState(SearchState::Searching);
    m_scan.timer.start(0);
}

void QTableViewExt::scanSearch() {
    if (!model()) {
        return;
    }
    QElapsedTimer budget;
    budget.start();
    const qint64 total = qint64(model()->rowCount()) * m_scan.columns.size();
    while (total > 0 && m_scan.remaining > 0) {
        m_scan.position = (m_scan.position % total + total) % total;
        const int row = m_scan.position / m_scan.columns.size();
        const int column =
            m_scan.columns.at(m_scan.position % m_scan.columns.size());
        const auto idx = model()->index(row, column);
        m_scan.position += m_scan.step;
        --m_scan.remaining;
        bool matches = false;
        if (!isRowHidden(row) && !isColumnHidden(column)) {
            const auto text =
                idx.data(m_search.roles.value(column, Qt::DisplayRole))
                    .toString();
            matches = m_scan.query.matches(text);
        }
        if (matches) {
            selectionModel()->setCurrentIndex(
                idx, QItemSelectionModel::ClearAndSelect |
                         (selectionBehavior() == SelectRows
                              ? QItemSelectionModel::Rows
                              : QItemSelectionModel::NoUpdate));
            scrollTo(idx);
            setSearchState(SearchState::Idle);
            Q_EMIT searchFinished(idx);
            return;
        }
        if (budget.elapsed() >= 4) {
            m_scan.timer.start(0);
            return;
        }
    }
    setSearchState(SearchState::NoMatch);
    Q_EMIT searchFinished({});
}

void QTableViewExt::setSearchState(SearchState state, const QString &detail) {
    if (m_searchControls.bar) {
        m_searchControls.bar->setState(state, detail);
    }
}

void QTableViewExt::updateStatus() {
    const int extent =
        style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
    const QSize iconSize(extent, extent);
    if (!m_sort.status && isSorting()) {
        m_sort.status = new QLabel(this);
        m_sort.status->setFixedSize(iconSize);
        m_sort.status->setToolTip(tr("Sorting..."));
        m_sort.status->setAccessibleName(tr("Sorting..."));
        m_sort.status->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    if (m_sort.status) {
        if (isSorting()) {
            m_sort.status->setFixedSize(iconSize);
            m_sort.status->setPixmap(SearchBar::busyIcon(palette()).pixmap(
                iconSize, devicePixelRatioF()));
        }
        m_sort.status->setVisible(isSorting());
    }
    if (!m_sort.layoutPending) {
        m_sort.layoutPending = true;
        QMetaObject::invokeMethod(
            this,
            [this] {
                m_sort.layoutPending = false;
                updateGeometries();
            },
            Qt::QueuedConnection);
    }
}

bool QTableViewExt::setColumnWidthPolicy(int column,
                                         const ColumnWidthPolicy &policy) {
    return m_widths->setPolicy(column, policy);
}

std::optional<QTableViewExt::ColumnWidthPolicy>
QTableViewExt::columnWidthPolicy(int column) const {
    return m_widths->policy(column);
}

void QTableViewExt::clearColumnWidthPolicy(int column) {
    m_widths->clearPolicy(column);
}

void QTableViewExt::resetColumnWidths(int column) { m_widths->reset(column); }

void QTableViewExt::scheduleWidths(bool contentsChanged) {
    if (m_widths) {
        m_widths->schedule(contentsChanged);
    }
}

void QTableViewExt::setItemDelegate(QAbstractItemDelegate *delegate) {
    QTableView::setItemDelegate(delegate);
    m_widths->watchDelegate(delegate);
    scheduleWidths(true);
}

void QTableViewExt::setItemDelegateForColumn(int column,
                                             QAbstractItemDelegate *delegate) {
    QTableView::setItemDelegateForColumn(column, delegate);
    m_widths->watchDelegate(delegate);
    m_widths->invalidateContents(column, column);
}

void QTableViewExt::setItemDelegateForRow(int row,
                                          QAbstractItemDelegate *delegate) {
    QTableView::setItemDelegateForRow(row, delegate);
    m_widths->watchDelegate(delegate);
    scheduleWidths(true);
}

void QTableViewExt::resizeEvent(QResizeEvent *event) {
    QTableView::resizeEvent(event);
    scheduleWidths();
}

void QTableViewExt::showEvent(QShowEvent *event) {
    QTableView::showEvent(event);
    scheduleWidths();
}

void QTableViewExt::changeEvent(QEvent *event) {
    QTableView::changeEvent(event);
    if (event->type() == QEvent::FontChange ||
        event->type() == QEvent::StyleChange) {
        scheduleWidths(true);
        updateStatus();
    } else if (event->type() == QEvent::PaletteChange) {
        updateStatus();
    }
}

void QTableViewExt::updateGeometries() {
    if (m_updatingGeometries) {
        return;
    }
    m_updatingGeometries = true;
    if (m_widths) {
        m_widths->synchronizeEmptyLayout();
    }
    QTableView::updateGeometries();
    int barHeight = 0;
    if (m_searchControls.bar && !m_searchControls.bar->isHidden()) {
        barHeight = m_searchControls.bar->sizeHint().height();
    }
    const auto margins = viewportMargins();
    setViewportMargins(margins.left(), margins.top(), margins.right(),
                       barHeight);
    if (m_searchControls.bar) {
        // The viewport already accounts for headers, both scrollbars and RTL.
        // Its reserved bottom margin is the only space owned by the search bar.
        const auto area = viewport()->geometry();
        m_searchControls.bar->setGeometry(area.left(), area.bottom() + 1,
                                          area.width(), barHeight);
        m_searchControls.bar->raise();
    }
    int statusBottom = viewport()->geometry().bottom();
    for (auto label : {m_sort.status}) {
        if (label && !label->isHidden()) {
            label->setGeometry(viewport()->geometry().left() + 4,
                               statusBottom - label->height(),
                               std::max(0, std::min(label->sizeHint().width(),
                                                    viewport()->width() - 8)),
                               label->height());
            label->raise();
            statusBottom -= label->height();
        }
    }
    m_updatingGeometries = false;
}

bool QTableViewExt::eventFilter(QObject *watched, QEvent *event) {
    if (m_search.enabled && m_searchControls.bar &&
        m_searchControls.bar->handleKeyEvent(event)) {
        return true;
    }
    return QTableView::eventFilter(watched, event);
}
