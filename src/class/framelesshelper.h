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

#ifndef FRAMELESSHELPER_H
#define FRAMELESSHELPER_H

#include "widgetframe/windowbar.h"
#include <QObject>

class FramelessHelper : public QObject {
    Q_OBJECT
public:
    FramelessHelper(QWidget *parent, bool isDialog);

    QWK::WindowBar *windowBar() const;

private:
    static void emulateLeaveEvent(QWidget *widget);

private:
    QWK::WindowBar *m_windowBar = nullptr;
};

#endif // FRAMELESSHELPER_H
