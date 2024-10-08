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

bool EditorSettingDialog::isInPluginPage() const { return false; }

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
