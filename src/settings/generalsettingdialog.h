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

#ifndef GENERALSETTINGDIALOG_H
#define GENERALSETTINGDIALOG_H

#include "WingPlugin/settingpage.h"
#include <QWidget>

namespace Ui {
class GeneralSettingDialog;
}

class GeneralSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit GeneralSettingDialog(QWidget *parent = nullptr);
    virtual ~GeneralSettingDialog() override;

private:
    Ui::GeneralSettingDialog *ui;

    void reload();

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;

    virtual void restore() override;
};

#endif // GENERALSETTINGDIALOG_H
