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

#include "searchabletextbrowser.h"

#include <QApplication>
#include <QResizeEvent>
#include <QTextDocument>

SearchableTextBrowser::SearchableTextBrowser(QWidget *parent)
    : QTextBrowser(parent) {
    installEventFilter(this);
    viewport()->installEventFilter(this);
    m_findTimer.setSingleShot(true);
    connect(&m_findTimer, &QTimer::timeout, this,
            &SearchableTextBrowser::performSearch);
    bindDocument();
}
SearchableTextBrowser::~SearchableTextBrowser() {
    m_findTimer.stop();
    disconnect(m_documentConnection);
    if (m_searchBar) {
        disconnect(m_searchBar, nullptr, this, nullptr);
    }
}
void SearchableTextBrowser::buildSearchBar() {
    if (m_searchBar) {
        return;
    }
    m_searchBar = new SearchBar(this, QStringLiteral("textSearch"));
    connect(m_searchBar, &SearchBar::queryChanged, this,
            &SearchableTextBrowser::cancelSearch);
    connect(m_searchBar, &SearchBar::searchRequested, this,
            &SearchableTextBrowser::startSearch);
    connect(m_searchBar, &SearchBar::showRequested, this,
            &SearchableTextBrowser::showSearch);
    connect(m_searchBar, &SearchBar::closeRequested, this,
            &SearchableTextBrowser::closeSearch);
    connect(m_searchBar, &SearchBar::geometryChanged, this,
            &SearchableTextBrowser::layoutSearchBar);
}
void SearchableTextBrowser::setSearchEnabled(bool enabled) {
    m_searchEnabled = enabled;
    if (enabled) {
        buildSearchBar();
    } else {
        closeSearch();
    }
}
bool SearchableTextBrowser::isSearchEnabled() const { return m_searchEnabled; }
SearchBar *SearchableTextBrowser::searchBar() const { return m_searchBar; }
void SearchableTextBrowser::setSearchText(const QString &text) {
    buildSearchBar();
    m_searchBar->setSearchText(text);
}
void SearchableTextBrowser::showSearch() {
    if (!m_searchEnabled) {
        return;
    }
    buildSearchBar();
    m_searchBar->activate();
    layoutSearchBar();
}
void SearchableTextBrowser::closeSearch() {
    cancelSearch();
    if (m_searchBar) {
        const bool focus =
            m_searchBar->isAncestorOf(QApplication::focusWidget());
        m_searchBar->hide();
        layoutSearchBar();
        if (focus) {
            setFocus();
        }
    }
}
void SearchableTextBrowser::bindDocument() {
    disconnect(m_documentConnection);
    m_documentConnection = connect(document(), &QTextDocument::contentsChanged,
                                   this, &SearchableTextBrowser::cancelSearch);
}
void SearchableTextBrowser::setDocument(QTextDocument *doc) {
    cancelSearch();
    QTextBrowser::setDocument(doc);
    bindDocument();
}
void SearchableTextBrowser::cancelSearch() {
    m_findTimer.stop();
    if (m_searchBar) {
        m_searchBar->cancelPendingSearch();
        m_searchBar->setState(SearchBar::State::Idle);
    }
    setExtraSelections({});
}
void SearchableTextBrowser::findNext() { startSearch(false); }
void SearchableTextBrowser::findPrevious() { startSearch(true); }
void SearchableTextBrowser::startSearch(bool backwards) {
    cancelSearch();
    if (!m_searchEnabled || !m_searchBar || !m_searchBar->isVisible() ||
        m_searchBar->searchText().isEmpty()) {
        return;
    }
    m_query = m_searchBar->query();
    if (!m_query.pattern.isValid()) {
        m_searchBar->setState(SearchBar::State::InvalidPattern,
                              m_query.pattern.errorString());
        return;
    }
    m_backwards = backwards;
    m_searchBar->setState(SearchBar::State::Searching);
    m_findTimer.start(0);
}
void SearchableTextBrowser::performSearch() {
    const auto options = m_query.options;
    QTextDocument::FindFlags flags;
    if (m_backwards) {
        flags |= QTextDocument::FindBackward;
    }
    if (options.caseSensitivity == Qt::CaseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (options.wholeWords) {
        flags |= QTextDocument::FindWholeWords;
    }
    const auto find = [this, options, flags](const QTextCursor &from) {
        return options.regularExpression
                   ? document()->find(m_query.pattern, from, flags)
                   : document()->find(m_query.text, from, flags);
    };
    auto match = find(textCursor());
    if (match.isNull() && options.wrap) {
        QTextCursor edge(document());
        edge.movePosition(m_backwards ? QTextCursor::End : QTextCursor::Start);
        match = find(edge);
    }
    if (match.isNull()) {
        m_searchBar->setState(SearchBar::State::NoMatch);
    } else {
        setTextCursor(match);
        ensureCursorVisible();
        QTextEdit::ExtraSelection selection;
        selection.cursor = match;
        selection.format.setBackground(palette().brush(QPalette::Highlight));
        selection.format.setForeground(
            palette().brush(QPalette::HighlightedText));
        setExtraSelections({selection});
        m_searchBar->setState(SearchBar::State::Idle);
    }
    Q_EMIT searchFinished(match);
}
void SearchableTextBrowser::layoutSearchBar() {
    if (m_layingOut) {
        return;
    }
    m_layingOut = true;
    const int height = m_searchBar && !m_searchBar->isHidden()
                           ? m_searchBar->sizeHint().height()
                           : 0;
    setViewportMargins(0, 0, 0, height);
    if (m_searchBar) {
        const auto area = viewport()->geometry();
        m_searchBar->setGeometry(area.left(), area.bottom() + 1, area.width(),
                                 height);
        m_searchBar->raise();
    }
    m_layingOut = false;
}
void SearchableTextBrowser::resizeEvent(QResizeEvent *event) {
    QTextBrowser::resizeEvent(event);
    layoutSearchBar();
}
bool SearchableTextBrowser::eventFilter(QObject *object, QEvent *event) {
    if (m_searchEnabled && m_searchBar && m_searchBar->handleKeyEvent(event)) {
        return true;
    }
    if ((object == viewport() && event->type() == QEvent::Resize) ||
        event->type() == QEvent::LayoutDirectionChange) {
        layoutSearchBar();
    }
    return QTextBrowser::eventFilter(object, event);
}
