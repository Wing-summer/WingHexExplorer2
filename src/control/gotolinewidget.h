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

#ifndef GOTOLINEWIDGET_H
#define GOTOLINEWIDGET_H

#include "control/codeeditcontrolwidget.h"

#include <QSpinBox>

class GotoLineWidget : public CodeEditControlWidget {
    Q_OBJECT
public:
    explicit GotoLineWidget(QWidget *parent = nullptr);

    void setTotalLines(int lineTotal);

signals:
    void onGotoLine(int line);

public:
    void setOriginLine(int line);

    // CodeEditControlWidget interface
public:
    virtual void cancel() override;

private:
    QSpinBox *m_sbline;
    int m_line = 1;
};

#endif // GOTOLINEWIDGET_H
