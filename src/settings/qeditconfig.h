#ifndef QEDITCONFIG_H
#define QEDITCONFIG_H

#include "WingPlugin/settingpage.h"
#include "wingcodeedit.h"

#include <QIcon>
#include <QString>
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

    WingCodeEdit *_edit;
    bool m_isConsole;

    QString _name;
    QString _id;
    QIcon _icon;
};

#endif // QEDITCONFIG_H
