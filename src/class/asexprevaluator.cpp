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

#include "asexprevaluator.h"
#include "utilities.h"

#include <fmt/format.h>

void reportError(const std::string &msg) { throw AsExprException(msg); }

void reportError(const antlr4::Token *tok, const std::string &msg) {
    if (!tok) {
        reportError(msg);
    }
    throw AsExprException(tok->getLine(), tok->getCharPositionInLine() + 1,
                          msg);
}
bool isStringValue(const AsExprValue &v) {
    return std::holds_alternative<QString>(v);
}

bool isDoubleValue(const AsExprValue &v) {
    return std::holds_alternative<double>(v);
}

bool isIntValue(const AsExprValue &v) {
    return std::holds_alternative<qint64>(v);
}

bool isBoolValue(const AsExprValue &v) {
    return std::holds_alternative<bool>(v);
}

QString toQStringValue(const AsExprValue &v) { return std::get<QString>(v); }

qint64 toIntValue(const AsExprValue &v, const antlr4::Token *tok) {
    if (auto p = std::get_if<qint64>(&v))
        return *p;
    if (auto p = std::get_if<bool>(&v))
        return *p ? 1 : 0;
    reportError(tok, "expected integer value");
    return 0;
}

double toDoubleValue(const AsExprValue &v, const antlr4::Token *tok) {
    if (auto p = std::get_if<double>(&v))
        return *p;
    if (auto p = std::get_if<qint64>(&v))
        return static_cast<double>(*p);
    if (auto p = std::get_if<bool>(&v))
        return *p ? 1.0 : 0.0;
    reportError(tok, "expected numeric value");
    return 0;
}

bool truthy(const AsExprValue &v, const antlr4::Token *tok) {
    if (auto p = std::get_if<bool>(&v))
        return *p;
    if (auto p = std::get_if<qint64>(&v))
        return *p != 0;
    if (auto p = std::get_if<double>(&v))
        return *p != 0.0;
    reportError(tok, "string cannot be used as a boolean");
    return false;
}

std::string binOpText(antlr4::ParserRuleContext *ctx, int index) {
    // children: lhs, op, rhs, op, rhs...
    return ctx->children.at(index * 2 - 1)->getText();
}

bool eqCompare(const AsExprValue &a, const AsExprValue &b,
               const std::string &op, const antlr4::Token *tok) {
    if (isStringValue(a) || isStringValue(b)) {
        if (!isStringValue(a) || !isStringValue(b)) {
            reportError(tok, "string can only be compared with string");
        }
        const QString sa = toQStringValue(a);
        const QString sb = toQStringValue(b);
        if (op == "==")
            return sa == sb;
        if (op == "!=")
            return sa != sb;
        reportError(tok, "invalid string equality operator");
    }

    if (isDoubleValue(a) || isDoubleValue(b)) {
        const double x = toDoubleValue(a, tok);
        const double y = toDoubleValue(b, tok);
        if (op == "==")
            return x == y;
        if (op == "!=")
            return x != y;
    } else {
        const qint64 x = toIntValue(a, tok);
        const qint64 y = toIntValue(b, tok);
        if (op == "==")
            return x == y;
        if (op == "!=")
            return x != y;
    }

    reportError(tok, "invalid equality operator");
    return false;
}

bool relCompare(const AsExprValue &a, const AsExprValue &b,
                const std::string &op, const antlr4::Token *tok) {
    if (isStringValue(a) || isStringValue(b)) {
        if (!isStringValue(a) || !isStringValue(b)) {
            reportError(tok, "string can only be compared with string");
        }
        const QString sa = toQStringValue(a);
        const QString sb = toQStringValue(b);
        if (op == "<")
            return sa < sb;
        if (op == "<=")
            return sa <= sb;
        if (op == ">")
            return sa > sb;
        if (op == ">=")
            return sa >= sb;
        reportError(tok, "invalid string relational operator");
    }

    if (isDoubleValue(a) || isDoubleValue(b)) {
        const double x = toDoubleValue(a, tok);
        const double y = toDoubleValue(b, tok);
        if (op == "<")
            return x < y;
        if (op == "<=")
            return x <= y;
        if (op == ">")
            return x > y;
        if (op == ">=")
            return x >= y;
    } else {
        const qint64 x = toIntValue(a, tok);
        const qint64 y = toIntValue(b, tok);
        if (op == "<")
            return x < y;
        if (op == "<=")
            return x <= y;
        if (op == ">")
            return x > y;
        if (op == ">=")
            return x >= y;
    }

    reportError(tok, "invalid relational operator");
    return false;
}

qint64 checkedLeftShift(qint64 lhs, qint64 rhs, const antlr4::Token *tok) {
    if (lhs < 0) {
        reportError(tok, "left shift on negative value is not allowed");
    }

    const quint64 u = static_cast<quint64>(lhs);
    const quint64 out = u << static_cast<quint64>(rhs);
    return static_cast<qint64>(out);
}

qint64 checkedArithmeticRightShift(qint64 lhs, qint64 rhs) {
    return lhs >> rhs;
}

qint64 checkedLogicalRightShift(qint64 lhs, qint64 rhs) {
    const quint64 out = static_cast<quint64>(lhs) >> static_cast<quint64>(rhs);
    return static_cast<qint64>(out);
}

QString stripSeparators(QString &&s) { return s.remove('\''); }

AsExprValue parseIntegerToken(const antlr4::Token *tok) {
    auto s = stripSeparators(QString::fromStdString(tok->getText()));
    bool ok;
    auto v = s.toLongLong(&ok, 0);
    if (!ok) {
        reportError(tok, "integer literal out of int64 range");
    }
    return AsExprValue{v};
}

AsExprValue parseFloatingToken(const antlr4::Token *tok) {
    auto s = stripSeparators(QString::fromStdString(tok->getText()));
    bool ok;
    auto v = s.toDouble(&ok);
    if (!ok) {
        reportError(tok, "invalid floating literal");
    }
    return AsExprValue{v};
}

AsExprValue parseCharToken(const antlr4::Token *tok) {
    auto str = QString::fromStdString(tok->getText());
    if (str.startsWith('\'') && str.startsWith('\'')) {
        str.slice(1, str.length() - 2);
        return AsExprValue{Utilities::translateStringEscapes(str)};
    }
    reportError(tok, "malformed quoted literal");
    return AsExprValue{QString{}};
}

AsExprValue parseStringToken(const antlr4::Token *tok) {
    auto str = QString::fromStdString(tok->getText());
    if (str.startsWith('"') && str.endsWith('"')) {
        str.slice(1, str.length() - 2);
        return AsExprValue{Utilities::translateStringEscapes(str)};
    }
    reportError(tok, "malformed quoted literal");
    return AsExprValue{QString{}};
}

AsExprEvaluator::AsExprEvaluator() {}

std::optional<bool>
AsExprEvaluator::eval(AngelscriptExprParserParser::ExprContext *tree) {
    auto r = visit(tree);
    try {
        AsExprValue v = std::any_cast<AsExprValue>(r);
        if (auto p = std::get_if<bool>(&v)) {
            return *p;
        }
        if (auto p = std::get_if<qint64>(&v)) {
            return *p;
        }
    } catch (...) {
    }
    reportError("final result must be bool or integer");
    return {};
}

antlrcpp::Any
AsExprEvaluator::visitExpr(AngelscriptExprParserParser::ExprContext *ctx) {
    return visit(ctx->logicalOrExpr());
}

antlrcpp::Any AsExprEvaluator::visitLogicalOrExpr(
    AngelscriptExprParserParser::LogicalOrExprContext *ctx) {
    const auto terms = ctx->logicalAndExpr();
    if (terms.empty()) {
        reportError(ctx->start, "empty logical OR expression");
    }

    auto first = std::any_cast<AsExprValue>(visit(terms[0]));
    if (truthy(first, terms[0]->start)) {
        return AsExprValue{true};
    }

    for (size_t i = 1; i < terms.size(); ++i) {
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));
        if (truthy(rhs, terms[i]->start)) {
            return AsExprValue{true};
        }
    }

    return AsExprValue{false};
}

antlrcpp::Any AsExprEvaluator::visitLogicalAndExpr(
    AngelscriptExprParserParser::LogicalAndExprContext *ctx) {
    const auto terms = ctx->bitOrExpr();
    if (terms.empty()) {
        reportError(ctx->start, "empty logical AND expression");
    }

    auto first = std::any_cast<AsExprValue>(visit(terms[0]));
    if (!truthy(first, terms[0]->start)) {
        return AsExprValue{false};
    }

    for (size_t i = 1; i < terms.size(); ++i) {
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));
        if (!truthy(rhs, terms[i]->start)) {
            return AsExprValue{false};
        }
    }

    return AsExprValue{true};
}

antlrcpp::Any AsExprEvaluator::visitBitOrExpr(
    AngelscriptExprParserParser::BitOrExprContext *ctx) {
    const auto terms = ctx->bitXorExpr();
    auto acc = std::any_cast<AsExprValue>(visit(terms[0]));

    for (size_t i = 1; i < terms.size(); ++i) {
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));
        qint64 a = toIntValue(acc, ctx->start);
        qint64 b = toIntValue(rhs, terms[i]->start);
        acc = AsExprValue{a | b};
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitBitXorExpr(
    AngelscriptExprParserParser::BitXorExprContext *ctx) {
    const auto terms = ctx->bitAndExpr();
    AsExprValue acc = std::any_cast<AsExprValue>(visit(terms[0]));

    for (size_t i = 1; i < terms.size(); ++i) {
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));
        qint64 a = toIntValue(acc, ctx->start);
        qint64 b = toIntValue(rhs, terms[i]->start);
        acc = AsExprValue{a ^ b};
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitBitAndExpr(
    AngelscriptExprParserParser::BitAndExprContext *ctx) {
    const auto terms = ctx->equalityExpr();
    auto acc = std::any_cast<AsExprValue>(visit(terms[0]));

    for (int i = 1; i < static_cast<int>(terms.size()); ++i) {
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));
        qint64 a = toIntValue(acc, ctx->start);
        qint64 b = toIntValue(rhs, terms[i]->start);
        acc = AsExprValue{a & b};
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitEqualityExpr(
    AngelscriptExprParserParser::EqualityExprContext *ctx) {
    const auto terms = ctx->relationalExpr();
    auto acc = std::any_cast<AsExprValue>(visit(terms[0]));

    for (int i = 1; i < terms.size(); ++i) {
        const auto op = binOpText(ctx, i);
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));
        acc = AsExprValue{eqCompare(acc, rhs, op, ctx->start)};
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitRelationalExpr(
    AngelscriptExprParserParser::RelationalExprContext *ctx) {
    const auto terms = ctx->shiftExpr();
    auto acc = std::any_cast<AsExprValue>(visit(terms[0]));

    for (int i = 1; i < terms.size(); ++i) {
        const auto op = binOpText(ctx, i);
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));
        acc = AsExprValue{relCompare(acc, rhs, op, ctx->start)};
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitShiftExpr(
    AngelscriptExprParserParser::ShiftExprContext *ctx) {
    const auto terms = ctx->additiveExpr();
    if (terms.empty()) {
        reportError(ctx->start, "empty shift expression");
    }

    auto acc = std::any_cast<AsExprValue>(visit(terms[0]));

    int childIndex = 1;
    for (int i = 1; i < static_cast<int>(terms.size()); ++i) {
        std::string op;
        op.reserve(3);
        while (childIndex < static_cast<int>(ctx->children.size()) &&
               dynamic_cast<antlr4::ParserRuleContext *>(
                   ctx->children[childIndex]) == nullptr) {
            op += ctx->children[childIndex]->getText();
            ++childIndex;
        }

        ++childIndex;

        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));

        if (isStringValue(acc) || isStringValue(rhs) || isDoubleValue(acc) ||
            isDoubleValue(rhs)) {
            reportError(terms[i]->start,
                        "shift operators only support integers");
        }

        const qint64 lhs = toIntValue(acc, terms[i]->start);
        const qint64 shift = toIntValue(rhs, terms[i]->start);

        if (op == "<<") {
            acc = AsExprValue{checkedLeftShift(lhs, shift, terms[i]->start)};
        } else if (op == ">>") {
            acc = AsExprValue{checkedArithmeticRightShift(lhs, shift)};
        } else if (op == ">>>") {
            acc = AsExprValue{checkedLogicalRightShift(lhs, shift)};
        } else {
            reportError(ctx->start,
                        fmt::format("unknown shift operator: {}", op));
        }
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitAdditiveExpr(
    AngelscriptExprParserParser::AdditiveExprContext *ctx) {
    const auto terms = ctx->multiplicativeExpr();
    auto acc = std::any_cast<AsExprValue>(visit(terms[0]));

    for (int i = 1; i < static_cast<int>(terms.size()); ++i) {
        const auto op = binOpText(ctx, i);
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));

        if (op == "+") {
            if (isStringValue(acc) || isStringValue(rhs)) {
                if (!isStringValue(acc) || !isStringValue(rhs)) {
                    reportError(ctx->start, "string concatenation requires "
                                            "both operands to be strings");
                }
                acc = AsExprValue{toQStringValue(acc) + toQStringValue(rhs)};
                continue;
            }

            if (isDoubleValue(acc) || isDoubleValue(rhs)) {
                acc = AsExprValue{toDoubleValue(acc, ctx->start) +
                                  toDoubleValue(rhs, terms[i]->start)};
            } else {
                qint64 out = 0;
                if (qAddOverflow(toIntValue(acc, ctx->start),
                                 toIntValue(rhs, terms[i]->start), &out)) {
                    reportError(ctx->start, "integer addition overflow");
                }
                acc = AsExprValue{out};
            }
        } else if (op == "-") {
            if (isStringValue(acc) || isStringValue(rhs)) {
                reportError(ctx->start, "string does not support subtraction");
            }

            if (isDoubleValue(acc) || isDoubleValue(rhs)) {
                acc = AsExprValue{toDoubleValue(acc, ctx->start) -
                                  toDoubleValue(rhs, terms[i]->start)};
            } else {
                qint64 out = 0;
                if (qSubOverflow(toIntValue(acc, ctx->start),
                                 toIntValue(rhs, terms[i]->start), &out)) {
                    reportError(ctx->start, "integer subtraction overflow");
                }
                acc = AsExprValue{out};
            }
        }
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitMultiplicativeExpr(
    AngelscriptExprParserParser::MultiplicativeExprContext *ctx) {
    const auto terms = ctx->unaryExpr();
    auto acc = std::any_cast<AsExprValue>(visit(terms[0]));

    for (int i = 1; i < static_cast<int>(terms.size()); ++i) {
        const auto op = binOpText(ctx, i);
        auto rhs = std::any_cast<AsExprValue>(visit(terms[i]));

        if (isStringValue(acc) || isStringValue(rhs)) {
            reportError(ctx->start, "string only supports + and comparisons");
        }

        if (op == "*") {
            if (isDoubleValue(acc) || isDoubleValue(rhs)) {
                acc = AsExprValue{toDoubleValue(acc, ctx->start) *
                                  toDoubleValue(rhs, terms[i]->start)};
            } else {
                qint64 out = 0;
                if (qMulOverflow(toIntValue(acc, ctx->start),
                                 toIntValue(rhs, terms[i]->start), &out)) {
                    reportError(ctx->start, "integer multiplication overflow");
                }
                acc = AsExprValue{out};
            }
        } else if (op == "/") {
            if (isDoubleValue(acc) || isDoubleValue(rhs)) {
                const double y = toDoubleValue(rhs, terms[i]->start);
                if (y == 0.0) {
                    reportError(terms[i]->start, "division by zero");
                }
                acc = AsExprValue{toDoubleValue(acc, ctx->start) / y};
            } else {
                const qint64 y = toIntValue(rhs, terms[i]->start);
                if (y == 0) {
                    reportError(terms[i]->start, "division by zero");
                }
                const qint64 x = toIntValue(acc, ctx->start);
                acc = AsExprValue{qint64(x / y)};
            }
        } else if (op == "%") {
            const qint64 y = toIntValue(rhs, terms[i]->start);
            if (y == 0) {
                reportError(terms[i]->start, "modulo by zero");
            }
            const qint64 x = toIntValue(acc, ctx->start);
            acc = AsExprValue{qint64(x % y)};
        }
    }

    return acc;
}

antlrcpp::Any AsExprEvaluator::visitUnaryExpr(
    AngelscriptExprParserParser::UnaryExprContext *ctx) {
    auto v = std::any_cast<AsExprValue>(visit(ctx->primaryExpr()));

    std::vector<std::string> ops;
    for (int i = 0; i + 1 < ctx->children.size(); ++i) {
        ops.push_back(ctx->children.at(i)->getText());
    }

    for (auto it = ops.crbegin(); it != ops.crend(); ++it) {
        const auto &op = *it;

        if (op == "+") {
            if (isStringValue(v)) {
                reportError(ctx->start, "unary + does not support string");
            }
        } else if (op == "-") {
            if (isStringValue(v)) {
                reportError(ctx->start, "unary - does not support string");
            }

            if (isDoubleValue(v)) {
                v = AsExprValue{-std::get<double>(v)};
            } else {
                const qint64 x = toIntValue(v, ctx->start);
                v = AsExprValue{static_cast<qint64>(-x)};
            }
        } else if (op == "!") {
            v = AsExprValue{!truthy(v, ctx->start)};
        } else if (op == "~") {
            if (isStringValue(v)) {
                reportError(ctx->start, "bitwise not does not support string");
            }
            if (isDoubleValue(v)) {
                reportError(ctx->start,
                            "bitwise not does not support floating point");
            }
            v = AsExprValue{static_cast<qint64>(~toIntValue(v, ctx->start))};
        } else {
            reportError(ctx->start,
                        fmt::format("unknown unary operator: {}", op));
        }
    }

    return v;
}

antlrcpp::Any AsExprEvaluator::visitPrimaryExpr(
    AngelscriptExprParserParser::PrimaryExprContext *ctx) {
    if (ctx->literal()) {
        return visit(ctx->literal());
    }

    return visit(ctx->expr());
}

antlrcpp::Any AsExprEvaluator::visitLiteral(
    AngelscriptExprParserParser::LiteralContext *ctx) {
    if (ctx->IntegerLiteral()) {
        return parseIntegerToken(ctx->IntegerLiteral()->getSymbol());
    }

    if (ctx->FloatingLiteral()) {
        return parseFloatingToken(ctx->FloatingLiteral()->getSymbol());
    }

    if (ctx->CharacterLiteral()) {
        return parseCharToken(ctx->CharacterLiteral()->getSymbol());
    }

    if (ctx->StringLiteral()) {
        return parseStringToken(ctx->StringLiteral()->getSymbol());
    }

    if (ctx->True_()) {
        return AsExprValue{true};
    }

    if (ctx->False_()) {
        return AsExprValue{false};
    }

    if (ctx->Nullptr()) {
        return AsExprValue{qint64(0)};
    }

    reportError("unknown literal");
    return AsExprValue{};
}
