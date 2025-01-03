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

#include "driverselectordialog.h"
#include "utilities.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QScrollBar>
#include <QShortcut>
#include <QVBoxLayout>

DriverSelectorDialog::DriverSelectorDialog(QWidget *parent)
    : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    widget->setMinimumWidth(400);
    auto layout = new QVBoxLayout(widget);

    drivers = new QListWidget(this);
    drivers->setSortingEnabled(false);
    auto ico = ICONRES("opendriver");
    layout->addWidget(new QLabel(tr("PleaseChooseDriver"), this));
    layout->addSpacing(5);
    for (auto &item : QStorageInfo::mountedVolumes()) {
        auto device = item.device();
#ifdef Q_OS_WINDOWS
        drivers->addItem(new QListWidgetItem(ico, item.rootPath()));
#else
        drivers->addItem(new QListWidgetItem(ico, item.device()));
#endif
        m_infos.push_back(item);
    }
    layout->addWidget(drivers);
    layout->addSpacing(5);
    infob = new QTextBrowser(this);
    layout->addWidget(infob);
    layout->addSpacing(5);
    layout->addWidget(new QLabel(tr("DriverTips"), this));
    layout->addSpacing(5);
    auto dbbox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(dbbox, &QDialogButtonBox::accepted, this,
            &DriverSelectorDialog::on_accepted);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &DriverSelectorDialog::on_rejected);
    layout->addWidget(dbbox);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &DriverSelectorDialog::on_accepted);
    connect(drivers, &QListWidget::itemSelectionChanged, this,
            &DriverSelectorDialog::on_list_selectionChanged);

    drivers->setCurrentRow(0); // 我不信你没有驱动器

    buildUpContent(widget);

    setWindowTitle(tr("OpenDriver"));
}

void DriverSelectorDialog::on_list_selectionChanged() {
    infob->clear();
#define Info(mem, info) infob->append(mem + " : " + info)
    auto item = m_infos.at(drivers->currentRow());
    Info(tr("device"), item.device());
    Info(tr("displayName"), item.displayName());
    Info(tr("fileSystemType"), item.fileSystemType());
    Info(tr("rootPath"), item.rootPath());
    Info(tr("name"), item.name());

    if (item.isReady()) {
        Info(tr("isReady"), "True");
        Info(tr("bytesAvailable"),
             Utilities::processBytesCount(item.bytesAvailable()));
        Info(tr("bytesTotal"), Utilities::processBytesCount(item.bytesTotal()));
    } else {
        Info(tr("isReady"), "False");
    }
    infob->verticalScrollBar()->setValue(0);
}

void DriverSelectorDialog::on_accepted() {
    m_si = m_infos.at(drivers->currentRow());
    done(1);
}

void DriverSelectorDialog::on_rejected() { done(0); }

QStorageInfo DriverSelectorDialog::GetResult() { return m_si; }
