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

#ifndef FRAMELESSDIALOGBASE_H
#define FRAMELESSDIALOGBASE_H

#ifdef WINGHEX_USE_FRAMELESS
#include "class/framelesshelper.h"
#endif

#include <QDialog>

class FramelessDialogBase : public QDialog {
    Q_OBJECT

public:
    explicit FramelessDialogBase(QWidget *parent = nullptr);

    virtual ~FramelessDialogBase();

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
