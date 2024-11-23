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

#ifndef WINGPROGRESSDIALOG_H
#define WINGPROGRESSDIALOG_H

#include <QProgressDialog>

#include "dialog/framelessdialogbase.h"

class WingProgressDialog {
public:
    explicit WingProgressDialog(QWidget *parent = nullptr);

    WingProgressDialog(const QString &labelText,
                       const QString &cancelButtonText, int minimum,
                       int maximum, QWidget *parent = nullptr);
    ~WingProgressDialog();

    FramelessDialogBase *pdialog() const;
    QProgressDialog *dialog() const;

private:
    QProgressDialog *m_dialog;
    FramelessDialogBase *m_d;
};

#endif // WINGPROGRESSDIALOG_H
