#ifndef PLUGINSETTINGDIALOG_H
#define PLUGINSETTINGDIALOG_H

#include "../plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class PluginSettingDialog;
}

class PluginSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit PluginSettingDialog(QWidget *parent = nullptr);
    ~PluginSettingDialog();

    void buildUp(const QList<WingHex::SettingPage *> &pages);

private:
    Ui::PluginSettingDialog *ui;
    QList<WingHex::SettingPage *> _pages;

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

#endif // PLUGINSETTINGDIALOG_H
