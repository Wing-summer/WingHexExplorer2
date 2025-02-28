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

#ifndef SCRIPTSETTINGDIALOG_H
#define SCRIPTSETTINGDIALOG_H

#include "WingPlugin/settingpage.h"
#include "class/scriptmanager.h"
#include <QWidget>

namespace Ui {
class ScriptSettingDialog;
}

class ScriptSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit ScriptSettingDialog(QWidget *parent = nullptr);
    virtual ~ScriptSettingDialog() override;

private:
    Ui::ScriptSettingDialog *ui;

    QStringList m_usrHideCats;
    QStringList m_sysHideCats;

private:
    void loadData();
    bool addCatagory(const ScriptManager::ScriptDirMeta &meta, bool isUser,
                     bool hidden);

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;

private slots:
    void on_btnRefresh_clicked();
    void on_listWidget_currentRowChanged(int currentRow);
};

#endif // SCRIPTSETTINGDIALOG_H
