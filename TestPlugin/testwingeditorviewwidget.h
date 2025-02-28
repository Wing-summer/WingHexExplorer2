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
