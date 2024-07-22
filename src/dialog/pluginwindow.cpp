#include "pluginwindow.h"
#include "src/utilities.h"
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>

#define Bool2String(b) (b ? "true" : "false")

PluginWindow::PluginWindow(QWidget *parent) : FramelessDialog(parent) {
    auto widget = new QWidget(this);
    auto layout = new QHBoxLayout(widget);

    this->setFixedSize(500, 750);
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
    Info(tr("PUID"), plg->puid());
    txtb->append(QString(10, '-'));
}
