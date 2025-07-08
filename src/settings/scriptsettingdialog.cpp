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

#include "scriptsettingdialog.h"
#include "ui_scriptsettingdialog.h"

#include "class/settingmanager.h"
#include "utilities.h"

#include <QListWidgetItem>

ScriptSettingDialog::ScriptSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::ScriptSettingDialog) {
    ui->setupUi(this);

    Utilities::addSpecialMark(ui->cbEnable);
    Utilities::addSpecialMark(ui->cbAllowUsrScript);
    connect(ui->cbAllowUsrScript,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &ScriptSettingDialog::optionNeedRestartChanged);

    loadData();

    connect(ui->listWidget, &QListWidget::itemChanged, this,
            [this](QListWidgetItem *item) {
                auto var = item->data(Qt::UserRole);
                if (var.isValid()) {
                    auto meta = var.value<ScriptManager::ScriptDirMeta>();
                    switch (item->checkState()) {
                    case Qt::Unchecked: {
                        if (meta.isSys) {
                            m_sysHideCats.append(meta.rawName);
                        } else {
                            m_usrHideCats.append(meta.rawName);
                        }
                    } break;
                    case Qt::Checked: {
                        if (meta.isSys) {
                            m_sysHideCats.removeOne(meta.rawName);
                        } else {
                            m_usrHideCats.removeOne(meta.rawName);
                        }
                    } break;
                    case Qt::PartiallyChecked:
                        break;
                    }
                }

                auto &set = SettingManager::instance();
                set.setUsrHideCats(m_sysHideCats);
                set.setSysHideCats(m_usrHideCats);
            });

    auto set = &SettingManager::instance();
    connect(ui->cbEnable, &QCheckBox::toggled, set,
            &SettingManager::setScriptEnabled);
    connect(ui->cbAllowUsrScript, &QCheckBox::toggled, set,
            &SettingManager::setAllowUsrScriptInRoot);
    connect(ui->sbTimeout, &QSpinBox::valueChanged, set,
            &SettingManager::setScriptTimeout);
}

ScriptSettingDialog::~ScriptSettingDialog() { delete ui; }

void ScriptSettingDialog::loadData() {
    auto &set = SettingManager::instance();

    this->blockSignals(true);
    ui->listWidget->clear();
    ui->cbEnable->setChecked(set.scriptEnabled());
    ui->cbAllowUsrScript->setChecked(set.allowUsrScriptInRoot());
    ui->sbTimeout->setValue(set.scriptTimeout());
    this->blockSignals(false);

    if (set.scriptEnabled()) {
        auto &sm = ScriptManager::instance();
        auto usrCats = sm.usrScriptsDbCats();
        auto hidden = set.usrHideCats();
        for (auto &cat : usrCats) {
            if (addCatagory(sm.usrDirMeta(cat), true, hidden.contains(cat))) {
                m_usrHideCats << cat;
            }
        }

        auto sysCats = sm.sysScriptsDbCats();
        hidden = set.sysHideCats();
        for (auto &cat : sysCats) {
            if (addCatagory(sm.sysDirMeta(cat), false, hidden.contains(cat))) {
                m_sysHideCats << cat;
            }
        }
    } else {
        ui->groupBox_2->setEnabled(false);
    }
}

bool ScriptSettingDialog::addCatagory(const ScriptManager::ScriptDirMeta &meta,
                                      bool isUser, bool hidden) {
    auto lw =
        new QListWidgetItem(isUser ? ICONRES(QStringLiteral("scriptfolderusr"))
                                   : ICONRES(QStringLiteral("scriptfolder")),
                            meta.name, ui->listWidget);
    lw->setData(Qt::UserRole, QVariant::fromValue(meta));
    lw->setCheckState(hidden ? Qt::Unchecked : Qt::Checked);
    return !hidden;
}

QIcon ScriptSettingDialog::categoryIcon() const { return ICONRES("script"); }

QString ScriptSettingDialog::name() const { return tr("Script"); }

QString ScriptSettingDialog::id() const { return QStringLiteral("Script"); }

void ScriptSettingDialog::restore() {
    auto &set = SettingManager::instance();
    set.reset(SettingManager::SCRIPT);
    loadData();
}

void ScriptSettingDialog::on_btnRefresh_clicked() {
    ui->listWidget->clear();
    ScriptManager::instance().refresh();
    loadData();
}

void ScriptSettingDialog::on_listWidget_currentRowChanged(int currentRow) {
    if (currentRow < 0) {
        ui->textBrowser->clear();
        return;
    }
    auto lw = ui->listWidget->item(currentRow);
    auto var = lw->data(Qt::UserRole);
    if (var.isValid()) {
        auto meta = var.value<ScriptManager::ScriptDirMeta>();
        auto info = ui->textBrowser;
        info->clear();
        info->append(tr("RawName:") + meta.rawName);
        info->append(tr("Name:") + meta.name);
        info->append(tr("Author:") + meta.author);
        info->append(tr("License:") + meta.license);
        info->append(tr("ContextMenu:") + (meta.isContextMenu
                                               ? QStringLiteral("true")
                                               : QStringLiteral("false")));
        info->append(tr("HomePage:") + meta.homepage);
        info->append(tr("Comment:"));
        auto cur = info->textCursor();
        cur.movePosition(QTextCursor::End);
        info->setTextCursor(cur);
        info->insertHtml(meta.comment);
    }
}
