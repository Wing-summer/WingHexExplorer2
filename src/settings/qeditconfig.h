#ifndef QEDITCONFIG_H
#define QEDITCONFIG_H

#include "WingPlugin/settingpage.h"
#include "control/codeedit.h"
#include <QWidget>

namespace Ui {
class QEditConfig;
}

class QEditConfig : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit QEditConfig(bool isConsole, QWidget *parent = nullptr);
    virtual ~QEditConfig();

public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual QString id() const override;
    virtual void apply() override;
    virtual void cancel() override;
    virtual void reset() override;

private:
    void reload();
    void loadThemes();
    void loadIndentations();

private:
    Ui::QEditConfig *ui;

    CodeEdit *_edit;
};

#endif // QEDITCONFIG_H
