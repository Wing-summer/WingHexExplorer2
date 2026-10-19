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

#include "luauindentation.h"

#include <Luau/Lexer.h>
#include <Luau/Parser.h>
#include <QSet>
#include <QVector>

namespace {
using Token = Luau::Lexeme;

quint64 positionKey(const Luau::Position &position) {
    return (quint64(position.line) << 32) | position.column;
}

// The recovering parser distinguishes statement `if` from Luau's expression
// `if`, including nested expressions and multiline conditions.
class StatementConditions : public Luau::AstVisitor {
public:
    QSet<quint64> positions;

    bool visit(Luau::AstStatIf *node) override {
        positions.insert(positionKey(node->location.begin));
        if (node->thenLocation) {
            positions.insert(positionKey(node->thenLocation->begin));
        }
        if (node->elseLocation) {
            positions.insert(positionKey(node->elseLocation->begin));
        }
        return true;
    }
};

int leadingColumns(const QString &line, int tabWidth) {
    int columns = 0;
    for (const auto ch : line) {
        if (ch == QLatin1Char(' ')) {
            ++columns;
        } else if (ch == QLatin1Char('\t')) {
            columns += tabWidth - columns % tabWidth;
        } else {
            break;
        }
    }
    return columns;
}

QString whitespace(int columns, const WingCodeEdit &editor) {
    if (editor.indentationMode() ==
        WingCodeEdit::IndentationMode::IndentSpaces) {
        return QString(columns, QLatin1Char(' '));
    }
    const int width = qMax(1, editor.tabWidth());
    return QString(columns / width, QLatin1Char('\t')) +
           QString(columns % width, QLatin1Char(' '));
}
} // namespace

static QString
computeIndentation(const QString &source, const WingCodeEdit &editor,
                   std::optional<QString> *lineIndent = nullptr) {
    const auto lines = source.split(QLatin1Char('\n'));
    const int tabWidth = qMax(1, editor.tabWidth());
    const int step =
        editor.indentationMode() == WingCodeEdit::IndentationMode::IndentTabs
            ? tabWidth
            : qMax(1, editor.indentWidth());
    const int baseline = leadingColumns(lines.last(), tabWidth);
    struct Frame {
        int close;
        int indent;
        int kind;
    };
    QVector<Frame> stack;
    const auto bytes = source.toUtf8();
    Luau::Allocator allocator;
    Luau::AstNameTable names(allocator);
    const auto parsed = Luau::Parser::parse(
        bytes.constData(), size_t(bytes.size()), names, allocator);
    StatementConditions conditions;
    if (parsed.root) {
        parsed.root->visit(&conditions);
    }
    Luau::Lexer lexer(bytes.constData(), size_t(bytes.size()), names);
    lexer.setSkipComments(true);
    int target = baseline;
    int interpolationDepth = 0;
    for (;;) {
        const auto token = lexer.next();
        const int type = token.type;
        if (type == Token::Eof) {
            break;
        }
        if (type == Token::InterpStringBegin) {
            ++interpolationDepth;
        }
        if (interpolationDepth > 0) {
            if (type == Token::InterpStringEnd) {
                --interpolationDepth;
            }
            continue;
        }
        if (type == Token::BrokenString || type == Token::BrokenComment ||
            type == Token::BrokenInterpDoubleBrace || type == Token::Error) {
            // An unfinished literal is not a block opener.
            return whitespace(baseline, editor);
        }
        const auto line = token.location.begin.line;
        const int indent = leadingColumns(lines.at(int(line)), tabWidth);
        const bool currentLine = int(line) == lines.size() - 1;
        const bool leadingToken =
            currentLine && lines.last()
                               .left(int(token.location.begin.column))
                               .trimmed()
                               .isEmpty();
        int close = 0;
        if (type == '(') {
            close = ')';
        } else if (type == '[') {
            close = ']';
        } else if (type == '{') {
            close = '}';
        } else if (type == Token::ReservedFunction ||
                   type == Token::ReservedDo) {
            close = Token::ReservedEnd;
        } else if (type == Token::ReservedRepeat) {
            close = Token::ReservedUntil;
        } else if (type == Token::ReservedIf &&
                   conditions.positions.contains(
                       positionKey(token.location.begin))) {
            // Luau's expression-form `if` has no terminating `end`.
            close = Token::ReservedEnd;
        }
        if (close) {
            stack.append({close, indent, type});
            if (currentLine) {
                target = indent + step;
            }
        } else if (type == ')' || type == ']' || type == '}' ||
                   type == Token::ReservedEnd || type == Token::ReservedUntil) {
            if (!stack.isEmpty() && stack.last().close == type) {
                const auto frame = stack.takeLast();
                if (lineIndent && leadingToken) {
                    *lineIndent = whitespace(frame.indent, editor);
                }
                if (currentLine) {
                    target = stack.isEmpty() ? frame.indent
                                             : stack.last().indent + step;
                }
            }
        } else if ((type == Token::ReservedElse ||
                    type == Token::ReservedElseif ||
                    type == Token::ReservedThen) &&
                   !stack.isEmpty() && stack.last().kind == Token::ReservedIf &&
                   conditions.positions.contains(
                       positionKey(token.location.begin))) {
            if (currentLine) {
                target = stack.last().indent + step;
            }
            if (lineIndent && leadingToken) {
                *lineIndent = whitespace(stack.last().indent, editor);
            }
        }
    }
    // Existing indentation remains authoritative on ordinary/blank lines.
    return whitespace(interpolationDepth > 0 ? baseline : qMax(0, target),
                      editor);
}

QString LuauIndentation::forNewLine(const QString &source,
                                    const WingCodeEdit &editor) {
    return computeIndentation(source, editor);
}

static bool isClosingLine(const QString &text) {
    const auto line = text.trimmed();
    int length = 0;
    while (length < line.size() && (line.at(length).isLetterOrNumber() ||
                                    line.at(length) == QLatin1Char('_'))) {
        ++length;
    }
    const auto word = line.left(length);
    if (word != QLatin1String("end") && word != QLatin1String("else") &&
        word != QLatin1String("elseif") && word != QLatin1String("until") &&
        !line.startsWith(QLatin1Char('}')) &&
        !line.startsWith(QLatin1Char(')')) &&
        !line.startsWith(QLatin1Char(']'))) {
        return false;
    }
    return true;
}

std::optional<QString>
LuauIndentation::forCurrentLine(const QString &source,
                                const WingCodeEdit &editor) {
    if (!isClosingLine(source.section(QLatin1Char('\n'), -1))) {
        return std::nullopt;
    }
    std::optional<QString> indent;
    computeIndentation(source, editor, &indent);
    return indent;
}

std::optional<QString>
LuauIndentation::forEditor(const WingCodeEdit::IndentationContext &context) {
    auto cursor = context.cursor;
    const bool currentLine =
        context.trigger == WingCodeEdit::IndentationTrigger::TextInput;
    if (currentLine) {
        if (!isClosingLine(cursor.block().text().mid(context.codeStart))) {
            return std::nullopt;
        }
        cursor.movePosition(QTextCursor::EndOfBlock);
    }
    cursor.setPosition(0, QTextCursor::KeepAnchor);
    auto source = cursor.selectedText();
    source.replace(QChar::ParagraphSeparator, QChar::LineFeed);
    return currentLine ? forCurrentLine(source, context.editor)
                       : forNewLine(source, context.editor);
}
