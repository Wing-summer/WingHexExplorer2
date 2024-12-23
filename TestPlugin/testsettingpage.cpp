#include "testsettingpage.h"

#include <QVBoxLayout>

TestSettingPage::TestSettingPage(const QString &id, const QString &content,
                                 QWidget *parent)
    : WingHex::SettingPage(parent), _id(id) {
    auto layout = new QVBoxLayout(this);
    _lbl = new QLabel(content, this);
    _lbl->setAlignment(Qt::AlignCenter);
    layout->addWidget(_lbl);
}

QIcon TestSettingPage::categoryIcon() const {
    return QIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));
}

QString TestSettingPage::name() const { return _id; }

QString TestSettingPage::id() const { return _id; }

void TestSettingPage::apply() {}

void TestSettingPage::reset() {}

void TestSettingPage::cancel() {}
