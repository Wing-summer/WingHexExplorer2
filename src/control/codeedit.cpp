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

#include "codeedit.h"
#include "class/qkeysequences.h"
#include "class/scriptsettings.h"
#include "class/skinmanager.h"
#include "control/searchreplacewidget.h"
#include "model/codecompletionmodel.h"

#include <QApplication>
#include <QModelIndex>
#include <QScrollBar>
#include <QShortcut>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

CodeEdit::CodeEdit(QWidget *parent) : WingCodeEdit(parent) {
    connect(this->document(), &QTextDocument::modificationChanged, this,
            &CodeEdit::contentModified);
    addEditorBasicShortcut();
    addMoveLineShortcut();

    connect(&ScriptSettings::instance(), &ScriptSettings::editorSettingsUpdate,
            this, &CodeEdit::applyEditorSetStyle);
    applyEditorSetStyle();

    m_searchWidget = new SearchReplaceWidget(this);
    connect(this, &CodeEdit::textChanged, this, [this] {
        if (m_searchWidget->isVisible())
            showSearchReplaceBar(false, false);
    });

    m_gotoWidget = new GotoLineWidget(this);
    connect(this, &CodeEdit::blockCountChanged, m_gotoWidget,
            &GotoLineWidget::setTotalLines);
    connect(m_gotoWidget, &GotoLineWidget::onGotoLine, this, [this](int line) {
        auto doc = document();
        auto block = doc->findBlockByNumber(line - 1);

        if (block.isValid()) {
            QTextCursor cursor(block);
            cursor.movePosition(QTextCursor::EndOfBlock);
            setTextCursor(cursor);
            ensureCursorVisible();
        }
    });

    showSearchReplaceBar(false, false);
    showGotoBar(false);
}

void CodeEdit::showSearchReplaceBar(bool show, bool replace) {
    if (m_gotoWidget->isVisible()) {
        showGotoBar(false);
    }
    m_searchWidget->setVisible(show);
    m_searchWidget->setEnabled(show);
    if (show) {
        const QTextCursor cursor = textCursor();
        if (cursor.hasSelection())
            m_searchWidget->setSearchText(cursor.selectedText());
        m_searchWidget->setReplaceMode(replace);
        m_searchWidget->activate();
    } else {
        clearLiveSearch();
        setFocus(Qt::OtherFocusReason);
    }
}

void CodeEdit::showGotoBar(bool show) {
    if (m_searchWidget->isVisible()) {
        showSearchReplaceBar(false, false);
    }
    m_gotoWidget->setOriginLine(textCursor().blockNumber() + 1);
    m_gotoWidget->setVisible(show);
    m_gotoWidget->setEnabled(show);
}

void CodeEdit::onCompletion(const QModelIndex &index) {
    WingCodeEdit::onCompletion(index);
    auto selfdata = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    if (selfdata.type == CodeInfoTip::Type::Function ||
        selfdata.type == CodeInfoTip::Type::ClsFunction) {
        auto args = selfdata.addinfo.value(CodeInfoTip::Args);
        auto cursor = textCursor();
        cursor.insertText(QStringLiteral("()"));
        if (!args.isEmpty()) {
            cursor.movePosition(QTextCursor::Left);
            setTextCursor(cursor);
        }
    }
}

void CodeEdit::addEditorBasicShortcut() {
    auto find = new QShortcut(QKeySequence(QKeySequence::Find), this);
    find->setContext(Qt::WidgetShortcut);
    connect(find, &QShortcut::activated, this,
            [this]() { showSearchReplaceBar(true, false); });

    auto replace = new QShortcut(QKeySequence(QKeySequence::Replace), this);
    replace->setContext(Qt::WidgetShortcut);
    connect(replace, &QShortcut::activated, this,
            [this]() { showSearchReplaceBar(true, true); });

    auto Goto = new QShortcut(
        QKeySequences::instance().keySequence(QKeySequences::Key::GOTO), this);
    connect(Goto, &QShortcut::activated, this, [this]() { showGotoBar(true); });
}

void CodeEdit::addMoveLineShortcut() {
    auto upLines = new QShortcut(
        QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Up), this);
    upLines->setContext(Qt::WidgetShortcut);
    connect(upLines, &QShortcut::activated, this,
            [this]() { moveLines(QTextCursor::PreviousBlock); });
    auto downLines = new QShortcut(
        QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Down),
        this);
    downLines->setContext(Qt::WidgetShortcut);
    connect(downLines, &QShortcut::activated, this,
            [this]() { moveLines(QTextCursor::NextBlock); });
}

void CodeEdit::applyEditorSetStyle() {
    auto &set = ScriptSettings::instance();
    auto dfont = QFont(set.editorFontFamily());
    dfont.setPointSize(set.editorFontSize());

    auto thname = set.editorTheme();
    if (thname.isEmpty()) {
        switch (SkinManager::instance().currentTheme()) {
        case SkinManager::Theme::Dark:
            this->setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::DarkTheme));
            break;
        case SkinManager::Theme::Light:
            this->setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::LightTheme));
            break;
        }
    } else {
        this->setTheme(syntaxRepo().theme(thname));
    }

    this->setFont(dfont);
    this->setTabWidth(set.editorTabWidth());
    this->setIndentationMode(WingCodeEdit::IndentationMode(set.editorInden()));
    this->setShowLineNumbers(set.editorShowLineNumber());
    this->setShowFolding(set.editorFolding());
    this->setShowIndentGuides(set.editorShowGuideLine());
    this->setWordWrap(set.editorWordWrap());
    this->setShowLongLineEdge(set.editorShowLineEdges());
    this->setMatchBraces(set.editorMatchBraces());
    this->setShowWhitespace(set.editorShowWhiteSpace());
    this->setAutoCloseChar(set.editorAutoCloseChar());
}

SearchReplaceWidget *CodeEdit::searchWidget() const { return m_searchWidget; }

void CodeEdit::resizeEvent(QResizeEvent *event) {
    if (event)
        WingCodeEdit::resizeEvent(event);

    // Move the search widget to the upper-right corner
    auto margins = this->contentsMargins();
    auto off =
        this->width() - margins.right() - this->verticalScrollBar()->width();
    auto top = margins.top();

    auto size = m_searchWidget->sizeHint();
    m_searchWidget->resize(size);
    m_searchWidget->move(off - size.width(), top);

    size = m_gotoWidget->sizeHint();
    m_gotoWidget->resize(size);
    m_gotoWidget->move(off - size.width(), top);
}

void CodeEdit::keyPressEvent(QKeyEvent *event) {
    bool unHandled = true;
    switch (event->key()) {
    case Qt::Key_Escape:
        if (m_searchWidget->isVisible()) {
            m_searchWidget->cancel();
            unHandled = false;
        }
        if (m_gotoWidget->isVisible()) {
            m_gotoWidget->cancel();
            unHandled = false;
        }
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_searchWidget->isVisible()) {
            m_searchWidget->accept();
            unHandled = false;
        }
        if (m_gotoWidget->isVisible()) {
            m_gotoWidget->accept();
            unHandled = false;
        }
        break;
    default:
        break;
    }
    if (unHandled) {
        WingCodeEdit::keyPressEvent(event);
    }
}
