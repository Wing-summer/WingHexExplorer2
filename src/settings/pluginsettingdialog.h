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

#include "WingPlugin/settingpage.h"
#include "class/changedstringlist.h"

#include <QListWidget>
#include <QTextBrowser>
#include <QWidget>

namespace Ui {
class PluginSettingDialog;
}

class PluginSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit PluginSettingDialog(QWidget *parent = nullptr);
    ~PluginSettingDialog();

private:
    Ui::PluginSettingDialog *ui;

    ChangedStringList _devChanged;
    ChangedStringList _plgChanged;

private:
    void reload();

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;

    virtual bool containUnsavedChanges() const override;
    virtual void highlightUnsavedChange() override;
    virtual void discard() override;
    virtual void restore() override;

private:
    void createPluginStandardMenu(QListWidget *widget);

    void resetChangedList();
    void resetUIChanged();

    void loadPluginInfo(const std::optional<WingHex::PluginInfo> &info,
                        const QString &name, const QString &comment,
                        const QList<WingHex::WingDependency> &dependencies,
                        QTextBrowser *t);

private:
    QString getWrappedText(const QString &str);
};

#endif // PLUGINSETTINGDIALOG_H
