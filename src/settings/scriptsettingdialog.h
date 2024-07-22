#ifndef SCRIPTSETTINGDIALOG_H
#define SCRIPTSETTINGDIALOG_H

#include "src/plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class ScriptSettingDialog;
}

class ScriptSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit ScriptSettingDialog(QWidget *parent = nullptr);
    virtual ~ScriptSettingDialog() override;

private:
    Ui::ScriptSettingDialog *ui;

    QStringList m_usrDisplayCats;
    QStringList m_sysDisplayCats;

private:
    void loadData();
    void addCatagory(const QString &cat, bool isUser, bool checked);

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual bool isInPluginPage() const override;
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;

    QStringList usrDisplayCats() const;

    QStringList sysDisplayCats() const;

private slots:
    void on_btnRefresh_clicked();
};

#endif // SCRIPTSETTINGDIALOG_H
