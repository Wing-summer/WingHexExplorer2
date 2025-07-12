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
#include "class/changedstringlist.h"
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

    QStringList _usrHideCats;
    QStringList _sysHideCats;

    ChangedStringList m_usrHideCats;
    ChangedStringList m_sysHideCats;

private:
    void loadData();
    bool addCatagory(const ScriptManager::ScriptDirMeta &meta, bool isUser,
                     bool hidden);

    void createPluginStandardMenu();

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;

    virtual bool containUnsavedChanges() const override;
    virtual void restore() override;
    virtual void discard() override;
};

#endif // SCRIPTSETTINGDIALOG_H
