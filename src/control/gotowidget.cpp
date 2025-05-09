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
        ui->lineEdit->setStyleSheet(QString());
        emit jumpToLine(p, isline);
    } else {
        ui->lineEdit->setStyleSheet(QStringLiteral("QLineEdit{color: red}"));
    }
}

void GotoWidget::jumpCancel() {
    emit jumpToLine(m_oldFileOffsetBeforeJump, false);
    hide();
}

void GotoWidget::jumpConfirm() {
    handleLineChanged();
    hide();
}

void GotoWidget::on_btnClose_clicked() { this->hide(); }

void GotoWidget::on_btnGoto_clicked() { /*emit onGoto(ui->spinBox->value()); */
}

qsizetype GotoWidget::convert2Pos(QString value, SEEKPOS &ps, bool isline) {
    qsizetype res = 0;
    if (value.length() > 0) {
        auto ch = value.at(0);
        if (ch == '+') {
            ps = SEEKPOS::Relative;
            value = value.remove(0, 1);

            bool ok = false;
            res = value.toLongLong(&ok, 0);

            if (!ok) {
                ps = SEEKPOS::Invaild;
            } else {
                if (isline) {
                    if (res + m_rowBeforeJump > m_maxFilelines) {
                        ps = SEEKPOS::Invaild;
                        res = 0;
                    } else {
                        res += m_rowBeforeJump;
                    }
                } else {
                    if (res + m_oldFileOffsetBeforeJump > m_maxFileBytes) {
                        ps = SEEKPOS::Invaild;
                        res = 0;
                    } else {
                        res += m_oldFileOffsetBeforeJump;
                    }
                }
            }

        } else if (ch == '-') {
            ps = SEEKPOS::Relative;

            value = value.remove(0, 1);

            bool ok = false;
            res = value.toLongLong(&ok, 0);

            if (!ok) {
                ps = SEEKPOS::Invaild;
            } else {
                if (isline) {
                    if (res - m_rowBeforeJump < 0) {
                        ps = SEEKPOS::Invaild;
                        res = 0;
                    } else {
                        res -= m_rowBeforeJump;
                    }
                } else {
                    if (qlonglong(m_oldFileOffsetBeforeJump) - res < 0) {
                        ps = SEEKPOS::Invaild;
                        res = 0;
                    } else {
                        res = qlonglong(m_oldFileOffsetBeforeJump) - res;
                    }
                }
            }

        } else if (ch == '<') {
            ps = SEEKPOS::End;
            value = value.remove(0, 1);

            bool ok = false;
            res = value.toLongLong(&ok, 0);

            if (!ok || res < 0) {
                ps = SEEKPOS::Invaild;
            } else {
                if (isline) {
                    if (m_maxFilelines - res < 0) {
                        ps = SEEKPOS::Invaild;
                        res = 0;
                    } else {
                        res = m_maxFilelines - res;
                    }
                } else {
                    if (qlonglong(m_maxFileBytes) - res < 0) {
                        ps = SEEKPOS::Invaild;
                        res = 0;
                    } else {
                        res = qlonglong(m_maxFileBytes) - res;
                    }
                }
            }
        } else {
            ps = SEEKPOS::Start;

            bool ok = false;
            res = value.toInt(&ok, 0);

            if (!ok) {
                ps = SEEKPOS::Invaild;
            } else {
                if (res < 0 || quint64(res) > (isline ? quint64(m_maxFilelines)
                                                      : m_maxFileBytes)) {
                    ps = SEEKPOS::Invaild;
                    res = 0;
                }
            }
        }
    } else {
        ps = SEEKPOS::Relative;
        res = isline ? qsizetype(m_rowBeforeJump)
                     : qsizetype(m_oldFileOffsetBeforeJump);
    }
    return res;
}
