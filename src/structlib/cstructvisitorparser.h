/*==============================================================================
** Copyright (C) 2025-2028 WingSummer
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

#ifndef CSTRUCTVISITORPARSER_H
#define CSTRUCTVISITORPARSER_H

#include <QHash>
#include <QPair>
#include <QString>

#include <optional>

#include "grammar/c/CStructBaseVisitor.h"
#include "structdefine.h"
#include "structlib/ctypeparser.h"

class CStructErrorListener;

class CStructVisitorParser : public CStructBaseVisitor {
public:
    explicit CStructVisitorParser(CTypeParser *container,
                                  CStructErrorListener *listener);

    // CStructVisitor interface
public:
    virtual std::any
    visitEnumSpecifier(CStructParser::EnumSpecifierContext *ctx) override;

    virtual std::any visitStructOrUnionSpecifier(
        CStructParser::StructOrUnionSpecifierContext *ctx) override;

    virtual std::any
    visitDefineDecl(CStructParser::DefineDeclContext *ctx) override;

    virtual std::any visitAssignmentExpressionDef(
        CStructParser::AssignmentExpressionDefContext *ctx) override;

    virtual std::any
    visitDeclaration(CStructParser::DeclarationContext *ctx) override;

private:
    using EnumDecl =
        QPair<QString, QHash<QString, std::variant<qint64, quint64>>>;

    std::optional<EnumDecl> parseEnum(CStructParser::EnumSpecifierContext *ctx);

    enum class StructMemType { Normal, Pointer, Enum, Struct, Union };

    std::optional<StructUnionDecl>
    parseStructOrUnion(CStructParser::StructOrUnionSpecifierContext *ctx,
                       int alignment = -1);

    struct Specifier {
        StructMemType type = StructMemType::Normal;
        QString tname;
    };

    std::optional<Specifier>
    getSpecifier(CStructParser::SpecifierQualifierListContext *ctx,
                 int alignment = -1);

    std::optional<Specifier>
    getSpecifier(CStructParser::TypeSpecifierContext *ctx, int alignment = -1);

    struct Declarator {
        QString retName;
        bool isPointer = false;
        size_t arrayCount = 1;
        size_t bitField = 0;
        CStructParser::DirectDeclaratorContext *next = nullptr;
    };

    std::optional<Declarator>
    getDeclarator(CStructParser::DirectDeclaratorContext *ctx);

    static QString
    getFinalDeclaratorName(CStructParser::DirectDeclaratorContext *ctx);

    std::optional<size_t> safeMultiply(const QVector<size_t> &vec);

private:
    CTypeParser *parser;

    CStructErrorListener *errlis;

    // CStructVisitor interface
public:
    virtual std::any visitInclusiveOrExpression(
        CStructParser::InclusiveOrExpressionContext *ctx) override;

    virtual std::any visitAssignmentExpression(
        CStructParser::AssignmentExpressionContext *ctx) override;

    virtual std::any visitExclusiveOrExpression(
        CStructParser::ExclusiveOrExpressionContext *ctx) override;

    virtual std::any
    visitAndExpression(CStructParser::AndExpressionContext *ctx) override;

    virtual std::any
    visitShiftExpression(CStructParser::ShiftExpressionContext *ctx) override;

    virtual std::any visitAdditiveExpression(
        CStructParser::AdditiveExpressionContext *ctx) override;

    virtual std::any visitMultiplicativeExpression(
        CStructParser::MultiplicativeExpressionContext *ctx) override;

    virtual std::any
    visitCastExpression(CStructParser::CastExpressionContext *ctx) override;

    virtual std::any
    visitUnaryExpression(CStructParser::UnaryExpressionContext *ctx) override;

    virtual std::any visitPostfixExpression(
        CStructParser::PostfixExpressionContext *ctx) override;

    virtual std::any visitPrimaryExpression(
        CStructParser::PrimaryExpressionContext *ctx) override;

private:
    std::variant<std::monostate, qint64, quint64>
    parseIntegerConstant(const std::string &text);

    void reportNumOutofRangeError(size_t line, size_t charPositionInLine,
                                  const QString &num);

    void reportDupDeclError(size_t line, size_t charPositionInLine,
                            const QString &var);

    void reportDupError(size_t line, size_t charPositionInLine,
                        const QString &var);

    void reportUnexpectedType(size_t line, size_t charPositionInLine,
                              const QString &var, const QStringList &expected);

    void reportUnexpectedToken(size_t line, size_t charPositionInLine,
                               const QString &token,
                               const QStringList &expected);

    void reportSyntaxDeclError(size_t line, size_t charPositionInLine);

    void reportUndeclaredType(size_t line, size_t charPositionInLine,
                              const QString &type);

    void reportArrayOutofLimit(size_t line, size_t charPositionInLine,
                               const QString &name, const QString &iden);

    void reportOverflowWarn(size_t line, size_t charPositionInLine,
                            const QString &op);

    void reportFiledBitOverflow(size_t line, size_t charPositionInLine,
                                const QString &type, quint64 bit);

    CTypeParser::StructResult reportCTypeError(size_t line,
                                               size_t charPositionInLine,
                                               CTypeParser::StructResult result,
                                               const QString &identifier);

private:
    bool isInteger(const QString &text);
};

#endif // CSTRUCTVISITORPARSER_H
