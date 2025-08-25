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

#include "scriptingconsolebase.h"

#include <QTextBlock>
#include <QTextStream>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

ScriptingConsoleBase::ScriptingConsoleBase(QWidget *parent)
    : QConsoleWidget(parent) {
    _warnCharFmt.setForeground(QColorConstants::Svg::gold);
    _errCharFmt.setForeground(Qt::red);
    setHighlighter(new WingConsoleHighligher);
    setSyntax(syntaxRepo().definitionForName("AngelScript"));

    _s.setDevice(this->device());
}

void ScriptingConsoleBase::stdOutLine(const QString &str) {
    if (str.isEmpty()) {
        return;
    }

    auto olen = this->blockCount();
    auto id = write(str);
    auto nlen = this->blockCount();
    auto total = nlen - olen + 1;
    auto doc = this->document();
    auto h = consoleHighligher();
    for (int i = 0; i < total; ++i) {
        auto blk = doc->findBlockByNumber(id + i);
        h->setBlockAsTextOnly(blk);
    }
}

void ScriptingConsoleBase::stdErrLine(const QString &str) {
    if (str.isEmpty()) {
        return;
    }

    auto olen = this->blockCount();
    auto id = write(str);
    auto nlen = this->blockCount();
    auto total = nlen - olen + 1;
    auto doc = this->document();
    auto h = consoleHighligher();
    for (int i = 0; i < total; ++i) {
        auto blk = doc->findBlockByNumber(id + i);
        h->setBlockAsTextWithFormat(blk, _errCharFmt);
    }
}

void ScriptingConsoleBase::stdWarnLine(const QString &str) {
    if (str.isEmpty()) {
        return;
    }

    auto olen = this->blockCount();
    auto id = write(str);
    auto nlen = this->blockCount();
    auto total = nlen - olen + 1;
    auto doc = this->document();
    auto h = consoleHighligher();
    for (int i = 0; i < total; ++i) {
        auto blk = doc->findBlockByNumber(id + i);
        h->setBlockAsTextWithFormat(blk, _warnCharFmt);
    }
}

void ScriptingConsoleBase::newLine() { _s << Qt::endl; }

void ScriptingConsoleBase::flush() { _s << Qt::flush; }

void ScriptingConsoleBase::initOutput() {
    stdWarnLine(tr("Scripting console for WingHexExplorer"));
    newLine();
    stdWarnLine(tr(">>>> Powered by AngelScript <<<<"));
    newLine();
    newLine();
    appendCommandPrompt();
}

void ScriptingConsoleBase::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;

    auto cursor = this->textCursor();
    if (!cursor.atBlockStart()) {
        cursor.insertBlock();
    }

    if (storeOnly) {
        commandPrompt += QStringLiteral("... > ");
    } else {
        commandPrompt += QStringLiteral("as > ");
    }

    _lastCommandPrompt = storeOnly;
    auto id = write(commandPrompt);
    auto blk = document()->findBlockByNumber(id);
    auto h = consoleHighligher();
    h->setBlockAsCodeWithPrefix(blk, commandPrompt.length());
}

bool ScriptingConsoleBase::lastCommandPrompt() const {
    return _lastCommandPrompt;
}

WingConsoleHighligher *ScriptingConsoleBase::consoleHighligher() const {
    return reinterpret_cast<WingConsoleHighligher *>(highlighter());
}

QTextStream &ScriptingConsoleBase::consoleStream() { return _s; }
