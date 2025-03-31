#include "scriptingconsolebase.h"
#include "class/wingconsolehighligher.h"
#include "wingsyntaxhighlighter.h"

#include <QTextBlock>
#include <QTextStream>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

ScriptingConsoleBase::ScriptingConsoleBase(QWidget *parent)
    : QConsoleWidget(parent) {
    _warnCharFmt.setForeground(QColorConstants::Svg::gold);
    setHighlighter(new WingConsoleHighligher);
    setSyntax(syntaxRepo().definitionForName("AngelScript"));

    _s.setDevice(this->device());
}

void ScriptingConsoleBase::stdOut(const QString &str) {
    writeStdOut(str);
    dontHighlightLastLine(true);
}

void ScriptingConsoleBase::stdErr(const QString &str) {
    writeStdErr(str);
    dontHighlightLastLine(false);
}

void ScriptingConsoleBase::stdWarn(const QString &str) {
    write(str, _warnCharFmt);
    dontHighlightLastLine(false);
}

void ScriptingConsoleBase::newLine() { _s << Qt::endl; }

void ScriptingConsoleBase::flush() { _s << Qt::flush; }

void ScriptingConsoleBase::initOutput() {
    stdWarn(tr("Scripting console for WingHexExplorer"));
    _s << Qt::endl;
    stdWarn(tr(">>>> Powered by AngelScript <<<<"));
    _s << Qt::endl << Qt::endl;
    appendCommandPrompt();
}

void ScriptingConsoleBase::appendCommandPrompt(bool storeOnly) {
    QString commandPrompt;

    auto cursor = this->textCursor();
    if (!cursor.atBlockStart()) {
        newLine();
    }

    if (storeOnly) {
        commandPrompt += QStringLiteral("... > ");
    } else {
        commandPrompt += QStringLiteral("as > ");
    }

    _lastCommandPrompt = storeOnly;

    writeStdOut(commandPrompt);
    dontHighlightLastOffset(commandPrompt.length());
}

void ScriptingConsoleBase::dontHighlightLastLine(bool followTheme) {
    dontHighlightLastOffset(-1, followTheme);
}

void ScriptingConsoleBase::dontHighlightLastOffset(int offset,
                                                   bool followTheme) {
    auto blk = document()->lastBlock();
    auto hl = highlighter();
    if (offset < 0) {
        offset = -1;
    }
    if (offset == -1 && followTheme) {
        hl->setProperty(blk, "cmdoff", -2);
    } else {
        hl->setProperty(blk, "cmdoff", offset);
    }
    hl->rehighlightBlock(blk);
}

bool ScriptingConsoleBase::lastCommandPrompt() const {
    return _lastCommandPrompt;
}

QTextStream &ScriptingConsoleBase::consoleStream() { return _s; }
