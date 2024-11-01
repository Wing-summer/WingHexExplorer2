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

#ifndef FRAMELESSMAINWINDOW_H
#define FRAMELESSMAINWINDOW_H

#ifdef WINGHEX_USE_FRAMELESS
#include "class/framelesshelper.h"
#endif

#include <QMainWindow>

class FramelessMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit FramelessMainWindow(QWidget *parent = nullptr);

    void buildUpContent(QWidget *content);

protected:
    void showEvent(QShowEvent *event) override;
    bool event(QEvent *event) override;

private:
#ifdef WINGHEX_USE_FRAMELESS
    FramelessHelper *_helper = nullptr;
    bool _useFrameLess = true;
#endif
#ifdef QT_DEBUG
    bool m_isBuilt = false;
#endif
};

#endif // FRAMELESSMAINWINDOW_H
