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
}

ScriptSettingDialog::~ScriptSettingDialog() { delete ui; }

void ScriptSettingDialog::loadData() {
    auto &set = SettingManager::instance();

    this->blockSignals(true);
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

void ScriptSettingDialog::apply() {
    QStringList usrHideCats;
    QStringList sysHideCats;

    auto total = ui->listWidget->count();
    for (int i = 0; i < total; ++i) {
        auto lw = ui->listWidget->item(i);
        auto var = lw->data(Qt::UserRole);
        if (var.isValid()) {
            auto meta = var.value<ScriptManager::ScriptDirMeta>();
            if (lw->checkState() == Qt::Unchecked) {
                if (meta.isSys) {
                    sysHideCats << meta.rawName;
                } else {
                    usrHideCats << meta.rawName;
                }
            }
        }
    }

    auto &set = SettingManager::instance();
    set.setScriptEnabled(ui->cbEnable->isChecked());
    set.setAllowUsrScriptInRoot(ui->cbAllowUsrScript->isChecked());
    set.setScriptTimeout(ui->sbTimeout->value());
    set.setUsrHideCats(usrHideCats);
    set.setSysHideCats(sysHideCats);
    set.save(SettingManager::SCRIPT);

    m_usrHideCats = usrHideCats;
    m_sysHideCats = sysHideCats;
}

void ScriptSettingDialog::reset() {
    auto &set = SettingManager::instance();
    set.reset(SettingManager::SCRIPT);
    loadData();
}

void ScriptSettingDialog::cancel() { loadData(); }

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
        info->append(tr("HomePage:") + meta.homepage);
        info->append(tr("Comment:"));
        auto cur = info->textCursor();
        cur.movePosition(QTextCursor::End);
        info->setTextCursor(cur);
        info->insertHtml(meta.comment);
    }
}
