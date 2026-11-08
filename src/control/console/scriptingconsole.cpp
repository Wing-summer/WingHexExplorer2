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

#include "scriptingconsole.h"

#include <QApplication>
#include <QColor>
#include <QEventLoop>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QPointer>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextBlock>

#include "QConsoleWidget/QConsoleIODevice.h"
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include "class/settings/thememanager.h"
#include "control/editor/editorlspevent.h"
#include "control/editor/luaucompletion.h"
#include "control/editor/luauindentation.h"
#include "luau/lsp/luaulanguageserver.h"
#include "luau/runtime/scriptmachine.h"
#include "luau/runtime/scriptsettings.h"
#include "model/codecompletionmodel.h"

ScriptingConsole::ScriptingConsole(QWidget *parent)
    : ScriptingConsoleBase(parent) {
    connect(&ScriptSettings::instance(),
            &ScriptSettings::sigOnConsoleSettingUpdate, this,
            &ScriptingConsole::applyScriptSettings);
    connect(&ThemeManager::instance(), &ThemeManager::sigOnThemeChanged, this,
            &ScriptingConsole::applyScriptSettings);
    applyScriptSettings();
    setIndentationStrategy(
        [this](const WingCodeEdit::IndentationContext &context)
            -> std::optional<QString> {
            const bool currentLine =
                context.trigger == WingCodeEdit::IndentationTrigger::TextInput;
            auto cursor = context.cursor;
            if (cursor.position() < m_inpos || m_isWaitingRead) {
                return std::nullopt;
            }
            if (currentLine) {
                cursor.movePosition(QTextCursor::EndOfBlock);
            }
            cursor.setPosition(m_inpos, QTextCursor::KeepAnchor);
            auto source = cursor.selectedText();
            source.replace(QChar::ParagraphSeparator, QChar::LineFeed);
            if (!m_codes.isEmpty()) {
                source.prepend(m_codes.join('\n') + '\n');
            }
            return currentLine ? LuauIndentation::forCurrentLine(source, *this)
                               : LuauIndentation::forNewLine(source, *this);
        });
}

ScriptingConsole::~ScriptingConsole() {
    if (m_isTerminal) {
        // assuming we enable lsp after setting the terminal flag
        auto &lsp = LuauLanguageServer::instance();
        auto url = lspURL();
        lsp.onCloseDocument(url);
    }
}

void ScriptingConsole::handleReturnKey(Qt::KeyboardModifiers mod) {
    QString code = currentCommandLine();
    code.replace(QChar::ParagraphSeparator, QChar::LineFeed);

    setEditMode(Output);
    if (m_isWaitingRead && !code.isEmpty()) {
        m_history.add(code);
    }

    newLine();

    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);

    if (m_isWaitingRead) {
        Q_EMIT sigOnInputSubmitted(code);
        return;
    }

    // append the newline char and
    // send signal / update iodevice
    if (m_iodevice->isOpen()) {
        m_iodevice->consoleWidgetInput(code);
    }

    if (mod == Qt::ControlModifier) {
        m_codes.append(code);
        appendContinuationPrompt();
    } else {
        if (!m_isWaitingRead) {
            Q_EMIT sigOnConsoleCommand(code);
        }
    }
}

void ScriptingConsole::appendContinuationPrompt() {
    const auto indent =
        autoIndent() && !m_isWaitingRead
            ? LuauIndentation::forNewLine(m_codes.join('\n'), *this)
            : QString();
    appendCommandPrompt(true);
    // Establish the editable boundary before inserting automatic whitespace.
    setMode(Input);
    replaceCommandLine(indent);
    setEditMode(Input);
}

void ScriptingConsole::init() {
    m_getInputFn = [this]() { return getInput(); };

    connect(this, &QConsoleWidget::sigOnConsoleCommand, this,
            &ScriptingConsole::runConsoleCommand);

    auto cm = new LuauCompletion(this, this);
    cm->setParent(this);
    cm->setEnabled(false);
}

bool ScriptingConsole::handleBackspaceAtInputStart() {
    auto cursor = textCursor();
    if (cursor.position() > m_inpos) {
        const auto start = cursor.position();
        cursor.movePosition(QTextCursor::PreviousBlock);
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.setPosition(start, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        setTextCursor(cursor);
        return true;
    }
    if (m_isWaitingRead || m_codes.isEmpty()) {
        return false;
    }
    if (m_codes.last().contains('\n') ||
        !restorePreviousCommandPrompt(m_codes.size() > 1)) {
        // A recalled or pasted command may itself contain multiple blocks.
        prepareInputNavigation();
        return handleBackspaceAtInputStart();
    }
    m_codes.removeLast();
    setEditMode(Input);
    return true;
}

void ScriptingConsole::prepareInputNavigation() {
    if (m_isWaitingRead || m_codes.isEmpty()) {
        return;
    }

    const auto code = currentCodes();
    const auto lines = code.split('\n');
    const auto sourceOffset = [this, &lines](const QTextCursor &cursor) {
        const auto pos = cursorPosition(cursor);
        if (pos.blockNumber < 0 || pos.positionInBlock < 0) {
            return 0;
        }
        int offset = pos.positionInBlock;
        for (int i = 0; i < pos.blockNumber; ++i) {
            offset += lines.at(i).size() + 1;
        }
        return offset;
    };
    auto cursor = textCursor();
    const auto position = sourceOffset(cursor);
    auto anchorCursor = cursor;
    anchorCursor.setPosition(cursor.anchor());
    const auto anchor = sourceOffset(anchorCursor);
    const auto firstBlock = document()->findBlockByNumber(sourceFirstBlock());
    const auto inputStart = firstBlock.position() + sourceLineStart(firstBlock);

    // Restore source offsets before refreshInputBlocks adds display prefixes.
    setEditMode(Output);
    cursor.movePosition(QTextCursor::End);
    cursor.setPosition(inputStart, QTextCursor::KeepAnchor);
    cursor.insertText(code);
    m_codes.clear();
    setLastCommandPrompt(false);
    setMode(Input);
    m_inpos = inputStart;
    cursor.setPosition(inputStart + anchor);
    cursor.setPosition(inputStart + position, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    setEditMode(Input);
}

void ScriptingConsole::clearConsole() {
    setEditMode(Output);

    auto cur = this->textCursor();
    auto lastCmd = this->currentCommandLine();

    clear();

    if (!m_codes.isEmpty()) {
        auto pl = m_codes.begin();
        appendCommandPrompt(false);
        write(*pl);

        pl++;
        for (; pl != m_codes.end(); pl++) {
            appendCommandPrompt(true);
            write(*pl);
        }
        appendCommandPrompt(true);
    } else {
        appendCommandPrompt(false);
    }

    setEditMode(Input);
    replaceCommandLine(lastCmd);
    cur = this->textCursor();
    cur.movePosition(QTextCursor::End);
    setTextCursor(cur);
}

void ScriptingConsole::clear() {
    m_lastOutputType = ScriptMachine::MessageType::Unknown;
    ScriptingConsoleBase::clear();
}

void ScriptingConsole::processKeyEvent(QKeyEvent *e) { keyPressEvent(e); }

void ScriptingConsole::onOutput(const ScriptMachine::MessageInfo &message) {
    auto doc = this->document();
    auto lastLine = doc->lastBlock();
    // each empty block has a QChar::ParagraphSeparator
    auto isNotBlockStart = lastLine.length() > 1;

    auto fmtMsg = [](const ScriptMachine::MessageInfo &message) -> QString {
        QString msgend;
        if (!message.section.isEmpty()) {
            QFileInfo finfo(message.section);
            msgend =
                QStringLiteral(" [") + finfo.fileName() + QStringLiteral("]");
        }
        if (message.row <= 0 || message.col <= 0) {
            return message.message + msgend;
        } else {
            QString header =
                QStringLiteral("(") + QString::number(message.row) +
                QStringLiteral(", ") + QString::number(message.col) +
                QStringLiteral(") ");
            return header + message.message + msgend;
        }
    };

    auto packMetaData =
        [](const ScriptMachine::MessageInfo &message) -> WingEditorMetaInfo {
        WingEditorMetaInfo info;
        info.isClickable = true;
        info.section = message.section;
        info.goToLine = message.row;
        info.goToCol = message.col;
        return info;
    };

    auto msgtype = message.type;

    if (m_lastOutputType == ScriptMachine::MessageType::Unknown) {
        m_lastOutputType = msgtype;
    }

    switch (msgtype) {
    case ScriptMachine::MessageType::ExecInfo:
    case ScriptMachine::MessageType::Info:
        if (isNotBlockStart) {
            newLine();
        }
        stdOutLine(tr("[Info]") + fmtMsg(message), packMetaData(message));
        flush();
        break;
    case ScriptMachine::MessageType::Warn:
        if (isNotBlockStart) {
            newLine();
        }
        stdWarnLine(tr("[Warn]") + fmtMsg(message), packMetaData(message));
        flush();
        break;
    case ScriptMachine::MessageType::Error:
        if (isNotBlockStart) {
            newLine();
        }
        stdErrLine(tr("[Error]") + fmtMsg(message), packMetaData(message));
        flush();
        break;
    case ScriptMachine::MessageType::Print:
        if (isNotBlockStart &&
            m_lastOutputType != ScriptMachine::MessageType::Print) {
            newLine();
        }
        stdOutLine(message.message);
        flush();
        break;

    case ScriptMachine::MessageType::Unknown:
        // should not go there
        break;
    }

    m_lastOutputType = msgtype;
}

void ScriptingConsole::abortCurrentCode() {
    setEditMode(Output);
    if (m_codes.isEmpty()) {
        replaceCommandLine({});
    } else {
        m_codes.clear();
        appendCommandPrompt();
    }
    setEditMode(Input);
}

void ScriptingConsole::applyScriptSettings() {
    auto &set = ScriptSettings::instance();

    auto thname = set.consoleTheme();
    if (thname.isEmpty()) {
        switch (ThemeManager::instance().currentTheme()) {
        case ThemeManager::Theme::Dark:
            setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::DarkTheme));
            break;
        case ThemeManager::Theme::System:
        case ThemeManager::Theme::Light:
            setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::LightTheme));
            break;
        }
    } else {
        setTheme(syntaxRepo().theme(thname));
    }

    setDefaultFont(set.consoleFont());
    this->setTabWidth(set.consoleTabWidth());
    this->setIndentationMode(WingCodeEdit::IndentationMode(set.consoleInden()));
    this->setMatchBraces(set.consoleMatchBraces());
    this->setShowWhitespace(set.consoleShowWhiteSpace());
    this->setAutoCloseChar(set.consoleAutoCloseChar());
    this->setAutoIndent(set.consoleAutoIden());
    consoleHighligher()->rehighlight();
}

void ScriptingConsole::runConsoleCommand(const QString &code) {
    hideHelpTooltip();
    setEditMode(Output);
    m_codes.append(code);
    ScriptMachine::instance().executeCode(
        ScriptMachine::Interactive, m_codes.join('\n'), [this](bool finished) {
            if (finished) {
                const auto code = m_codes.join('\n');
                if (!code.trimmed().isEmpty()) {
                    m_history.add(code);
                }
                m_codes.clear();
                appendCommandPrompt(false);
                setEditMode(Input);
                Q_EMIT sigOnConsoleScriptRunFinished();
            } else {
                appendContinuationPrompt();
            }
        });
}

QString ScriptingConsole::getInput(ScriptMachine::ConsoleMode mode) {
    if (m_isWaitingRead) {
        return QString();
    }
    QEventLoop loop;
    QString result;
    QPointer<ScriptingConsole> guard(this);
    connect(this, &ScriptingConsole::sigOnInputSubmitted, &loop,
            [&result, &loop](const QString &text) {
                result = text;
                loop.quit();
            });
    connect(this, &QObject::destroyed, &loop, &QEventLoop::quit);
    connect(this, &ScriptingConsole::sigOnInputClosed, &loop,
            &QEventLoop::quit);
    connect(&ScriptMachine::instance(), &ScriptMachine::sigOnInputCancelled,
            &loop, [&loop, mode](ScriptMachine::ConsoleMode cancelled) {
                if (cancelled == mode) {
                    loop.quit();
                }
            });
    appendCommandPrompt(true);
    m_isWaitingRead = true;
    setEditMode(Input);
    auto block = textCursor().block();
    consoleHighligher()->setBlockAsTextOnly(block);
    loop.exec();
    if (guard) {
        m_isWaitingRead = false;
        setEditMode(Output);
    }
    return result;
}

QString ScriptingConsole::packUpLoggingStr(const QString &message) {
    return tr("[Console]") + ' ' + message;
}

void ScriptingConsole::keyPressEvent(QKeyEvent *e) {
    if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_L) {
        clearConsole();
    } else {
        QConsoleWidget::keyPressEvent(e);
    }
}

void ScriptingConsole::mousePressEvent(QMouseEvent *e) {
    if (isHelpTooltipVisible()) {
        hideHelpTooltip();
    }
    ScriptingConsoleBase::mousePressEvent(e);
}

void ScriptingConsole::wheelEvent(QWheelEvent *e) {
    if (isHelpTooltipVisible()) {
        hideHelpTooltip();
    }
    ScriptingConsoleBase::wheelEvent(e);
}

bool ScriptingConsole::event(QEvent *event) {
    if (m_isWaitingRead &&
        (event->type() == QEvent::Hide || event->type() == QEvent::Close)) {
        Q_EMIT sigOnInputClosed();
    }
    if (EditorLspEvent::processEvent(event, this)) {
        return true;
    }
    return ScriptingConsoleBase::event(event);
}

void ScriptingConsole::onCompletion(const QModelIndex &index) {
    auto completer = this->completer();
    if (!completer || completer->widget() != this) {
        return;
    }

    auto tip = index.data(Qt::SelfDataRole).value<CodeInfoTip>();
    auto edit = textCursor();
    edit.beginEditBlock();
    tip.applyEdit(this, completer->completionPrefix(), false);
    reindentCurrentLine();
    edit.endEditBlock();
}

void ScriptingConsole::paste() {
    if (ScriptMachine::instance().isRunning(ScriptMachine::Interactive)) {
        return;
    }

    QConsoleWidget::paste();
}

void ScriptingConsole::syncDocChange() {
    const auto source = currentCodes();
    m_lspDocument.setPlainText(source);
    LuauLanguageServer::instance().onUpdateDocument(lspURL(), &m_lspDocument);
}

void ScriptingConsole::syncSemanticTokens() {
    // SemanticTokens are not supported with console
}

QVector<lsp::SemanticToken> ScriptingConsole::parseSemanticTokens() {
    // SemanticTokens are not supported with console
    return {};
}

lsp::DocumentUri ScriptingConsole::lspURL() {
    return QUrl(QStringLiteral("dev://luau_console"));
}

void ScriptingConsole::setEditMode(ConsoleMode mode) {
    setMode(mode);
    if (mode == Input && !m_isWaitingRead) {
        completer()->setEnabled(true);
        Q_EMIT textChanged();
    } else {
        completer()->setEnabled(false);
        clearSquiggle();
    }
}

LspEditorInterace::CursorPos
ScriptingConsole::cursorPosition(const QTextCursor &cursor) const {
    if (mode() != Input || m_isWaitingRead) {
        return {};
    }
    const auto block = cursor.block();
    const auto line = block.blockNumber() - sourceFirstBlock();
    const auto column = cursor.positionInBlock() - sourceLineStart(block);
    if (line < 0 || column < 0) {
        return {};
    }
    return {line, column};
}

int ScriptingConsole::sourceFirstBlock() const {
    auto precedingLines = m_codes.size();
    for (const auto &code : m_codes) {
        precedingLines += code.count('\n');
    }
    return document()->findBlock(m_inpos).blockNumber() - precedingLines;
}

int ScriptingConsole::sourceLineStart(const QTextBlock &block) const {
    const auto inputBlock = document()->findBlock(m_inpos);
    if (block == inputBlock) {
        return m_inpos - block.position();
    }
    return consoleHighligher()->blockPrefixLength(block);
}

LspEditorInterace::CursorPos
ScriptingConsole::consolePosition(const lsp::Position &position) const {
    const auto doc = document();
    const auto firstBlock = sourceFirstBlock();
    if (firstBlock < 0 || firstBlock >= doc->blockCount() ||
        position.line >= uint(doc->blockCount() - firstBlock)) {
        return {};
    }
    const auto block = doc->findBlockByNumber(firstBlock + int(position.line));
    if (!block.isValid()) {
        return {};
    }
    const auto prefix = sourceLineStart(block);
    // Block length includes a paragraph separator, even for the final block.
    const auto codeLength = block.length() - 1 - prefix;
    if (prefix < 0 || codeLength < 0 || position.character > uint(codeLength)) {
        return {};
    }
    return {block.blockNumber(), prefix + int(position.character)};
}

LspEditorInterace::CursorPos ScriptingConsole::currentPosition() const {
    return cursorPosition(textCursor());
}

void ScriptingConsole::showFunctionTip(
    const QList<WingSignatureTooltip::Signature> &sigs) {
    showHelpTooltip(sigs);
}

void ScriptingConsole::clearFunctionTip() { hideHelpTooltip(); }

bool ScriptingConsole::isTerminal() const { return m_isTerminal; }

void ScriptingConsole::setIsTerminal(bool newIsTerminal) {
    m_isTerminal = newIsTerminal;
}

QList<QTextBlock> ScriptingConsole::visibleTextBlocks() const {
    QList<QTextBlock> blocks;

    auto first = firstVisibleBlock();
    if (!first.isValid()) {
        return {};
    }

    auto block = first;
    auto rect = viewport()->rect();
    auto bottom = rect.bottom();
    while (block.isValid()) {
        auto blockRect =
            blockBoundingGeometry(block).translated(contentOffset()).toRect();

        if (blockRect.top() > bottom) {
            break;
        }

        if (blockRect.intersects(rect)) {
            blocks.append(block);
        }

        block = block.next();
    }

    return blocks;
}

const WingCodeEdit *ScriptingConsole::editorPtr() const { return this; }

lsp::DocumentUri ScriptingConsole::lspFileNameURL() const { return lspURL(); }

QString ScriptingConsole::currentCodes() const {
    const auto code = currentCommandLine();
    if (m_codes.isEmpty()) {
        return code;
    }
    return m_codes.join('\n') + '\n' + code;
}

void ScriptingConsole::enableLSP() {
    if (!m_isTerminal) {
        return;
    }

    auto lsp = &LuauLanguageServer::instance();
    connect(
        lsp, &LuauLanguageServer::sigOnPublishDiagnostic, this,
        [this](const QUrl &url, const QVector<lsp::Diagnostic> &diagnostics) {
            if (url == lspURL() && mode() == Input && !m_isWaitingRead) {
                auto lsps = [](lsp::DiagnosticSeverity s)
                    -> WingCodeEdit::SeverityLevel {
                    switch (s) {
                    case lsp::DiagnosticSeverity::Error:
                        return WingCodeEdit::SeverityLevel::Error;
                    case lsp::DiagnosticSeverity::Warning:
                        return WingCodeEdit::SeverityLevel::Warning;
                    case lsp::DiagnosticSeverity::Information:
                        return WingCodeEdit::SeverityLevel::Information;
                    case lsp::DiagnosticSeverity::Hint:
                        return WingCodeEdit::SeverityLevel::Hint;
                    }
                    return WingCodeEdit::SeverityLevel::Information;
                };

                clearSquiggle();
                for (const auto &d : diagnostics) {
                    const auto start = consolePosition(d.range.start);
                    const auto end = consolePosition(d.range.end);
                    if (start.blockNumber < 0 || end.blockNumber < 0) {
                        continue;
                    }
                    // Squiggles use one-based lines and zero-based columns.
                    addSquiggle(lsps(d.severity),
                                {start.blockNumber + 1, start.positionInBlock},
                                {end.blockNumber + 1, end.positionInBlock},
                                d.message);
                }
                highlightAllSquiggle();
            }
        });

    lsp->onOpenDocument(lspURL(), &m_lspDocument);
    connect(this, &ScriptingConsole::textChanged, this, [this]() {
        if (m_mode == Output || m_isWaitingRead) {
            return;
        }
        syncDocChange();
    });

    completer()->setEnabled(true);
}

void ScriptingConsole::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    auto &theme = ThemeManager::instance();
    auto a = menu.addAction(
        theme.standardIcon(ThemeManager::StandardIcon::Copy), tr("Copy"),
        QKeySequence(QKeySequence::Copy), this, &ScriptingConsole::copy);
    a->setShortcutContext(Qt::WidgetShortcut);
    a = menu.addAction(theme.standardIcon(ThemeManager::StandardIcon::Cut),
                       tr("Cut"), QKeySequence(QKeySequence::Cut), this,
                       &ScriptingConsole::cut);
    a->setShortcutContext(Qt::WidgetShortcut);
    a->setEnabled(canCut());
    a = menu.addAction(theme.standardIcon(ThemeManager::StandardIcon::Paste),
                       tr("Paste"), QKeySequence(QKeySequence::Paste), this,
                       &ScriptingConsole::paste);
    a->setShortcutContext(Qt::WidgetShortcut);
    a->setEnabled(mode() == Input && canPaste());

    if (m_isTerminal) {
        a = menu.addAction(
            theme.standardIcon(ThemeManager::StandardIcon::Delete), tr("Clear"),
            QKeySequence(Qt::ControlModifier | Qt::Key_L), this,
            &ScriptingConsole::clearConsole);
        a->setShortcutContext(Qt::WidgetShortcut);
        menu.addSeparator();
        a = menu.addAction(
            theme.standardIcon(ThemeManager::StandardIcon::DebugStop),
            tr("Terminate script"),
            QKeySequence(Qt::ControlModifier | Qt::Key_Q), this, [this]() {
                auto &m = ScriptMachine::instance();
                if (m_isTerminal) {
                    if (m.isRunning(ScriptMachine::Interactive)) {
                        m.abortScript(ScriptMachine::Interactive);
                    } else {
                        abortCurrentCode();
                    }
                } else {
                    m.abortScript(ScriptMachine::Background);
                }
            });
        a->setShortcutContext(Qt::WidgetShortcut);
    } else {
        a = menu.addAction(
            theme.standardIcon(ThemeManager::StandardIcon::Delete), tr("Clear"),
            QKeySequence(Qt::ControlModifier | Qt::Key_L), this,
            &ScriptingConsole::clear);
        a->setShortcutContext(Qt::WidgetShortcut);
    }

    menu.exec(event->globalPos());
}
