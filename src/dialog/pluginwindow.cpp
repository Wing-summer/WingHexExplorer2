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

#include "pluginwindow.h"
#include "utilities.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>

#define Bool2String(b) (b ? QStringLiteral("true") : QStringLiteral("false"))

PluginWindow::PluginWindow(QWidget *parent) : FramelessDialogBase(parent) {
    auto widget = new QWidget(this);
    auto layout = new QHBoxLayout(widget);

    this->setWindowTitle(tr("plugin"));
    txtc = new QTextBrowser(this);
    layout->addWidget(txtc);
    layout->addSpacing(5);
    plglist = new QListWidget(this);
    plglist->setFixedHeight(200);
    layout->addWidget(plglist);
    layout->addSpacing(5);
    txtb = new QTextBrowser(this);
    txtb->setMinimumHeight(300);
    layout->addWidget(txtb);
    layout->addSpacing(10);
    connect(plglist, &QListWidget::itemSelectionChanged, this,
            &PluginWindow::on_list_selchanged);
    auto btn = new QPushButton(tr("Refresh"), this);
    connect(btn, &QPushButton::clicked, this,
            [=] { this->setPluginSystem(m_pluginsys); });
    layout->addWidget(btn);
}

PluginWindow::~PluginWindow() {}

void PluginWindow::setPluginSystem(PluginSystem *pluginsys) {
    m_pluginsys = pluginsys;
    plglist->clear();
    auto pico = ICONRES("plugin");
    for (auto &item : pluginsys->plugins()) {
        plglist->addItem(new QListWidgetItem(pico, item->pluginName()));
    }
    txtc->clear();
}

void PluginWindow::on_list_selchanged() {
    txtb->clear();
#define Info(mem, info) txtb->append(mem + " : " + info)

    auto plg = m_pluginsys->plugins().at(plglist->currentRow());
    Info(tr("pluginName"), plg->pluginName());
    Info(tr("pluginAuthor"), plg->pluginAuthor());
    Info(tr("pluginVersion"), QString::number(plg->pluginVersion()));
    Info(tr("pluginComment"), plg->pluginComment());
}
