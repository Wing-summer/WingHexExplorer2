#include "editorsettingdialog.h"
#include "ui_editorsettingdialog.h"

#include "../class/settingmanager.h"
#include "../utilities.h"

EditorSettingDialog::EditorSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::EditorSettingDialog) {
    ui->setupUi(this);
    ui->cbEncodingBox->addItems(Utilities::getEncodings());
    reload();
}

EditorSettingDialog::~EditorSettingDialog() { delete ui; }

void EditorSettingDialog::reload() {
    auto &set = SettingManager::instance();
    ui->cbShowaddr->setChecked(set.editorShowAddr());
    ui->cbShowcol->setChecked(set.editorShowcol());
    ui->cbShowtext->setChecked(set.editorShowtext());
    ui->cbEncodingBox->setCurrentText(set.editorEncoding());
    ui->sbFontSize->setValue(set.appfontSize());
    ui->sbFindMax->setValue(set.findmaxcount());
    ui->sbDecStrLimit->setValue(set.decodeStrlimit());
    ui->sbCopyLimit->setValue(set.copylimit());
}

QIcon EditorSettingDialog::categoryIcon() const { return ICONRES("edit"); }

QString EditorSettingDialog::name() const { return tr("Editor"); }

bool EditorSettingDialog::isInPluginPage() const { return false; }

void EditorSettingDialog::apply() {
    auto &set = SettingManager::instance();
    set.setEditorShowAddr(ui->cbShowaddr->isChecked());
    set.setEditorShowcol(ui->cbShowcol->isChecked());
    set.setEditorShowtext(ui->cbShowtext->isChecked());
    set.setEditorEncoding(ui->cbEncodingBox->currentText());
    set.setEditorfontSize(ui->sbFontSize->value());
    set.setFindmaxcount(ui->sbFindMax->value());
    set.setDecodeStrlimit(ui->sbDecStrLimit->value());
    set.setCopylimit(ui->sbCopyLimit->value());
    set.save(SettingManager::SETTING::EDITOR);

    emit sigEditorShowAddrChanged(set.editorShowAddr());
    emit sigEditorShowcolChanged(set.editorShowcol());
    emit sigEditorShowtextChanged(set.editorShowtext());
    emit sigEditorEncodingChanged(set.editorEncoding());
    emit sigEditorfontSizeChanged(set.editorfontSize());
    emit sigFindmaxcountChanged(set.findmaxcount());
    emit sigDecodeStrlimitChanged(set.decodeStrlimit());
    emit sigCopylimitChanged(set.copylimit());
}

void EditorSettingDialog::reset() {
    SettingManager::instance().reset(SettingManager::SETTING::EDITOR);
}

void EditorSettingDialog::cancel() { reload(); }
