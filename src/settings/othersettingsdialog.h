#ifndef OTHERSETTINGSDIALOG_H
#define OTHERSETTINGSDIALOG_H

#include "plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class OtherSettingsDialog;
}

class OtherSettingsDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit OtherSettingsDialog(QWidget *parent = nullptr);
    virtual ~OtherSettingsDialog();

private:
    Ui::OtherSettingsDialog *ui;

    void reload();

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual bool isInPluginPage() const override;
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;
};

#endif // OTHERSETTINGSDIALOG_H
