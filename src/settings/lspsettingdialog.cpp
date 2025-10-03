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

#include "lspsettingdialog.h"
#include "ui_lspsettingdialog.h"

#include "class/angellsp.h"
#include "class/scriptmachine.h"
#include "class/settingmanager.h"
#include "class/wingfiledialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "predefgen.h"
#include "utilities.h"

LspSettingDialog::LspSettingDialog(QWidget *parent)
    : WingHex::SettingPage(parent), ui(new Ui::LspSettingDialog) {
    ui->setupUi(this);

    auto e = QMetaEnum::fromType<AngelLsp::TraceMode>();
    auto total = e.keyCount();
    for (int i = 0; i < total; ++i) {
        ui->cbTrace->addItem(e.key(i));
    }
    ui->cbTrace->setCurrentIndex(0);

    reload();

    Utilities::addSpecialMark(ui->cbEnabled);

    if (SettingManager::instance().scriptEnabled()) {
        connect(ui->cbEnabled,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                &QCheckBox::checkStateChanged,
#else
                &QCheckBox::stateChanged,
#endif
                this, [this](Qt::CheckState state) {
                    auto &lsp = AngelLsp::instance();
                    lsp.setEnabled(state != Qt::Unchecked);
                    Q_EMIT optionNeedRestartChanged();
                });

        connect(ui->sbIndent, &QSpinBox::valueChanged, this, [](int value) {
            auto &lsp = AngelLsp::instance();
            lsp.setIndentSpace(value);
        });

        connect(ui->cbUseTabStop,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
                &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
                this, [](Qt::CheckState state) {
                    auto &lsp = AngelLsp::instance();
                    lsp.setUseTabIndent(state != Qt::Unchecked);
                });

        connect(ui->cbTrace,
                QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [](int index) {
                    Q_STATIC_ASSERT(int(AngelLsp::TraceMode::off) == 0);
                    Q_STATIC_ASSERT(int(AngelLsp::TraceMode::messages) == 1);
                    Q_STATIC_ASSERT(int(AngelLsp::TraceMode::verbose) == 2);
                    auto &lsp = AngelLsp::instance();
                    lsp.setTraceMode(AngelLsp::TraceMode(index));
                });

        connect(ui->btnExportPredef, &QPushButton::clicked, this, [parent]() {
            auto path = WingFileDialog::getExistingDirectory(
                parent, tr("ExportPredef"), {});
            if (path.isEmpty()) {
                return;
            }

            QDir dir(path);
            auto file = dir.absoluteFilePath(QStringLiteral("as.predefined"));
            if (QFile::exists(file)) {
                auto r = WingMessageBox::warning(
                    parent, tr("FileExists"), tr("Sure2Replace"),
                    QMessageBox::Yes | QMessageBox::No);
                if (r == QMessageBox::No) {
                    return;
                }
            }
            generateScriptPredefined(ScriptMachine::instance().engine(), file);
            Toast::toast(parent, NAMEICONRES("angellsp"),
                         tr("ExportSuccessfully"));
        });
        connect(ui->btnRestartlsp, &QPushButton::clicked, this, [parent]() {
            auto ret = WingMessageBox::warning(
                parent, tr("RestartLSP"), tr("Sure2RestartAngelLsp"),
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                auto &lsp = AngelLsp::instance();
                if (lsp.restart()) {
                    Toast::toast(parent, NAMEICONRES("angellsp"),
                                 tr("RestartSuccessfully"));
                } else {
                    WingMessageBox::critical(parent, tr("RestartLSP"),
                                             tr("RestartFailed"));
                }
            }
        });

        auto &lsp = AngelLsp::instance();
        connect(&lsp, &AngelLsp::serverStarted, this,
                &LspSettingDialog::reload);
        connect(&lsp, &AngelLsp::serverExited, this, &LspSettingDialog::reload);
    }
}

LspSettingDialog::~LspSettingDialog() { delete ui; }

QIcon LspSettingDialog::categoryIcon() const {
    return ICONRES(QStringLiteral("angellsp"));
}

QString LspSettingDialog::name() const { return tr("AngelLSP"); }

QString LspSettingDialog::id() const { return QStringLiteral("AngelLSP"); }

void LspSettingDialog::restore() {
    auto &lsp = AngelLsp::instance();
    lsp.resetSettings();
}

void LspSettingDialog::reload() {
    if (SettingManager::instance().scriptEnabled()) {
        auto &lsp = AngelLsp::instance();
        if (lsp.isActive()) {
            ui->cbEnabled->setEnabled(true);
            ui->gbDebug->setEnabled(true);
            ui->gbFormat->setEnabled(true);

            ui->cbEnabled->setChecked(lsp.enabled());
            ui->cbUseTabStop->setChecked(lsp.useTabIndent());
            ui->sbIndent->setValue(lsp.indentSpace());

            ui->cbTrace->setCurrentIndex(int(lsp.traceMode()));
        } else {
            ui->cbEnabled->setEnabled(false);
            ui->gbDebug->setEnabled(false);
            ui->gbFormat->setEnabled(false);
        }
    } else {
        setEnabled(false);
    }
}
