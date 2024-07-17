#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>

namespace WingHex {

class SettingPage : public QWidget {
public:
    SettingPage();

public:
    virtual QIcon categoryIcon() const = 0;
};

} // namespace WingHex

#endif // SETTINGPAGE_H
