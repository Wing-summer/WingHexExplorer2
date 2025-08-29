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
#include <QIcon>
#include <QMenu>
#include <QModelIndex>
#include <QScrollBar>
#include <QShortcut>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

CodeEdit::CodeEdit(QWidget *parent) : WingCodeEditLSP(parent) {
    connect(this->document(), &QTextDocument::modificationChanged, this,
            &CodeEdit::contentModified);
    addEditorBasicShortcut();
    addMoveLineShortcut();

    connect(&ScriptSettings::instance(), &ScriptSettings::editorSettingsUpdate,
            this, &CodeEdit::applyEditorSetStyle);
    applyEditorSetStyle();
    setShowSymbolMark(true);

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
        m_searchWidget->resize(m_searchWidget->sizeHint());
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
    m_gotoWidget->resize(m_gotoWidget->sizeHint());
}

void CodeEdit::onCompletion(const QModelIndex &index) {
    WingCodeEdit::onCompletion(index);
    auto selfdata = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    if (selfdata.type == LSP::CompletionItemKind::Function) {
        // TODO
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
    this->setAutoIndent(set.editorAutoIden());
}

SearchReplaceWidget *CodeEdit::searchWidget() const { return m_searchWidget; }

void CodeEdit::setContentModified(bool b) { Q_EMIT contentModified(b); }

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
    if (event->matches(QKeySequence::Find)) {
        showSearchReplaceBar(true, false);
        unHandled = false;
    } else if (event->matches(QKeySequence::Replace)) {
        showSearchReplaceBar(true, true);
        unHandled = false;
    } else {
        auto gotoSc =
            QKeySequences::instance().keySequence(QKeySequences::Key::GOTO);
        if (QKeySequence(event->keyCombination()).matches(gotoSc)) {
            showGotoBar(true);
            unHandled = false;
        } else {
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
        }
    }

    if (unHandled) {
        WingCodeEdit::keyPressEvent(event);
    }
}

void CodeEdit::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    menu.addAction(QIcon(QStringLiteral(":/qeditor/copy.png")), tr("Copy"),
                   QKeySequence(QKeySequence::Copy), this, &CodeEdit::copy);
    menu.addAction(QIcon(QStringLiteral(":/qeditor/cut.png")), tr("Cut"),
                   QKeySequence(QKeySequence::Cut), this, &CodeEdit::cut);
    menu.addAction(QIcon(QStringLiteral(":/qeditor/paste.png")), tr("Paste"),
                   QKeySequence(QKeySequence::Paste), this, &CodeEdit::paste);
    menu.addSeparator();

    menu.addAction(QIcon(QStringLiteral(":/qeditor/find.png")), tr("Find"),
                   QKeySequence(QKeySequence::Find), this,
                   [this]() { showSearchReplaceBar(true, false); });
    menu.addAction(QIcon(QStringLiteral(":/qeditor/replace.png")),
                   tr("Replace"), QKeySequence(QKeySequence::Replace), this,
                   [this]() { showSearchReplaceBar(true, true); });
    menu.addAction(
        QIcon(QStringLiteral(":/qeditor/goto.png")), tr("Goto"),
        QKeySequences::instance().keySequence(QKeySequences::Key::GOTO), this,
        [this]() { showGotoBar(true); });
    menu.addSeparator();

    menu.addAction(tr("Comment/UnComment"),
                   QKeySequence(Qt::ControlModifier | Qt::Key_Backslash), this,
                   [this]() {
                       auto cursor = this->textCursor();
                       if (cursor.hasSelection()) {
                           auto start = cursor.selectionStart();
                           auto end = cursor.selectionEnd();
                           auto cmtbegin = QStringLiteral("/*");
                           auto cmtend = QStringLiteral("*/");

                           // only add comment
                           cursor.beginEditBlock();
                           cursor.setPosition(end);
                           cursor.insertText(cmtend);
                           cursor.setPosition(start);
                           cursor.insertText(cmtbegin);
                           cursor.endEditBlock();
                       } else {
                           auto blk = cursor.block();
                           auto txt = blk.text();
                           auto comment = QStringLiteral("//");

                           cursor.movePosition(QTextCursor::StartOfLine);
                           auto r = std::find_if_not(
                               txt.begin(), txt.end(),
                               [](const QChar &ch) { return ch.isSpace(); });
                           auto len = std::distance(txt.begin(), r);
                           cursor.movePosition(QTextCursor::NextCharacter,
                                               QTextCursor::MoveAnchor, len);
                           auto oldcur = cursor;
                           if (r == txt.end() || len + 2 > txt.length()) {
                               cursor.insertText(comment);
                           } else {
                               cursor.movePosition(QTextCursor::NextCharacter,
                                                   QTextCursor::KeepAnchor, 2);
                               auto sel = cursor.selectedText();
                               if (sel == comment) {
                                   cursor.removeSelectedText();
                               } else {
                                   oldcur.insertText(comment);
                               }
                           }
                       }
                   });

    menu.addAction(tr("Fold/UnFold"), this, [this]() {
        if (isCurrentLineFolded()) {
            unfoldCurrentLine();
        } else {
            foldCurrentLine();
        }
    });
    menu.addAction(tr("FoldAll"), this, [this]() { foldAll(); });

    menu.exec(event->globalPos());
}
