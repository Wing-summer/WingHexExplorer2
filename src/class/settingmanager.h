#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QByteArray>

class SettingManager {
public:
    static SettingManager &instance();

    ~SettingManager();

    QByteArray dockLayout() const;

    int themeID() const;

    void setThemeID(int newThemeID);

    void setDockLayout(const QByteArray &newDockLayout);

private:
    SettingManager();

private:
    int m_themeID = 0;
    QByteArray m_dockLayout;
};

#endif // SETTINGMANAGER_H
