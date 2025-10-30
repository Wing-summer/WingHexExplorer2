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

#ifndef LAYOUTDELDIALOG_H
#define LAYOUTDELDIALOG_H

#include "framelessdialogbase.h"

namespace Ui {
class LayoutDelDialog;
}

class LayoutDelDialog : public QWidget {
    Q_OBJECT

public:
    explicit LayoutDelDialog(const QStringList &layouts,
                             QWidget *parent = nullptr);
    ~LayoutDelDialog();

public:
    QStringList getResult() const;

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

public slots:
    int exec();

private:
    Ui::LayoutDelDialog *ui;
    FramelessDialogBase *_dialog;
};

#endif // LAYOUTDELDIALOG_H
