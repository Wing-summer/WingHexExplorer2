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

#ifndef PLUGINSETTINGDIALOG_H
#define PLUGINSETTINGDIALOG_H

#include "../plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class PluginSettingDialog;
}

class PluginSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit PluginSettingDialog(QWidget *parent = nullptr);
    ~PluginSettingDialog();

    void buildUp(const QList<WingHex::SettingPage *> &pages);

private:
    Ui::PluginSettingDialog *ui;
    QList<WingHex::SettingPage *> _pages;

    void reload();

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual bool isInPluginPage() const override;
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;

private slots:
    void on_plglist_itemSelectionChanged();
};

#endif // PLUGINSETTINGDIALOG_H
