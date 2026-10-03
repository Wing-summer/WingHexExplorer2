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
#include "QConsoleWidget/QConsoleIODevice.h"
#include "class/editorlspevent.h"
#include "class/luaucompletion.h"
#include "class/scriptmachine.h"
#include "class/scriptsettings.h"
#include "class/skinmanager.h"
#include "luau/lsp/luaulanguageserver.h"
#include "model/codecompletionmodel.h"
#include "utilities.h"

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QTextBlock>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

ScriptingConsole::ScriptingConsole(QWidget *parent)
    : ScriptingConsoleBase(parent) {
    connect(&ScriptSettings::instance(), &ScriptSettings::consoleSettingUpdate,
            this, &ScriptingConsole::applyScriptSettings);
    applyScriptSettings();
}

ScriptingConsole::~ScriptingConsole() {
    if (_isTerminal) {
        // assuming we enable lsp after setting the terminal flag
        auto &lsp = LuauLanguageServer::instance();
        auto url = lspURL();
        lsp.onCloseDocument(url);
    }
}

void ScriptingConsole::handleReturnKey(Qt::KeyboardModifiers mod) {
    QString code = getCommandLine();

    setEditMode(Output);
    if (!code.isEmpty()) {
        history_.add(code);
    }

    newLine();

    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);

    // append the newline char and
    // send signal / update iodevice
    if (iodevice_->isOpen())
        iodevice_->consoleWidgetInput(code);

    if (mod == Qt::ControlModifier) {
        _codes.append(code);
        appendCommandPrompt(true);
        setEditMode(Input);
    } else {
        if (!_isWaitingRead) {
            Q_EMIT consoleCommand(code);
        }
    }
}

void ScriptingConsole::init() {
    _getInputFn = std::bind(&ScriptingConsole::getInput, this);

    connect(this, &QConsoleWidget::consoleCommand, this,
            &ScriptingConsole::runConsoleCommand);

    auto cm = new LuauCompletion(this, this);
    cm->setParent(this);
    cm->setEnabled(false);
}

void ScriptingConsole::clearConsole() {
    setEditMode(Output);

    auto cur = this->textCursor();
    auto lastCmd = this->currentCommandLine();

    clear();

    if (lastCommandPrompt()) {
        auto pl = _codes.begin();
        appendCommandPrompt(false);
        write(*pl);

        pl++;
        for (; pl != _codes.end(); pl++) {
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
    _lastOutputType = ScriptMachine::MessageType::Unknown;
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

    if (_lastOutputType == ScriptMachine::MessageType::Unknown) {
        _lastOutputType = msgtype;
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
            _lastOutputType != ScriptMachine::MessageType::Print) {
            newLine();
        }
        stdOutLine(message.message);
        flush();
        break;

    case ScriptMachine::MessageType::Unknown:
        // should not go there
        break;
    }

    _lastOutputType = msgtype;
}

void ScriptingConsole::abortCurrentCode() {
    setEditMode(Output);
    if (_codes.isEmpty()) {
        replaceCommandLine({});
    } else {
        _codes.clear();
        appendCommandPrompt();
    }
    setEditMode(Input);
}

void ScriptingConsole::applyScriptSettings() {
    auto &set = ScriptSettings::instance();
    auto dfont = QFont(set.consoleFontFamily());
    dfont.setPointSize(set.consoleFontSize());

    auto thname = set.consoleTheme();
    if (thname.isEmpty()) {
        switch (SkinManager::instance().currentTheme()) {
        case SkinManager::Theme::Dark:
            setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::DarkTheme));
            break;
        case SkinManager::Theme::Light:
            setTheme(syntaxRepo().defaultTheme(
                KSyntaxHighlighting::Repository::LightTheme));
            break;
        }
    } else {
        setTheme(syntaxRepo().theme(thname));
    }

    this->setFont(dfont);
    this->setTabWidth(set.consoleTabWidth());
    this->setIndentationMode(WingCodeEdit::IndentationMode(set.consoleInden()));
    this->setMatchBraces(set.consoleMatchBraces());
    this->setShowWhitespace(set.consoleShowWhiteSpace());
    this->setAutoCloseChar(set.consoleAutoCloseChar());
}

void ScriptingConsole::runConsoleCommand(const QString &code) {
    hideHelpTooltip();
    auto exec = code.trimmed();
    setEditMode(Output);
    _codes.append(exec);
    ScriptMachine::instance().executeCode(
        ScriptMachine::Interactive, _codes.join('\n'),
        [this, exec](bool finished) {
            if (finished) {
                _codes.clear();
                appendCommandPrompt(false);
                setEditMode(Input);
                Q_EMIT consoleScriptRunFinished();
            } else {
                appendCommandPrompt(true);
                setEditMode(Input);
            }
        });
}

QString ScriptingConsole::getInput() {
    auto &s = consoleStream();
    appendCommandPrompt(true);
    _isWaitingRead = true;
    setEditMode(Input);
    auto blk = textCursor().block();
    consoleHighligher()->setBlockAsTextOnly(blk);
    auto d = s.device();
    auto ba = d->bytesAvailable();
    d->skip(ba);
    d->waitForReadyRead(-1);
    QString instr;
    s >> instr;
    _isWaitingRead = false;
    setEditMode(Output);
    return instr;
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
    tip.applyEdit(this, completer->completionPrefix(), false);
}

void ScriptingConsole::paste() {
    if (ScriptMachine::instance().isRunning(ScriptMachine::Interactive)) {
        return;
    }

    const QMimeData *const clipboard = QApplication::clipboard()->mimeData();
    QString text = clipboard->text();
    if (!text.isEmpty()) {
        if (text.contains('\n')) {
            // TODO: mutiline codes?
            return;
        }
        if (isCursorInEditZone()) {
            auto cursor = this->textCursor();
            cursor.insertText(text);
        } else {
            replaceCommandLine(text);
        }
    }
}

void ScriptingConsole::syncDocChange() {
    const auto source = currentCodes();
    _lspDocument.setPlainText(source);
    LuauLanguageServer::instance().onUpdateDocument(lspURL(), &_lspDocument);
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
    if (mode == Input && !_isWaitingRead) {
        completer()->setEnabled(true);
        Q_EMIT textChanged();
    } else {
        completer()->setEnabled(false);
    }
}

LspEditorInterace::CursorPos
ScriptingConsole::cursorPosition(const QTextCursor &cursor) const {
    auto block = cursor.block();
    int prefixLen = 0;
    auto hl = consoleHighligher();
    if (hl) {
        prefixLen = hl->blockPrefixLength(block);
    }
    LspEditorInterace::CursorPos pos;
    pos.blockNumber = _codes.length() + 1;
    pos.positionInBlock = cursor.positionInBlock() - prefixLen - 1;
    return pos;
}

LspEditorInterace::CursorPos ScriptingConsole::currentPosition() const {
    return cursorPosition(textCursor());
}

void ScriptingConsole::showFunctionTip(
    const QList<WingSignatureTooltip::Signature> &sigs) {
    showHelpTooltip(sigs);
}

void ScriptingConsole::clearFunctionTip() { hideHelpTooltip(); }

bool ScriptingConsole::isTerminal() const { return _isTerminal; }

void ScriptingConsole::setIsTerminal(bool newIsTerminal) {
    _isTerminal = newIsTerminal;
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

        if (blockRect.top() > bottom)
            break;

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
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(inpos_, QTextCursor::KeepAnchor);
    if (_codes.isEmpty()) {
        return textCursor.selectedText();
    }
    return _codes.join('\n') + textCursor.selectedText();
}

void ScriptingConsole::enableLSP() {
    if (!_isTerminal) {
        return;
    }

    auto lsp = &LuauLanguageServer::instance();
    connect(
        lsp, &LuauLanguageServer::onPublishDiagnostic, this,
        [this](const QUrl &url, const QVector<lsp::Diagnostic> &diagnostics) {
            if (url == lspURL()) {
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

                auto doc = document();
                auto block = doc->lastBlock();
                auto hl = this->consoleHighligher();
                auto prefix = hl->blockPrefixLength(block) + 1;

                clearSquiggle();
                auto t = _codes.size();
                auto offline = block.blockNumber() - t;
                for (const auto &d : diagnostics) {
                    addSquiggle(lsps(d.severity),
                                {offline + d.range.start.line,
                                 prefix + d.range.start.character},
                                {offline + d.range.end.line,
                                 prefix + d.range.end.character},
                                d.message);
                }
                highlightAllSquiggle();
            }
        });

    lsp->onOpenDocument(lspURL(), &_lspDocument);
    connect(this, &ScriptingConsole::textChanged, this, [this]() {
        if (mode_ == Output || _isWaitingRead) {
            return;
        }
        syncDocChange();
    });

    completer()->setEnabled(true);
}

void ScriptingConsole::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    auto a = menu.addAction(QIcon(QStringLiteral(":/qeditor/copy.png")),
                            tr("Copy"), QKeySequence(QKeySequence::Copy), this,
                            &ScriptingConsole::copy);
    a->setShortcutContext(Qt::WidgetShortcut);
    a = menu.addAction(QIcon(QStringLiteral(":/qeditor/cut.png")), tr("Cut"),
                       QKeySequence(QKeySequence::Cut), this,
                       &ScriptingConsole::cut);
    a->setShortcutContext(Qt::WidgetShortcut);
    a = menu.addAction(QIcon(QStringLiteral(":/qeditor/paste.png")),
                       tr("Paste"), QKeySequence(QKeySequence::Paste), this,
                       &ScriptingConsole::paste);
    a->setShortcutContext(Qt::WidgetShortcut);

    if (_isTerminal) {
        a = menu.addAction(ICONRES(QStringLiteral("del")), tr("Clear"),
                           QKeySequence(Qt::ControlModifier | Qt::Key_L), this,
                           &ScriptingConsole::clearConsole);
        a->setShortcutContext(Qt::WidgetShortcut);
        menu.addSeparator();
        a = menu.addAction(
            ICONRES(QStringLiteral("dbgstop")), tr("AbortScript"),
            QKeySequence(Qt::ControlModifier | Qt::Key_Q), this, [this]() {
                auto &m = ScriptMachine::instance();
                if (_isTerminal) {
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
        a = menu.addAction(ICONRES(QStringLiteral("del")), tr("Clear"),
                           QKeySequence(Qt::ControlModifier | Qt::Key_L), this,
                           &ScriptingConsole::clear);
        a->setShortcutContext(Qt::WidgetShortcut);
    }

    menu.exec(event->globalPos());
}
