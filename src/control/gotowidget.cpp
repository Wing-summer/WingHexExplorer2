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

#include "gotowidget.h"
#include "ui_gotowidget.h"

#include "grammar/NumCal/NumCalBaseVisitor.h"
#include "grammar/NumCal/NumCalLexer.h"
#include "grammar/NumCal/NumCalParser.h"

#include <QShortcut>

class Calculator : public NumCalBaseVisitor {
public:
    void eval(const QString &exp) {
        if (exp.isEmpty()) {
            lastPos = GotoWidget::SEEKPOS::Invaild;
            lastAddr = 0;
            return;
        }

        antlr4::ANTLRInputStream input(exp.toStdString());

        NumCalLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);

        NumCalParser parser(&tokens);
        parser.removeErrorListeners();
        parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

        try {
            visit(parser.entryExpression());
        } catch (...) {
        }
    }

public:
    virtual std::any
    visitEntryExpression(NumCalParser::EntryExpressionContext *ctx) override {
        if (ctx == nullptr) {
            lastPos = GotoWidget::SEEKPOS::Invaild;
            lastAddr = 0;
            return defaultResult();
        }

        if (ctx->prefixGoto() == nullptr) {
            lastPos = GotoWidget::SEEKPOS::Start;
        } else {
            auto prefix = ctx->prefixGoto();
            if (prefix->Plus()) {
                lastPos = GotoWidget::SEEKPOS::RelativeAdd;
            } else if (prefix->Minus()) {
                lastPos = GotoWidget::SEEKPOS::RelativeMin;
            } else if (prefix->LessThan()) {
                lastPos = GotoWidget::SEEKPOS::End;
            } else {
                lastPos = GotoWidget::SEEKPOS::Invaild;
                lastAddr = 0;
                return defaultResult();
            }
        }

        if (ctx->IntegerConstant()) {
            auto r = parseIntegerConstant(ctx->IntegerConstant()->getText());
            if (r) {
                lastAddr = r.value();
            } else {
                lastPos = GotoWidget::SEEKPOS::Invaild;
                lastAddr = 0;
            }
        } else {
            auto r = visitAssignmentExpression(ctx->assignmentExpression());
            if (r.has_value()) {
                auto addr = std::any_cast<qint64>(r);
                if (addr < 0) {
                    lastPos = GotoWidget::SEEKPOS::Invaild;
                    lastAddr = 0;
                }
                lastAddr = addr;
            } else {
                lastPos = GotoWidget::SEEKPOS::Invaild;
                lastAddr = 0;
            }
        }

        return defaultResult();
    }

public:
    qint64 lastAddr = 0;
    GotoWidget::SEEKPOS lastPos = GotoWidget::SEEKPOS::Invaild;

public:
    std::any
    visitCastExpression(NumCalParser::CastExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        if (ctx->IntegerConstant()) {
            auto r = parseIntegerConstant(ctx->IntegerConstant()->getText());
            if (r) {
                return r.value();
            }
        } else if (ctx->unaryExpression()) {
            return visitUnaryExpression(ctx->unaryExpression());
        }

        return defaultResult();
    }

    std::any
    visitUnaryExpression(NumCalParser::UnaryExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        auto op = ctx->unaryOperator();
        auto r = visitCastExpression(ctx->castExpression());
        if (r.has_value()) {
            auto v = std::any_cast<qint64>(r);
            if (op->Minus()) {
                return -v;
            } else if (op->Plus()) {
                return +v;
            } else if (op->Tilde()) {
                return ~v;
            } else {
                return defaultResult();
            }
        }

        return visitChildren(ctx);
    }

    std::any visitInclusiveOrExpression(
        NumCalParser::InclusiveOrExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        qint64 ret = 0;
        for (auto &v : ctx->exclusiveOrExpression()) {
            auto r = visitExclusiveOrExpression(v);
            if (r.has_value()) {
                auto rr = std::any_cast<qint64>(r);
                ret |= rr;
            } else {
                // error
                return defaultResult();
            }
        }
        return ret;
    }

    std::any visitAssignmentExpression(
        NumCalParser::AssignmentExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        if (ctx->IntegerConstant()) {
            auto r = parseIntegerConstant(ctx->IntegerConstant()->getText());
            if (r) {
                return r.value();
            }
        } else if (ctx->inclusiveOrExpression()) {
            return visitInclusiveOrExpression(ctx->inclusiveOrExpression());
        }

        return defaultResult();
    }

    std::any visitExclusiveOrExpression(
        NumCalParser::ExclusiveOrExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        qint64 v = 0;
        for (auto &ex : ctx->andExpression()) {
            auto r = visitAndExpression(ex);
            if (r.has_value()) {
                auto rv = std::any_cast<qint64>(r);
                v ^= rv;
            } else {
                return defaultResult();
            }
        }

        return v;
    }

    std::any
    visitAndExpression(NumCalParser::AndExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        quint64 v = std::numeric_limits<quint64>::max();
        for (auto &ex : ctx->shiftExpression()) {
            auto r = visitShiftExpression(ex);
            if (r.has_value()) {
                auto rv = std::any_cast<qint64>(r);
                v &= quint64(rv);
            } else {
                return defaultResult();
            }
        }
        return qint64(v);
    }

    std::any
    visitShiftExpression(NumCalParser::ShiftExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        auto data = ctx->additiveExpression();
        auto total = data.size();

        qint64 ret = 0;
        auto retv = visitAdditiveExpression(data.front());
        if (retv.has_value()) {
            ret = std::any_cast<qint64>(retv);
        } else {
            return defaultResult();
        }

        for (size_t i = 1; i < total; ++i) {
            auto op = ctx->children[2 * i - 1]->getText();
            auto r = visitAdditiveExpression(data.at(i));
            if (op == "<<") {
                if (r.has_value()) {
                    auto rv = std::any_cast<qint64>(r);
                    ret <<= rv;
                } else {
                    return defaultResult();
                }
            } else if (op == ">>") {
                if (r.has_value()) {
                    auto rv = std::any_cast<qint64>(r);
                    ret >>= rv;
                } else {
                    return defaultResult();
                }
            } else {
                return defaultResult();
            }
        }

        return ret;
    }

    std::any visitAdditiveExpression(
        NumCalParser::AdditiveExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        auto data = ctx->multiplicativeExpression();
        auto total = data.size();

        qint64 ret = 0;
        auto retv = visitMultiplicativeExpression(data.front());
        if (retv.has_value()) {
            ret = std::any_cast<qint64>(retv);
        } else {
            return defaultResult();
        }

        for (size_t i = 1; i < total; i++) {
            auto r = visitMultiplicativeExpression(data.at(i));
            auto op = ctx->children[2 * i - 1]->getText();
            if (r.has_value()) {
                auto rv = std::any_cast<qint64>(r);
                if (op == "+") {
                    ret += rv;
                } else if (op == "-") {
                    ret -= rv;
                } else {
                    return defaultResult();
                }
            } else {
                return defaultResult();
            }
        }

        return ret;
    }

    std::any visitMultiplicativeExpression(
        NumCalParser::MultiplicativeExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        auto data = ctx->castExpression();
        auto total = data.size();

        qint64 ret = 0;
        auto retv = visitCastExpression(data.front());
        if (retv.has_value()) {
            ret = std::any_cast<qint64>(retv);
        } else {
            return defaultResult();
        }

        for (size_t i = 1; i < total; i++) {
            auto r = visitCastExpression(data.at(i));
            auto op = ctx->children[2 * i - 1]->getText();
            if (r.has_value()) {
                auto rv = std::any_cast<qint64>(r);
                if (op == "*") {
                    ret *= rv;
                } else if (op == "/") {
                    ret /= rv;
                } else if (op == "%") {
                    ret %= rv;
                } else {
                    return defaultResult();
                }
            } else {
                return defaultResult();
            }
        }

        return ret;
    }

    std::any visitPrimaryExpression(
        NumCalParser::PrimaryExpressionContext *ctx) override {
        if (ctx == nullptr) {
            return defaultResult();
        }

        if (ctx->IntegerConstant()) {
            auto r = parseIntegerConstant(ctx->IntegerConstant()->getText());
            if (r) {
                return r.value();
            }
        } else if (ctx->assignmentExpression()) {
            return visitAssignmentExpression(ctx->assignmentExpression());
        }

        return defaultResult();
    }

private:
    std::optional<qint64> parseIntegerConstant(const std::string &text) {
        Q_STATIC_ASSERT_X(
            QT_VERSION >= QT_VERSION_CHECK(6, 4, 0),
            "If you want to support Qt version lower than 6.4.0, You should "
            "implement '0b' prefix integer converstion on your own!");

        bool b;
        auto ct = QString::fromStdString(text);
        auto num = ct.toLongLong(&b, 0);
        if (b) {
            return num;
        } else {
            return std::nullopt;
        }
    }
};

GotoWidget::GotoWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::GotoWidget) {
    ui->setupUi(this);
    auto sc = QKeySequence(Qt::Key_Escape);
    ui->btnClose->setShortcut(sc);

    connect(ui->lineEdit, &QLineEdit::returnPressed, this,
            &GotoWidget::jumpConfirm);
    connect(ui->lineEdit, &QLineEdit::textChanged, this,
            &GotoWidget::handleLineChanged);
    connect(ui->btnGoto, &QPushButton::clicked, this, &GotoWidget::jumpConfirm);
}

GotoWidget::~GotoWidget() { delete ui; }

void GotoWidget::activeInput(qsizetype oldrow, qsizetype oldcolumn,
                             qsizetype oldoffset, qsizetype maxfilebytes,
                             qsizetype maxfilelines) {
    m_rowBeforeJump = oldrow;
    m_columnBeforeJump = oldcolumn;
    m_oldFileOffsetBeforeJump = oldoffset;
    m_maxFileBytes = maxfilebytes;
    m_maxFilelines = maxfilelines;
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();
    show();
}

void GotoWidget::handleLineChanged() {
    QString content = ui->lineEdit->text();
    auto ps = SEEKPOS::Invaild;
    auto isline = ui->rbLine->isChecked();
    auto p = convert2Pos(content, ps, isline);
    if (ps != SEEKPOS::Invaild) {
        ui->lineEdit->setStyleSheet({});
    } else {
        ui->lineEdit->setStyleSheet(QStringLiteral("QLineEdit{color: red}"));
    }
    Q_EMIT jumpToLine(p, isline);
}

void GotoWidget::jumpCancel() {
    Q_EMIT jumpToLine(m_oldFileOffsetBeforeJump, false);
    hide();
}

void GotoWidget::jumpConfirm() {
    handleLineChanged();
    hide();
}

void GotoWidget::on_btnClose_clicked() { this->hide(); }

qsizetype GotoWidget::convert2Pos(const QString &value, SEEKPOS &ps,
                                  bool isline) {
    Calculator cal;
    cal.eval(value);

    // you should ensure return value is origin position if invalid

    qsizetype res = cal.lastAddr;
    ps = cal.lastPos;
    switch (cal.lastPos) {
    case SEEKPOS::Invaild:
        res = isline ? qsizetype(m_rowBeforeJump)
                     : qsizetype(m_oldFileOffsetBeforeJump);
        break;
    case SEEKPOS::Start:
        if (res < 0 || quint64(res) > (isline ? quint64(m_maxFilelines)
                                              : m_maxFileBytes)) {
            ps = SEEKPOS::Invaild;
            res = isline ? qsizetype(m_rowBeforeJump)
                         : qsizetype(m_oldFileOffsetBeforeJump);
        }
        break;
    case SEEKPOS::End:
        if (isline) {
            if (m_maxFilelines - res < 0) {
                ps = SEEKPOS::Invaild;
                res = isline ? qsizetype(m_rowBeforeJump)
                             : qsizetype(m_oldFileOffsetBeforeJump);
            } else {
                res = m_maxFilelines - res;
            }
        } else {
            if (qlonglong(m_maxFileBytes) - res < 0) {
                ps = SEEKPOS::Invaild;
                res = isline ? qsizetype(m_rowBeforeJump)
                             : qsizetype(m_oldFileOffsetBeforeJump);
            } else {
                res = qlonglong(m_maxFileBytes) - res;
            }
        }
        break;
    case SEEKPOS::RelativeAdd:
        if (isline) {
            if (res + m_rowBeforeJump > m_maxFilelines) {
                ps = SEEKPOS::Invaild;
                res = isline ? qsizetype(m_rowBeforeJump)
                             : qsizetype(m_oldFileOffsetBeforeJump);
            } else {
                res += m_rowBeforeJump;
            }
        } else {
            if (res + m_oldFileOffsetBeforeJump > m_maxFileBytes) {
                ps = SEEKPOS::Invaild;
                res = isline ? qsizetype(m_rowBeforeJump)
                             : qsizetype(m_oldFileOffsetBeforeJump);
            } else {
                res += m_oldFileOffsetBeforeJump;
            }
        }
        break;
    case SEEKPOS::RelativeMin:
        if (isline) {
            if (res - m_rowBeforeJump < 0) {
                ps = SEEKPOS::Invaild;
                res = isline ? qsizetype(m_rowBeforeJump)
                             : qsizetype(m_oldFileOffsetBeforeJump);
            } else {
                res -= m_rowBeforeJump;
            }
        } else {
            if (qlonglong(m_oldFileOffsetBeforeJump) - res < 0) {
                ps = SEEKPOS::Invaild;
                res = isline ? qsizetype(m_rowBeforeJump)
                             : qsizetype(m_oldFileOffsetBeforeJump);
            } else {
                res = qlonglong(m_oldFileOffsetBeforeJump) - res;
            }
        }
        break;
    }

    return res;
}
