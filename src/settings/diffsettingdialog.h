/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef DIFFSETTINGDIALOG_H
#define DIFFSETTINGDIALOG_H

#include "WingPlugin/settingpage.h"

namespace Ui {
class DiffSettingDialog;
}

class DiffSettingDialog final : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit DiffSettingDialog(QWidget *parent = nullptr);

public:
    virtual ~DiffSettingDialog() override;
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;
    virtual void restore() override;

private:
    void reload();
    void setDetailedLimit(int count);
    void setFileLimit(int mebibytes);
    void setMemoryBudget(int index);

private:
    Ui::DiffSettingDialog *ui;
};

#endif
