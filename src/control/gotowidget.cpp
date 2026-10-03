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

#include <QShortcut>

Q_STATIC_ASSERT_X(
    QT_VERSION >= QT_VERSION_CHECK(6, 4, 0),
    "If you want to support Qt version lower than 6.4.0, You should "
    "implement '0b' prefix integer converstion on your own!");

class Calculator {
public:
    using Value = quint64;

    void eval(const QString &expression) {
        reset();

        if (expression.isEmpty()) {
            return;
        }

        Parser parser(expression);
        Value value = 0;
        GotoWidget::SEEKPOS seekPos = GotoWidget::SEEKPOS::Start;
        if (!parser.parse(value, seekPos)) {
            return;
        }
        lastPos = seekPos;
        lastAddr = value;
    }

public:
    Value lastAddr = 0;
    GotoWidget::SEEKPOS lastPos = GotoWidget::SEEKPOS::Invaild;

private:
    static constexpr int MaxShift = 64;

    void reset() {
        lastAddr = 0;
        lastPos = GotoWidget::SEEKPOS::Invaild;
    }

private:
    class Parser {
    public:
        explicit Parser(QStringView text) : text_(text) {}

        bool parse(Value &result, GotoWidget::SEEKPOS &seekPos) {
            skipSpaces();
            if (atEnd()) {
                return false;
            }

            /*
             * entryExpression:
             *
             *   prefixGoto? IntegerConstant EOF
             *
             *   (prefixGoto Colon)?
             *       assignmentExpression EOF
             *
             *   prefixGoto '[' assignmentExpression ']'
             *
             *   prefixGoto '(' assignmentExpression ')'
             */
            const auto prefix = parsePrefix();
            if (prefix != Prefix::None) {
                seekPos = toSeekPos(prefix);
                skipSpaces();

                /*
                 * +123 / -123 / <123
                 */
                if (isNumberStart()) {
                    if (!parseNumber(result)) {
                        return false;
                    }
                    skipSpaces();
                    return atEnd();
                }

                /*
                 * +:expr / -:expr / <:expr
                 */
                if (consume(':')) {
                    if (!parseAssignmentExpression(result)) {
                        return false;
                    }
                    skipSpaces();
                    return atEnd();
                }

                /*
                 * +[expr] / -[expr] / <[expr]
                 */
                if (consume('[')) {
                    if (!parseAssignmentExpression(result)) {
                        return false;
                    }
                    skipSpaces();
                    if (!consume(']')) {
                        return false;
                    }
                    skipSpaces();
                    return atEnd();
                }

                /*
                 * +(expr) / -(expr) / <(expr)
                 */
                if (consume('(')) {
                    if (!parseAssignmentExpression(result)) {
                        return false;
                    }
                    skipSpaces();
                    if (!consume(')')) {
                        return false;
                    }
                    skipSpaces();
                    return atEnd();
                }
                return false;
            }

            /*
             *   assignmentExpression EOF
             */
            seekPos = GotoWidget::SEEKPOS::Start;
            if (!parseAssignmentExpression(result)) {
                return false;
            }
            skipSpaces();
            return atEnd();
        }

    private:
        enum class Prefix : quint8 {
            None,
            Add,
            Sub,
            End,
        };

        QStringView text_;
        qsizetype pos_ = 0;

    private:
        bool atEnd() const noexcept { return pos_ >= text_.size(); }

        QChar current() const noexcept {
            return atEnd() ? QChar() : text_[pos_];
        }

        QChar peek(qsizetype offset) const noexcept {
            const qsizetype index = pos_ + offset;
            return index < text_.size() ? text_[index] : QChar();
        }

        void skipSpaces() noexcept {
            while (!atEnd() && current().isSpace()) {
                ++pos_;
            }
        }

        bool consume(QChar ch) noexcept {
            if (!atEnd() && current() == ch) {
                ++pos_;
                return true;
            }
            return false;
        }

        bool consumeOperator(QStringView op) noexcept {
            if (pos_ + op.size() > text_.size()) {
                return false;
            }

            if (text_.sliced(pos_, op.size()) == op) {
                pos_ += op.size();
                return true;
            }
            return false;
        }

        Prefix parsePrefix() noexcept {
            if (atEnd()) {
                return Prefix::None;
            }
            switch (current().unicode()) {
            case '+':
                ++pos_;
                return Prefix::Add;
            case '-':
                ++pos_;
                return Prefix::Sub;
            case '<':
                ++pos_;
                return Prefix::End;
            default:
                return Prefix::None;
            }
        }

        static GotoWidget::SEEKPOS toSeekPos(Prefix prefix) {
            switch (prefix) {
            case Prefix::Add:
                return GotoWidget::SEEKPOS::RelativeAdd;
            case Prefix::Sub:
                return GotoWidget::SEEKPOS::RelativeMin;
            case Prefix::End:
                return GotoWidget::SEEKPOS::End;
            case Prefix::None:
                return GotoWidget::SEEKPOS::Start;
            }
            return GotoWidget::SEEKPOS::Invaild;
        }

    private:
        /*
         * assignmentExpression
         *     : inclusiveOrExpression
         */
        bool parseAssignmentExpression(Value &result) {
            return parseInclusiveOrExpression(result);
        }

        /*
         * inclusiveOrExpression
         *     : exclusiveOrExpression
         *       ('|' exclusiveOrExpression)*
         */
        bool parseInclusiveOrExpression(Value &result) {
            if (!parseExclusiveOrExpression(result))
                return false;

            while (true) {
                skipSpaces();
                if (!consume('|')) {
                    break;
                }
                Value rhs = 0;
                if (!parseExclusiveOrExpression(rhs)) {
                    return false;
                }
                result |= rhs;
            }
            return true;
        }

        /*
         * exclusiveOrExpression
         *     : andExpression
         *       ('^' andExpression)*
         */
        bool parseExclusiveOrExpression(Value &result) {
            if (!parseAndExpression(result)) {
                return false;
            }
            while (true) {
                skipSpaces();
                if (!consume('^')) {
                    break;
                }
                Value rhs = 0;
                if (!parseAndExpression(rhs)) {
                    return false;
                }
                result ^= rhs;
            }
            return true;
        }

        /*
         * andExpression
         *     : shiftExpression
         *       ('&' shiftExpression)*
         */
        bool parseAndExpression(Value &result) {
            if (!parseShiftExpression(result)) {
                return false;
            }
            while (true) {
                skipSpaces();
                if (!consume('&')) {
                    break;
                }
                Value rhs = 0;
                if (!parseShiftExpression(rhs)) {
                    return false;
                }
                result &= rhs;
            }

            return true;
        }

        /*
         * shiftExpression
         *     : additiveExpression
         *       (('<<' | '>>') additiveExpression)*
         */
        bool parseShiftExpression(Value &result) {
            if (!parseAdditiveExpression(result)) {
                return false;
            }
            while (true) {
                skipSpaces();
                if (consumeOperator(QStringLiteral("<<"))) {
                    Value rhs = 0;
                    if (!parseAdditiveExpression(rhs)) {
                        return false;
                    }
                    if (rhs >= MaxShift) {
                        return false;
                    }
                    result <<= rhs;
                    continue;
                }

                if (consumeOperator(u">>")) {
                    Value rhs = 0;
                    if (!parseAdditiveExpression(rhs)) {
                        return false;
                    }
                    if (rhs >= MaxShift) {
                        return false;
                    }
                    result >>= rhs;
                    continue;
                }

                break;
            }

            return true;
        }

        /*
         * additiveExpression
         *     : multiplicativeExpression
         *       (('+' | '-')
         *        multiplicativeExpression)*
         */
        bool parseAdditiveExpression(Value &result) {
            if (!parseMultiplicativeExpression(result)) {
                return false;
            }
            while (true) {
                skipSpaces();
                if (consume('+')) {
                    Value rhs = 0;
                    if (!parseMultiplicativeExpression(rhs)) {
                        return false;
                    }
                    Value value = 0;
                    if (qAddOverflow(result, rhs, &value)) {
                        return false;
                    }
                    result = value;
                    continue;
                }

                if (consume('-')) {
                    Value rhs = 0;
                    if (!parseMultiplicativeExpression(rhs)) {
                        return false;
                    }
                    Value value = 0;
                    if (qSubOverflow(result, rhs, &value)) {
                        return false;
                    }
                    result = value;
                    continue;
                }

                break;
            }

            return true;
        }

        /*
         * multiplicativeExpression
         *     : unaryExpression
         *       (('*' | '/' | '%') unaryExpression)*
         */
        bool parseMultiplicativeExpression(Value &result) {
            if (!parseUnaryExpression(result)) {
                return false;
            }
            while (true) {
                skipSpaces();
                if (consume('*')) {
                    Value rhs = 0;
                    if (!parseUnaryExpression(rhs)) {
                        return false;
                    }
                    Value value = 0;
                    if (qMulOverflow(result, rhs, &value)) {
                        return false;
                    }
                    result = value;
                    continue;
                }
                if (consume('/')) {
                    Value rhs = 0;
                    if (!parseUnaryExpression(rhs)) {
                        return false;
                    }
                    if (rhs == 0) {
                        return false;
                    }
                    result /= rhs;
                    continue;
                }

                if (consume('%')) {
                    Value rhs = 0;
                    if (!parseUnaryExpression(rhs)) {
                        return false;
                    }
                    if (rhs == 0) {
                        return false;
                    }
                    result %= rhs;
                    continue;
                }

                break;
            }

            return true;
        }

        /*
         * unaryExpression
         *
         *     ~ unaryExpression
         *     primaryExpression
         */
        bool parseUnaryExpression(Value &result) {
            skipSpaces();
            if (consume('~')) {
                if (!parseUnaryExpression(result)) {
                    return false;
                }
                result = ~result;
                return true;
            }
            return parsePrimaryExpression(result);
        }

        /*
         * primaryExpression
         *
         *     IntegerConstant
         *     '(' assignmentExpression ')'
         */
        bool parsePrimaryExpression(Value &result) {
            skipSpaces();
            if (consume('(')) {
                if (!parseAssignmentExpression(result)) {
                    return false;
                }
                skipSpaces();
                if (!consume(')')) {
                    return false;
                }
                return true;
            }

            return parseNumber(result);
        }

        /*
         * IntegerConstant
         */
        bool parseNumber(Value &result) {
            skipSpaces();
            if (!isNumberStart()) {
                return false;
            }
            const qsizetype begin = pos_;
            while (!atEnd()) {
                const QChar ch = current();
                if (ch.isSpace() || ch == '(' || ch == ')' || ch == '[' ||
                    ch == ']' || ch == ':' || isOperatorCharacter(ch)) {
                    break;
                }
                ++pos_;
            }
            if (begin == pos_) {
                return false;
            }
            const auto token = text_.sliced(begin, pos_ - begin);
            bool ok = false;
            const Value value = token.toULongLong(&ok, 0);
            if (!ok) {
                return false;
            }
            result = value;
            return true;
        }

        bool isNumberStart() const noexcept {
            if (atEnd()) {
                return false;
            }
            const QChar ch = current();
            return ch.isDigit();
        }

        static bool isOperatorCharacter(QChar ch) noexcept {
            switch (ch.unicode()) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '&':
            case '|':
            case '^':
            case '<':
            case '>':
            case '~':
                return true;
            default:
                return false;
            }
        }
    };
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

    const auto origin = isline ? m_rowBeforeJump : m_oldFileOffsetBeforeJump;
    if (cal.lastPos == SEEKPOS::Invaild) {
        ps = SEEKPOS::Invaild;
        return origin;
    }

    const auto offset = cal.lastAddr;
    const auto maximum =
        isline ? quint64(m_maxFilelines) : quint64(m_maxFileBytes);
    const auto base =
        isline ? quint64(m_rowBeforeJump) : quint64(m_oldFileOffsetBeforeJump);

    quint64 result = 0;
    switch (cal.lastPos) {
    case SEEKPOS::Start: {
        if (offset > maximum) {
            ps = SEEKPOS::Invaild;
            return origin;
        }
        result = offset;
        break;
    }

    case SEEKPOS::End: {
        if (offset > maximum) {
            ps = SEEKPOS::Invaild;
            return origin;
        }
        result = maximum - offset;
        break;
    }

    case SEEKPOS::RelativeAdd: {
        if (base > maximum || offset > maximum - base) {
            ps = SEEKPOS::Invaild;
            return origin;
        }
        result = base + offset;
        break;
    }

    case SEEKPOS::RelativeMin: {
        if (offset > base) {
            ps = SEEKPOS::Invaild;
            return origin;
        }
        result = base - offset;
        break;
    }

    case SEEKPOS::Invaild:
        ps = SEEKPOS::Invaild;
        return origin;
    }
    ps = cal.lastPos;
    return qsizetype(result);
}
