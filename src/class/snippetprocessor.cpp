/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** std::any WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "snippetprocessor.h"

#include "grammar/Snippet/SnippetBaseVisitor.h"
#include "grammar/Snippet/SnippetLexer.h"

#include <QRegularExpression>

class SnippetExpansionVisitor : public SnippetBaseVisitor {
public:
    SnippetExpansionVisitor(const SnippetProcessor::Resolver &resolver);

    virtual std::any visitSnippet(SnippetParser::SnippetContext *ctx) override;
    virtual std::any visitText(SnippetParser::TextContext *ctx) override;
    virtual std::any
    visitEscapedChar(SnippetParser::EscapedCharContext *ctx) override;
    virtual std::any
    visitVariable(SnippetParser::VariableContext *ctx) override;
    virtual std::any visitVariableWithDefault(
        SnippetParser::VariableWithDefaultContext *ctx) override;
    virtual std::any
    visitPlaceholder(SnippetParser::PlaceholderContext *ctx) override;
    virtual std::any visitTabstop(SnippetParser::TabstopContext *ctx) override;
    virtual std::any visitTabstopWithDefault(
        SnippetParser::TabstopWithDefaultContext *ctx) override;
    virtual std::any visitChoice(SnippetParser::ChoiceContext *ctx) override;

    QString getResult() const { return result_; }
    qsizetype getCursorOffset() const { return cursorOffset_; }

private:
    QString result_;
    qsizetype cursorOffset_;
    SnippetProcessor::Resolver resolver_;

    QString processEscapedChar(const QString &escaped);
    QString extractDefaultValue(const QString &content);
    QString extractChoiceFirstOption(const QString &choiceContent);
};

SnippetExpansionVisitor::SnippetExpansionVisitor(
    const SnippetProcessor::Resolver &resolver)
    : cursorOffset_(-1), resolver_(resolver) {
    Q_ASSERT(resolver);
}

std::any
SnippetExpansionVisitor::visitSnippet(SnippetParser::SnippetContext *ctx) {
    auto ret = visitChildren(ctx);
    if (cursorOffset_ < 0) {
        cursorOffset_ = result_.size();
    }
    return ret;
}

std::any SnippetExpansionVisitor::visitText(SnippetParser::TextContext *ctx) {
    result_.append(QString::fromStdString(ctx->TEXT_CONTENT()->getText()));
    return defaultResult();
}

std::any SnippetExpansionVisitor::visitEscapedChar(
    SnippetParser::EscapedCharContext *ctx) {
    auto escaped = QString::fromStdString(ctx->ESCAPED_CHAR()->getText());
    result_.append(processEscapedChar(escaped));
    return defaultResult();
}

std::any
SnippetExpansionVisitor::visitVariable(SnippetParser::VariableContext *ctx) {
    QString varText = QString::fromStdString(ctx->VARIABLE()->getText());
    varText.slice(1);
    result_.append(resolver_(varText));
    return defaultResult();
}

std::any SnippetExpansionVisitor::visitVariableWithDefault(
    SnippetParser::VariableWithDefaultContext *ctx) {
    std::string varText = ctx->VARIABLE_WITH_DEFAULT()->getText();

    size_t colonPos = varText.find(':');
    if (colonPos != std::string::npos) {
        auto varName = varText.substr(2, colonPos - 2);
        auto defaultValue =
            varText.substr(colonPos + 1, varText.length() - colonPos - 2);
        result_.append(defaultValue);
    }
    return nullptr;
}

std::any SnippetExpansionVisitor::visitPlaceholder(
    SnippetParser::PlaceholderContext *ctx) {
    if (cursorOffset_ < 0) {
        cursorOffset_ = result_.length();
    }
    return defaultResult();
}

std::any
SnippetExpansionVisitor::visitTabstop(SnippetParser::TabstopContext *ctx) {
    // do nothing...
    return defaultResult();
}

std::any SnippetExpansionVisitor::visitTabstopWithDefault(
    SnippetParser::TabstopWithDefaultContext *ctx) {
    std::string tabstopText = ctx->TABSTOP_WITH_DEFAULT()->getText();
    size_t colonPos = tabstopText.find(':');
    if (colonPos != std::string::npos) {
        auto defaultValue =
            QString::fromUtf8(tabstopText.data() + colonPos + 1,
                              tabstopText.length() - colonPos - 2);
        result_.append(defaultValue);
    }
    return defaultResult();
}

std::any
SnippetExpansionVisitor::visitChoice(SnippetParser::ChoiceContext *ctx) {
    auto choice = QString::fromStdString(ctx->CHOICE()->getText());
    result_.append(extractChoiceFirstOption(choice));
    return defaultResult();
}

QString SnippetExpansionVisitor::processEscapedChar(const QString &escaped) {
    if (escaped.length() < 2) {
        return escaped;
    }

    auto escapedChar = escaped[1].unicode();
    switch (escapedChar) {
    case '$':
        return QStringLiteral("$");
    case '{':
        return QStringLiteral("{");
    case '}':
        return QStringLiteral("}");
    case '[':
        return QStringLiteral("[");
    case ']':
        return QStringLiteral("]");
    case '\\':
        return QStringLiteral("\\");
    default:
        return QString(escapedChar);
    }
}

QString SnippetExpansionVisitor::extractDefaultValue(const QString &content) {
    auto colonPos = content.indexOf(':');
    if (colonPos < 0) {
        return {};
    }

    auto endPos = content.lastIndexOf('}');
    if (endPos < 0) {
        return content.sliced(colonPos + 1);
    }

    return content.sliced(colonPos + 1, endPos - colonPos - 1);
}

QString SnippetExpansionVisitor::extractChoiceFirstOption(
    const QString &choiceContent) {
    auto firstPipe = choiceContent.indexOf('|');
    auto lastPipe = choiceContent.lastIndexOf('|');

    if (firstPipe < 0 || lastPipe < 0 || firstPipe >= lastPipe) {
        return {};
    }

    auto optionsStr =
        choiceContent.sliced(firstPipe + 1, lastPipe - firstPipe - 1);

    auto commaPos = optionsStr.indexOf(',');
    if (commaPos < 0) {
        return optionsStr.first(commaPos);
    }

    return optionsStr;
}

SnippetProcessor::SnippetProcessor(const Resolver &resolver)
    : _resolver(resolver) {
    Q_ASSERT(resolver);
}

SnippetResult SnippetProcessor::process(const QString &snippet) {
    antlr4::ANTLRInputStream input(snippet.toStdString());
    SnippetLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    SnippetParser parser(&tokens);
    parser.removeErrorListeners();
    parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

    SnippetExpansionVisitor visitor(_resolver);
    try {
        visitor.visit(parser.snippet());
        return SnippetResult(visitor.getResult(), visitor.getCursorOffset());
    } catch (const std::exception &e) {
        return SnippetResult(snippet, snippet.length());
    }
}
