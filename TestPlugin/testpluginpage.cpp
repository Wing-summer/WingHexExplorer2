#include "testpluginpage.h"

#include <QVBoxLayout>

TestPluginPage::TestPluginPage(QWidget *parent) : WingHex::PluginPage(parent) {
    auto layout = new QVBoxLayout(this);
    _lbl = new QLabel(QStringLiteral("TestPluginPage"), this);
    _lbl->setAlignment(Qt::AlignCenter);
    layout->addWidget(_lbl);
}

QIcon TestPluginPage::categoryIcon() const {
    return QIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));
}

QString TestPluginPage::name() const { return tr("TestPluginPage"); }

QString TestPluginPage::id() const { return QStringLiteral("TestPluginPage"); }
