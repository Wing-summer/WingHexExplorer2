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

#ifndef READERTESTFORM_H
#define READERTESTFORM_H

#include <QTextBrowser>
#include <QWidget>

#include "WingPlugin/iwingplugin.h"

namespace Ui {
class ReaderTestForm;
}

class ReaderTestForm : public QWidget, public WingHex::IWingPluginCalls {
    Q_OBJECT

public:
    explicit ReaderTestForm(WingHex::IWingPlugin *plg, QTextBrowser *br,
                            QWidget *parent = nullptr);
    ~ReaderTestForm();

private slots:
    void on_btnReadInt8_clicked();

    void on_btnReadInt16_clicked();

    void on_btnReadInt32_clicked();

    void on_btnReadInt64_clicked();

    void on_btnReadFloat_clicked();

    void on_btnReadDouble_clicked();

    void on_btnReadString_clicked();

    void on_btnReadByteArray_clicked();

    void on_btnStatus_clicked();

private:
    QString getPrintableHexPosition(const WingHex::HexPosition &pos);

private:
    Ui::ReaderTestForm *ui;

    WingHex::IWingPlugin *_plg;
    QTextBrowser *_br;
};

#endif // READERTESTFORM_H
