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

#include "scriptingconsole.h"
#include "QConsoleWidget/QConsoleIODevice.h"
#include "class/angellsp.h"
#include "class/scriptmachine.h"
#include "class/scriptsettings.h"
#include "class/skinmanager.h"
#include "class/snippetprocessor.h"
#include "class/wingmessagebox.h"
#include "control/consolecodeedit.h"
#include "dialog/framelessdialogbase.h"
#include "model/codecompletionmodel.h"
#include "utilities.h"

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QIcon>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QRandomGenerator>
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
        auto &lsp = AngelLsp::instance();
        if (lsp.isActive()) {
            lsp.closeDocument(lspURL());
        }
    }
}

void ScriptingConsole::handleReturnKey(Qt::KeyboardModifiers mod) {
    QString code = getCommandLine();

    setEditMode(Output);
    if (code.isEmpty()) {
        if (mod.testFlags(Qt::ControlModifier | Qt::AltModifier)) {
            if (ScriptMachine::instance().isRunning(
                    ScriptMachine::Interactive)) {
                return;
            }

            // pop up a coding dialog
            auto edialog = new FramelessDialogBase;

            auto editor = new ConsoleCodeEdit(edialog);
            connect(editor, &ConsoleCodeEdit::onCloseEvent, edialog,
                    &FramelessDialogBase::accept);
            edialog->buildUpContent(editor);
            edialog->setWindowTitle(tr("ConsoleMutiLine"));
            edialog->setMinimumSize(400, 450);

            auto ret = edialog->exec();
            if (ret) {
                code = editor->toPlainText();
                auto lines = code.split('\n');
                if (!lines.isEmpty()) {
                    auto fline = lines.at(0);
                    write(fline);
                    for (qsizetype i = 1; i < lines.size(); i++) {
                        newLine();
                        appendCommandPrompt(true);
                        write(lines.at(i));
                    }
                }
                static QRegularExpression regex("[\\r\\n]");
                code.remove(regex);
                history_.add(code);
                edialog->deleteLater();
            } else {
                setEditMode(Input);
                edialog->deleteLater();
                return;
            }
        }
    } else {
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
        if (_codes.isEmpty()) {
            _codes = code;
        } else {
            _codes.append('\n').append(code);
        }
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

    auto cm = new AsConsoleCompletion(this);
    cm->setParent(this);
    cm->setEnabled(false);
}

void ScriptingConsole::clearConsole() {
    setEditMode(Output);

    auto cur = this->textCursor();
    auto off = cur.position() - this->currentHeaderPos();
    auto lastCmd = this->currentCommandLine();
    auto dis = lastCmd.length() - off;

    clear();

    if (lastCommandPrompt()) {
        auto lines = _codes.split('\n');
        auto pl = lines.begin();
        appendCommandPrompt(false);
        write(*pl);

        pl++;
        for (; pl != lines.end(); pl++) {
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
    cur.movePosition(QTextCursor::EndOfBlock);
    cur.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, dis);
    setTextCursor(cur);
}

void ScriptingConsole::processKeyEvent(QKeyEvent *e) { keyPressEvent(e); }

void ScriptingConsole::onOutput(const ScriptMachine::MessageInfo &message) {
    auto doc = this->document();
    auto lastLine = doc->lastBlock();
    // each empty block has a QChar::ParagraphSeparator
    auto isNotBlockStart = lastLine.length() > 1;

    auto fmtMsg = [](const ScriptMachine::MessageInfo &message) -> QString {
        if (message.row <= 0 || message.col <= 0) {
            return message.message;
        } else {
            return QStringLiteral("(") + QString::number(message.row) +
                   QStringLiteral(", ") + QString::number(message.col) +
                   QStringLiteral(") ") + message.message;
        }
    };

    switch (message.type) {
    case ScriptMachine::MessageType::Info:
        if (isNotBlockStart) {
            newLine();
        }
        stdOutLine(tr("[Info]") + fmtMsg(message));
        flush();
        break;
    case ScriptMachine::MessageType::Warn:
        if (isNotBlockStart) {
            newLine();
        }
        stdWarnLine(tr("[Warn]") + fmtMsg(message));
        flush();
        break;
    case ScriptMachine::MessageType::Error:
        if (isNotBlockStart) {
            newLine();
        }
        stdErrLine(tr("[Error]") + fmtMsg(message));
        flush();
        break;
    case ScriptMachine::MessageType::Print:
        if (isNotBlockStart &&
            _lastOutputType != ScriptMachine::MessageType::Print) {
            newLine();
        }
        stdOutLine(message.message);
        break;
    case ScriptMachine::MessageType::Unknown:
        // should not go there
        break;
    }

    _lastOutputType = message.type;
}

void ScriptingConsole::abortCurrentCode() {
    setEditMode(Output);
    _codes.clear();
    appendCommandPrompt();
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

void ScriptingConsole::onSendFullTextChangeCompleted() {
    if (!_lastSent) {
        sendDocChange();
        _lastSent = true;
    }
    _ok = true;
}

void ScriptingConsole::runConsoleCommand(const QString &code) {
    hideHelpTooltip();
    auto exec = code.trimmed();
    if (exec == QStringLiteral("#ls")) {
        auto &ins = ScriptMachine::instance();
        auto mod = ins.module(ScriptMachine::Interactive);
        if (mod) {
            auto total = mod->GetGlobalVarCount();

            setEditMode(Output);

            if (total == 0) {
                stdOutLine("<none>");
            } else {
                auto &sm = ScriptMachine::instance();
                for (asUINT i = 0; i < total; ++i) {
                    const char *name;
                    int typeID;
                    auto decl = mod->GetGlobalVarDeclaration(i);
                    if (decl && mod->GetGlobalVar(i, &name, nullptr, &typeID) ==
                                    asSUCCESS) {
                        stdOutLine(decl + QStringLiteral(";"));
                        newLine();
                    }
                }
            }

            _codes.clear();
            appendCommandPrompt();
            setEditMode(Input);
        }
    } else if (exec.startsWith(QStringLiteral("#del"))) {
        // this is special command
        auto &ins = ScriptMachine::instance();
        auto mod = ins.module(ScriptMachine::Interactive);
        if (mod) {
            // first check whether contains \n
            auto idx = exec.indexOf('\n');
            if (idx >= 0) {
                setEditMode(Output);
                stdErrLine(tr("InvalidDelCmd"));
            } else {
                // ok, then tokens should be devided by the space
                exec.remove(0, 4);
                auto vars = exec.split(' ', Qt::SkipEmptyParts);

                QList<asUINT> indices;

                // then check
                setEditMode(Output);
                for (auto &v : vars) {
                    auto idx = mod->GetGlobalVarIndexByName(v.toUtf8());
                    if (idx >= 0) {
                        indices.append(idx);
                    } else {
                        stdWarnLine(tr("NotFoundIgnore:") + v);
                    }
                }

                std::sort(indices.begin(), indices.end(), std::greater<int>());

                // ok, remove
                for (auto &i : indices) {
                    mod->RemoveGlobalVar(i);
                }
            }
        }
        _codes.clear();
        appendCommandPrompt();
        setEditMode(Input);
        Q_EMIT consoleScriptRunFinished();
    } else if (exec == QStringLiteral("#cls")) {
        auto &ins = ScriptMachine::instance();
        auto mod = ins.module(ScriptMachine::Interactive);
        if (mod) {
            auto total = mod->GetGlobalVarCount();
            if (total) {
                asUINT i = total;
                do {
                    --i;
                    mod->RemoveGlobalVar(i);
                } while (i);
            }
        }
        _codes.clear();
        appendCommandPrompt();
        setEditMode(Input);
        Q_EMIT consoleScriptRunFinished();
    } else if (exec == QStringLiteral("#hiscls")) {
        history_.strings_.clear();
    } else {
        setEditMode(Output);
        if (_codes.isEmpty()) {
            _codes = exec;
        } else {
            _codes.append('\n').append(exec);
        }
        ScriptMachine::instance().executeCode(ScriptMachine::Interactive,
                                              _codes);
        _codes.clear();
        appendCommandPrompt();
        setEditMode(Input);
        Q_EMIT consoleScriptRunFinished();
    }
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
    } else if (e->modifiers() == Qt::NoModifier) {
        QConsoleWidget::keyPressEvent(e);
        auto key = e->key();
        if (key == Qt::Key_Comma) {
            auto &lsp = AngelLsp::instance();
            auto url = lspFileNameURL();
            auto tc = currentPosition();
            auto line = tc.blockNumber;
            auto character = tc.positionInBlock;

            while (isContentLspUpdated()) {
                // wait for a moment
            }

            auto r = lsp.requestSignatureHelp(url, line, character);
            auto sigs = r["signatures"].toArray();
            QList<WingSignatureTooltip::Signature> ss;
            for (auto &&sig : sigs) {
                QJsonValue js = sig;
                WingSignatureTooltip::Signature s;
                s.label = js["label"].toString();
                s.doc = js["documentation"].toString();
                ss.append(s);
            }
            showFunctionTip(ss);
        } else if (key == Qt::Key_Semicolon) {
            hideHelpTooltip();
        }
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

void ScriptingConsole::onCompletion(const QModelIndex &index) {
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
        auto r = snipt.process(selfdata.completion());

        static QRegularExpression regex(QStringLiteral("[\\r\\n]"));
        // replace with space
        r.expandedText.replace(regex, QStringLiteral(" "));

        tc.insertText(r.expandedText);
        auto roff = r.expandedText.size() - r.cursorOffset;
        tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, roff);
    } else {
        tc.insertText(comp);
    }

    setTextCursor(tc);
}

void ScriptingConsole::paste() {
    if (ScriptMachine::instance().isRunning(ScriptMachine::Interactive)) {
        return;
    }

    const QMimeData *const clipboard = QApplication::clipboard()->mimeData();
    const QString text = clipboard->text();
    if (!text.isEmpty()) {
        if (text.indexOf('\n') < 0) {
            if (isCursorInEditZone()) {
                auto cursor = this->textCursor();
                cursor.insertText(text);
            } else {
                replaceCommandLine(text);
            }
        } else {
            auto ret = WingMessageBox::question(
                nullptr, tr("MultiCodeCanNotUndo"), text);
            if (ret == QMessageBox::No) {
                return;
            }
            auto lines = text.split('\n');
            if (lines.isEmpty()) {
                return;
            }

            setEditMode(Output);
            auto pl = lines.begin();
            auto pend = std::prev(lines.end());
            write(*pl);
            pl++;
            for (; pl != pend; pl++) {
                appendCommandPrompt(true);
                write(*pl);
            }
            appendCommandPrompt(true);
            setEditMode(Input);
            replaceCommandLine(*pl);
            lines.removeLast();
            _codes = lines.join('\n');
        }
    }
}

bool ScriptingConsole::increaseVersion() {
    version++;
    if (version == 0) { // test overflow
        version = 1;
        return true;
    }
    return false;
}

void ScriptingConsole::sendDocChange() {
    auto &lsp = AngelLsp::instance();
    if (lsp.isActive()) {
        auto url = lspURL();
        auto txt = currentCodes();
        txt.prepend(QStringLiteral("void f(){\n"))
            .append(QStringLiteral("\n}"))
            .prepend(ScriptMachine::instance().getGlobalDecls());

        // test overflow
        if (increaseVersion()) {
            lsp.closeDocument(url);
            lsp.openDocument(url, 0, txt);
        } else {
            lsp.changeDocument(url, getVersion(), txt);
        }

        _ok = false;
        _timer->reset(300);
    }
}

QString ScriptingConsole::lspURL() {
    return QStringLiteral("dev://as_console");
}

void ScriptingConsole::setEditMode(ConsoleMode mode) {
    setMode(mode);
    if (AngelLsp::instance().isActive()) {
        if (mode == Input && !_isWaitingRead) {
            completer()->setEnabled(true);
            Q_EMIT textChanged();
        } else {
            completer()->setEnabled(false);
        }
    }
}

bool ScriptingConsole::isContentLspUpdated() const { return _ok; }

LspEditorInterace::CursorPos ScriptingConsole::currentPosition() const {
    auto cursor = textCursor();
    auto block = cursor.block();

    int prefixLen = 0;

    auto hl = consoleHighligher();
    if (hl) {
        prefixLen = hl->blockPrefixLength(block);
    }

    LspEditorInterace::CursorPos pos;
    pos.blockNumber = _codes.length() + 1;
    pos.positionInBlock = cursor.positionInBlock() - prefixLen;
    return pos;
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

const WingCodeEdit *ScriptingConsole::editorPtr() const { return this; }

QString ScriptingConsole::lspFileNameURL() const { return lspURL(); }

QString ScriptingConsole::currentCodes() const {
    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    textCursor.setPosition(inpos_, QTextCursor::KeepAnchor);
    if (_codes.isEmpty()) {
        return textCursor.selectedText();
    }
    return _codes + '\n' + textCursor.selectedText();
}

void ScriptingConsole::enableLSP() {
    if (!_isTerminal) {
        return;
    }

    auto &lsp = AngelLsp::instance();
    connect(&lsp, &AngelLsp::serverStarted, this, [this]() {
        completer()->setEnabled(true);
        auto &lsp = AngelLsp::instance();
        auto txt = currentCodes();
        txt.prepend(QStringLiteral("void f(){\n"))
            .append(QStringLiteral("\n}"));
        lsp.openDocument(lspFileNameURL(), 0, txt);
        version = 1;
    });
    connect(&lsp, &AngelLsp::serverExited, this,
            [this]() { completer()->setEnabled(false); });
    connect(
        &lsp, &AngelLsp::diagnosticsPublished, this,
        [this](const QString &url, const QList<LSP::Diagnostics> &diagnostics) {
            if (url == lspURL()) {
                auto lsps = [](LSP::DiagnosticSeverity s)
                    -> WingCodeEdit::SeverityLevel {
                    switch (s) {
                    case LSP::DiagnosticSeverity::None:
                        return WingCodeEdit::SeverityLevel::Information;
                    case LSP::DiagnosticSeverity::Error:
                        return WingCodeEdit::SeverityLevel::Error;
                    case LSP::DiagnosticSeverity::Warning:
                        return WingCodeEdit::SeverityLevel::Warning;
                    case LSP::DiagnosticSeverity::Information:
                        return WingCodeEdit::SeverityLevel::Information;
                    case LSP::DiagnosticSeverity::Hint:
                        return WingCodeEdit::SeverityLevel::Hint;
                    }
                    return WingCodeEdit::SeverityLevel::Information;
                };

                auto doc = document();
                auto block = doc->lastBlock();
                auto hl = this->consoleHighligher();
                auto prefix = hl->blockPrefixLength(block);

                clearSquiggle();
                auto offline = block.blockNumber();
                for (auto &d : diagnostics) {
                    auto t = _codes.count('\n') + 1;
                    if (d.range.start.line == t) {
                        addSquiggle(lsps(d.severity),
                                    {offline + d.range.start.line,
                                     prefix + d.range.start.character},
                                    {offline + d.range.end.line,
                                     prefix + d.range.end.character},
                                    d.message);
                    }
                }
                highlightAllSquiggle();
            }
        });

    lsp.openDocument(lspURL(), 0, {});
    connect(this, &ScriptingConsole::textChanged, this, [this]() {
        if (mode_ == Output || _isWaitingRead) {
            return;
        }
        if (!_ok) {
            _lastSent = false;
            return;
        }
        sendDocChange();
    });

    _timer = new ResettableTimer(this);
    connect(_timer, &ResettableTimer::timeoutTriggered, this,
            &ScriptingConsole::onSendFullTextChangeCompleted);

    completer()->setEnabled(lsp.isActive());
}

quint64 ScriptingConsole::getVersion() const { return version; }

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
            ICONRES(QStringLiteral("console")), tr("MutiConsole"),
            QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Enter),
            [this]() {
                if (ScriptMachine::instance().isRunning(
                        ScriptMachine::Interactive)) {
                    return;
                }
                replaceCommandLine({});
                handleReturnKey(Qt::ControlModifier | Qt::AltModifier);
            });
        a->setShortcutContext(Qt::WidgetShortcut);
        a->setEnabled(
            !ScriptMachine::instance().isRunning(ScriptMachine::Interactive));
        a = menu.addAction(
            ICONRES(QStringLiteral("dbgstop")), tr("AbortScript"),
            QKeySequence(Qt::ControlModifier | Qt::Key_Q), [this]() {
                ScriptMachine::instance().abortScript(
                    _isTerminal ? ScriptMachine::Interactive
                                : ScriptMachine::Background);
            });
        a->setShortcutContext(Qt::WidgetShortcut);
        a->setEnabled(ScriptMachine::instance().isRunning(
            _isTerminal ? ScriptMachine::Interactive
                        : ScriptMachine::Background));
    } else {
        a = menu.addAction(ICONRES(QStringLiteral("del")), tr("Clear"),
                           QKeySequence(Qt::ControlModifier | Qt::Key_L), this,
                           &ScriptingConsole::clear);
        a->setShortcutContext(Qt::WidgetShortcut);
    }

    menu.exec(event->globalPos());
}
