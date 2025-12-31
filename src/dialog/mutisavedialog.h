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

#ifndef MUTISAVEDIALOG_H
#define MUTISAVEDIALOG_H

#include "class/editorinfo.h"
#include "framelessdialogbase.h"

#include <QVector>

namespace Ui {
class MutiSaveDialog;
}

class MutiSaveDialog : public QWidget {
    Q_OBJECT

public:
    enum StatusCode {
        SAVE_CANCEL = QDialog::Rejected,
        SAVE_SAVE = QDialog::Accepted,
        SAVE_DISCARD,
        SAVE_NOOP
    };

public:
    explicit MutiSaveDialog(const QVector<EditorInfo *> &editors,
                            EditorsCtl *edctl, QWidget *parent = nullptr);
    ~MutiSaveDialog();

public slots:
    int exec();

private slots:
    void onClickFileButton(const QModelIndex &index);

private:
    Ui::MutiSaveDialog *ui;
    FramelessDialogBase *_dialog;

    QVector<EditorInfo *> _editors;
    EditorsCtl *_esctl = nullptr;
};

#endif // MUTISAVEDIALOG_H
