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

#ifndef SEARCHABLETEXTBROWSER_H
#define SEARCHABLETEXTBROWSER_H

#include "searchbar.h"
#include <QTextBrowser>
#include <QTimer>

class SearchableTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    explicit SearchableTextBrowser(QWidget *parent = nullptr);
    ~SearchableTextBrowser() override;
    void setSearchEnabled(bool enabled);
    bool isSearchEnabled() const;
    void setSearchText(const QString &text);
    SearchBar *searchBar() const;
    void setDocument(QTextDocument *document);
public slots:
    void showSearch();
    void closeSearch();
    void findNext();
    void findPrevious();
signals:
    void searchFinished(const QTextCursor &match);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void buildSearchBar();
    void layoutSearchBar();
    void bindDocument();
    void cancelSearch();
    void startSearch(bool backwards);
    void performSearch();
    SearchBar *m_searchBar = nullptr;
    QTimer m_findTimer;
    SearchBar::Query m_query;
    QMetaObject::Connection m_documentConnection;
    bool m_searchEnabled = false;
    bool m_backwards = false;
    bool m_layingOut = false;
};
#endif
