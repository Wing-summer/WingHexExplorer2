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

#ifndef ASEXPREVALUATOR_H
#define ASEXPREVALUATOR_H

#include "grammar/AngelscriptExprParser/AngelscriptExprParserBaseVisitor.h"

#include <QString>

using AsExprValue = std::variant<std::monostate, bool, qint64, double, QString>;

class AsExprException : public antlr4::RuntimeException {
public:
    qint64 line = 0;
    qint64 column = 0;

    AsExprException(qint64 l, qint64 c, const std::string &msg)
        : antlr4::RuntimeException(msg), line(l), column(c) {}

    explicit AsExprException(const std::string &msg)
        : antlr4::RuntimeException(msg) {}
};

class AsExprEvaluator : public AngelscriptExprParserBaseVisitor {
public:
    explicit AsExprEvaluator();

    std::optional<bool> eval(AngelscriptExprParserParser::ExprContext *tree);

public:
    antlrcpp::Any
    visitExpr(AngelscriptExprParserParser::ExprContext *ctx) override;
    antlrcpp::Any visitLogicalOrExpr(
        AngelscriptExprParserParser::LogicalOrExprContext *ctx) override;
    antlrcpp::Any visitLogicalAndExpr(
        AngelscriptExprParserParser::LogicalAndExprContext *ctx) override;
    antlrcpp::Any
    visitBitOrExpr(AngelscriptExprParserParser::BitOrExprContext *ctx) override;
    antlrcpp::Any visitBitXorExpr(
        AngelscriptExprParserParser::BitXorExprContext *ctx) override;
    antlrcpp::Any visitBitAndExpr(
        AngelscriptExprParserParser::BitAndExprContext *ctx) override;
    antlrcpp::Any visitEqualityExpr(
        AngelscriptExprParserParser::EqualityExprContext *ctx) override;
    antlrcpp::Any visitRelationalExpr(
        AngelscriptExprParserParser::RelationalExprContext *ctx) override;
    antlrcpp::Any
    visitShiftExpr(AngelscriptExprParserParser::ShiftExprContext *ctx) override;
    antlrcpp::Any visitAdditiveExpr(
        AngelscriptExprParserParser::AdditiveExprContext *ctx) override;
    antlrcpp::Any visitMultiplicativeExpr(
        AngelscriptExprParserParser::MultiplicativeExprContext *ctx) override;
    antlrcpp::Any
    visitUnaryExpr(AngelscriptExprParserParser::UnaryExprContext *ctx) override;
    antlrcpp::Any visitPrimaryExpr(
        AngelscriptExprParserParser::PrimaryExprContext *ctx) override;
    antlrcpp::Any
    visitLiteral(AngelscriptExprParserParser::LiteralContext *ctx) override;
};

#endif // ASEXPREVALUATOR_H
