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

#include "historydeldialog.h"
#include "ui_historydeldialog.h"

#include <QListWidgetItem>

#include "class/plugin/pluginsystem.h"
#include "class/settings/thememanager.h"
#include "utilities.h"

HistoryDelDialog::HistoryDelDialog(
    const QList<RecentFileManager::RecentInfo> &files, bool isScriptFile,
    QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryDelDialog), m_infos(files),
      m_isScriptFile(isScriptFile) {
    ui->setupUi(this);

    auto &theme = ThemeManager::instance();
    for (const auto &info : files) {
        const auto &url = info.url;
        auto lw = new QListWidgetItem;

        if (url.isLocalFile()) {
            QMimeDatabase db;
            auto fileName = info.url.toLocalFile();
            auto mt = db.mimeTypeForFile(fileName);
            auto title =
                RecentFileManager::getDisplayFileName(info, m_isScriptFile);
            lw->setText(title);
            if (m_isScriptFile) {
                lw->setIcon(
                    Utilities::getIconFromFile(qApp->style(), fileName));
            } else {
                if (info.isWorkSpace) {
                    lw->setText(title);
                    lw->setIcon(theme.standardIcon(
                        ThemeManager::StandardIcon::Project));
                    auto font = lw->font();
                    font.setUnderline(true);
                    lw->setFont(font);
                } else {
                    lw->setText(title + QStringLiteral(" (") + mt.name() +
                                QStringLiteral(")"));
                    lw->setIcon(
                        Utilities::getIconFromFile(qApp->style(), fileName));
                }
            }
        } else {
            // plugin extension
            auto &plgsys = PluginSystem::instance();
            auto plgID = url.authority();
            lw->setText(RecentFileManager::getDisplayFileName(info, false));
            auto r = plgsys.ext2Device(plgID);
            if (r) {
                lw->setIcon(r->supportedFileIcon());
            } else {
                lw->setIcon(
                    theme.standardIcon(ThemeManager::StandardIcon::Device));
            }
        }

        lw->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable |
                     Qt::ItemIsSelectable);
        lw->setCheckState(Qt::Unchecked);
        ui->lwHistory->addItem(lw);
    }

    m_dialog = new FramelessDialogBase(parent);
    m_dialog->buildUpContent(this);
    m_dialog->setWindowIcon(
        theme.standardIcon(ThemeManager::StandardIcon::ClearHistory));
    m_dialog->setWindowTitle(this->windowTitle());
    m_dialog->setMinimumSize(600, 400);
    Utilities::moveToCenter(m_dialog);
}

HistoryDelDialog::~HistoryDelDialog() { delete ui; }

QVector<int> HistoryDelDialog::getResult() const {
    QVector<int> ret;
    auto total = ui->lwHistory->count();
    for (int i = 0; i < total; ++i) {
        auto cs = ui->lwHistory->item(i)->checkState();
        if (cs == Qt::Checked) {
            ret.append(i);
        }
    }
    return ret;
}

int HistoryDelDialog::exec() { return m_dialog->exec(); }

void HistoryDelDialog::on_lwHistory_currentRowChanged(int currentRow) {
    auto info = m_infos.at(currentRow);
    ui->tbInfo->setText(
        RecentFileManager::getDisplayTooltip(info, m_isScriptFile));
}

void HistoryDelDialog::on_buttonBox_accepted() { m_dialog->accept(); }

void HistoryDelDialog::on_buttonBox_rejected() { m_dialog->reject(); }
