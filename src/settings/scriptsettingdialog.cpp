#include "scriptsettingdialog.h"
#include "../class/scriptmanager.h"
#include "../class/settingmanager.h"
#include "../utilities.h"
#include "ui_scriptsettingdialog.h"

#include <QListWidgetItem>

ScriptSettingDialog::ScriptSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::ScriptSettingDialog) {
    ui->setupUi(this);
    loadData();
}

ScriptSettingDialog::~ScriptSettingDialog() { delete ui; }

QStringList ScriptSettingDialog::sysDisplayCats() const {
    return m_sysDisplayCats;
}

QStringList ScriptSettingDialog::usrDisplayCats() const {
    return m_usrDisplayCats;
}

void ScriptSettingDialog::loadData() {
    auto &sm = ScriptManager::instance();
    auto &set = SettingManager::instance();

    auto usrCats = sm.usrScriptsDbCats();
    auto checked = set.usrDisplayCats();
    for (auto &cat : usrCats) {
        addCatagory(cat, true, checked.contains(cat));
        m_usrDisplayCats << cat;
    }

    auto sysCats = sm.sysScriptsDbCats();
    checked = set.sysDisplayCats();
    for (auto &cat : sysCats) {
        addCatagory(cat, false, checked.contains(cat));
        m_sysDisplayCats << cat;
    }
}

void ScriptSettingDialog::addCatagory(const QString &cat, bool isUser,
                                      bool checked) {
    auto lw =
        new QListWidgetItem(isUser ? ICONRES(QStringLiteral("scriptfolderusr"))
                                   : ICONRES(QStringLiteral("scriptfolder")),
                            cat, ui->listWidget);
    lw->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

QIcon ScriptSettingDialog::categoryIcon() const { return ICONRES("script"); }

QString ScriptSettingDialog::name() const { return tr("Script"); }

bool ScriptSettingDialog::isInPluginPage() const { return false; }

void ScriptSettingDialog::apply() {}

void ScriptSettingDialog::reset() {}

void ScriptSettingDialog::cancel() {}

void ScriptSettingDialog::on_btnRefresh_clicked() {
    ui->listWidget->clear();
    ScriptManager::instance().refresh();
    loadData();
}
