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

#ifndef TESTWINGEDITORVIEWWIDGET_H
#define TESTWINGEDITORVIEWWIDGET_H

#include "WingPlugin/iwingplugin.h"

#include <QLabel>

class TestWingEditorViewWidget : public WingHex::WingEditorViewWidget {
    Q_OBJECT

public:
    class Creator : public WingHex::WingEditorViewWidget::Creator {
    public:
        Creator() : WingHex::WingEditorViewWidget::Creator() {}

    public:
        virtual QIcon icon() const override;
        virtual QString name() const override;
        virtual QString id() const override;

    public:
        virtual WingEditorViewWidget *create(WingHex::IWingPlugin *plg,
                                             QWidget *parent) const override {
            return new TestWingEditorViewWidget(plg, parent);
        }
    };

public:
    explicit TestWingEditorViewWidget(WingHex::IWingPlugin *plg,
                                      QWidget *parent = nullptr);
    virtual ~TestWingEditorViewWidget();

    void setEnableMeta(bool b);

    // WingEditorViewWidget interface
    static QList<TestWingEditorViewWidget *> instances();

public slots:
    virtual void toggled(bool isVisible) override;
    virtual WingHex::WingEditorViewWidget *clone() override;

    virtual void loadState(const QByteArray &state) override;
    virtual bool hasUnsavedState() override;
    virtual QByteArray saveState() override;

    virtual void onWorkSpaceNotify(bool isWorkSpace) override;

private:
    QLabel *_lbl = nullptr;

    bool m_unSaved = false;
    static QList<TestWingEditorViewWidget *> m_instances;
};

#endif // TESTWINGEDITORVIEWWIDGET_H
