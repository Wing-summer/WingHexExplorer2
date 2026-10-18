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

#include <QScopedValueRollback>
#include <QSignalBlocker>
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
    setSyntax(syntaxRepo().definitionForName("Luau"));

    _s.setDevice(this->device());
    connect(this, &QPlainTextEdit::textChanged, this, [this]() {
        if (mode() == Input) {
            refreshInputBlocks();
        }
    });
}

void ScriptingConsoleBase::setTabWidth(int width) {
    QConsoleWidget::setTabWidth(width);
    consoleHighligher()->rehighlight();
}

void ScriptingConsoleBase::changeEvent(QEvent *event) {
    QConsoleWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange) {
        QMetaObject::invokeMethod(
            this, [this]() { setTabWidth(tabWidth()); }, Qt::QueuedConnection);
    }
}

void ScriptingConsoleBase::refreshInputBlocks() {
    if (_refreshingInput) {
        return;
    }
    QScopedValueRollback refreshing(_refreshingInput, true);
    const QSignalBlocker blocker(this);
    auto cursor = textCursor();
    auto block = document()->findBlock(inpos_);
    if (!block.isValid()) {
        return;
    }
    auto h = consoleHighligher();
    const auto prefixLength = inpos_ - block.position();
    h->setBlockAsCodeWithPrefix(block, prefixLength, {}, !_lastCommandPrompt);
    block = block.next();
    const auto prompt = this->commandPrompt(true);
    while (block.isValid()) {
        if (!h->hasContinuationPrompt(block)) {
            QTextCursor prefixCursor(block);
            prefixCursor.insertText(prompt);
        }
        h->setBlockAsCodeWithPrefix(block, prompt.length());
        block = block.next();
    }
    setTextCursor(cursor);
}

QString ScriptingConsoleBase::commandPrompt(bool storeOnly) {
    if (storeOnly) {
        return QStringLiteral(".... > ");
    } else {
        return QStringLiteral("luau > ");
    }
}

QString ScriptingConsoleBase::currentCommandLine() const {
    QStringList lines;
    auto block = document()->findBlock(inpos_);
    while (block.isValid()) {
        const auto start = inputStartForBlock(block) - block.position();
        lines.append(block.text().sliced(start));
        block = block.next();
    }
    return lines.join('\n');
}

int ScriptingConsoleBase::inputStartForBlock(const QTextBlock &block) const {
    return qMax(inpos_, block.position() +
                            consoleHighligher()->blockPrefixLength(block));
}

void ScriptingConsoleBase::stdOutLine(QString str,
                                      const WingEditorMetaInfo &info) {
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
        h->setBlockAsTextOnly(blk, info);
    }
}

void ScriptingConsoleBase::stdErrLine(QString str,
                                      const WingEditorMetaInfo &info) {
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
        h->setBlockAsTextWithFormat(blk, _errCharFmt, info);
    }
}

void ScriptingConsoleBase::stdWarnLine(QString str,
                                       const WingEditorMetaInfo &info) {
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
        h->setBlockAsTextWithFormat(blk, _warnCharFmt, info);
    }
}

void ScriptingConsoleBase::newLine() { _s << Qt::endl; }

void ScriptingConsoleBase::flush() { _s << Qt::flush; }

void ScriptingConsoleBase::initOutput() {
    stdWarnLine(tr("Scripting console for WingHexExplorer"));
    newLine();
    newLine();
    appendCommandPrompt();
}

void ScriptingConsoleBase::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;

    auto doc = this->document();
    auto blk = doc->lastBlock();
    auto h = consoleHighligher();
    if (blk.isValid()) {
        if (!h->isDefaultBlock(blk)) {
            if (!blk.text().isEmpty()) {
                newLine();
            }
        }
    }

    commandPrompt += this->commandPrompt(storeOnly);
    _lastCommandPrompt = storeOnly;
    auto id = write(commandPrompt);
    blk = doc->findBlockByNumber(id);
    h->setBlockAsCodeWithPrefix(blk, commandPrompt.length(), {}, !storeOnly);
}

bool ScriptingConsoleBase::lastCommandPrompt() const {
    return _lastCommandPrompt;
}

bool ScriptingConsoleBase::restorePreviousCommandPrompt(bool storeOnly) {
    auto block = document()->findBlock(inpos_).previous();
    auto h = consoleHighligher();
    const auto prefixLength = h->blockPrefixLength(block);
    if (!block.isValid() || prefixLength <= 0) {
        return false;
    }

    const auto inputStart = block.position() + prefixLength;
    const auto joinPosition = block.position() + block.length() - 1;
    setMode(Output);
    auto cursor = textCursor();
    cursor.setPosition(joinPosition);
    cursor.setPosition(inpos_, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    setMode(Input);
    inpos_ = inputStart;
    cursor.setPosition(joinPosition);
    setTextCursor(cursor);
    _lastCommandPrompt = storeOnly;
    h->setBlockAsCodeWithPrefix(block, prefixLength, {}, !storeOnly);
    return true;
}

WingConsoleHighligher *ScriptingConsoleBase::consoleHighligher() const {
    return reinterpret_cast<WingConsoleHighligher *>(highlighter());
}

QTextStream &ScriptingConsoleBase::consoleStream() { return _s; }
