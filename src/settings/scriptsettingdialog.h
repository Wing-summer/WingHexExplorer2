#ifndef SCRIPTSETTINGDIALOG_H
#define SCRIPTSETTINGDIALOG_H

#include "class/scriptmanager.h"
#include "plugin/settingpage.h"
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

    QStringList m_usrHideCats;
    QStringList m_sysHideCats;

private:
    void loadData();
    bool addCatagory(const ScriptManager::ScriptDirMeta &meta, bool isUser,
                     bool hidden);

    // SettingPage interface
public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual bool isInPluginPage() const override;
    virtual void apply() override;
    virtual void reset() override;
    virtual void cancel() override;

private slots:
    void on_btnRefresh_clicked();
    void on_listWidget_currentRowChanged(int currentRow);
};

#endif // SCRIPTSETTINGDIALOG_H
