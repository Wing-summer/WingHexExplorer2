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

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "WingPlugin/settingpage.h"
#include "framelessdialogbase.h"
#include <QAbstractButton>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QWidget {
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    virtual ~SettingDialog();

public:
    void addPage(WingHex::SettingPage *page);
    void build(); // you can only call once
    void showConfig(int index = -1);
    void showConfig(const QString &id);

public slots:
    QByteArray saveLayout();
    void restoreLayout(const QByteArray &layout);

signals:
    void onClosing();

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void toastTakeEffectReboot();

private:
    Ui::SettingDialog *ui;
    QList<WingHex::SettingPage *> m_pages;
    FramelessDialogBase *_dialog = nullptr;
};

#endif // SETTINGDIALOG_H
