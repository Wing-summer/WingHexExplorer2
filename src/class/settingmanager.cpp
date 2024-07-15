#include "settingmanager.h"

#include "setting.h"

constexpr auto SKIN_THEME = "skin.theme";
constexpr auto DOCK_LAYOUT = "dock.layout";

SettingManager::SettingManager() {
    HANDLE_CONFIG;
    READ_CONFIG_INT(m_themeID, SKIN_THEME, 0);
    m_dockLayout = READ_CONFIG(DOCK_LAYOUT, QByteArray()).toByteArray();
}

SettingManager &SettingManager::instance() {
    static SettingManager ins;
    return ins;
}

SettingManager::~SettingManager() {
    HANDLE_CONFIG;
    WRITE_CONFIG(SKIN_THEME, m_themeID);
    WRITE_CONFIG(DOCK_LAYOUT, m_dockLayout);
}

QByteArray SettingManager::dockLayout() const { return m_dockLayout; }

int SettingManager::themeID() const { return m_themeID; }

void SettingManager::setThemeID(int newThemeID) { m_themeID = newThemeID; }

void SettingManager::setDockLayout(const QByteArray &newDockLayout) {
    m_dockLayout = newDockLayout;
}
