#ifndef QFORMATCONFIG_H
#define QFORMATCONFIG_H

#include "plugin/settingpage.h"
#include <QWidget>

namespace Ui {
class QFormatConfig;
}

class QFormatScheme;

class QFormatConfig : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit QFormatConfig(QWidget *parent = nullptr);
    virtual ~QFormatConfig();

    bool isAutonomous() const;

    bool hasUnsavedChanges() const;

    QList<QFormatScheme *> schemes() const;

public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual bool isInPluginPage() const override;
    virtual void apply() override;
    virtual void cancel() override;
    virtual void reset() override;

public slots:
    void setAutonomous(bool y);

    void addScheme(const QString &name, QFormatScheme *scheme);
    void removeScheme(QFormatScheme *scheme);

    void setCurrentScheme(QFormatScheme *scheme);

protected:
    virtual void hideEvent(QHideEvent *e) override;

private slots:
    void on_m_selector_currentIndexChanged(int idx);

private:
    QList<int> modifiedFormats() const;

private:
    Ui::QFormatConfig *ui;
    bool m_autonomous;
    QFormatScheme *m_currentScheme;
    QList<QFormatScheme *> m_schemes;
};

#endif // QFORMATCONFIG_H
