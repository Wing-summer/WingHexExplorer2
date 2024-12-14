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

#ifndef TESTFORM_H
#define TESTFORM_H

#include "iwingplugin.h"

#include <QWidget>

namespace Ui {
class TestForm;
}

class TestForm : public QWidget {
    Q_OBJECT

public:
    // copy from LogError
    enum Level : int {
        q1ERROR,
        q2WARN,
        q3INFO,
        q4DEBUG,
        q5TRACE,
        LEVEL_BEGIN = q1ERROR,
        LEVEL_LAST = q5TRACE,
        LEVEL_END = LEVEL_LAST + 1
    };
    Q_ENUM(Level)

public:
    explicit TestForm(WingHex::IWingPlugin *plg, QWidget *parent = nullptr);
    ~TestForm();

private slots:
    void on_btnSendLog_clicked();

    void on_btnSendToast_clicked();

private:
    void initLogCombo();

    void initStyleCombo();

private:
    Ui::TestForm *ui;

    WingHex::IWingPlugin *_plg;
};

#endif // TESTFORM_H
