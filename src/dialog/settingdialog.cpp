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

#include "settingdialog.h"
#include "ui_settingdialog.h"

#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "utilities.h"

#include <QApplication>
#include <QBuffer>
#include <QCloseEvent>
#include <QDataStream>
#include <QMenu>
#include <QPushButton>
#include <QTimer>

SettingDialog::SettingDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::SettingDialog) {
    ui->setupUi(this);

    connect(ui->listWidget, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                if (previous) {
                    auto page = m_pages.at(
                        ui->listWidget->indexFromItem(previous).row());
                    if (page->containUnsavedChanges()) {
                        auto ret = WingMessageBox::question(
                            this, tr("UnsavedChanges"),
                            tr("SaveChangesDiscardLeave?"));
                        if (ret == QMessageBox::Yes) {
                            page->discard();
                        } else {
                            QTimer::singleShot(100, this, [this, previous]() {
                                ui->listWidget->blockSignals(true);
                                ui->listWidget->setCurrentItem(previous);
                                ui->listWidget->blockSignals(false);
                            });
                            page->highlightUnsavedChange();
                            return;
                        }
                    }
                }
                if (current) {
                    auto curpage = m_pages.at(
                        ui->listWidget->indexFromItem(current).row());
                    ui->stackedWidget->setCurrentWidget(curpage);
                }
            });
    connect(ui->btnRestore, &QPushButton::clicked, this, [this]() {
        auto idx = ui->listWidget->currentRow();
        auto page = m_pages.at(idx);
        page->restore();
    });

    _dialog = new FramelessDialogBase(parent);
    _dialog->installEventFilter(this);
    _dialog->buildUpContent(this);

    ui->btnRestore->setStyleSheet(
        QStringLiteral("QToolButton::down-arrow {width:10px; height:10px; "
                       "subcontrol-position:right center; "
                       "subcontrol-origin:content; left: -2px;}"));
    auto menu = new QMenu(ui->btnRestore);
    auto a = menu->addAction(tr("Restore current"));
    connect(a, &QAction::triggered, this, [this]() {
        auto ret = WingMessageBox::warning(this, tr("Restore"),
                                           tr("RestoreCurPageSets?"),
                                           QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) {
            return;
        }
        auto idx = ui->listWidget->currentRow();
        auto page = m_pages.at(idx);
        if (idx >= 0) {
            page->restore();
        }
        toastTakeEffectReboot(page);
    });

    a = menu->addAction(tr("Restore all"));
    connect(a, &QAction::triggered, this, [this]() {
        auto ret =
            WingMessageBox::critical(this, tr("Restore"), tr("RestoreAllSets?"),
                                     QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) {
            return;
        }
        for (auto &p : m_pages) {
            p->restore();
        }
        toastTakeEffectReboot(nullptr);
    });
    ui->btnRestore->setMenu(menu);

    Utilities::moveToCenter(this);
}

SettingDialog::~SettingDialog() {
    delete ui;
    _dialog->deleteLater();
}

void SettingDialog::addPage(WingHex::SettingPage *page) {
    if (page == nullptr) {
        return;
    }
    m_pages << page;
}

void SettingDialog::build() {
    for (auto &page : m_pages) {
        auto name = page->name();
        auto icon = page->categoryIcon();
        auto item = new QListWidgetItem(icon, name);
        item->setSizeHint(QSize(0, 40));
        ui->listWidget->addItem(item);
        ui->stackedWidget->addWidget(page);
    }
    if (m_pages.size()) {
        ui->listWidget->setCurrentRow(0);
    }
}

void SettingDialog::showConfig(int index) {
    if (index >= 0 && index < m_pages.size()) {
        ui->listWidget->setCurrentRow(index);
    }
    Utilities::moveToCenter(this);
    _dialog->exec();
}

void SettingDialog::showConfig(const QString &id) {
    if (id.isEmpty()) {
        ui->listWidget->setCurrentRow(0);
    }
    auto r = std::find_if(
        m_pages.begin(), m_pages.end(), [id](const WingHex::SettingPage *page) {
            return id.compare(page->id(), Qt::CaseInsensitive) == 0;
        });
    if (r == m_pages.end()) {
        ui->listWidget->setCurrentRow(0);
    }
    ui->listWidget->setCurrentRow(m_pages.indexOf(*r));
    Utilities::moveToCenter(this);
    _dialog->exec();
}

QByteArray SettingDialog::saveLayout() {
    QByteArray ret;
    QDataStream buffer(&ret, QDataStream::WriteOnly);
    buffer << _dialog->saveGeometry();
    buffer << ui->splitter->saveState();
    return ret;
}

void SettingDialog::restoreLayout(const QByteArray &layout) {
    auto data = layout;
    QDataStream buffer(&data, QDataStream::ReadOnly);
    QByteArray ret;
    buffer >> ret;
    if (!ret.isEmpty()) {
        _dialog->restoreGeometry(ret);
    }
    buffer >> ret;
    if (!ret.isEmpty()) {
        ui->splitter->restoreState(ret);
    }
}

bool SettingDialog::eventFilter(QObject *, QEvent *event) {
    if (event->type() == QEvent::Close) {
        auto e = static_cast<QCloseEvent *>(event);
        auto page = m_pages.at(ui->listWidget->currentRow());
        if (page->containUnsavedChanges()) {
            auto ret = WingMessageBox::question(this, tr("UnsavedChanges"),
                                                tr("SaveChangesDiscardLeave?"));
            if (ret == QMessageBox::Yes) {
                page->discard();
            } else {
                page->highlightUnsavedChange();
                event->ignore();
                return true;
            }
        }
        Q_EMIT onClosing();
    }
    return false;
}

void SettingDialog::toastTakeEffectReboot() {
    auto page = qobject_cast<WingHex::SettingPage *>(sender());
    if (page) {
        auto icon = page->categoryIcon();
        auto avsize = icon.availableSizes();
        Q_ASSERT(!avsize.isEmpty());
        Toast::toast(_dialog, icon.pixmap(avsize.first()),
                     tr("TakeEffectRestart"));
    }
}

void SettingDialog::toastTakeEffectReboot(WingHex::SettingPage *page) {
    if (page) {
        auto icon = page->categoryIcon();
        auto avsize = icon.availableSizes();
        Q_ASSERT(!avsize.isEmpty());
        Toast::toast(_dialog, icon.pixmap(avsize.first()),
                     tr("TakeEffectRestart"));
    } else {
        Toast::toast(_dialog, NAMEICONRES(QStringLiteral("setting")),
                     tr("TakeEffectRestart"));
    }
}
