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

#include "cstructvisitorparser.h"

#include "cstructerrorlistener.h"
#include "grammar/c/CStructLexer.h"

#include <QCoreApplication>
#include <QMetaEnum>
#include <QScopeGuard>

CStructVisitorParser::CStructVisitorParser(CTypeParser *container,
                                           CStructErrorListener *listener)
    : parser(container), errlis(listener) {
    Q_ASSERT(parser);
    Q_ASSERT(errlis);
}

std::any CStructVisitorParser::visitEnumSpecifier(
    CStructParser::EnumSpecifierContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    auto sym = ctx->getStart();
    if (ctx->enumeratorList()) {
        auto e = parseEnum(ctx);
        if (e) {
            reportCTypeError(sym->getLine(), sym->getCharPositionInLine(),
                             parser->defineEnum(e->first, e->second), e->first);
        }
    } else {
        auto iden = ctx->Identifier();
        Q_ASSERT(iden);
        auto name = QString::fromStdString(iden->getText());
        reportCTypeError(sym->getLine(), sym->getCharPositionInLine(),
                         parser->addForwardEnum(name), name);
    }
    return defaultResult();
}

std::any CStructVisitorParser::visitInclusiveOrExpression(
    CStructParser::InclusiveOrExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    quint64 ret = 0;
    bool isUnsigned = false;
    for (auto &v : ctx->exclusiveOrExpression()) {
        auto r = visitExclusiveOrExpression(v);
        if (r.type() == typeid(quint64)) {
            auto rr = std::any_cast<quint64>(r);
            ret |= rr;
            isUnsigned = true;
        } else if (r.type() == typeid(qint64)) {
            auto rr = std::any_cast<qint64>(r);
            ret |= rr;
        } else {
            auto t = v->start;
            reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                 QString::fromUtf8(r.type().name()),
                                 {"uint64", "int64"});
            return defaultResult();
        }
    }
    if (isUnsigned) {
        return ret;
    } else {
        return qint64(ret);
    }
}

std::any CStructVisitorParser::visitAssignmentExpression(
    CStructParser::AssignmentExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    if (ctx->IntegerConstant()) {
        auto t = ctx->IntegerConstant();
        auto num = ctx->IntegerConstant()->getText();
        auto v = parseIntegerConstant(num);
        if (std::holds_alternative<quint64>(v)) {
            return std::get<quint64>(v);
        } else if (std::holds_alternative<qint64>(v)) {
            return std::get<qint64>(v);
        } else {
            auto tk = t->getSymbol();
            reportNumOutofRangeError(tk->getLine(), tk->getCharPositionInLine(),
                                     QString::fromStdString(num));
            return defaultResult();
        }
    } else if (ctx->inclusiveOrExpression()) {
        return visitInclusiveOrExpression(ctx->inclusiveOrExpression());
    }

    return defaultResult();
}

std::any CStructVisitorParser::visitExclusiveOrExpression(
    CStructParser::ExclusiveOrExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    quint64 v = 0;
    bool isUnsigned = false;
    for (auto &ex : ctx->andExpression()) {
        auto r = visitAndExpression(ex);
        if (r.type() == typeid(qint64)) {
            auto rv = std::any_cast<qint64>(r);
            v ^= rv;
        } else if (r.type() == typeid(quint64)) {
            auto rv = std::any_cast<quint64>(r);
            v ^= rv;
            isUnsigned = true;
        } else {
            auto t = ex->start;
            reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                 QStringLiteral("<unknown>"),
                                 {"uint64", "int64"});
            return defaultResult();
        }
    }

    if (isUnsigned) {
        return v;
    } else {
        return qint64(v);
    }
}

std::variant<std::monostate, qint64, quint64>
CStructVisitorParser::parseIntegerConstant(const std::string &text) {
    auto txt = QByteArray::fromStdString(text);

    if (txt.size() >= 3 && txt.front() == '\'' && txt.back() == '\'') {
        auto inner = txt.sliced(1, text.size() - 2);

        if (inner.size() == 1) {
            return qint64(inner[0]);
        } else if (inner[0] == '\\') {
            if (inner.length() == 4) {
                if (std::all_of(std::next(inner.begin()), inner.end(),
                                [](const QChar &ch) {
                                    auto num = ch.toLatin1();
                                    return num >= '0' && num < '8';
                                })) {
                    auto toNum = [](const QChar &num) {
                        return num.toLatin1() - '0';
                    };
                    return qint64(toNum(inner[1]) * 64 + toNum(inner[2]) * 8 +
                                  toNum(inner[3]));
                }
            } else {
                static QHash<QByteArray, char> buffer{
                    {R"(\n)", '\n'}, {R"(\t)", '\t'}, {R"(\0)", '\0'},
                    {R"(\\)", '\\'}, {R"(\')", '\''}, {R"(\")", '"'},
                    {R"(\?)", '\?'}, {R"(\a)", '\a'}, {R"(\b)", '\b'},
                    {R"(\f)", '\f'}, {R"(\r)", '\r'}, {R"(\v)", '\v'}};

                if (buffer.contains(inner)) {
                    return qint64(buffer.value(inner));
                }

                auto n = inner[1];
                if (n == 'x' || n == 'X' || n == 'b' || n == 'B') {
                    bool b = false;
                    auto v = inner.toLongLong(&b, 0);
                    if (b) {
                        return v;
                    } else {
                        auto v = inner.toULongLong(&b, 0);
                        if (b) {
                            return v;
                        }
                    }
                }
            }
        }
    } else {
        qsizetype i = 0;
        for (i = txt.size(); i > 0; --i) {
            auto v = txt[i - 1];
            if (v != 'u' && v != 'U' && v != 'l' && v != 'L')
                break;
        }
        auto numberPart = txt.sliced(0, i);
        auto suffix = txt.sliced(i);
        bool b = false;
        auto v = numberPart.toLongLong(&b, 0);
        if (b) {
            if (suffix.contains('u') || suffix.contains('U')) {
                return std::variant<std::monostate, qint64, quint64>{
                    std::in_place_type<quint64>, v};
            }
            return std::variant<std::monostate, qint64, quint64>{
                std::in_place_type<qint64>, v};
        } else {
            auto v = numberPart.toULongLong(&b, 0);
            if (b) {
                return std::variant<std::monostate, qint64, quint64>{
                    std::in_place_type<quint64>, v};
            }
        }
    }

    return {};
}

void CStructVisitorParser::reportOutofRangeError(size_t line,
                                                 size_t charPositionInLine) {
    errlis->reportError(line, charPositionInLine,
                        QStringLiteral("out of range"));
}

void CStructVisitorParser::reportNumOutofRangeError(size_t line,
                                                    size_t charPositionInLine,
                                                    const QString &num) {
    errlis->reportError(
        line, charPositionInLine,
        QStringLiteral("\"%1\" is out of range with number").arg(num));
}

void CStructVisitorParser::reportDupDeclError(size_t line,
                                              size_t charPositionInLine,
                                              const QString &var) {
    auto e = QMetaEnum::fromType<CTypeParser::CType>();
    errlis->reportError(
        line, charPositionInLine,
        QStringLiteral("\"%1\" is already declared with type %2")
            .arg(var)
            .arg(e.valueToKey(int(parser->type(var)))));
}

void CStructVisitorParser::reportDupError(size_t line,
                                          size_t charPositionInLine,
                                          const QString &var) {
    errlis->reportError(
        line, charPositionInLine,
        QStringLiteral("\"%1\" is duplicated declaration").arg(var));
}

void CStructVisitorParser::reportUnexpectedType(size_t line,
                                                size_t charPositionInLine,
                                                const QString &var,
                                                const QStringList &expected) {
    errlis->reportError(line, charPositionInLine,
                        QStringLiteral("Unexprected type \"%1\", expecting: %2")
                            .arg(var, expected.join(QStringLiteral(", "))));
}

void CStructVisitorParser::reportUnexpectedToken(size_t line,
                                                 size_t charPositionInLine,
                                                 const QString &token,
                                                 const QStringList &expected) {
    errlis->reportError(
        line, charPositionInLine,
        QStringLiteral("Unexprected token \"%1\", expecting: {%2}")
            .arg(token, expected.join(QStringLiteral(", "))));
}

void CStructVisitorParser::reportSyntaxDeclError(size_t line,
                                                 size_t charPositionInLine) {
    errlis->reportError(line, charPositionInLine,
                        QStringLiteral("Declaring syntax error"));
}

void CStructVisitorParser::reportUndeclaredType(size_t line,
                                                size_t charPositionInLine,
                                                const QString &type) {
    errlis->reportError(line, charPositionInLine,
                        QStringLiteral("\"%1\" is unknown type").arg(type));
}

void CStructVisitorParser::reportArrayOutofLimit(size_t line,
                                                 size_t charPositionInLine,
                                                 const QString &name,
                                                 const QString &iden) {

    errlis->reportError(
        line, charPositionInLine,
        QStringLiteral("Too large array \"%1\" in %2").arg(name, iden));
}

void CStructVisitorParser::reportOverflowWarn(size_t line,
                                              size_t charPositionInLine,
                                              const QString &op) {
    errlis->reportWarn(
        line, charPositionInLine,
        QStringLiteral("Operation \"%1\" will cause number overflow").arg(op));
}

void CStructVisitorParser::reportFiledBitOverflow(size_t line,
                                                  size_t charPositionInLine,
                                                  const QString &type,
                                                  quint64 bit) {
    errlis->reportError(
        line, charPositionInLine,
        QStringLiteral("Too many bits(%1) for type %2").arg(bit).arg(type));
}

CTypeParser::StructResult
CStructVisitorParser::reportCTypeError(size_t line, size_t charPositionInLine,
                                       CTypeParser::StructResult result,
                                       const QString &identifier) {
    switch (result) {
    case CTypeParser::StructResult::Ok:
        // nothing
        break;
    case CTypeParser::StructResult::NameConflict: {
        reportDupDeclError(line, charPositionInLine, identifier);
    } break;
    case CTypeParser::StructResult::DuplicateDefinition:
        reportDupError(line, charPositionInLine, identifier);
        break;
    case CTypeParser::StructResult::IncompleteReference:
        errlis->reportError(
            line, charPositionInLine,
            QStringLiteral("\"%1\" is incompleted type").arg(identifier));
        break;
    case CTypeParser::StructResult::OutofMemory:
        errlis->reportError(
            line, charPositionInLine,
            QStringLiteral("\"%1\" is too large type").arg(identifier));
        break;
    case CTypeParser::StructResult::CountOfLimit:
        errlis->reportError(
            line, charPositionInLine,
            QStringLiteral("Too many symbols when adding \"%1\"")
                .arg(identifier));
        break;
    }
    return result;
}

bool CStructVisitorParser::isInteger(const QString &text) {
    auto type = parser->typeMapValue(text);
    switch (type) {
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Long:
    case QMetaType::Short:
    case QMetaType::Char:
    case QMetaType::Char16:
    case QMetaType::Char32:
    case QMetaType::ULong:
    case QMetaType::UShort:
    case QMetaType::UChar:
    case QMetaType::SChar:
        return true;
    default:
        return false;
    }
}

std::any CStructVisitorParser::visitAndExpression(
    CStructParser::AndExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    quint64 v = std::numeric_limits<quint64>::max();
    bool isUnsigned = false;
    for (auto &ex : ctx->shiftExpression()) {
        auto r = visitShiftExpression(ex);
        if (r.type() == typeid(qint64)) {
            auto rv = std::any_cast<qint64>(r);
            v &= rv;
        } else if (r.type() == typeid(quint64)) {
            auto rv = std::any_cast<quint64>(r);
            v &= rv;
            isUnsigned = true;
        } else {
            auto t = ex->start;
            reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                 QStringLiteral("<unknown>"),
                                 {"uint64", "int64"});
            return defaultResult();
        }
    }
    if (isUnsigned) {
        return v;
    } else {
        return qint64(v);
    }
}

std::any CStructVisitorParser::visitShiftExpression(
    CStructParser::ShiftExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    auto data = ctx->additiveExpression();
    auto total = data.size();

    if (total < 1) {
        return defaultResult();
    }

    std::variant<qint64, quint64> ret;
    auto retv = visitAdditiveExpression(data.front());
    if (retv.type() == typeid(qint64)) {
        ret = std::any_cast<qint64>(retv);
    } else if (retv.type() == typeid(quint64)) {
        ret = std::any_cast<quint64>(retv);
    } else {
        auto t = data.front()->start;
        reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                             QStringLiteral("<unknown>"), {"uint64", "int64"});
        return defaultResult();
    }

    for (size_t i = 1; i < total; ++i) {
        auto op = ctx->children[2 * i - 1]->getText();
        auto ex = data.at(i);
        auto r = visitAdditiveExpression(ex);
        if (op == "<<") {
            if (r.type() == typeid(qint64)) {
                auto rv = std::any_cast<qint64>(r);
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v << rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v << rv;
                }
            } else if (r.type() == typeid(quint64)) {
                auto rv = std::any_cast<quint64>(r);
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v << rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v << rv;
                }
            } else {
                auto t = ex->start;
                reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                     QStringLiteral("<unknown>"),
                                     {"uint64", "int64"});
                return defaultResult();
            }
        } else if (op == ">>") {
            if (r.type() == typeid(qint64)) {
                auto rv = std::any_cast<qint64>(r);
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v >> rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v >> rv;
                }
            } else if (r.type() == typeid(quint64)) {
                auto rv = std::any_cast<quint64>(r);
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v >> rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v >> rv;
                }
            } else {
                auto t = ex->start;
                reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                     QStringLiteral("<unknown>"),
                                     {"uint64", "int64"});
                return defaultResult();
            }
        } else {
            auto t = ex->start;
            reportUnexpectedToken(t->getLine(), t->getCharPositionInLine(),
                                  QString::fromStdString(op),
                                  {R"("<<")", R"(">>")"});
            return defaultResult();
        }
    }

    if (std::holds_alternative<qint64>(ret)) {
        return std::get<qint64>(ret);
    } else {
        return std::get<quint64>(ret);
    }
}

std::any CStructVisitorParser::visitAdditiveExpression(
    CStructParser::AdditiveExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    auto data = ctx->multiplicativeExpression();
    auto total = data.size();

    if (total < 1) {
        return defaultResult();
    }

    std::variant<qint64, quint64> ret;
    auto retv = visitMultiplicativeExpression(data.front());
    if (retv.type() == typeid(qint64)) {
        ret = std::any_cast<qint64>(retv);
    } else if (retv.type() == typeid(quint64)) {
        ret = std::any_cast<quint64>(retv);
    } else {
        auto t = data.front()->start;
        reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                             QStringLiteral("<unknown>"), {"uint64", "int64"});
        return defaultResult();
    }

    for (size_t i = 1; i < total; i++) {
        auto ex = data.at(i);
        auto r = visitMultiplicativeExpression(ex);
        auto opSym = ctx->children[2 * i - 1];
        auto op = opSym->getText();
        Q_ASSERT(antlr4::tree::TerminalNode::is(opSym));
        auto osym =
            reinterpret_cast<antlr4::tree::TerminalNode *>(opSym)->getSymbol();
        if (r.type() == typeid(qint64)) {
            auto rv = std::any_cast<qint64>(r);
            if (op == "+") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    qint64 r;
                    if (qAddOverflow(v, rv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" + ") +
                                QString::number(rv));
                        ret = qint64(v + rv);
                    } else {
                        ret = r;
                    }
                } else {
                    auto v = std::get<quint64>(ret);
                    if (rv < 0) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QStringLiteral(
                                "implicit cast negtive int64 to uint64"));
                    }
                    ret = v + quint64(rv);
                }
            } else if (op == "-") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    qint64 r;
                    if (qSubOverflow(v, rv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" - ") +
                                QString::number(rv));
                        ret = qint64(v - rv);
                    } else {
                        ret = r;
                    }
                } else {
                    auto v = std::get<quint64>(ret);
                    if (rv < 0) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QStringLiteral(
                                "implicit cast negtive int64 to uint64"));
                    }
                    ret = v - quint64(rv);
                }
            } else {
                auto t = ex->start;
                reportUnexpectedToken(t->getLine(), t->getCharPositionInLine(),
                                      QString::fromStdString(op),
                                      {R"("+")", R"("-")"});
                return defaultResult();
            }
        } else if (r.type() == typeid(quint64)) {
            auto rv = std::any_cast<quint64>(r);
            if (op == "+") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    qint64 r;
                    if (v < 0) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QStringLiteral(
                                "implicit cast negtive int64 to uint64"));
                    }
                    ret = quint64(v) + rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    quint64 r;
                    if (qAddOverflow(v, rv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" + ") +
                                QString::number(rv));
                        ret = quint64(v + rv);
                    } else {
                        ret = r;
                    }
                }
            } else if (op == "-") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    if (v < 0) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QStringLiteral(
                                "implicit cast negtive int64 to uint64"));
                    }
                    ret = quint64(v) - rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    quint64 r;
                    if (qSubOverflow(v, rv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" - ") +
                                QString::number(rv));
                        ret = quint64(v - rv);
                    } else {
                        ret = r;
                    }
                }
            } else {
                auto t = ex->start;
                reportUnexpectedToken(t->getLine(), t->getCharPositionInLine(),
                                      QString::fromStdString(op),
                                      {R"("+")", R"("-")"});
                return defaultResult();
            }
        } else {
            auto t = ex->start;
            reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                 QStringLiteral("<unknown>"),
                                 {"uint64", "int64"});
            return defaultResult();
        }
    }

    if (std::holds_alternative<qint64>(ret)) {
        return std::get<qint64>(ret);
    } else {
        return std::get<quint64>(ret);
    }
}

std::any CStructVisitorParser::visitMultiplicativeExpression(
    CStructParser::MultiplicativeExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    auto data = ctx->castExpression();
    auto total = data.size();

    if (total < 1) {
        return defaultResult();
    }

    std::variant<qint64, quint64> ret;
    auto retv = visitCastExpression(data.front());
    if (retv.type() == typeid(qint64)) {
        ret = std::any_cast<qint64>(retv);
    } else if (retv.type() == typeid(quint64)) {
        ret = std::any_cast<quint64>(retv);
    } else {
        auto t = data.front()->start;
        reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                             QStringLiteral("<unknown>"), {"uint64", "int64"});
        return defaultResult();
    }

    for (size_t i = 1; i < total; i++) {
        auto ex = data.at(i);
        auto r = visitCastExpression(ex);
        auto opSym = ctx->children[2 * i - 1];
        auto op = opSym->getText();
        Q_ASSERT(antlr4::tree::TerminalNode::is(opSym));
        auto osym =
            reinterpret_cast<antlr4::tree::TerminalNode *>(opSym)->getSymbol();
        if (r.type() == typeid(qint64)) {
            auto rv = std::any_cast<qint64>(r);
            if (op == "*") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    qint64 r;
                    if (qMulOverflow(v, rv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" * ") +
                                QString::number(rv));
                        ret = v * rv;
                    } else {
                        ret = r;
                    }
                } else {
                    auto v = std::get<quint64>(ret);
                    auto rvv = quint64(rv);
                    quint64 r;
                    if (qMulOverflow(v, rvv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" * ") +
                                QString::number(rvv));
                        ret = v * rvv;
                    } else {
                        ret = r;
                    }
                }
            } else if (op == "/") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v / rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v / rv;
                }
            } else if (op == "%") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v % rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v % rv;
                }
            } else {
                auto t = ex->start;
                reportUnexpectedToken(t->getLine(), t->getCharPositionInLine(),
                                      QString::fromStdString(op),
                                      {R"("*")", R"("/")", R"("%")"});
                return defaultResult();
            }
        } else if (r.type() == typeid(quint64)) {
            auto rv = std::any_cast<quint64>(r);
            if (op == "*") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = quint64(std::get<qint64>(ret));
                    quint64 r;
                    if (qMulOverflow(v, rv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" * ") +
                                QString::number(rv));
                        ret = v * rv;
                    } else {
                        ret = r;
                    }
                } else {
                    auto v = std::get<quint64>(ret);
                    quint64 r;
                    if (qMulOverflow(v, rv, &r)) {
                        reportOverflowWarn(
                            osym->getLine(), osym->getCharPositionInLine(),
                            QString::number(v) + QStringLiteral(" * ") +
                                QString::number(rv));
                        ret = v * rv;
                    } else {
                        ret = r;
                    }
                }
            } else if (op == "/") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v / rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v / rv;
                }
            } else if (op == "%") {
                if (std::holds_alternative<qint64>(ret)) {
                    auto v = std::get<qint64>(ret);
                    ret = v % rv;
                } else {
                    auto v = std::get<quint64>(ret);
                    ret = v % rv;
                }
            } else {
                auto t = ex->start;
                reportUnexpectedToken(t->getLine(), t->getCharPositionInLine(),
                                      QString::fromStdString(op),
                                      {R"("*")", R"("/")", R"("%")"});
                return defaultResult();
            }
        } else {
            auto t = ex->start;
            reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                 QStringLiteral("<unknown>"),
                                 {"uint64", "int64"});
            return defaultResult();
        }
    }

    if (std::holds_alternative<qint64>(ret)) {
        return std::get<qint64>(ret);
    } else {
        return std::get<quint64>(ret);
    }
}

std::any CStructVisitorParser::visitCastExpression(
    CStructParser::CastExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    if (ctx->IntegerConstant()) {
        auto ex = ctx->IntegerConstant();
        auto num = ex->getText();
        auto r = parseIntegerConstant(num);
        if (std::holds_alternative<quint64>(r)) {
            return std::get<quint64>(r);
        } else if (std::holds_alternative<qint64>(r)) {
            return std::get<qint64>(r);
        } else {
            auto tk = ex->getSymbol();
            reportNumOutofRangeError(tk->getLine(), tk->getCharPositionInLine(),
                                     QString::fromStdString(num));
            return defaultResult();
        }
    } else if (ctx->castExpression()) {
        auto tname = QString::fromStdString(ctx->typeName()->getText());
        auto isUnsigned = parser->isUnsignedBasicType(tname);
        quint64 mask = 0;
        if (parser->isBasicType(tname)) {
            auto size = parser->getTypeSize(tname);
            Q_ASSERT(size && size.value() > 0);
            switch (size.value()) {
            case sizeof(quint8):
                mask = std::numeric_limits<quint8>::max();
                break;
            case sizeof(quint16):
                mask = std::numeric_limits<quint16>::max();
                break;
            case sizeof(quint32):
                mask = std::numeric_limits<quint32>::max();
                break;
            case sizeof(quint64):
                mask = std::numeric_limits<quint64>::max();
                break;
            default: {
                auto t = ctx->typeName()->start;
                errlis->reportError(
                    t->getLine(), t->getCharPositionInLine(),
                    QStringLiteral("Invalid typeName Mask with converting"));
                return defaultResult();
            }
            }
        } else {
            if (!parser->containsEnum(tname)) {
                auto t = ctx->typeName()->start;
                errlis->reportError(
                    t->getLine(), t->getCharPositionInLine(),
                    QStringLiteral("Can not convert Interger to \"%1\"")
                        .arg(tname));
                return defaultResult();
            }
            mask = std::numeric_limits<int>::max();
        }

        auto r = visitCastExpression(ctx->castExpression());
        if (r.has_value()) {
            if (r.type() == typeid(qint64)) {
                auto v = std::any_cast<qint64>(r) & mask;
                if (isUnsigned) {
                    return v;
                } else {
                    return qint64(v);
                }
            } else if (r.type() == typeid(quint64)) {
                auto v = std::any_cast<quint64>(r) & mask;
                if (isUnsigned) {
                    return v;
                } else {
                    return qint64(v);
                }
            }
        }
    } else if (ctx->unaryExpression()) {
        return visitUnaryExpression(ctx->unaryExpression());
    }

    return defaultResult();
}

std::any CStructVisitorParser::visitUnaryExpression(
    CStructParser::UnaryExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    auto minCount = ctx->MinusMinus().size();
    auto addCount = ctx->PlusPlus().size();

    quint64 leftCount;
    bool isAdd;
    if (addCount >= minCount) {
        isAdd = true;
        leftCount = addCount - minCount;
    } else {
        isAdd = false;
        leftCount = minCount - addCount;
    }

    if (ctx->postfixExpression()) {
        return visitPostfixExpression(ctx->postfixExpression());
    } else {
        auto tryIncOverflow = [](bool isAdd, quint64 leftCount, quint64 v,
                                 quint64 &r) {
            return isAdd && qAddOverflow(v, leftCount, &r);
        };
        auto tryDecOverflow = [](bool isAdd, quint64 leftCount, quint64 v,
                                 quint64 &r) {
            return !isAdd && qSubOverflow(v, leftCount, &r);
        };

        auto op = ctx->unaryOperator();
        auto r = visitCastExpression(ctx->castExpression());
        if (r.type() == typeid(quint64)) {
            auto v = std::any_cast<quint64>(r);
            if (op->Minus()) {
                auto sym = op->Minus()->getSymbol();
                if (v > std::numeric_limits<qint64>::max()) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QStringLiteral("make unsigned number negative"));
                }
                v = -v;
                quint64 r;
                sym = ctx->getStart();
                if (tryIncOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v + leftCount;
                } else if (tryDecOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v - leftCount;
                } else {
                    return r;
                }
            } else if (op->Plus()) {
                quint64 r;
                auto sym = ctx->getStart();
                if (tryIncOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v + leftCount;
                } else if (tryDecOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v - leftCount;
                } else {
                    return r;
                }
            } else if (op->Tilde()) {
                v = ~v;
                quint64 r;
                auto sym = ctx->getStart();
                if (tryIncOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v + leftCount;
                } else if (tryDecOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v - leftCount;
                } else {
                    return r;
                }
            } else {
                auto t = op->start;
                reportUnexpectedToken(t->getLine(), t->getCharPositionInLine(),
                                      QString::fromStdString(op->getText()),
                                      {R"("+")", R"("-")", R"("~")"});
                return defaultResult();
            }
        } else if (r.type() == typeid(qint64)) {
            auto v = std::any_cast<qint64>(r);
            auto sym = ctx->getStart();
            auto rv = qint64(leftCount);
            if (rv < 0) {
                reportOverflowWarn(
                    sym->getLine(), sym->getCharPositionInLine(),
                    QStringLiteral("too many incremental operation"));
            }

            auto tryIncOverflow = [](bool isAdd, qint64 leftCount, qint64 v,
                                     qint64 &r) {
                return isAdd && qAddOverflow(v, leftCount, &r);
            };
            auto tryDecOverflow = [](bool isAdd, qint64 leftCount, qint64 v,
                                     qint64 &r) {
                return !isAdd && qSubOverflow(v, leftCount, &r);
            };

            if (op->Minus()) {
                qint64 r;
                v = -v;
                if (tryIncOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v + leftCount;
                } else if (tryDecOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v - leftCount;
                } else {
                    return r;
                }
            } else if (op->Plus()) {
                qint64 r;
                if (tryIncOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v + leftCount;
                } else if (tryDecOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v - leftCount;
                } else {
                    return r;
                }
            } else if (op->Tilde()) {
                v = ~v;
                qint64 r;
                auto sym = ctx->getStart();
                if (tryIncOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v + leftCount;
                } else if (tryDecOverflow(isAdd, leftCount, v, r)) {
                    reportOverflowWarn(
                        sym->getLine(), sym->getCharPositionInLine(),
                        QString::number(v) + QStringLiteral("(++/--): ") +
                            QString::number(leftCount));
                    return v - leftCount;
                } else {
                    return r;
                }
            } else {
                auto t = op->start;
                reportUnexpectedToken(t->getLine(), t->getCharPositionInLine(),
                                      QString::fromStdString(op->getText()),
                                      {R"("+")", R"("-")", R"("~")"});
                return defaultResult();
            }
        } else {
            auto tk = ctx->castExpression()->start;
            reportUnexpectedType(tk->getLine(), tk->getCharPositionInLine(),
                                 QStringLiteral("<unknown>"),
                                 {"uint64", "int64"});
        }
    }

    return visitChildren(ctx);
}

std::any CStructVisitorParser::visitPostfixExpression(
    CStructParser::PostfixExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    auto r = visitPrimaryExpression(ctx->primaryExpression());

    auto minCount = ctx->MinusMinus().size();
    auto addCount = ctx->PlusPlus().size();

    quint64 leftCount;
    bool isAdd;
    if (addCount >= minCount) {
        isAdd = true;
        leftCount = addCount - minCount;
    } else {
        isAdd = false;
        leftCount = minCount - addCount;
    }

    auto sym = ctx->getStart();
    if (r.type() == typeid(quint64)) {
        auto v = std::any_cast<quint64>(r);
        quint64 r;
        if (isAdd && qAddOverflow(v, leftCount, &r)) {
            reportOverflowWarn(sym->getLine(), sym->getCharPositionInLine(),
                               QString::number(v) +
                                   QStringLiteral("(++/--): ") +
                                   QString::number(leftCount));
            return v + leftCount;
        } else if (!isAdd && qSubOverflow(v, leftCount, &r)) {
            reportOverflowWarn(sym->getLine(), sym->getCharPositionInLine(),
                               QString::number(v) +
                                   QStringLiteral("(++/--): ") +
                                   QString::number(leftCount));
            return v - leftCount;
        } else {
            return v;
        }
    } else if (r.type() == typeid(qint64)) {
        auto v = std::any_cast<qint64>(r);
        auto rv = qint64(leftCount);
        qint64 r;
        if (rv < 0) {
            reportOverflowWarn(
                sym->getLine(), sym->getCharPositionInLine(),
                QStringLiteral("too many incremental operation"));
        }
        if (isAdd && qAddOverflow(v, rv, &r)) {
            reportOverflowWarn(sym->getLine(), sym->getCharPositionInLine(),
                               QString::number(v) +
                                   QStringLiteral("(++/--): ") +
                                   QString::number(leftCount));
            return qint64(v + rv);
        } else if (!isAdd && qSubOverflow(v, rv, &r)) {
            reportOverflowWarn(sym->getLine(), sym->getCharPositionInLine(),
                               QString::number(v) +
                                   QStringLiteral("(++/--): ") +
                                   QString::number(leftCount));
            return qint64(v - rv);
        } else {
            return r;
        }
    } else {
        auto tk = ctx->primaryExpression()->start;
        reportUnexpectedType(tk->getLine(), tk->getCharPositionInLine(),
                             QStringLiteral("<unknown>"), {"uint64", "int64"});
        return defaultResult();
    }
}

std::any CStructVisitorParser::visitPrimaryExpression(
    CStructParser::PrimaryExpressionContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    if (ctx->IntegerConstant()) {
        auto t = ctx->IntegerConstant();
        auto num = t->getText();
        auto r = parseIntegerConstant(num);
        if (std::holds_alternative<quint64>(r)) {
            return std::get<quint64>(r);
        } else if (std::holds_alternative<qint64>(r)) {
            return std::get<qint64>(r);
        } else {
            auto tk = t->getSymbol();
            reportNumOutofRangeError(tk->getLine(), tk->getCharPositionInLine(),
                                     QString::fromStdString(num));
            return defaultResult();
        }
    } else if (ctx->Identifier()) {
        auto t = ctx->Identifier();
        auto vname = QString::fromStdString(t->getText());
        if (parser->containsConstVar(vname)) {
            auto v = parser->constVarValue(vname);
            if (std::holds_alternative<qint64>(v)) {
                return std::get<qint64>(v);
            } else if (std::holds_alternative<quint64>(v)) {
                return std::get<quint64>(v);
            } else {
                auto tk = t->getSymbol();
                reportUnexpectedType(tk->getLine(), tk->getCharPositionInLine(),
                                     QStringLiteral("<unknown>"),
                                     {"uint64", "int64"});
                return defaultResult();
            }
        }
    } else if (ctx->assignmentExpression()) {
        return visitAssignmentExpression(ctx->assignmentExpression());
    } else if (ctx->SizeOf()) {
        auto type = ctx->specifierQualifierList();
        auto spec = getSpecifier(type);
        if (spec) {
            auto v = parser->getTypeSize(spec->tname);
            if (v) {
                return v.value();
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    return defaultResult();
}

std::any CStructVisitorParser::visitStructOrUnionSpecifier(
    CStructParser::StructOrUnionSpecifierContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }
    if (ctx->structDeclarationList()) {
        auto decl = parseStructOrUnion(ctx);
        if (decl) {
            auto sym = ctx->getStart();
            reportCTypeError(
                sym->getLine(), sym->getCharPositionInLine(),
                parser->defineStructOrUnion(decl->isStruct, decl->name,
                                            decl->members, decl->alignment),
                decl->name);
        }
    } else {
        if (ctx->LeftBrace()) {
            auto sym = ctx->getStart();

            QString decl_name;
            auto iden = ctx->Identifier();
            if (iden) {
                decl_name = QString::fromStdString(iden->getText());
            } else {
                decl_name =
                    QString::number(parser->generateAnomyID()).prepend('?');
            }

            reportCTypeError(
                sym->getLine(), sym->getCharPositionInLine(),
                parser->defineStructOrUnion(ctx->structOrUnion()->Struct(),
                                            decl_name, {}, 1),
                decl_name);
        } else {
            auto iden = ctx->Identifier();
            Q_ASSERT(iden);
            auto sym = iden->getSymbol();
            auto name = QString::fromStdString(iden->getText());
            if (ctx->structOrUnion()->Struct()) {
                reportCTypeError(sym->getLine(), sym->getCharPositionInLine(),
                                 parser->addForwardStruct(name), name);
            } else {
                reportCTypeError(sym->getLine(), sym->getCharPositionInLine(),
                                 parser->addForwardUnion(name), name);
            }
        }
    }
    return defaultResult();
}

std::any
CStructVisitorParser::visitDefineDecl(CStructParser::DefineDeclContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    std::string def;

    if (ctx->DirectiveInclude()) {
        auto tdinc = ctx->DirectiveInclude();
        def = tdinc->getText();
        auto t = tdinc->getSymbol();

        constexpr auto prefixLen = std::char_traits<char>::length("#include");
        antlr4::ANTLRInputStream input(def.data() + prefixLen,
                                       def.length() - prefixLen);
        CStructLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);

        QScopeGuard guard([this]() { errlis->resetPositionOffset(); });
        errlis->setPositionOffset(t->getLine() - 1, prefixLen);

        tokens.fill();

        if (tokens.size() == 2) {
            if (tokens.get(1)->getType() != CStructLexer::EOF) {
                return defaultResult();
            }

            auto t = tokens.get(0);
            if (t->getType() == CStructLexer::StringLiteral) {
                auto path = QString::fromStdString(t->getText())
                                .removeFirst()
                                .removeLast();

                auto p = tdinc->getSymbol()->getTokenSource()->getSourceName();

                QDir dir;
                if (p.empty()) {
                    dir.setPath(qApp->applicationDirPath());
                } else {
                    QFileInfo pinfo(QString::fromStdString(p));
                    dir = pinfo.absoluteDir();
                }

                auto header = dir.absoluteFilePath(path);
                if (QFile::exists(header)) {
                    if (!parser->parse(header)) {
                        errlis->reportWarn(
                            t->getLine(), t->getCharPositionInLine(),
                            QStringLiteral("Invalid #include marco failed"));
                    }
                    return defaultResult();
                } else {
                    errlis->reportWarn(
                        t->getLine(), t->getCharPositionInLine(),
                        QStringLiteral(
                            "Invalid #include file is not exists: ") +
                            header.prepend('"').append('"'));
                }
            }
        }

        // report warning and ignored
        errlis->reportWarn(t->getLine(), t->getCharPositionInLine(),
                           QStringLiteral("Invalid #include marco syntax"));
    } else {
        antlr4::Token *t = nullptr;

        if (ctx->DirectiveDefine()) {
            auto ex = ctx->DirectiveDefine();
            t = ex->getSymbol();
            def = ex->getText();
        } else if (ctx->MultiLineMacroDefine()) {
            auto ex = ctx->MultiLineMacroDefine();
            t = ex->getSymbol();
            def = ex->getText();

            size_t pos = 0;
            constexpr auto es = "\\\n";
            constexpr auto esLen = std::char_traits<char>::length(es);
            auto strp = def.find(es);
            while (strp != std::string::npos) {
                def.replace(strp, esLen, es);
                strp += esLen;
                strp = def.find(es, strp);
            }
        }

        if (t == nullptr) {
            return defaultResult();
        }

        constexpr auto prefixLen = std::char_traits<char>::length("#define");
        antlr4::ANTLRInputStream input(def.data() + prefixLen,
                                       def.length() - prefixLen);
        CStructLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);

        QScopeGuard guard([this]() { errlis->resetPositionOffset(); });
        errlis->setPositionOffset(t->getLine() - 1, prefixLen);

        tokens.fill();
        auto ts = tokens.getTokens();
        t = ts.front();
        auto identifer = t->getText();

        auto dname = QString::fromStdString(identifer);
        if (parser->containsType(dname) || parser->containsConstVar(dname)) {
            // report error
            reportDupDeclError(t->getLine(), t->getCharPositionInLine(), dname);
            return defaultResult();
        }

        tokens.consume(); // remove the first token

        CStructVisitorParser visitor(parser, errlis);
        CStructParser parser(&tokens);

        auto ret = visitor.visit(parser.assignmentExpressionDef());
        if (ret.type() == typeid(quint64)) {
            this->parser->defineConstVar(dname, std::any_cast<quint64>(ret));
        } else if (ret.type() == typeid(qint64)) {
            this->parser->defineConstVar(dname, std::any_cast<qint64>(ret));
        } else {
            reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                 QStringLiteral("<unknown>"),
                                 {"uint64", "int64"});
        }
    }

    return defaultResult();
}

std::any CStructVisitorParser::visitAssignmentExpressionDef(
    CStructParser::AssignmentExpressionDefContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    if (ctx->assignmentExpression()) {
        return visitAssignmentExpression(ctx->assignmentExpression());
    }

    return defaultResult();
}

std::any
CStructVisitorParser::visitDeclaration(CStructParser::DeclarationContext *ctx) {
    if (ctx == nullptr) {
        return defaultResult();
    }

    if (ctx->TypeDef()) {
        auto ex = ctx->Identifier();
        auto iden = QString::fromStdString(ex->getText());

        if (parser->containsType(iden) || parser->containsConstVar(iden)) {
            // error report
            auto t = ex->getSymbol();
            reportDupDeclError(t->getLine(), t->getCharPositionInLine(), iden);
            return defaultResult();
        }

        auto spec = getSpecifier(ctx->typeSpecifier());
        if (spec) {
            parser->defineTypedef(iden, spec->tname, ctx->pointer() != nullptr);
        }
        return defaultResult();
    } else {
        return visitChildren(ctx);
    }
}

std::optional<CStructVisitorParser::Specifier>
CStructVisitorParser::getSpecifier(CStructParser::TypeSpecifierContext *ctx,
                                   int alignment) {
    if (ctx == nullptr) {
        return std::nullopt;
    }

    Specifier sq;

    if (ctx->Void()) {
        sq.type = StructMemType::Normal;
        sq.tname = QStringLiteral("void");
    } else if (ctx->internalBasicTypes()) {
        auto btype = ctx->internalBasicTypes();
        sq.type = StructMemType::Normal;

        if (btype->Char()) {
            sq.tname = QStringLiteral("char");

            auto sous = btype->signOrUnsigned();
            if (sous) {
                if (sous->Unsigned()) {
                    sq.tname = QStringLiteral("uchar");
                }
            }
        } else if (btype->Short()) {
            sq.tname = QStringLiteral("short");

            auto sous = btype->signOrUnsigned();
            if (sous) {
                if (sous->Unsigned()) {
                    sq.tname = QStringLiteral("ushort");
                }
            }
        } else if (btype->Int()) {
            sq.tname = QStringLiteral("int");

            auto sous = btype->signOrUnsigned();
            if (sous) {
                if (sous->Unsigned()) {
                    sq.tname = QStringLiteral("uint");
                }
            }
        } else if (btype->Float()) {
            sq.tname = QStringLiteral("float");
        } else if (btype->Double()) {
            sq.tname = QStringLiteral("double");
        } else {
            // for long or longlong
            auto len = btype->Long().size();
            auto sous = btype->signOrUnsigned();
            if (sous) {
                switch (len) {
                case 1:
                    sq.tname = QStringLiteral("long");
                    break;
                case 2:
                    sq.tname = QStringLiteral("longlong");
                    break;
                default: {
                    auto t = ctx->start;
                    reportSyntaxDeclError(t->getLine(),
                                          t->getCharPositionInLine());
                    return std::nullopt;
                }
                }
            } else {
                switch (len) {
                case 1:
                    sq.tname = QStringLiteral("ulong");
                    break;
                case 2:
                    sq.tname = QStringLiteral("ulonglong");
                    break;
                default: {
                    auto t = ctx->start;
                    reportSyntaxDeclError(t->getLine(),
                                          t->getCharPositionInLine());
                    return std::nullopt;
                }
                }
            }
        }
    } else if (ctx->Identifier()) {
        sq.type = StructMemType::Normal;
        sq.tname = QString::fromStdString(ctx->Identifier()->getText());
    } else if (ctx->enumSpecifier()) {
        sq.type = StructMemType::Enum;

        auto es = ctx->enumSpecifier();
        auto iden = es->Identifier();
        if (es->enumeratorList()) {
            auto en = parseEnum(es);
            if (en) {
                if (en->first.isEmpty()) {
                    // anonymous? I will give you a name!
                    en->first =
                        QString::number(parser->generateAnomyID()).prepend('?');
                }
                sq.tname = en->first;

                // store it
                auto sym = iden->getSymbol();
                auto ret = reportCTypeError(
                    sym->getLine(), sym->getCharPositionInLine(),
                    parser->defineEnum(en->first, en->second), en->first);
                if (ret != CTypeParser::StructResult::Ok) {
                    return std::nullopt;
                }
            } else {
                // error occurred, reported in parseEnum
                return std::nullopt;
            }
        } else {
            // iden can not be nullptr
            sq.tname = QString::fromStdString(iden->getText());
        }
    } else if (ctx->structOrUnionSpecifier()) {
        sq.type = StructMemType::Struct;

        auto sus = ctx->structOrUnionSpecifier();
        auto iden = sus->Identifier();
        if (sus->structDeclarationList()) {
            auto st = parseStructOrUnion(sus, alignment);
            if (st) {
                if (st->name.isEmpty()) {
                    st->name =
                        QString::number(parser->generateAnomyID()).prepend('?');
                }

                sq.tname = st->name;
                // store it
                auto sym = sus->getStart();
                auto ret = reportCTypeError(
                    sym->getLine(), sym->getCharPositionInLine(),
                    parser->defineStructOrUnion(st->isStruct, st->name,
                                                st->members, st->alignment),
                    sq.tname);
                if (ret != CTypeParser::StructResult::Ok) {
                    return std::nullopt;
                }
            } else {
                // error occurred, reported in parseStructOrUnion
                return std::nullopt;
            }
        } else {
            // iden can not be nullptr
            sq.tname = QString::fromStdString(iden->getText());
        }
    } else {
        auto sym = ctx->getStart();
        errlis->reportError(sym->getLine(), sym->getCharPositionInLine(),
                            QStringLiteral("Unknown type of specifier"));
        return std::nullopt;
    }

    return sq;
}

QString CStructVisitorParser::getFinalDeclaratorName(
    CStructParser::DirectDeclaratorContext *ctx) {
    while (ctx) {
        if (ctx->Identifier()) {
            return QString::fromStdString(ctx->Identifier()->getText());
        } else if (ctx->declarator()) {
            ctx = ctx->declarator()->directDeclarator();
            continue;
        }
        ctx = ctx->directDeclarator();
    }
    return {};
}

std::optional<qint64>
CStructVisitorParser::safeMultiply(const QVector<qint64> &vec) {
    if (vec.isEmpty()) {
        return 0;
    }

    qint64 result = 1;
    for (const auto &value : vec) {
        qint64 temp;
        if (qMulOverflow(result, value, &temp)) {
            return std::nullopt;
        }
        result = temp;
    }
    return result;
}

std::optional<CStructVisitorParser::Declarator>
CStructVisitorParser::getDeclarator(
    CStructParser::DirectDeclaratorContext *ctx) {
    if (!ctx) {
        return std::nullopt;
    }

    Declarator dor;

    if (ctx->Identifier()) {
        // Identifier only
        dor.retName = QString::fromStdString(ctx->Identifier()->getText());
    } else if (ctx->QuestionMark()) {
        dor.retName.clear();
    } else if (ctx->declarator()) {
        auto d = ctx->declarator();

        // only pointer in the first level is valid member in struct
        if (d->pointer()) {
            dor.retName = getFinalDeclaratorName(d->directDeclarator());
            dor.isPointer = true;
        } else {
            auto sym = d->getStart();
            reportSyntaxDeclError(sym->getLine(), sym->getCharPositionInLine());
            return std::nullopt;
        }
    } else if (ctx->assignmentExpression()) {
        // array
        auto sym = ctx->assignmentExpression();
        auto r = visitAssignmentExpression(sym);
        if (r.type() == typeid(quint64)) {
            auto v = std::any_cast<quint64>(r);
            if (v > std::numeric_limits<qint64>::max()) {
                auto t = sym->getStart();
                reportOutofRangeError(t->getLine(), t->getCharPositionInLine());
                return std::nullopt;
            }
            dor.arrayCount = v;
        } else if (r.type() == typeid(qint64)) {
            auto v = std::any_cast<qint64>(r);
            if (v < 0) {
                auto t = sym->getStart();
                reportOutofRangeError(t->getLine(), t->getCharPositionInLine());
            }
            dor.arrayCount = v;
        } else {
            auto t = sym->getStart();
            reportUnexpectedType(t->getLine(), t->getCharPositionInLine(),
                                 QStringLiteral("<unknown>"),
                                 {"uint64", "int64"});
            return std::nullopt;
        }
        dor.next = ctx->directDeclarator();
    } else {
        auto sym = ctx->getStart();
        errlis->reportError(sym->getLine(), sym->getCharPositionInLine(),
                            QStringLiteral("Unknown type of declarator"));
        return std::nullopt;
    }

    return dor;
}

std::optional<CStructVisitorParser::Specifier>
CStructVisitorParser::getSpecifier(
    CStructParser::SpecifierQualifierListContext *ctx, int alignment) {
    if (!ctx) {
        return std::nullopt;
    }
    auto spec = ctx->typeSpecifier();
    return getSpecifier(spec, alignment);
}

std::optional<StructUnionDecl> CStructVisitorParser::parseStructOrUnion(
    CStructParser::StructOrUnionSpecifierContext *ctx, int alignment) {
    if (ctx == nullptr) {
        return std::nullopt;
    }

    StructUnionDecl decl;
    decl.isStruct = ctx->structOrUnion()->Struct();

    auto mems = ctx->structDeclarationList();
    Q_ASSERT(mems);

    auto name = ctx->Identifier();
    if (name) {
        auto decl_name = QString::fromStdString(name->getText());
        decl.name = decl_name;
        if (parser->containsType(decl.name) ||
            parser->containsConstVar(decl.name)) {
            auto sym = name->getSymbol();
            reportDupDeclError(sym->getLine(), sym->getCharPositionInLine(),
                               decl_name);
            return std::nullopt;
        }
    }

    if (ctx->alignAsAttr()) {
        auto integer = ctx->alignAsAttr()->IntegerConstant();
        auto numstr = integer->getText();
        auto num = parseIntegerConstant(numstr);

        int v = 0;
        constexpr auto lmax = std::numeric_limits<int>::max();
        if (std::holds_alternative<quint64>(num)) {
            auto r = std::get<quint64>(num);
            if (r > lmax) {
                v = 0;
            } else {
                v = r;
            }
        } else if (std::holds_alternative<qint64>(num)) {
            auto r = std::get<qint64>(num);
            if (r > lmax) {
                v = 0;
            } else {
                v = r;
            }
        } else {
            v = 0;
        }

        if (v) {
            static std::array<int, 5> allowList{1, 2, 4, 8, 16};
            if (std::find(allowList.begin(), allowList.end(), v) !=
                allowList.end()) {
                decl.alignment = v;
            }
        } else {
            // report warning and ignored
            auto sym = integer->getSymbol();
            errlis->reportWarn(
                sym->getLine(), sym->getCharPositionInLine(),
                QStringLiteral("Unsupported alignas value: %1 , "
                               "supported alignas = {1, 2, 4, 8, 16}")
                    .arg(QString::fromStdString(numstr)));
            decl.alignment = alignment > 0 ? alignment : parser->padAlignment();
        }
    } else {
        decl.alignment = alignment > 0 ? alignment : parser->padAlignment();
    }

    QStringList used_names;

    for (auto &m : mems->structDeclaration()) {
        auto dl = getSpecifier(m->specifierQualifierList(), decl.alignment);
        if (!dl) {
            return std::nullopt;
        }

        // cannot be enum
        if (parser->resolveType(dl->tname) == CTypeParser::CType::Enum) {
            auto sym = m->getStart();
            errlis->reportWarn(
                sym->getLine(), sym->getCharPositionInLine(),
                QStringLiteral("The enum type in struct or union is not "
                               "allowed and will be treated as int type"));
            dl->tname = QStringLiteral("int");
            dl->type = StructMemType::Normal;
        }

        auto fmtdecl = m->basicTypeFmt();
        QString fmttype;
        if (fmtdecl) {
            if (isInteger(dl->tname)) {
                fmttype =
                    QString::fromStdString(fmtdecl->Identifier()->getText());
                if (parser->containsType(fmttype)) {
                    if (!parser->containsEnum(fmttype)) {
                        auto sym = fmtdecl->getStart();
                        errlis->reportWarn(
                            sym->getLine(), sym->getCharPositionInLine(),
                            QStringLiteral(
                                "The type in [[FMT_TYPE]] should be an"
                                "enum type"));
                        fmttype.clear();
                    }
                } else {
                    parser->addForwardEnum(fmttype);
                }
            } else {
                auto sym = m->getStart();
                errlis->reportWarn(
                    sym->getLine(), sym->getCharPositionInLine(),
                    QStringLiteral("The type with [[FMT_TYPE]] should be "
                                   "completed basic numberic"));
            }
        }

        auto dclist = m->structDeclaratorList();
        if (dclist) {
            auto sd = dclist->structDeclarator();
            if (sd.size() >= std::numeric_limits<qsizetype>::max()) {
                auto sym = dclist->getStart();
                reportCTypeError(sym->getLine(), sym->getCharPositionInLine(),
                                 CTypeParser::StructResult::CountOfLimit,
                                 decl.name);
                return std::nullopt;
            }

            for (auto &sub : sd) {
                VariableDeclaration var;
                var.data_type = dl->tname;
                var.fmt_type = fmttype;

                auto declor = sub->declarator();
                if (declor) {
                    // bit field
                    if (sub->Colon()) {
                        auto sym = m->getStart();

                        if (!isInteger(dl->tname)) {
                            errlis->reportError(
                                sym->getLine(), sym->getCharPositionInLine(),
                                QStringLiteral("Only bit field feature of "
                                               "completed numberic types "
                                               "is supported"));
                            return std::nullopt;
                        }

                        auto tbits = parser->getTypeSize(dl->tname).value() * 8;
                        auto bits = visitAssignmentExpression(
                            sub->assignmentExpression());
                        if (bits.type() == typeid(quint64)) {
                            auto b = std::any_cast<quint64>(bits);
                            if (b > tbits || b == 0 ||
                                b > std::numeric_limits<qint64>::max()) {
                                reportFiledBitOverflow(
                                    sym->getLine(),
                                    sym->getCharPositionInLine(), dl->tname, b);
                                return std::nullopt;
                            }
                            var.bit_size = b;
                        } else if (bits.type() == typeid(qint64)) {
                            auto b = std::any_cast<qint64>(bits);
                            if (b > tbits || b <= 0) {
                                reportFiledBitOverflow(
                                    sym->getLine(),
                                    sym->getCharPositionInLine(), dl->tname, b);
                                return std::nullopt;
                            }
                            var.bit_size = b;
                        } else {
                            reportUnexpectedType(sym->getLine(),
                                                 sym->getCharPositionInLine(),
                                                 QStringLiteral("<unknown>"),
                                                 {"uint64", "int64"});
                            return std::nullopt;
                        }
                    }

                    auto d = declor->directDeclarator();
                    if (declor->pointer() && !sub->Colon()) {
                        var.is_pointer = true;
                        var.var_name = getFinalDeclaratorName(d);
                        if (used_names.contains(var.var_name) ||
                            parser->containsConstVar(var.var_name)) {
                            auto sym = d->getStart();
                            reportDupError(sym->getLine(),
                                           sym->getCharPositionInLine(),
                                           var.var_name);
                            return std::nullopt;
                        }

                        decl.members.append(var);

                        if (!var.var_name.isEmpty()) {
                            used_names.append(var.var_name);
                        }
                    } else {
                        auto info = getDeclarator(d);
                        if (info) {
                            // array or pointer is not allowed
                            if (var.bit_size) {
                                if (info->isPointer ||
                                    d->assignmentExpression()) {
                                    auto sym = d->getStart();
                                    errlis->reportError(
                                        sym->getLine(),
                                        sym->getCharPositionInLine(),
                                        QStringLiteral(
                                            "Only bit field feature of "
                                            "completed numberic types "
                                            "is supported"));
                                    return std::nullopt;
                                }
                            }

                            var.is_pointer = info->isPointer;
                            if (info->arrayCount == 0) {
                                var.array_dims = {0};
                                decl.members.append(var);
                                used_names.append(var.var_name);
                                continue; // no nessesary to deep parse
                            }
                            auto nestedInfo = getDeclarator(info->next);

                            QVector<qint64> dims;
                            dims.append(info->arrayCount);

                            if (nestedInfo) {
                                while (nestedInfo &&
                                       nestedInfo->arrayCount > 1 &&
                                       nestedInfo->next) {
                                    dims.prepend(nestedInfo->arrayCount);
                                    nestedInfo =
                                        getDeclarator(nestedInfo->next);
                                }
                                var.var_name = getFinalDeclaratorName(d);
                                if (used_names.contains(var.var_name) ||
                                    parser->containsConstVar(var.var_name)) {
                                    auto sym = d->getStart();
                                    reportDupError(sym->getLine(),
                                                   sym->getCharPositionInLine(),
                                                   var.var_name);
                                    return std::nullopt;
                                }
                                var.array_dims = dims;
                            } else {
                                var.var_name = info->retName;
                                if (used_names.contains(var.var_name) ||
                                    parser->containsConstVar(var.var_name)) {
                                    auto sym = d->getStart();
                                    reportDupError(sym->getLine(),
                                                   sym->getCharPositionInLine(),
                                                   var.var_name);
                                    return std::nullopt;
                                }
                            }

                            auto ec = safeMultiply(dims);
                            if (ec) {
                                var.element_count = ec.value();
                            } else {
                                auto sym = d->getStart();
                                reportArrayOutofLimit(
                                    sym->getLine(),
                                    sym->getCharPositionInLine(),
                                    (var.var_name.isEmpty()
                                         ? QStringLiteral("?")
                                         : var.var_name),
                                    decl.name);
                                return std::nullopt;
                            }

                            decl.members.append(var);

                            if (!var.var_name.isEmpty()) {
                                used_names.append(var.var_name);
                            }
                        } else {
                            // error occurred, reported in getDeclarator
                            return std::nullopt;
                        }
                    }
                } else {
                    // must be bit field
                    var.var_name.clear();
                    auto sym = m->getStart();
                    if (!isInteger(dl->tname)) {
                        errlis->reportError(
                            sym->getLine(), sym->getCharPositionInLine(),
                            QStringLiteral("Only bit field feature of "
                                           "completed numberic types "
                                           "is supported"));
                        return std::nullopt;
                    }

                    auto tbits = parser->getTypeSize(dl->tname).value() * 8;
                    auto bits =
                        visitAssignmentExpression(sub->assignmentExpression());
                    if (bits.type() == typeid(quint64)) {
                        auto b = std::any_cast<quint64>(bits);
                        if (b > tbits || b == 0) {
                            reportFiledBitOverflow(sym->getLine(),
                                                   sym->getCharPositionInLine(),
                                                   dl->tname, b);
                            return std::nullopt;
                        }
                        var.bit_size = b;
                    } else if (bits.type() == typeid(qint64)) {
                        auto b = std::any_cast<qint64>(bits);
                        if (b > tbits || b <= 0) {
                            reportFiledBitOverflow(sym->getLine(),
                                                   sym->getCharPositionInLine(),
                                                   dl->tname, b);
                            return std::nullopt;
                        }
                        var.bit_size = b;
                    } else {
                        reportUnexpectedType(
                            sym->getLine(), sym->getCharPositionInLine(),
                            QStringLiteral("<unknown>"), {"uint64", "int64"});
                    }
                    decl.members.append(var);
                }
            }
        }
    }

    return decl;
}

std::optional<CStructVisitorParser::EnumDecl>
CStructVisitorParser::parseEnum(CStructParser::EnumSpecifierContext *ctx) {
    if (ctx == nullptr) {
        return std::nullopt;
    }

    auto name = ctx->Identifier();
    auto enums = ctx->enumeratorList();
    Q_ASSERT(enums);

    QString decl_name;
    QHash<QString, std::variant<qint64, quint64>> decl_enums;

    if (name) {
        decl_name = QString::fromStdString(name->getText());
        if (parser->containsType(decl_name) ||
            parser->containsConstVar(decl_name)) {
            auto sym = name->getSymbol();
            reportDupDeclError(sym->getLine(), sym->getCharPositionInLine(),
                               decl_name);
            return std::nullopt;
        }
    }

    std::variant<qint64, quint64> i;

    auto es = enums->enumerator();
    if (es.size() >= std::numeric_limits<qsizetype>::max()) {
        auto sym = enums->getStart();
        reportCTypeError(sym->getLine(), sym->getCharPositionInLine(),
                         CTypeParser::StructResult::CountOfLimit, decl_name);
        return std::nullopt;
    }
    for (auto &e : es) {
        QString name;

        auto en = e->enumerationConstant();
        if (en) {
            auto sym = en->Identifier()->getSymbol();
            name = QString::fromStdString(sym->getText());
            if (decl_enums.contains(name)) {
                reportDupError(sym->getLine(), sym->getCharPositionInLine(),
                               name);
                return std::nullopt;
            }
        } else {
            auto sym = enums->getStart();
            reportSyntaxDeclError(sym->getLine(), sym->getCharPositionInLine());
            return std::nullopt;
        }

        auto value = e->assignmentExpression();
        if (value) {
            auto v = visitAssignmentExpression(value);
            if (v.type() == typeid(qint64)) {
                i = std::any_cast<qint64>(v);
            } else if (v.type() == typeid(quint64)) {
                i = std::any_cast<quint64>(v);
            } else {
                auto tk = value->getStart();
                reportUnexpectedType(tk->getLine(), tk->getCharPositionInLine(),
                                     QStringLiteral("<unknown>"),
                                     {"int64", "uint64"});
                return std::nullopt;
            }
        }

        decl_enums.insert(name, i);

        if (std::holds_alternative<quint64>(i)) {
            auto v = std::get<quint64>(i);
            i = v + 1;
        } else {
            auto v = std::get<qint64>(i);
            i = v + 1;
        }
    }

    return qMakePair(decl_name, decl_enums);
}
