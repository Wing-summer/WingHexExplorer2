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
#include "class/snippetprocessor.h"
#include "control/searchreplacewidget.h"
#include "model/codecompletionmodel.h"
#include "wingcompleter.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QMetaEnum>
#include <QModelIndex>
#include <QRandomGenerator>
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
    auto completer = this->completer();
    if (completer->widget() != this) {
        return;
    }

    auto selfdata = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    selfdata.resolve();

    QTextCursor tc = textCursor();
    if (!completer->completionPrefix().isEmpty()) {
        tc.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
        tc.removeSelectedText();
    }

    auto resolver = [this](const QString &name) -> QString {
        static QHash<QString, SnippetProcessor::TM_CODE> maps;

        if (maps.isEmpty()) {
            auto e = QMetaEnum::fromType<SnippetProcessor::TM_CODE>();
            auto total = e.keyCount();
            for (int i = 0; i < total; ++i) {
                maps.insert(e.key(i), SnippetProcessor::TM_CODE(e.value(i)));
            }
        }

        if (!maps.contains(name)) {
            return {};
        }

        auto en = maps.value(name);
        switch (en) {
        case SnippetProcessor::TM_CODE::TM_SELECTED_TEXT: {
            auto completer = this->completer();
            return completer->completionPrefix();
        }
        case SnippetProcessor::TM_CODE::TM_CURRENT_LINE: {
            auto tc = textCursor();
            return tc.block().text();
        }
        case SnippetProcessor::TM_CODE::TM_CURRENT_WORD: {
            auto tc = textCursor();
            tc.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            return tc.selectedText();
        }
        case SnippetProcessor::TM_CODE::TM_LINE_INDEX: {
            auto tc = textCursor();
            return QString::number(tc.blockNumber());
        }
        case SnippetProcessor::TM_CODE::TM_LINE_NUMBER: {
            auto tc = textCursor();
            return QString::number(tc.blockNumber() + 1);
        }
        case SnippetProcessor::TM_CODE::TM_FILENAME: {
            // Assuming fileName is stored in it
            return windowFilePath();
        }
        case SnippetProcessor::TM_CODE::RELATIVE_FILEPATH:
        case SnippetProcessor::TM_CODE::TM_FILENAME_BASE: {
            auto fileName = windowFilePath();
            QFileInfo info(fileName);
            return info.fileName();
        }
        case SnippetProcessor::TM_CODE::TM_DIRECTORY: {
            auto fileName = windowFilePath();
            QFileInfo info(fileName);
            return info.filePath();
        }
        case SnippetProcessor::TM_CODE::TM_FILEPATH: {
            auto fileName = windowFilePath();
            QFileInfo info(fileName);
            return info.absoluteFilePath();
        }
        case SnippetProcessor::TM_CODE::CLIPBOARD:
            return QApplication::clipboard()->text();
        case SnippetProcessor::TM_CODE::WORKSPACE_NAME:
        case SnippetProcessor::TM_CODE::WORKSPACE_FOLDER:
            return {};
        case SnippetProcessor::TM_CODE::CURRENT_YEAR: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("yyyy"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_YEAR_SHORT: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("yy"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_MONTH: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("M"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_MONTH_NAME: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("MMMM"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_MONTH_NAME_SHORT: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("MMM"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_DATE: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("d"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_DAY_NAME: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("dddd"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_DAY_NAME_SHORT: {
            auto date = QDate::currentDate();
            return date.toString(QStringLiteral("ddd"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_HOUR: {
            auto time = QTime::currentTime();
            return time.toString(QStringLiteral("h"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_MINUTE: {
            auto time = QTime::currentTime();
            return time.toString(QStringLiteral("m"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_SECOND: {
            auto time = QTime::currentTime();
            return time.toString(QStringLiteral("s"));
        }
        case SnippetProcessor::TM_CODE::CURRENT_SECONDS_UNIX:
            return QString::number(QDateTime::currentSecsSinceEpoch());
        case SnippetProcessor::TM_CODE::RANDOM: {
            auto ran = QRandomGenerator::global();
            QString buffer(6, QChar{});
            for (int i = 0; i < 6; ++i) {
                buffer[i] = QChar(ran->bounded(0, 9) + '0');
            }
            return buffer;
        }
        case SnippetProcessor::TM_CODE::RANDOM_HEX: {
            auto ran = QRandomGenerator::global();
            QString buffer(6, QChar{});
            for (int i = 0; i < 6; ++i) {
                auto n = ran->bounded(0, 16);
                if (n >= 10) {
                    buffer[i] = QChar(n - 10 + 'A');
                } else {
                    buffer[i] = QChar(n + '0');
                }
            }
            return buffer;
        }
        case SnippetProcessor::TM_CODE::UUID:
            return QUuid::createUuid().toString();
        case SnippetProcessor::TM_CODE::BLOCK_COMMENT_START:
            return QStringLiteral("/*");
        case SnippetProcessor::TM_CODE::BLOCK_COMMENT_END:
            return QStringLiteral("*/");
        case SnippetProcessor::TM_CODE::LINE_COMMENT:
            return QStringLiteral("//");
            break;
        }
        return {};
    };

    auto comp = selfdata.completion();
    if (selfdata.isSnippet()) {
        SnippetProcessor snipt(resolver);
        auto r = snipt.process(comp);
        tc.insertText(r.expandedText);
        auto roff = r.expandedText.size() - r.cursorOffset;
        tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, roff);
    } else {
        tc.insertText(comp);
    }

    setTextCursor(tc);
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

void CodeEdit::toggleComment() {
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
        auto r = std::find_if_not(txt.begin(), txt.end(),
                                  [](const QChar &ch) { return ch.isSpace(); });
        auto len = std::distance(txt.begin(), r);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                            len);
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
            case Qt::Key_Slash: {
                if (event->modifiers() == Qt::ControlModifier) {
                    toggleComment();
                }
            } break;
            default:
                break;
            }
        }
    }

    if (unHandled) {
        if (isHelpTooltipVisible()) {
            if (event->modifiers() == Qt::KeyboardModifier::NoModifier) {
                auto key = event->key();
                if (key == Qt::Key_Semicolon) {
                    hideHelpTooltip();
                } else if (auto c = completer()) {
                    if (c) {
                        auto pp = c->popup();
                        if (!pp || !pp->isVisible()) {
                            if (key == Qt::Key_Enter || key == Qt::Key_Return) {
                                hideHelpTooltip();
                            }
                        }
                    }
                }
            }
        }
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
                   QKeySequence(Qt::ControlModifier | Qt::Key_Slash), this,
                   &CodeEdit::toggleComment);

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

void CodeEdit::mousePressEvent(QMouseEvent *e) {
    if (isHelpTooltipVisible()) {
        hideHelpTooltip();
    }
    WingCodeEdit::mousePressEvent(e);
}

void CodeEdit::wheelEvent(QWheelEvent *e) {
    if (isHelpTooltipVisible()) {
        hideHelpTooltip();
    }
    WingCodeEdit::wheelEvent(e);
}
