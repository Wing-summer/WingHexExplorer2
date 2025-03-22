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

#ifndef QEDITCONFIG_H
#define QEDITCONFIG_H

#include "WingPlugin/settingpage.h"
#include "wingcodeedit.h"

#include <QIcon>
#include <QString>
#include <QWidget>

namespace Ui {
class QEditConfig;
}

class QEditConfig : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit QEditConfig(bool isConsole, QWidget *parent = nullptr);
    virtual ~QEditConfig();

public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;
    virtual void apply() override;
    virtual void cancel() override;
    virtual void reset() override;

private:
    void reload();
    void loadThemes();
    void loadIndentations();

private:
    Ui::QEditConfig *ui;

    WingCodeEdit *_edit;
    bool m_isConsole;

    QString _name;
    QString _id;
    QIcon _icon;
};

#endif // QEDITCONFIG_H
