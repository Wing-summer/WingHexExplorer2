#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>

namespace WingHex {

class SettingPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingPage(QWidget *parent = nullptr);

public:
    virtual QIcon categoryIcon() const = 0;
    virtual QString name() const = 0;
    virtual bool isInPluginPage() const = 0;
    virtual void apply() = 0;
    virtual void reset() = 0;
    virtual void cancel() = 0;
};

} // namespace WingHex

#endif // SETTINGPAGE_H
