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

#ifndef TESTSETTINGPAGE_H
#define TESTSETTINGPAGE_H

#include "WingPlugin/settingpage.h"

#include <QLabel>

class TestSettingPage : public WingHex::SettingPage {
    Q_OBJECT
public:
    explicit TestSettingPage(const QString &id, const QString &content,
                             bool isShowInRibbon, QWidget *parent = nullptr);

    // PageBase interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;

    virtual bool showInRibbon() const override;

    // SettingPage interface
public:
    virtual void restore() override;

private:
    QLabel *_lbl = nullptr;
    bool _isShownInRibbton;
    QString _id;
};

#endif // TESTSETTINGPAGE_H
