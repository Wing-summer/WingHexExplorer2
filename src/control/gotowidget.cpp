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

#include "class/settings/thememanager.h"

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
        GotoWidget::SeekPos seekPos = GotoWidget::SeekPos::Start;
        if (!parser.parse(value, seekPos)) {
            return;
        }
        lastPos = seekPos;
        lastAddr = value;
    }

public:
    Value lastAddr = 0;
    GotoWidget::SeekPos lastPos = GotoWidget::SeekPos::Invaild;

private:
    static constexpr int MaxShift = 64;

    void reset() {
        lastAddr = 0;
        lastPos = GotoWidget::SeekPos::Invaild;
    }

private:
    class Parser {
    public:
        explicit Parser(QStringView text) : m_text(text) {}

        bool parse(Value &result, GotoWidget::SeekPos &seekPos) {
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
            seekPos = GotoWidget::SeekPos::Start;
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

        QStringView m_text;
        qsizetype m_pos = 0;

    private:
        bool atEnd() const noexcept { return m_pos >= m_text.size(); }

        QChar current() const noexcept {
            return atEnd() ? QChar() : m_text[m_pos];
        }

        QChar peek(qsizetype offset) const noexcept {
            const qsizetype index = m_pos + offset;
            return index < m_text.size() ? m_text[index] : QChar();
        }

        void skipSpaces() noexcept {
            while (!atEnd() && current().isSpace()) {
                ++m_pos;
            }
        }

        bool consume(QChar ch) noexcept {
            if (!atEnd() && current() == ch) {
                ++m_pos;
                return true;
            }
            return false;
        }

        bool consumeOperator(QStringView op) noexcept {
            if (m_pos + op.size() > m_text.size()) {
                return false;
            }

            if (m_text.sliced(m_pos, op.size()) == op) {
                m_pos += op.size();
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
                ++m_pos;
                return Prefix::Add;
            case '-':
                ++m_pos;
                return Prefix::Sub;
            case '<':
                ++m_pos;
                return Prefix::End;
            default:
                return Prefix::None;
            }
        }

        static GotoWidget::SeekPos toSeekPos(Prefix prefix) {
            switch (prefix) {
            case Prefix::Add:
                return GotoWidget::SeekPos::RelativeAdd;
            case Prefix::Sub:
                return GotoWidget::SeekPos::RelativeMin;
            case Prefix::End:
                return GotoWidget::SeekPos::End;
            case Prefix::None:
                return GotoWidget::SeekPos::Start;
            }
            return GotoWidget::SeekPos::Invaild;
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
            if (!parseExclusiveOrExpression(result)) {
                return false;
            }

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
            const qsizetype begin = m_pos;
            while (!atEnd()) {
                const QChar ch = current();
                if (ch.isSpace() || ch == '(' || ch == ')' || ch == '[' ||
                    ch == ']' || ch == ':' || isOperatorCharacter(ch)) {
                    break;
                }
                ++m_pos;
            }
            if (begin == m_pos) {
                return false;
            }
            const auto token = m_text.sliced(begin, m_pos - begin);
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

    auto &theme = ThemeManager::instance();
    ui->btnGoto->setIcon(
        theme.standardIcon(ThemeManager::StandardIcon::Location));
    ui->btnClose->setIcon(
        theme.standardIcon(ThemeManager::StandardIcon::CloseFile));

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
    auto ps = SeekPos::Invaild;
    auto isline = ui->rbLine->isChecked();
    auto p = convert2Pos(content, ps, isline);
    if (ps != SeekPos::Invaild) {
        ui->lineEdit->setStyleSheet({});
    } else {
        ui->lineEdit->setStyleSheet(QStringLiteral("QLineEdit{color: red}"));
    }
    Q_EMIT sigOnJumpToLine(p, isline);
}

void GotoWidget::jumpCancel() {
    Q_EMIT sigOnJumpToLine(m_oldFileOffsetBeforeJump, false);
    hide();
}

void GotoWidget::jumpConfirm() {
    handleLineChanged();
    hide();
}

void GotoWidget::on_btnClose_clicked() { this->hide(); }

qsizetype GotoWidget::convert2Pos(const QString &value, SeekPos &ps,
                                  bool isline) {
    Calculator cal;
    cal.eval(value);

    const auto origin = isline ? m_rowBeforeJump : m_oldFileOffsetBeforeJump;
    if (cal.lastPos == SeekPos::Invaild) {
        ps = SeekPos::Invaild;
        return origin;
    }

    const auto offset = cal.lastAddr;
    const auto maximum =
        isline ? quint64(m_maxFilelines) : quint64(m_maxFileBytes);
    const auto base =
        isline ? quint64(m_rowBeforeJump) : quint64(m_oldFileOffsetBeforeJump);

    quint64 result = 0;
    switch (cal.lastPos) {
    case SeekPos::Start: {
        if (offset > maximum) {
            ps = SeekPos::Invaild;
            return origin;
        }
        result = offset;
        break;
    }

    case SeekPos::End: {
        if (offset > maximum) {
            ps = SeekPos::Invaild;
            return origin;
        }
        result = maximum - offset;
        break;
    }

    case SeekPos::RelativeAdd: {
        if (base > maximum || offset > maximum - base) {
            ps = SeekPos::Invaild;
            return origin;
        }
        result = base + offset;
        break;
    }

    case SeekPos::RelativeMin: {
        if (offset > base) {
            ps = SeekPos::Invaild;
            return origin;
        }
        result = base - offset;
        break;
    }

    case SeekPos::Invaild:
        ps = SeekPos::Invaild;
        return origin;
    }
    ps = cal.lastPos;
    return qsizetype(result);
}
