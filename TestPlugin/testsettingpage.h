#ifndef TESTSETTINGPAGE_H
#define TESTSETTINGPAGE_H

#include "settingpage.h"

#include <QLabel>

class TestSettingPage : public WingHex::SettingPage {
    Q_OBJECT
public:
    explicit TestSettingPage(const QString &id, const QString &content,
                             QWidget *parent = nullptr);

    // PageBase interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;

    // SettingPage interface
public:
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;

private:
    QLabel *_lbl = nullptr;

    QString _id;
};

#endif // TESTSETTINGPAGE_H
