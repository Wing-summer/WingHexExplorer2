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

#ifndef GOTOWIDGET_H
#define GOTOWIDGET_H

#include <QWidget>

namespace Ui {
class GotoWidget;
}

class GotoWidget : public QWidget {
    Q_OBJECT

public:
    enum class SEEKPOS { Invaild, Start, End, RelativeAdd, RelativeMin };

public:
    explicit GotoWidget(QWidget *parent = nullptr);
    ~GotoWidget();

public slots:
    void activeInput(qsizetype oldrow, qsizetype oldcolumn, qsizetype oldoffset,
                     qsizetype maxfilebytes, qsizetype maxfilelines);

    void handleLineChanged();

    void jumpCancel();

    void jumpConfirm();

signals:
    void jumpToLine(qsizetype pos, bool isline);
    void pressEsc();

private slots:
    void on_btnClose_clicked();

private:
    qsizetype convert2Pos(const QString &value, SEEKPOS &ps, bool isline);

private:
    Ui::GotoWidget *ui;

    qsizetype m_rowBeforeJump = 0;
    qsizetype m_columnBeforeJump = 0;
    qsizetype m_oldFileOffsetBeforeJump = 0;
    qsizetype m_maxFileBytes = 0;
    qsizetype m_maxFilelines = 0;
};

#endif // GOTOWIDGET_H
