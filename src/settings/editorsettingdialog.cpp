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

#include "editorsettingdialog.h"
#include "ui_editorsettingdialog.h"

#include "class/settingmanager.h"
#include "utilities.h"

EditorSettingDialog::EditorSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::EditorSettingDialog) {
    ui->setupUi(this);
    ui->cbEncodingBox->addItems(Utilities::getEncodings());
    reload();
}

EditorSettingDialog::~EditorSettingDialog() { delete ui; }

void EditorSettingDialog::reload() {
    auto &set = SettingManager::instance();
    ui->cbShowaddr->setChecked(set.editorShowHeader());
    ui->cbShowcol->setChecked(set.editorShowcol());
    ui->cbShowtext->setChecked(set.editorShowtext());
    ui->cbEncodingBox->setCurrentText(set.editorEncoding());
    ui->sbFontSize->setValue(set.appfontSize());
    ui->sbDecStrLimit->setValue(set.decodeStrlimit());
    ui->sbCopyLimit->setValue(set.copylimit());
}

QIcon EditorSettingDialog::categoryIcon() const { return ICONRES("edit"); }

QString EditorSettingDialog::name() const { return tr("Editor"); }

QString EditorSettingDialog::id() const { return QStringLiteral("Editor"); }

void EditorSettingDialog::apply() {
    auto &set = SettingManager::instance();
    set.setEditorShowHeader(ui->cbShowaddr->isChecked());
    set.setEditorShowcol(ui->cbShowcol->isChecked());
    set.setEditorShowtext(ui->cbShowtext->isChecked());
    set.setEditorEncoding(ui->cbEncodingBox->currentText());
    set.setEditorfontSize(ui->sbFontSize->value());
    set.setDecodeStrlimit(ui->sbDecStrLimit->value());
    set.setCopylimit(ui->sbCopyLimit->value());
    set.save(SettingManager::SETTING::EDITOR);
}

void EditorSettingDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::EDITOR);
    reload();
}

void EditorSettingDialog::cancel() { reload(); }
