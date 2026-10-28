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

#include "searchabletreewidget.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QScopedValueRollback>

SearchableTreeWidget::SearchableTreeWidget(QWidget *parent)
    : QTreeWidget(parent) {
    m_searchBar = new SearchBar(this, QStringLiteral("treeSearch"));
    installEventFilter(this);
    viewport()->installEventFilter(this);
    m_scanTimer.setSingleShot(true);
    connect(&m_scanTimer, &QTimer::timeout, this,
            &SearchableTreeWidget::scanSearch);
    connect(m_searchBar, &SearchBar::queryChanged, this,
            &SearchableTreeWidget::cancelSearch);
    connect(m_searchBar, &SearchBar::searchRequested, this,
            &SearchableTreeWidget::startSearch);
    connect(m_searchBar, &SearchBar::showRequested, this,
            &SearchableTreeWidget::showSearch);
    connect(m_searchBar, &SearchBar::closeRequested, this,
            &SearchableTreeWidget::closeSearch);
    connect(m_searchBar, &SearchBar::geometryChanged, this,
            &SearchableTreeWidget::updateGeometries);
    const auto modelChanged = [this] {
        if (!m_traversing) {
            cancelSearch();
        }
    };
    connect(model(), &QAbstractItemModel::modelAboutToBeReset, this,
            modelChanged);
    connect(model(), &QAbstractItemModel::rowsAboutToBeRemoved, this,
            modelChanged);
    connect(model(), &QAbstractItemModel::rowsInserted, this, modelChanged);
    connect(model(), &QAbstractItemModel::layoutChanged, this, modelChanged);
    connect(model(), &QAbstractItemModel::dataChanged, this, modelChanged);
}
SearchableTreeWidget::~SearchableTreeWidget() {
    m_scanTimer.stop();
    disconnect(model(), nullptr, this, nullptr);
    disconnect(m_searchBar, nullptr, this, nullptr);
}
SearchBar *SearchableTreeWidget::searchBar() const { return m_searchBar; }
void SearchableTreeWidget::showSearch() {
    m_searchBar->activate();
    updateGeometries();
}
void SearchableTreeWidget::closeSearch() {
    cancelSearch();
    const bool hadFocus =
        m_searchBar->isAncestorOf(QApplication::focusWidget());
    m_searchBar->hide();
    updateGeometries();
    if (hadFocus) {
        setFocus();
    }
}
void SearchableTreeWidget::findNext() { startSearch(false); }
void SearchableTreeWidget::findPrevious() { startSearch(true); }
void SearchableTreeWidget::cancelSearch() {
    m_scanTimer.stop();
    m_session = {};
    m_searchBar->cancelPendingSearch();
    m_searchBar->setState(SearchBar::State::Idle);
}
void SearchableTreeWidget::startSearch(bool backwards) {
    cancelSearch();
    if (!m_searchBar->isVisible() || m_searchBar->searchText().isEmpty()) {
        return;
    }
    m_session.query = m_searchBar->query();
    if (!m_session.query.pattern.isValid()) {
        m_searchBar->setState(SearchBar::State::InvalidPattern,
                              m_session.query.pattern.errorString());
        return;
    }
    m_session.backwards = backwards;
    m_session.wrap = m_session.query.options.wrap;
    m_session.origin = indexFromItem(currentItem(), 0);
    m_session.position = m_session.origin;
    m_session.advanceFirst = m_session.origin.isValid();
    m_searchBar->setState(SearchBar::State::Searching);
    m_scanTimer.start(0);
}
void SearchableTreeWidget::prepareSearchItem(QTreeWidgetItem *) {}
bool SearchableTreeWidget::isSearchableItem(const QTreeWidgetItem *) const {
    return true;
}
void SearchableTreeWidget::prepare(QTreeWidgetItem *item) {
    QScopedValueRollback<bool> guard(m_traversing, true);
    prepareSearchItem(item);
}
QTreeWidgetItem *SearchableTreeWidget::lastDescendant(QTreeWidgetItem *item) {
    while (item && !item->isHidden()) {
        prepare(item);
        if (item->childCount() == 0) {
            break;
        }
        item = item->child(item->childCount() - 1);
    }
    return item;
}
QTreeWidgetItem *SearchableTreeWidget::edge() {
    if (topLevelItemCount() == 0) {
        return nullptr;
    }
    return m_session.backwards
               ? lastDescendant(topLevelItem(topLevelItemCount() - 1))
               : topLevelItem(0);
}
QTreeWidgetItem *SearchableTreeWidget::adjacent(QTreeWidgetItem *item) {
    if (!item) {
        return nullptr;
    }
    if (!m_session.backwards && !item->isHidden()) {
        prepare(item);
        if (item->childCount() > 0) {
            return item->child(0);
        }
    }
    while (item) {
        auto parent = item->parent();
        auto siblings = parent ? parent : invisibleRootItem();
        const int row = siblings->indexOfChild(item);
        if (m_session.backwards) {
            return row > 0 ? lastDescendant(siblings->child(row - 1)) : parent;
        }
        if (row + 1 < siblings->childCount()) {
            return siblings->child(row + 1);
        }
        item = parent;
    }
    return nullptr;
}
void SearchableTreeWidget::scanSearch() {
    // Lazy inserts can trigger pending tree sorting as indexes are read.
    // Keep those synchronous notifications inside the same search batch.
    QScopedValueRollback<bool> traversal(m_traversing, true);
    QElapsedTimer budget;
    budget.start();
    auto item = itemFromIndex(m_session.position);
    if (m_session.advanceFirst) {
        item = adjacent(item);
        m_session.advanceFirst = false;
    } else if (!item && !m_session.origin.isValid()) {
        item = edge();
    }
    while (true) {
        if (!item) {
            if (!m_session.wrap || m_session.wrapped ||
                !m_session.origin.isValid()) {
                finishSearch();
                return;
            }
            m_session.wrapped = true;
            item = edge();
            if (!item) {
                finishSearch();
                return;
            }
        }
        const auto index = indexFromItem(item, 0);
        if (!item->isHidden() && !isColumnHidden(0) && isSearchableItem(item)) {
            if (m_session.query.matches(item->text(0))) {
                finishSearch(item);
                return;
            }
        }
        if (m_session.wrapped && index == m_session.origin) {
            finishSearch();
            return;
        }
        m_session.position = index;
        m_session.advanceFirst = true;
        if (budget.elapsed() >= 4) {
            m_scanTimer.start(0);
            return;
        }
        item = adjacent(item);
        m_session.advanceFirst = false;
    }
}
void SearchableTreeWidget::finishSearch(QTreeWidgetItem *item) {
    if (item) {
        for (auto parent = item->parent(); parent; parent = parent->parent()) {
            parent->setExpanded(true);
        }
        setCurrentItem(item, 0,
                       QItemSelectionModel::ClearAndSelect |
                           QItemSelectionModel::Rows);
        scrollToItem(item);
    }
    m_searchBar->setState(item ? SearchBar::State::Idle
                               : SearchBar::State::NoMatch);
    Q_EMIT searchFinished(indexFromItem(item, 0));
}
void SearchableTreeWidget::updateGeometries() {
    if (m_updatingGeometries) {
        return;
    }
    QScopedValueRollback<bool> guard(m_updatingGeometries, true);
    QTreeWidget::updateGeometries();
    const auto margins = viewportMargins();
    const int height = m_searchBar && !m_searchBar->isHidden()
                           ? m_searchBar->sizeHint().height()
                           : 0;
    setViewportMargins(margins.left(), margins.top(), margins.right(), height);
    if (m_searchBar) {
        const auto area = viewport()->geometry();
        m_searchBar->setGeometry(area.left(), area.bottom() + 1, area.width(),
                                 height);
        m_searchBar->raise();
    }
}
bool SearchableTreeWidget::eventFilter(QObject *watched, QEvent *event) {
    if (m_searchBar && m_searchBar->handleKeyEvent(event)) {
        return true;
    }
    return QTreeWidget::eventFilter(watched, event);
}
