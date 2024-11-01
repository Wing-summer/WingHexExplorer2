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

#ifndef OPENREGIONDIALOG_H
#define OPENREGIONDIALOG_H

#include <QDialog>
#include <QMainWindow>
#include <QSpinBox>

#include "QPathEdit/qpathedit.h"
#include "framelessdialogbase.h"

struct RegionFileResult {
    QString filename;
    qsizetype start;
    qsizetype length;
};

class OpenRegionDialog : public FramelessDialogBase {
    Q_OBJECT
public:
    OpenRegionDialog(const QString &filename, int start = 0, int length = 1024,
                     QWidget *parent = nullptr);
    RegionFileResult getResult();

private:
    void on_accept();
    void on_reject();

private:
    QPathEdit *filepath;
    QSpinBox *sbStart, *sbLength;

    RegionFileResult res;
};

#endif // OPENREGIONDIALOG_H
