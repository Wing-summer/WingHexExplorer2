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

#include "consolecodeedit.h"
#include "class/angellsp.h"
#include "class/asconsolecompletion.h"
#include "class/scriptmachine.h"

#include <QJsonArray>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

ConsoleCodeEdit::ConsoleCodeEdit(QWidget *parent) : CodeEdit(parent) {
    setSyntax(CodeEdit::syntaxRepo().definitionForName("AngelScript"));
    auto cm = new AsConsoleCompletion(this);
    cm->setParent(this);
    cm->setEnabled(AngelLsp::instance().isActive());

    auto &lsp = AngelLsp::instance();
    connect(&lsp, &AngelLsp::serverStarted, this, [this]() {
        completer()->setEnabled(true);
        auto &lsp = AngelLsp::instance();
        lsp.openDocument(lspFileNameURL(), 0, toPlainText());
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

                clearSquiggle();
                for (auto &d : diagnostics) {
                    addSquiggle(
                        lsps(d.severity),
                        {d.range.start.line + 1, d.range.start.character},
                        {d.range.end.line + 1, d.range.end.character},
                        d.message);
                }
                highlightAllSquiggle();
            }
        });

    lsp.openDocument(lspURL(), 0, {});
    connect(this, &ConsoleCodeEdit::textChanged, this, [this]() {
        if (!_ok) {
            _lastSent = false;
            return;
        }
        sendDocChange();
    });

    _timer = new ResettableTimer(this);
    connect(_timer, &ResettableTimer::timeoutTriggered, this,
            &ConsoleCodeEdit::onSendFullTextChangeCompleted);
}

ConsoleCodeEdit::~ConsoleCodeEdit() {
    AngelLsp::instance().closeDocument(lspURL());
}

const WingCodeEdit *ConsoleCodeEdit::editorPtr() const { return this; }

QString ConsoleCodeEdit::lspFileNameURL() const { return lspURL(); }

bool ConsoleCodeEdit::isContentLspUpdated() const { return _ok; }

void ConsoleCodeEdit::sendDocChange() {
    auto &lsp = AngelLsp::instance();
    auto url = lspURL();
    auto txt = toPlainText();
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

void ConsoleCodeEdit::showFunctionTip(
    const QList<WingSignatureTooltip::Signature> &sigs) {
    showHelpTooltip(sigs);
}

void ConsoleCodeEdit::clearFunctionTip() { hideHelpTooltip(); }

void ConsoleCodeEdit::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Comma) {
        auto &lsp = AngelLsp::instance();
        auto url = lspFileNameURL();
        auto tc = currentPosition();
        auto line = tc.blockNumber;
        auto character = tc.positionInBlock;

        sendDocChange();
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
    }
    CodeEdit::keyPressEvent(event);
}

LspEditorInterace::CursorPos ConsoleCodeEdit::currentPosition() const {
    auto cursor = textCursor();
    LspEditorInterace::CursorPos pos;
    pos.blockNumber = cursor.blockNumber() + 1;
    pos.positionInBlock = cursor.positionInBlock();
    return pos;
}

void ConsoleCodeEdit::keyReleaseEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::ControlModifier &&
        (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
        Q_EMIT onCloseEvent();
    }
}

void ConsoleCodeEdit::onSendFullTextChangeCompleted() {
    if (!_lastSent) {
        sendDocChange();
        _lastSent = true;
    }
    _ok = true;
}

QString ConsoleCodeEdit::lspURL() {
    return QStringLiteral("dev://as_console_mt");
}

quint64 ConsoleCodeEdit::getVersion() const { return version; }

bool ConsoleCodeEdit::increaseVersion() {
    version++;
    if (version == 0) { // test overflow
        version = 1;
        return true;
    }
    return false;
}
