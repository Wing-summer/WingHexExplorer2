#ifndef SCRIPTSETTINGDIALOG_H
#define SCRIPTSETTINGDIALOG_H

#include "../plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class ScriptSettingDialog;
}

class ScriptSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit ScriptSettingDialog(QWidget *parent = nullptr);
    ~ScriptSettingDialog();

private:
    Ui::ScriptSettingDialog *ui;

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual bool isInPluginPage() const override;
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;
};

#endif // SCRIPTSETTINGDIALOG_H
