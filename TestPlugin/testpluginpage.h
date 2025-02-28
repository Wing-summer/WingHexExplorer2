#ifndef TESTPLUGINPAGE_H
#define TESTPLUGINPAGE_H

#include "WingPlugin/settingpage.h"

#include <QLabel>

class TestPluginPage : public WingHex::PluginPage {
    Q_OBJECT
public:
    explicit TestPluginPage(QWidget *parent = nullptr);

    // PageBase interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;

private:
    QLabel *_lbl = nullptr;
};

#endif // TESTPLUGINPAGE_H
