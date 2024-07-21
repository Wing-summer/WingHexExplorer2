#ifndef GENERALSETTINGDIALOG_H
#define GENERALSETTINGDIALOG_H

#include "../plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class GeneralSettingDialog;
}

class GeneralSettingDialog : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit GeneralSettingDialog(QWidget *parent = nullptr);
    virtual ~GeneralSettingDialog() override;

signals:
    void sigAppfontSizeChanged(int v);
    void sigAppFontFamilyChanged(const QString &font);
    void sigThemeIDChanged(int id);
    void sigDefaultWinStateChanged(Qt::WindowState s);

private:
    Ui::GeneralSettingDialog *ui;

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

#endif // GENERALSETTINGDIALOG_H
