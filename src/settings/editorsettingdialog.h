#ifndef EDITORSETTINGDIALOG_H
#define EDITORSETTINGDIALOG_H

#include "plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class EditorSettingDialog;
}

class EditorSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit EditorSettingDialog(QWidget *parent = nullptr);
    ~EditorSettingDialog();

private:
    Ui::EditorSettingDialog *ui;

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

#endif // EDITORSETTINGDIALOG_H
