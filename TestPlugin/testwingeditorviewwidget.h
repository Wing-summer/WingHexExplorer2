#ifndef TESTWINGEDITORVIEWWIDGET_H
#define TESTWINGEDITORVIEWWIDGET_H

#include "iwingplugin.h"

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
        virtual WingEditorViewWidget *create(QWidget *parent) const override {
            return new TestWingEditorViewWidget(parent);
        }
    };

public:
    explicit TestWingEditorViewWidget(QWidget *parent = nullptr);

    // WingEditorViewWidget interface

public slots:
    virtual void toggled(bool isVisible) override;
    virtual WingHex::WingEditorViewWidget *clone() override;

private:
    QLabel *_lbl = nullptr;
};

#endif // TESTWINGEDITORVIEWWIDGET_H
