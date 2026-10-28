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

#ifndef SEARCHABLETREEWIDGET_H
#define SEARCHABLETREEWIDGET_H

#include "searchbar.h"
#include <QPersistentModelIndex>
#include <QTreeWidget>

// Navigates names in tree order without filtering or eagerly expanding nodes.
// Lazy trees supply children through prepareSearchItem().
class SearchableTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit SearchableTreeWidget(QWidget *parent = nullptr);
    ~SearchableTreeWidget() override;
    SearchBar *searchBar() const;

public slots:
    void showSearch();
    void closeSearch();
    void findNext();
    void findPrevious();

signals:
    void searchFinished(const QModelIndex &match);

protected:
    virtual void prepareSearchItem(QTreeWidgetItem *item);
    virtual bool isSearchableItem(const QTreeWidgetItem *item) const;
    void updateGeometries() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct SearchSession {
        QPersistentModelIndex origin;
        QPersistentModelIndex position;
        SearchBar::Query query;
        bool backwards = false;
        bool wrap = true;
        bool wrapped = false;
        bool advanceFirst = false;
    };
    void startSearch(bool backwards);
    void scanSearch();
    void cancelSearch();
    void finishSearch(QTreeWidgetItem *item = nullptr);
    void prepare(QTreeWidgetItem *item);
    QTreeWidgetItem *lastDescendant(QTreeWidgetItem *item);
    QTreeWidgetItem *adjacent(QTreeWidgetItem *item);
    QTreeWidgetItem *edge();

    SearchBar *m_searchBar = nullptr;
    QTimer m_scanTimer;
    SearchSession m_session;
    bool m_traversing = false;
    bool m_updatingGeometries = false;
};
#endif
