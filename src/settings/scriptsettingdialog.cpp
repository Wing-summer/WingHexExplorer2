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

    loadData();

    connect(ui->cbAllowUsrScript,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this, &ScriptSettingDialog::optionNeedRestartChanged);

    connect(ui->listWidget, &QListWidget::itemChanged, this,
            [this](QListWidgetItem *item) {
                auto var = item->data(Qt::UserRole);
                if (var.isValid()) {
                    auto meta = var.value<ScriptManager::ScriptDirMeta>();
                    switch (item->checkState()) {
                    case Qt::Unchecked: {
                        if (meta.isSys) {
                            m_sysHideCats.pushAddItem(meta.rawName);
                        } else {
                            m_usrHideCats.pushAddItem(meta.rawName);
                        }
                    } break;
                    case Qt::Checked: {
                        if (meta.isSys) {
                            m_sysHideCats.pushRemoveItem(meta.rawName);
                        } else {
                            m_usrHideCats.pushRemoveItem(meta.rawName);
                        }
                    } break;
                    case Qt::PartiallyChecked:
                        break;
                    }

                    ui->btnSave->setEnabled(containUnsavedChanges());
                }
            });

    auto set = &SettingManager::instance();
    connect(ui->cbEnable, &QCheckBox::toggled, set,
            &SettingManager::setScriptEnabled);
    connect(ui->cbAllowUsrScript, &QCheckBox::toggled, set,
            &SettingManager::setAllowUsrScriptInRoot);
    connect(ui->sbTimeout, &QSpinBox::valueChanged, set,
            &SettingManager::setScriptTimeout);

    connect(ui->btnSave, &QPushButton::clicked, this, [this]() {
        auto &set = SettingManager::instance();
        set.setUsrHideCats(m_usrHideCats.getContents());
        set.setSysHideCats(m_sysHideCats.getContents());
        m_sysHideCats.clear();
        m_usrHideCats.clear();
        ui->btnSave->setEnabled(false);
        Q_EMIT optionNeedRestartChanged();
    });

    connect(ui->listWidget, &QListWidget::currentRowChanged, this,
            [this](int currentRow) {
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
                    info->append(tr("ContextMenu:") +
                                 (meta.isContextMenu
                                      ? QStringLiteral("true")
                                      : QStringLiteral("false")));
                    info->append(tr("HomePage:") + meta.homepage);
                    info->append(tr("Comment:"));
                    auto cur = info->textCursor();
                    cur.movePosition(QTextCursor::End);
                    info->setTextCursor(cur);
                    info->insertHtml(meta.comment);
                }
            });

    createPluginStandardMenu();
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

        QStringList buffer;
        for (auto &cat : usrCats) {
            if (addCatagory(sm.usrDirMeta(cat), true, hidden.contains(cat))) {
                buffer.append(cat);
            }
        }
        _usrHideCats = buffer;
        m_usrHideCats.setContents(buffer);

        buffer.clear();
        auto sysCats = sm.sysScriptsDbCats();
        hidden = set.sysHideCats();
        for (auto &cat : sysCats) {
            if (addCatagory(sm.sysDirMeta(cat), false, hidden.contains(cat))) {
                buffer.append(cat);
            }
        }
        _sysHideCats = buffer;
        m_sysHideCats.setContents(buffer);
    } else {
        ui->groupBox_2->setEnabled(false);
    }
}

bool ScriptSettingDialog::addCatagory(const ScriptManager::ScriptDirMeta &meta,
                                      bool isUser, bool hidden) {
    auto name = meta.name;
    if (meta.isEmptyDir) {
        name.append(' ').append(tr("[Empty]"));
    }
    auto lw =
        new QListWidgetItem(isUser ? ICONRES(QStringLiteral("scriptfolderusr"))
                                   : ICONRES(QStringLiteral("scriptfolder")),
                            name, ui->listWidget);
    lw->setData(Qt::UserRole, QVariant::fromValue(meta));
    lw->setCheckState(hidden ? Qt::Unchecked : Qt::Checked);
    return hidden;
}

void ScriptSettingDialog::createPluginStandardMenu() {
    auto widget = ui->listWidget;
    widget->setSelectionMode(QListWidget::ExtendedSelection);
    widget->setContextMenuPolicy(Qt::ActionsContextMenu);
    widget->addAction(tr("SelectShow"), widget, [widget]() {
        for (auto &item : widget->selectedItems()) {
            item->setCheckState(Qt::Checked);
        }
    });
    widget->addAction(tr("SelectHide"), widget, [widget]() {
        for (auto &item : widget->selectedItems()) {
            item->setCheckState(Qt::Unchecked);
        }
    });
    auto a = new QAction(widget);
    a->setSeparator(true);
    widget->addAction(a);
    widget->addAction(tr("SelectAll"), widget, &QListWidget::selectAll);
    widget->addAction(tr("SelectClear"), widget, &QListWidget::clearSelection);
    a = new QAction(widget);
    a->setSeparator(true);
    widget->addAction(a);
    widget->addAction(tr("DiscardChanges"), this,
                      &ScriptSettingDialog::discard);
}

QIcon ScriptSettingDialog::categoryIcon() const { return ICONRES("script"); }

QString ScriptSettingDialog::name() const { return tr("Script"); }

QString ScriptSettingDialog::id() const { return QStringLiteral("Script"); }

bool ScriptSettingDialog::containUnsavedChanges() const {
    return m_usrHideCats.containChanges() || m_sysHideCats.containChanges();
}

void ScriptSettingDialog::restore() {
    auto &set = SettingManager::instance();
    set.reset(SettingManager::SCRIPT);
    loadData();
}

void ScriptSettingDialog::discard() {
    m_usrHideCats.setContents(_usrHideCats);
    m_sysHideCats.setContents(_sysHideCats);

    ui->listWidget->blockSignals(true);
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        auto item = ui->listWidget->item(i);
        auto var = item->data(Qt::UserRole);
        if (var.isValid()) {
            auto meta = var.value<ScriptManager::ScriptDirMeta>();
            if (meta.isSys) {
                if (_sysHideCats.contains(meta.rawName)) {
                    item->setCheckState(Qt::Unchecked);
                } else {
                    item->setCheckState(Qt::Checked);
                }
            } else {
                if (_usrHideCats.contains(meta.rawName)) {
                    item->setCheckState(Qt::Unchecked);
                } else {
                    item->setCheckState(Qt::Checked);
                }
            }
        }
    }
    ui->listWidget->blockSignals(false);
}
