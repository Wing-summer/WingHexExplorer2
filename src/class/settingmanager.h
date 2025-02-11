/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QByteArray>
#include <QFont>
#include <QObject>
#include <QString>
#include <QStringList>

#include "recentfilemanager.h"

class SettingManager : public QObject {
    Q_OBJECT

public:
    enum SETTING {
        NONE = 0,
        APP = 1,
        PLUGIN = 2,
        EDITOR = 4,
        SCRIPT = 8,
        OTHER = 16,
        CODEEDIT = 32,
        ALL = APP | PLUGIN | EDITOR | SCRIPT | OTHER | CODEEDIT
    };
    Q_DECLARE_FLAGS(SETTINGS, SETTING)
private:
    // Flags to indicate whether the modification has been made.
    // There are a maximum of 32 flags,
    // but it is impossible to have more than this.
    enum class SETTING_ITEM : quint32 {
        DOCK_LAYOUT = 1u,
        SCRIPT_DOCK_LAYOUT = 1u << 1,
        APP_LASTUSED_PATH = 1u << 2,
        SKIN_THEME = 1u << 3,
        APP_FONTFAMILY = 1u << 4,
        APP_FONTSIZE = 1u << 5,
        APP_WINDOWSIZE = 1u << 6,
        APP_LANGUAGE = 1u << 7,
        PLUGIN_ENABLE = 1u << 8,
        PLUGIN_ENABLE_ROOT = 1u << 9,
        EDITOR_FONTSIZE = 1u << 10,
        EDITOR_SHOW_ADDR = 1u << 11,
        EDITOR_SHOW_COL = 1u << 12,
        EDITOR_SHOW_TEXT = 1u << 13,
        // EDITOR_ENCODING = 1u << 14, // Reserved
        EDITOR_FIND_MAXCOUNT = 1u << 15,
        EDITOR_COPY_LIMIT = 1u << 16,
        EDITOR_DECSTRLIMIT = 1u << 17,
        EDITOR_RECENTFILES = 1u << 18,
        SCRIPT_RECENTFILES = 1u << 19,
        SCRIPT_ENABLE = 1u << 20,
        SCRIPT_ALLOW_USRSCRIPT_INROOT = 1u << 21,
        SCRIPT_USRHIDECATS = 1u << 22,
        SCRIPT_SYSHIDECATS = 1u << 23,
        OTHER_USESYS_FILEDIALOG = 1u << 24,
        OTHER_USE_NATIVE_TITLEBAR = 1u << 25,
        OTHER_DONT_USE_SPLASH = 1u << 26,
        OTHER_LOG_LEVEL = 1u << 27,
        OTHER_CHECK_UPDATE = 1u << 28,
        OTHER_LOG_COUNT = 1u << 29,
    };
    Q_DECLARE_FLAGS(SETTING_ITEMS, SETTING_ITEM)

public:
    static SettingManager &instance();

    ~SettingManager();

    QByteArray dockLayout() const;
    void setDockLayout(const QByteArray &newDockLayout);

    int themeID() const;
    void setThemeID(int newThemeID);

    QString defaultLang() const;
    void setDefaultLang(const QString &newDefaultLang);

    bool enablePlgInRoot() const;
    void setEnablePlgInRoot(bool newEnablePlgInRoot);

    int appfontSize() const;
    void setAppfontSize(int newAppfontSize);

    int editorfontSize() const;
    void setEditorfontSize(int newEditorfontSize);

    bool editorShowcol() const;
    void setEditorShowcol(bool newEditorShowcol);

    bool editorShowtext() const;
    void setEditorShowtext(bool newEditorShowtext);

    qsizetype copylimit() const;
    void setCopylimit(qsizetype newCopylimit);

    qsizetype decodeStrlimit() const;
    void setDecodeStrlimit(qsizetype newDecodeStrlimit);

    Qt::WindowState defaultWinState() const;
    void setDefaultWinState(Qt::WindowState newDefaultWinState);

    void save(SETTINGS cat = SETTING::ALL);
    void reset(SETTINGS cat);

    QList<RecentFileManager::RecentInfo> recentHexFiles() const;
    void
    setRecentFiles(const QList<RecentFileManager::RecentInfo> &newRecentFiles);

    bool enablePlugin() const;
    void setEnablePlugin(bool newEnablePlugin);

    bool editorShowHeader() const;
    void setEditorShowHeader(bool newEditorShowAddr);

    QString appFontFamily() const;
    void setAppFontFamily(const QString &newAppFontFamily);

    QList<RecentFileManager::RecentInfo> recentScriptFiles() const;
    void setRecentScriptFiles(
        const QList<RecentFileManager::RecentInfo> &newRecentScriptFiles);

    QStringList usrHideCats() const;
    QStringList sysHideCats() const;

    void setSysHideCats(const QStringList &newSysHideCats);
    void setUsrHideCats(const QStringList &newUsrHideCats);

    QString lastUsedPath() const;
    void setLastUsedPath(const QString &newLastUsedPath);

    QByteArray scriptDockLayout() const;
    void setScriptDockLayout(const QByteArray &newScriptDockLayout);

    bool useNativeFileDialog() const;
    void setUseNativeFileDialog(bool newUseNativeFileDialog);

    bool useNativeTitleBar() const;
    void setUseNativeTitleBar(bool newUseNativeTitleBar);

    int logLevel() const;
    void setLogLevel(int newLogLevel);

    bool allowUsrScriptInRoot() const;
    void setAllowUsrScriptInRoot(bool newAllowUsrScriptInRoot);

    bool scriptEnabled() const;
    void setScriptEnabled(bool newScriptEnabled);

    bool dontUseSplash() const;
    void setDontUseSplash(bool newDontUseSplash);

    bool checkUpdate() const;
    void setCheckUpdate(bool newCheckUpdate);

    qsizetype logCount() const;
    void setLogCount(qsizetype newLogCount);

public:
    void checkWriteableAndWarn();

signals:
    void sigEditorfontSizeChanged(int v);
    void sigDecodeStrlimitChanged(int v);
    void sigCopylimitChanged(int v);

    void logLevelChanged();

private:
    void loadCodeEditorConfig();
    void saveCodeEditorConfig();

private:
    SettingManager();

    void load();

    QList<RecentFileManager::RecentInfo>
    getDataFromVarList(const QVariantList &varlist) const;
    QVariantList
    getVarList(const QList<RecentFileManager::RecentInfo> &infos) const;

    Q_DISABLE_COPY(SettingManager)
private:
    int m_themeID = 0;
    int m_appfontSize = 18;
    int m_editorfontSize = 18;
    bool m_enablePlugin = true;
    bool m_enablePlgInRoot = false;
    QString m_defaultLang;
    QByteArray m_dockLayout;
    QByteArray m_scriptDockLayout;
    QString m_appFontFamily;

    bool m_editorShowHeader = true;
    bool m_editorShowcol = true;
    bool m_editorShowtext = true;

    qsizetype m_copylimit = 100;
    qsizetype m_decodeStrlimit = 10;

    QList<RecentFileManager::RecentInfo> m_recentHexFiles;
    QList<RecentFileManager::RecentInfo> m_recentScriptFiles;
    Qt::WindowState m_defaultWinState = Qt::WindowMaximized;

    bool m_scriptEnabled = true;
    bool m_allowUsrScriptInRoot = false;
    QStringList m_usrHideCats;
    QStringList m_sysHideCats;

    QString m_lastUsedPath;

    bool m_dontUseSplash = false;
    bool m_useNativeFileDialog = true;
    bool m_useNativeTitleBar = false;
    bool m_checkUpdate = false;
    int m_logLevel = 0;
    qsizetype m_logCount = 20;

private:
    QFont _defaultFont;
    SETTING_ITEMS _setUnsaved;
};

#endif // SETTINGMANAGER_H
