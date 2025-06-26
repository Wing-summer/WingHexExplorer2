/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
** =============================================================================
*/

#include "testsettingpage.h"

#include <QVBoxLayout>

TestSettingPage::TestSettingPage(const QString &id, const QString &content,
                                 bool isShowInRibbon, QWidget *parent)
    : WingHex::SettingPage(parent), _id(id), _isShownInRibbton(isShowInRibbon) {
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

bool TestSettingPage::showInRibbon() const { return _isShownInRibbton; }

void TestSettingPage::apply() {}

void TestSettingPage::reset() {}

void TestSettingPage::cancel() {}
