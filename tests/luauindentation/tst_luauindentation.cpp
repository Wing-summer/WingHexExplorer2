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

#include "QConsoleWidget/QConsoleWidget.h"
#include "control/editor/luauindentation.h"

#include <QtTest>

namespace {
void configure(WingCodeEdit &editor) {
    editor.setAutoIndent(true);
    editor.setAutoCloseChar(false);
    editor.setTabWidth(4);
    editor.setIndentWidth(4);
    editor.setIndentationMode(WingCodeEdit::IndentationMode::IndentSpaces);
    editor.setIndentationStrategy(LuauIndentation::forEditor);
}

void prepare(WingCodeEdit &editor, const QString &source) {
    editor.setPlainText(source);
    editor.moveCursor(QTextCursor::End);
}

class ConsoleHarness : public QConsoleWidget {
public:
    QString observedSource;
    int calls = 0;

    void prepareInput(const QString &previous, const QString &input) {
        setMode(Output);
        setPlainText(QStringLiteral("old output: for end\n... "));
        moveCursor(QTextCursor::End);
        setMode(Input);
        replaceCommandLine(input);
        configure(*this);
        // Only the host source adapter is a fixture; widget and policy are
        // real.
        setIndentationStrategy([this, previous](const auto &context)
                                   -> std::optional<QString> {
            ++calls;
            auto cursor = context.cursor;
            if (context.trigger == IndentationTrigger::TextInput) {
                cursor.movePosition(QTextCursor::EndOfBlock);
            }
            cursor.setPosition(inpos_, QTextCursor::KeepAnchor);
            observedSource = cursor.selectedText();
            observedSource.replace(QChar::ParagraphSeparator, QChar::LineFeed);
            observedSource.prepend(previous + QLatin1Char('\n'));
            return context.trigger == IndentationTrigger::TextInput
                       ? LuauIndentation::forCurrentLine(observedSource, *this)
                       : LuauIndentation::forNewLine(observedSource, *this);
        });
    }
};
} // namespace

class LuauIndentationTests : public QObject {
    Q_OBJECT

private slots:
    void newLine_data();
    void newLine();
    void indentationModes_data();
    void indentationModes();
    void currentLine_data();
    void currentLine();
    void typedClosing_data();
    void typedClosing();
    void newlineSelection();
    void identifierSuffix();
    void completion();
    void controls();
    void policyFallback();
    void console();
};

void LuauIndentationTests::newLine_data() {
    QTest::addColumn<QString>("source");
    QTest::addColumn<QString>("expected");
    const struct {
        const char *name;
        const char *source;
        int columns;
    } cases[] = {
        {"empty", "", 0},
        {"for", "for i = 1, 10 do", 4},
        {"while", "while ready do", 4},
        {"do", "do", 4},
        {"function", "local function f()", 4},
        {"anonymous-function", "local f = function(a, b)", 4},
        {"if", "if ready then", 4},
        {"multiline-condition", "if ready\nthen", 4},
        {"else", "if ready then\n    work()\nelse", 4},
        {"elseif", "if ready then\n    work()\nelseif other then", 4},
        {"repeat", "repeat", 4},
        {"until", "repeat\n    work()\nuntil ready", 0},
        {"end", "for i = 1, 2 do\n    work()\nend", 0},
        {"nested-end", "if a then\n    if b then\n        work()\n    end", 4},
        {"balanced-inline", "if ready then work() end", 0},
        {"table", "local t = {", 4},
        {"table-end", "local t = {\n    value = 1,\n}", 0},
        {"call", "call(", 4},
        {"function-parameters", "function f(\n    a, b\n)", 4},
        {"ordinary-line", "if ready then\n    work()", 4},
        {"blank-line", "if ready then\n    ", 4},
        {"comment", "-- if ready then", 0},
        {"trailing-comment", "if ready then -- end", 4},
        {"block-comment", "--[=[\nif ready then\n]=]", 0},
        {"quoted-string", "local s = 'then do function repeat {'", 0},
        {"long-string", "local s = [=[\nif ready then\n]=]", 0},
        {"unfinished-string", "    local s = [=[\n    if ready then", 4},
        {"expression-if", "local x = if ready then 1 else 2", 0},
        {"nested-expression-if",
         "local x = if a then if b then c else d else e", 0},
        {"multiline-expression-if", "local x =\n    if ready then 1 else 2", 4},
        {"expression-in-block", "if a then\n    local x = if b then 1 else 2",
         4},
        {"interpolated-string", "local s = `literal then {if a then b else c}`",
         0},
        {"unfinished-interpolation", "    local s = `unfinished {", 4},
        {"unicode-string", "local s = '中文😀'", 0},
    };
    for (const auto &test : cases) {
        QTest::newRow(test.name) << QString::fromUtf8(test.source)
                                 << QString(test.columns, QLatin1Char(' '));
    }
}

void LuauIndentationTests::newLine() {
    QFETCH(QString, source);
    QFETCH(QString, expected);
    WingCodeEdit editor;
    configure(editor);
    QCOMPARE(LuauIndentation::forNewLine(source, editor), expected);
    prepare(editor, source);
    QTest::keyClick(&editor, Qt::Key_Return);
    // Whitespace-only previous lines are intentionally cleaned by the editor.
    auto prefix = source;
    const auto last = source.section(QLatin1Char('\n'), -1);
    if (!expected.isEmpty() && last == expected) {
        prefix.chop(last.size());
    }
    QCOMPARE(editor.toPlainText(), prefix + QLatin1Char('\n') + expected);
    editor.undo();
    QCOMPARE(editor.toPlainText(), source);
    editor.redo();
    QCOMPARE(editor.toPlainText(), prefix + QLatin1Char('\n') + expected);
}

void LuauIndentationTests::indentationModes_data() {
    QTest::addColumn<int>("mode");
    QTest::addColumn<int>("indentWidth");
    QTest::addColumn<int>("tabWidth");
    QTest::addColumn<QString>("expected");
    QTest::newRow("spaces-two") << 0 << 2 << 4 << QStringLiteral("  ");
    QTest::newRow("tabs-eight") << 1 << 2 << 8 << QStringLiteral("\t");
    QTest::newRow("mixed-two") << 2 << 2 << 4 << QStringLiteral("  ");
    QTest::newRow("mixed-six") << 2 << 6 << 4 << QStringLiteral("\t  ");
}

void LuauIndentationTests::indentationModes() {
    QFETCH(int, mode);
    QFETCH(int, indentWidth);
    QFETCH(int, tabWidth);
    QFETCH(QString, expected);
    WingCodeEdit editor;
    configure(editor);
    editor.setIndentationMode(WingCodeEdit::IndentationMode(mode));
    editor.setIndentWidth(indentWidth);
    editor.setTabWidth(tabWidth);
    QCOMPARE(LuauIndentation::forNewLine(QStringLiteral("if a then"), editor),
             expected);
    prepare(editor, QStringLiteral("if a then"));
    QTest::keyClick(&editor, Qt::Key_Return);
    QCOMPARE(editor.toPlainText(), QStringLiteral("if a then\n") + expected);
    QTest::keyClicks(&editor, QStringLiteral("end"));
    QCOMPARE(editor.toPlainText(), QStringLiteral("if a then\nend"));
}

void LuauIndentationTests::currentLine_data() {
    QTest::addColumn<QString>("source");
    QTest::addColumn<bool>("handled");
    QTest::addColumn<QString>("expected");
    const struct {
        const char *name;
        const char *source;
        int columns; // -1 means the policy must decline the request.
    } cases[] = {
        {"for-end", "for i = 1, 2 do\n    end", 0},
        {"nested-end", "for i = 1, 2 do\n    for j = 1, 2 do\n        end", 4},
        {"else", "if a then\n    else", 0},
        {"elseif", "if a then\n    elseif b then", 0},
        {"until", "repeat\n    until ready", 0},
        {"brace", "local t = {\n    }", 0},
        {"parenthesis", "call(\n    )", 0},
        {"bracket", "local x = t[\n    ]", 0},
        {"comment", "for i = 1, 2 do\n    -- end", -1},
        {"long-string", "local s = [=[\n    end", -1},
        {"long-comment", "--[=[\n    end", -1},
        {"identifier", "for i = 1, 2 do\n    endless", -1},
        {"identifier-underscore", "for i = 1, 2 do\n    end_value", -1},
        {"unmatched-end", "    end", -1},
        {"expression-else", "local x = if a then 1\n    else 2", -1},
        {"ordinary-line", "    work()", -1},
    };
    for (const auto &test : cases) {
        QTest::newRow(test.name)
            << QString::fromUtf8(test.source) << (test.columns >= 0)
            << QString(qMax(0, test.columns), QLatin1Char(' '));
    }
}

void LuauIndentationTests::currentLine() {
    QFETCH(QString, source);
    QFETCH(bool, handled);
    QFETCH(QString, expected);
    WingCodeEdit editor;
    configure(editor);
    const auto result = LuauIndentation::forCurrentLine(source, editor);
    QCOMPARE(result.has_value(), handled);
    if (handled) {
        QCOMPARE(*result, expected);
    }
}

void LuauIndentationTests::typedClosing_data() {
    QTest::addColumn<QString>("before");
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("after");
    const struct {
        const char *name;
        const char *before;
        const char *key;
        const char *after;
    } cases[] = {
        {"end", "for i = 1, 2 do\n    en", "d", "for i = 1, 2 do\nend"},
        {"nested-end", "if a then\n    for i = 1, 2 do\n        en", "d",
         "if a then\n    for i = 1, 2 do\n    end"},
        {"else", "if a then\n    els", "e", "if a then\nelse"},
        {"elseif", "if a then\n    elsei", "f", "if a then\nelseif"},
        {"until", "repeat\n    unti", "l", "repeat\nuntil"},
        {"brace", "local t = {\n    ", "}", "local t = {\n}"},
        {"string", "local s = [[\n    en", "d", "local s = [[\n    end"},
        {"comment", "for i = 1, 2 do\n    -- en", "d",
         "for i = 1, 2 do\n    -- end"},
    };
    for (const auto &test : cases) {
        QTest::newRow(test.name)
            << QString::fromUtf8(test.before) << QString::fromUtf8(test.key)
            << QString::fromUtf8(test.after);
    }
}

void LuauIndentationTests::typedClosing() {
    QFETCH(QString, before);
    QFETCH(QString, key);
    QFETCH(QString, after);
    WingCodeEdit editor;
    configure(editor);
    prepare(editor, before);
    QTest::keyClicks(&editor, key);
    QCOMPARE(editor.toPlainText(), after);
    QCOMPARE(editor.textCursor().position(), after.size());
    editor.undo();
    QCOMPARE(editor.toPlainText(), before);
    editor.redo();
    QCOMPARE(editor.toPlainText(), after);
}

void LuauIndentationTests::newlineSelection() {
    WingCodeEdit editor;
    configure(editor);
    const auto source = QStringLiteral("    if ready thenREPLACEtail");
    prepare(editor, source);
    auto cursor = editor.textCursor();
    cursor.setPosition(source.indexOf(QStringLiteral("REPLACE")));
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 7);
    editor.setTextCursor(cursor);
    QTest::keyClick(&editor, Qt::Key_Return);
    QCOMPARE(editor.toPlainText(),
             QStringLiteral("    if ready then\n        tail"));
    editor.undo();
    QCOMPARE(editor.toPlainText(), source);
}

void LuauIndentationTests::identifierSuffix() {
    WingCodeEdit editor;
    configure(editor);
    const auto before = QStringLiteral("for i = 1, 2 do\n    enless");
    prepare(editor, before);
    auto cursor = editor.textCursor();
    cursor.setPosition(before.size() - 4);
    editor.setTextCursor(cursor);
    QTest::keyClicks(&editor, QStringLiteral("d"));
    QCOMPARE(editor.toPlainText(),
             QStringLiteral("for i = 1, 2 do\n    endless"));
}

void LuauIndentationTests::completion() {
    WingCodeEdit editor;
    configure(editor);
    const auto before = QStringLiteral("for i = 1, 2 do\n    en");
    prepare(editor, before);
    auto cursor = editor.textCursor();
    cursor.beginEditBlock();
    cursor.insertText(QStringLiteral("d"));
    editor.setTextCursor(cursor);
    editor.reindentCurrentLine();
    cursor.endEditBlock();
    QCOMPARE(editor.toPlainText(), QStringLiteral("for i = 1, 2 do\nend"));
    editor.undo();
    QCOMPARE(editor.toPlainText(), before);
}

void LuauIndentationTests::controls() {
    WingCodeEdit editor;
    configure(editor);
    const auto source = QStringLiteral("for i = 1, 2 do\n    en");
    editor.setAutoIndent(false);
    prepare(editor, source);
    QTest::keyClicks(&editor, QStringLiteral("d"));
    QCOMPARE(editor.toPlainText(), source + QLatin1Char('d'));
    QTest::keyClick(&editor, Qt::Key_Return);
    QCOMPARE(editor.toPlainText(), source + QStringLiteral("d\n"));
    editor.setAutoIndent(true);
    editor.setReadOnly(true);
    prepare(editor, source);
    QTest::keyClicks(&editor, QStringLiteral("d"));
    QCOMPARE(editor.toPlainText(), source);
    prepare(editor, source + QLatin1Char('d'));
    editor.reindentCurrentLine();
    QCOMPARE(editor.toPlainText(), source + QLatin1Char('d'));
}

void LuauIndentationTests::policyFallback() {
    WingCodeEdit editor;
    configure(editor);
    const auto source = QStringLiteral("    if a then");
    editor.setIndentationStrategy({});
    prepare(editor, source);
    QTest::keyClick(&editor, Qt::Key_Return);
    QCOMPARE(editor.toPlainText(), source + QStringLiteral("\n    "));
    editor.setIndentationStrategy(
        [](const auto &) -> std::optional<QString> { return std::nullopt; });
    prepare(editor, source);
    QTest::keyClick(&editor, Qt::Key_Return);
    QCOMPARE(editor.toPlainText(), source + QStringLiteral("\n    "));
    editor.setIndentationStrategy(
        [](const auto &) -> std::optional<QString> { return QString(); });
    prepare(editor, source);
    QTest::keyClick(&editor, Qt::Key_Return);
    QCOMPARE(editor.toPlainText(), source + QLatin1Char('\n'));
}

void LuauIndentationTests::console() {
    ConsoleHarness console;
    const auto previous = QStringLiteral("for i = 1, 2 do");
    console.prepareInput(previous, QStringLiteral("    en"));
    QVERIFY(!console.isUndoRedoEnabled());
    QTest::keyClicks(&console, QStringLiteral("d"));
    QCOMPARE(console.toPlainText(),
             QStringLiteral("old output: for end\n... end"));
    QCOMPARE(console.observedSource, previous + QStringLiteral("\n    end"));
    const auto calls = console.calls;
    auto cursor = console.textCursor();
    cursor.setPosition(0);
    console.setTextCursor(cursor);
    console.reindentCurrentLine();
    QCOMPARE(console.calls, calls);
    QCOMPARE(console.toPlainText(),
             QStringLiteral("old output: for end\n... end"));
}

QTEST_MAIN(LuauIndentationTests)
#include "tst_luauindentation.moc"
