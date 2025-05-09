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

#include "WingPlugin/wingpluginwidget.h"

namespace Ui {
class TestForm;
}

class TestForm : public WingHex::WingPluginWidget {
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

    void on_btnAboutQt_clicked();

    void on_btnQuestion_clicked();

    void on_btnWarning_clicked();

    void on_btnCritical_clicked();

    void on_btnAbout_clicked();

    void on_btnMsgBox_clicked();

    void on_btnText_clicked();

    void on_btnMultiLineText_clicked();

    void on_btnItem_clicked();

    void on_btnInt_clicked();

    void on_btnDouble_clicked();

    void on_btnExistingDirectory_clicked();

    void on_btnOpenFileName_clicked();

    void on_btnOpenFileNames_clicked();

    void on_btnSaveFileName_clicked();

    void on_btnGetColor_clicked();

    void on_btnStatusVisible_clicked();

    void on_btnStatusInvisible_clicked();

private:
    void initLogCombo();

    void initStyleCombo();

    void initMsgBoxCheckedBtnCombo();

    void initMsgBoxBtnCombo();

    void initMsgBoxIconCombo();

    void initFileDialogOps();

private:
    QMessageBox::StandardButtons getMsgButtons() const;

    QFileDialog::Options getFileDialogOptions() const;

private:
    Ui::TestForm *ui;
};

#endif // TESTFORM_H
