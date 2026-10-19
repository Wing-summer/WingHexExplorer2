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

#ifndef HISTORYDELDIALOG_H
#define HISTORYDELDIALOG_H

#include "class/file/recentfilemanager.h"
#include "framelessdialogbase.h"

namespace Ui {
class HistoryDelDialog;
}

class HistoryDelDialog : public QWidget {
    Q_OBJECT

public:
    explicit HistoryDelDialog(const QList<RecentFileManager::RecentInfo> &files,
                              bool isScriptFile, QWidget *parent = nullptr);
    virtual ~HistoryDelDialog();

public:
    QVector<int> getResult() const;

public slots:
    int exec();

private slots:
    void on_lwHistory_currentRowChanged(int currentRow);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::HistoryDelDialog *ui;
    FramelessDialogBase *_dialog;

    QList<RecentFileManager::RecentInfo> _infos;
    bool _isScriptFile;
};

#endif // HISTORYDELDIALOG_H
