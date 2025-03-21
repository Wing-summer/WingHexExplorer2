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
    dontHighlightLastLine();
}

void ScriptingConsoleBase::stdErr(const QString &str) {
    writeStdErr(str);
    dontHighlightLastLine();
}

void ScriptingConsoleBase::stdWarn(const QString &str) {
    write(str, _warnCharFmt);
    dontHighlightLastLine();
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

void ScriptingConsoleBase::dontHighlightLastLine() {
    dontHighlightLastOffset(-1);
}

void ScriptingConsoleBase::dontHighlightLastOffset(int offset) {
    auto blk = document()->lastBlock();
    auto hl = highlighter();
    hl->setProperty(blk, "cmdoff", offset);
    hl->rehighlightBlock(blk);
}

bool ScriptingConsoleBase::lastCommandPrompt() const {
    return _lastCommandPrompt;
}

QTextStream &ScriptingConsoleBase::consoleStream() { return _s; }
