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
        ALL = APP | PLUGIN | EDITOR | SCRIPT | OTHER
    };
    Q_DECLARE_FLAGS(SETTINGS, SETTING)

public:
    static SettingManager &instance();
    virtual ~SettingManager();

    QByteArray dockLayout() const;
    int themeID() const;
    QString defaultLang() const;
    bool enablePlgInRoot() const;
    int appfontSize() const;
    int editorfontSize() const;
    bool editorShowcol() const;
    bool editorShowtext() const;
    qsizetype copylimit() const;
    qsizetype decodeStrlimit() const;
    Qt::WindowState defaultWinState() const;

    void reset(SETTINGS cat);
    void __reset(SETTINGS cat);

    QList<RecentFileManager::RecentInfo> recentHexFiles() const;
    bool enablePlugin() const;
    bool enableHexExt() const;
    bool editorShowHeader() const;
    QString appFontFamily() const;
    QList<RecentFileManager::RecentInfo> recentScriptFiles() const;

    QStringList usrHideCats() const;
    QStringList sysHideCats() const;

    QString lastUsedPath() const;
    QString lastUsedScriptPath() const;
    QByteArray scriptDockLayout() const;
    bool useNativeFileDialog() const;
    bool useNativeTitleBar() const;
    int logLevel() const;
    bool allowUsrScriptInRoot() const;
    bool scriptEnabled() const;
    bool dontUseSplash() const;
    bool checkUpdate() const;
    qsizetype logCount() const;
    int scriptTimeout() const;

    bool scriptFileEnableOverwrite() const;
    bool scriptFileSystemWrite() const;

    QStringList enabledExtPlugins() const;
    QStringList enabledDevPlugins() const;

    QByteArray settingsLayout() const;
    QByteArray settingsScriptLayout() const;

    QStringList watchExpressions() const;
    bool findStrDecShow() const;
    QVector<bool> metaHeaderHidden() const;

public slots:
    void setThemeID(int newThemeID);
    void setDockLayout(const QByteArray &newDockLayout);
    void setDefaultLang(const QString &newDefaultLang);
    void setEnablePlgInRoot(bool newEnablePlgInRoot);
    void setAppfontSize(int newAppfontSize);
    void setEditorfontSize(int newEditorfontSize);
    void setEditorShowcol(bool newEditorShowcol);
    void setEditorShowtext(bool newEditorShowtext);
    void setCopylimit(qsizetype newCopylimit);
    void setDecodeStrlimit(qsizetype newDecodeStrlimit);
    void setDefaultWinState(Qt::WindowState newDefaultWinState);
    void
    setRecentFiles(const QList<RecentFileManager::RecentInfo> &newRecentFiles);
    void setEnablePlugin(bool newEnablePlugin);
    void setEditorShowHeader(bool newEditorShowAddr);
    void setAppFontFamily(const QString &newAppFontFamily);
    void setRecentScriptFiles(
        const QList<RecentFileManager::RecentInfo> &newRecentScriptFiles);
    void setSysHideCats(const QStringList &newSysHideCats);
    void setUsrHideCats(const QStringList &newUsrHideCats);
    void setLastUsedPath(const QString &newLastUsedPath);
    void setLastUsedScriptPath(const QString &newLastUsedPath);
    void setScriptDockLayout(const QByteArray &newScriptDockLayout);
    void setUseNativeFileDialog(bool newUseNativeFileDialog);
    void setUseNativeTitleBar(bool newUseNativeTitleBar);
    void setLogLevel(int newLogLevel);
    void setAllowUsrScriptInRoot(bool newAllowUsrScriptInRoot);
    void setScriptEnabled(bool newScriptEnabled);
    void setDontUseSplash(bool newDontUseSplash);
    void setCheckUpdate(bool newCheckUpdate);
    void setLogCount(qsizetype newLogCount);
    void setScriptTimeout(int newScriptTimeout);

    void setEnabledExtPlugins(const QStringList &newEnabledPlugins);
    void setEnabledDevPlugins(const QStringList &newEnabledDevPlugins);
    void setEnableHexExt(bool newEnableHexExt);

    void setSettingsLayout(const QByteArray &newSetLayout);
    void setSettingsScriptLayout(const QByteArray &newSetScriptLayout);
    void setWatchExpressions(const QStringList &newWatchExpressions);
    void setFindStrDecShow(bool newFindStrDecShow);
    void setMetaHeaderHidden(const QVector<bool> &newMetaHeaderHidden);

    void setScriptFileEnableOverwrite(bool b);
    void setScriptFileSystemWrite(bool b);

public:
    void checkWriteableAndWarn();

    QStringList readPluginRule(const QByteArray &data);
    QByteArray savePluginRule(const QStringList &rules);

signals:
    void sigEditorfontSizeChanged(int v);
    void sigDecodeStrlimitChanged(int v);
    void sigCopylimitChanged(int v);

    void logLevelChanged();

private:
    SettingManager();

    void load();

    QList<RecentFileManager::RecentInfo>
    getDataFromVarList(const QVariantList &varlist) const;
    QVariantList
    getVarList(const QList<RecentFileManager::RecentInfo> &infos) const;

    Q_DISABLE_COPY_MOVE(SettingManager)
private:
    int m_themeID = 0;
    int m_appfontSize = 18;
    int m_editorfontSize = 18;
    bool m_enablePlugin = true;
    bool m_enablePlgInRoot = false;
    bool m_enableHexExt = true;

    QString m_defaultLang;
    QByteArray m_dockLayout;
    QByteArray m_scriptDockLayout;
    QByteArray m_setLayout;
    QByteArray m_setScriptLayout;
    QString m_appFontFamily;

    QStringList m_watchExpressions;

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

    QStringList m_enabledExtPlugins;
    QStringList m_enabledDevPlugins;
    QString m_lastUsedPath;
    QString m_lastUsedScriptPath;

    bool m_scriptFileEnableOverwrite = false;
    bool m_scriptFileSystemWrite = false;

    bool m_dontUseSplash = false;
    bool m_useNativeFileDialog = true;
    bool m_useNativeTitleBar = false;
    bool m_checkUpdate = false;
    int m_logLevel = 0;
    qsizetype m_logCount = 20;

    int m_scriptTimeout = 60; // min

    QVector<bool> m_metaHeaderHidden; // fixed size = 5
    bool m_findStrDecShow = true;

private:
    QFont _defaultFont;
};

#endif // SETTINGMANAGER_H
