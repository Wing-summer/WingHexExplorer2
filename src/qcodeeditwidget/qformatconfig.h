#ifndef QFORMATCONFIG_H
#define QFORMATCONFIG_H

#include "WingPlugin/settingpage.h"
#include "formatconfigmodel.h"
#include <QWidget>

namespace Ui {
class QFormatConfig;
}

class QFormatScheme;

class QFormatConfig : public WingHex::SettingPage {
    Q_OBJECT

public:
    explicit QFormatConfig(const QHash<QString, QFormatScheme *> &schemes,
                           const QString &defaultScheme,
                           QWidget *parent = nullptr);
    virtual ~QFormatConfig();

public:
    virtual QIcon categoryIcon() const override;
    virtual QString name() const override;
    virtual void apply() override;
    virtual void cancel() override;
    virtual void reset() override;
    virtual QString id() const override;

private:
    void addScheme(const QString &name, QFormatScheme *scheme);

    void setCurrentScheme(QFormatScheme *scheme);

private:
    Ui::QFormatConfig *ui;
    FormatConfigModel *m_model;

    bool m_autonomous;
    QFormatScheme *m_currentScheme;
    QList<QFormatScheme *> m_schemes;
};

#endif // QFORMATCONFIG_H
