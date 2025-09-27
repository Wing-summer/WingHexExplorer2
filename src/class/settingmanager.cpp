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

#include "settingmanager.h"

#include "class/logger.h"
#include "class/skinmanager.h"
#include "settings/settings.h"

#include "utilities.h"

#include <QApplication>
#include <QFileInfo>
#include <QMetaEnum>

Q_GLOBAL_STATIC_WITH_ARGS(QString, DOCK_LAYOUT, ("dock.layout"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_DOCK_LAYOUT, ("script.layout"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_LASTUSED_PATH, ("app.lastusedpath"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SKIN_THEME, ("skin.theme"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_FONTFAMILY, ("app.fontfamily"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_FONTSIZE, ("app.fontsize"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_WINDOWSIZE, ("app.windowsize"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, APP_LANGUAGE, ("app.lang"))

Q_GLOBAL_STATIC_WITH_ARGS(QString, PLUGIN_ENABLE, ("plugin.enableplugin"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, PLUGIN_ENABLE_MANAGER, ("plugin.enableman"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, PLUGIN_ENABLE_HEXEXT,
                          ("plugin.enablehexext"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, PLUGIN_ENABLE_ROOT,
                          ("plugin.rootenableplugin"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, PLUGIN_ENABLEDPLUGINS_EXT, ("plugin.ext"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, PLUGIN_ENABLEDPLUGINS_DEV, ("plugin.dev"))

Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_FONTSIZE, ("editor.fontsize"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_SHOW_ADDR, ("editor.showaddr"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_SHOW_COL, ("editor.showcol"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_SHOW_TEXT, ("editor.showtext"))

Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_FIND_MAXCOUNT,
                          ("editor.findmaxcount"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_COPY_LIMIT, ("editor.copylimit"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_DECSTRLIMIT, ("editor.decstrlimit"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, EDITOR_RECENTFILES, ("editor.recentfiles"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_RECENTFILES, ("script.recentfiles"))

Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_ALLOW_USRSCRIPT_INROOT,
                          ("script.allowUsrScriptRoot"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_ENABLE, ("script.enable"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_TIMEOUT, ("script.timeout"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_USRHIDECATS, ("script.usrHideCats"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_SYSHIDECATS, ("script.sysHideCats"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, SCRIPT_WATCH_VARS, ("script.watchexps"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_USESYS_FILEDIALOG,
                          ("sys.nativeDialog"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_USE_NATIVE_TITLEBAR,
                          ("sys.nativeTitleBar"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_DONT_USE_SPLASH, ("sys.dontUseSplash"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_LOG_LEVEL, ("sys.loglevel"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_LOG_COUNT, ("sys.logCount"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_CHECK_UPDATE, ("sys.checkUpdate"))

Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_SET_LAYOUT, ("set.layout"))
Q_GLOBAL_STATIC_WITH_ARGS(QString, OTHER_SET_SCRIPT_LAYOUT,
                          ("set.script.layout"))

SettingManager::SettingManager() {
    _defaultFont = qApp->font();
    qRegisterMetaType<RecentFileManager::RecentInfo>();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaTypeStreamOperators<RecentFileManager::RecentInfo>();
#endif
    load();
}

QString SettingManager::lastUsedPath() const { return m_lastUsedPath; }

void SettingManager::setLastUsedPath(const QString &newLastUsedPath) {
    if (m_lastUsedPath != newLastUsedPath) {
        HANDLE_CONFIG;
        WRITE_CONFIG(APP_LASTUSED_PATH, newLastUsedPath);
        m_lastUsedPath = newLastUsedPath;
    }
}

void SettingManager::load() {
    auto defaultFontSize = _defaultFont.pointSize();
    Q_ASSERT(defaultFontSize > 0);
    HANDLE_CONFIG;

    READ_CONFIG_INT(m_themeID, SKIN_THEME, 0);
    m_themeID = qBound(0, m_themeID,
                       QMetaEnum::fromType<SkinManager::Theme>().keyCount());
    m_defaultLang = READ_CONFIG(APP_LANGUAGE, {}).toString();

    m_dockLayout = READ_CONFIG(DOCK_LAYOUT, {}).toByteArray();
    m_scriptDockLayout = READ_CONFIG(SCRIPT_DOCK_LAYOUT, {}).toByteArray();
    READ_CONFIG_STRING(m_appFontFamily, APP_FONTFAMILY, _defaultFont.family());

    // check font
    QFont fm(m_appFontFamily);
    if (!QFontInfo(fm).exactMatch()) {
        m_appFontFamily = _defaultFont.family();
    }
    READ_CONFIG_INT_POSITIVE(m_appfontSize, APP_FONTSIZE, defaultFontSize);
    this->setDefaultWinState(READ_CONFIG(APP_WINDOWSIZE, Qt::WindowMaximized)
                                 .value<Qt::WindowState>());

    READ_CONFIG_BOOL(m_enablePlugin, PLUGIN_ENABLE, true);
    READ_CONFIG_BOOL(m_enablePlgInRoot, PLUGIN_ENABLE_ROOT, false);
    READ_CONFIG_BOOL(m_enableMonitor, PLUGIN_ENABLE_MANAGER, true);
    READ_CONFIG_BOOL(m_enableHexExt, PLUGIN_ENABLE_HEXEXT, true);

    {
        auto data = READ_CONFIG(PLUGIN_ENABLEDPLUGINS_EXT, {});
        if (data.isValid()) {
            auto rawRules = data.toByteArray();
            m_enabledExtPlugins = readPluginRule(rawRules);
        }

        data = READ_CONFIG(PLUGIN_ENABLEDPLUGINS_DEV, {});
        if (data.isValid()) {
            auto rawRules = data.toByteArray();
            m_enabledDevPlugins = readPluginRule(rawRules);
        }
    }

    READ_CONFIG_INT_POSITIVE(m_editorfontSize, EDITOR_FONTSIZE,
                             defaultFontSize);
    m_editorfontSize = qBound(5, m_editorfontSize, 25);
    READ_CONFIG_BOOL(m_editorShowHeader, EDITOR_SHOW_ADDR, true);
    READ_CONFIG_BOOL(m_editorShowcol, EDITOR_SHOW_COL, true);
    READ_CONFIG_BOOL(m_editorShowtext, EDITOR_SHOW_TEXT, true);

    READ_CONFIG_BOOL(m_dontUseSplash, OTHER_DONT_USE_SPLASH, false);
    READ_CONFIG_BOOL(m_useNativeFileDialog, OTHER_USESYS_FILEDIALOG, true);
#ifdef WINGHEX_USE_FRAMELESS
    READ_CONFIG_BOOL(m_useNativeTitleBar, OTHER_USE_NATIVE_TITLEBAR, false);
#else
    m_useNativeTitleBar = true;
#endif
    READ_CONFIG_INT_POSITIVE(m_logLevel, OTHER_LOG_LEVEL,
                             Logger::defaultLevel());
    READ_CONFIG_BOOL(m_checkUpdate, OTHER_CHECK_UPDATE, false);
    m_logLevel =
        qBound(int(Logger::LEVEL_BEGIN), m_logLevel, int(Logger::LEVEL_LAST));

    READ_CONFIG_INT_POSITIVE(m_logCount, OTHER_LOG_COUNT, 20);
    m_logCount = qBound(qsizetype(20), m_logCount, qsizetype(100));

    READ_CONFIG_QSIZETYPE(m_copylimit, EDITOR_COPY_LIMIT, 100);
    m_copylimit = qBound(qsizetype(100), m_copylimit, qsizetype(1024));
    READ_CONFIG_QSIZETYPE(m_decodeStrlimit, EDITOR_DECSTRLIMIT, 100);
    m_decodeStrlimit =
        qBound(qsizetype(100), m_decodeStrlimit, qsizetype(1024));
    m_recentHexFiles =
        getDataFromVarList(READ_CONFIG(EDITOR_RECENTFILES, {}).toList());
    m_recentScriptFiles =
        getDataFromVarList(READ_CONFIG(SCRIPT_RECENTFILES, {}).toList());

    READ_CONFIG_BOOL(m_scriptEnabled, SCRIPT_ENABLE, true);
    READ_CONFIG_BOOL(m_allowUsrScriptInRoot, SCRIPT_ALLOW_USRSCRIPT_INROOT,
                     false);
    READ_CONFIG_INT(m_scriptTimeout, SCRIPT_TIMEOUT, 10);
    m_scriptTimeout = qBound(0, m_scriptTimeout, 312480);
    m_usrHideCats = READ_CONFIG(SCRIPT_USRHIDECATS, {}).toStringList();
    m_sysHideCats = READ_CONFIG(SCRIPT_SYSHIDECATS, {}).toStringList();
    m_watchExpressions = READ_CONFIG(SCRIPT_WATCH_VARS, {}).toStringList();

    m_lastUsedPath = READ_CONFIG(APP_LASTUSED_PATH, {}).toString();
    if (!m_lastUsedPath.isEmpty()) {
        QFileInfo info(m_lastUsedPath);
        if (info.exists() && info.isDir()) {
        } else {
            m_lastUsedPath.clear();
        }
    }

    m_setLayout = READ_CONFIG(OTHER_SET_LAYOUT, {}).toByteArray();
    m_setScriptLayout = READ_CONFIG(OTHER_SET_SCRIPT_LAYOUT, {}).toByteArray();

    Q_EMIT sigEditorfontSizeChanged(m_editorfontSize);
    Q_EMIT sigCopylimitChanged(m_copylimit);
    Q_EMIT sigDecodeStrlimitChanged(m_decodeStrlimit);
}

QList<RecentFileManager::RecentInfo>
SettingManager::getDataFromVarList(const QVariantList &varlist) const {
    QList<RecentFileManager::RecentInfo> infos;
    for (auto &var : varlist) {
        if (var.canConvert<RecentFileManager::RecentInfo>()) {
            infos.append(var.value<RecentFileManager::RecentInfo>());
        }
    }
    return infos;
}

QVariantList SettingManager::getVarList(
    const QList<RecentFileManager::RecentInfo> &infos) const {
    QVariantList varlist;
    for (auto &info : infos) {
        varlist.append(QVariant::fromValue(info));
    }
    return varlist;
}

QStringList SettingManager::watchExpressions() const {
    return m_watchExpressions;
}

void SettingManager::setWatchExpressions(
    const QStringList &newWatchExpressions) {
    if (m_watchExpressions != newWatchExpressions) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_WATCH_VARS, newWatchExpressions);
        m_watchExpressions = newWatchExpressions;
    }
}

QByteArray SettingManager::settingsScriptLayout() const {
    return m_setScriptLayout;
}

void SettingManager::setSettingsScriptLayout(
    const QByteArray &newSetScriptLayout) {
    if (m_setScriptLayout != newSetScriptLayout) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_SET_SCRIPT_LAYOUT, newSetScriptLayout);
        m_setScriptLayout = newSetScriptLayout;
    }
}

QByteArray SettingManager::settingsLayout() const { return m_setLayout; }

void SettingManager::setSettingsLayout(const QByteArray &newSetLayout) {
    if (m_setLayout != newSetLayout) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_SET_LAYOUT, newSetLayout);
        m_setLayout = newSetLayout;
    }
}

bool SettingManager::enableHexExt() const { return m_enableHexExt; }

void SettingManager::setEnableHexExt(bool newEnableHexExt) {
    if (m_enableHexExt != newEnableHexExt) {
        HANDLE_CONFIG;
        WRITE_CONFIG(PLUGIN_ENABLE_HEXEXT, newEnableHexExt);
        m_enableHexExt = newEnableHexExt;
    }
}

bool SettingManager::enableMonitor() const { return m_enableMonitor; }

void SettingManager::setEnableMonitor(bool newEnableMonitor) {
    if (m_enableMonitor != newEnableMonitor) {
        HANDLE_CONFIG;
        WRITE_CONFIG(PLUGIN_ENABLE_MANAGER, newEnableMonitor);
        m_enableMonitor = newEnableMonitor;
    }
}

QStringList SettingManager::enabledDevPlugins() const {
    return m_enabledDevPlugins;
}

void SettingManager::setEnabledDevPlugins(
    const QStringList &newEnabledDevPlugins) {
    if (m_enabledDevPlugins != newEnabledDevPlugins) {
        HANDLE_CONFIG;
        WRITE_CONFIG(PLUGIN_ENABLEDPLUGINS_DEV,
                     savePluginRule(newEnabledDevPlugins));
        m_enabledDevPlugins = newEnabledDevPlugins;
    }
}

QStringList SettingManager::enabledExtPlugins() const {
    return m_enabledExtPlugins;
}

void SettingManager::setEnabledExtPlugins(
    const QStringList &newEnabledPlugins) {
    if (m_enabledExtPlugins != newEnabledPlugins) {
        HANDLE_CONFIG;
        WRITE_CONFIG(PLUGIN_ENABLEDPLUGINS_EXT,
                     savePluginRule(newEnabledPlugins));
        m_enabledExtPlugins = newEnabledPlugins;
    }
}

int SettingManager::scriptTimeout() const { return m_scriptTimeout; }

void SettingManager::setScriptTimeout(int newScriptTimeout) {
    newScriptTimeout = qBound(0, newScriptTimeout, 312480);
    if (m_scriptTimeout != newScriptTimeout) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_TIMEOUT, newScriptTimeout);
        m_scriptTimeout = newScriptTimeout;
    }
}

qsizetype SettingManager::logCount() const { return m_logCount; }

void SettingManager::setLogCount(qsizetype newLogCount) {
    if (m_logCount != newLogCount) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_LOG_COUNT, newLogCount);
        m_logCount = newLogCount;
    }
}

void SettingManager::checkWriteableAndWarn() {
    HANDLE_CONFIG;
    if (!CONFIG.isWritable()) {
        Logger::warning(QStringLiteral("<i><u>") + tr("ConfigUnableSave") +
                        QStringLiteral("</u></i>"));
    }
}

QStringList SettingManager::readPluginRule(const QByteArray &data) {
    if (!data.isEmpty()) {
        auto up = qUncompress(data);
        if (!up.isEmpty()) {
            auto rules = QString::fromUtf8(up);
            auto rlist = rules.split('|', Qt::SkipEmptyParts);
            rlist.removeIf([](const QString &str) {
                return !Utilities::isValidIdentifier(str);
            });
            return rlist;
        }
    }
    return {};
}

QByteArray SettingManager::savePluginRule(const QStringList &rules) {
    auto plgs = rules.join('|');
    auto data = qCompress(plgs.toUtf8());
    return data;
}

bool SettingManager::checkUpdate() const { return m_checkUpdate; }

void SettingManager::setCheckUpdate(bool newCheckUpdate) {
    if (m_checkUpdate != newCheckUpdate) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_CHECK_UPDATE, newCheckUpdate);
        m_checkUpdate = newCheckUpdate;
    }
}

bool SettingManager::dontUseSplash() const { return m_dontUseSplash; }

void SettingManager::setDontUseSplash(bool newDontUseSplash) {
    if (m_dontUseSplash != newDontUseSplash) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_DONT_USE_SPLASH, newDontUseSplash);
        m_dontUseSplash = newDontUseSplash;
    }
}

bool SettingManager::scriptEnabled() const { return m_scriptEnabled; }

void SettingManager::setScriptEnabled(bool newScriptEnabled) {
    if (m_scriptEnabled != newScriptEnabled) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_ENABLE, newScriptEnabled);
        m_scriptEnabled = newScriptEnabled;
    }
}

bool SettingManager::allowUsrScriptInRoot() const {
    return m_allowUsrScriptInRoot;
}

void SettingManager::setAllowUsrScriptInRoot(bool newAllowUsrScriptInRoot) {
    if (m_allowUsrScriptInRoot != newAllowUsrScriptInRoot) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_ALLOW_USRSCRIPT_INROOT, newAllowUsrScriptInRoot);
        m_allowUsrScriptInRoot = newAllowUsrScriptInRoot;
    }
}

void SettingManager::setUsrHideCats(const QStringList &newUsrHideCats) {
    if (m_usrHideCats != newUsrHideCats) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_USRHIDECATS, newUsrHideCats);
        m_usrHideCats = newUsrHideCats;
    }
}

void SettingManager::setSysHideCats(const QStringList &newSysHideCats) {
    if (m_sysHideCats != newSysHideCats) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_SYSHIDECATS, newSysHideCats);
        m_sysHideCats = newSysHideCats;
    }
}

int SettingManager::logLevel() const { return m_logLevel; }

void SettingManager::setLogLevel(int newLogLevel) {
    if (m_logLevel != newLogLevel) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_LOG_LEVEL, newLogLevel);
        m_logLevel = newLogLevel;
    }
}

bool SettingManager::useNativeTitleBar() const { return m_useNativeTitleBar; }

void SettingManager::setUseNativeTitleBar(bool newUseNativeTitleBar) {
    if (m_useNativeTitleBar != newUseNativeTitleBar) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_USE_NATIVE_TITLEBAR, newUseNativeTitleBar);
        m_useNativeTitleBar = newUseNativeTitleBar;
    }
}

bool SettingManager::useNativeFileDialog() const {
    return m_useNativeFileDialog;
}

void SettingManager::setUseNativeFileDialog(bool newUseNativeFileDialog) {
    if (m_useNativeFileDialog != newUseNativeFileDialog) {
        HANDLE_CONFIG;
        WRITE_CONFIG(OTHER_USESYS_FILEDIALOG, newUseNativeFileDialog);
        m_useNativeFileDialog = newUseNativeFileDialog;
    }
}

QByteArray SettingManager::scriptDockLayout() const {
    return m_scriptDockLayout;
}

void SettingManager::setScriptDockLayout(
    const QByteArray &newScriptDockLayout) {
    if (m_scriptDockLayout != newScriptDockLayout) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_DOCK_LAYOUT, newScriptDockLayout);
        m_scriptDockLayout = newScriptDockLayout;
    }
}

QStringList SettingManager::sysHideCats() const { return m_sysHideCats; }

QStringList SettingManager::usrHideCats() const { return m_usrHideCats; }

QList<RecentFileManager::RecentInfo> SettingManager::recentScriptFiles() const {
    return m_recentScriptFiles;
}

void SettingManager::setRecentScriptFiles(
    const QList<RecentFileManager::RecentInfo> &newRecentScriptFiles) {
    if (m_recentScriptFiles != newRecentScriptFiles) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SCRIPT_RECENTFILES, getVarList(newRecentScriptFiles));
        m_recentScriptFiles = newRecentScriptFiles;
    }
}

QString SettingManager::appFontFamily() const { return m_appFontFamily; }

void SettingManager::setAppFontFamily(const QString &newAppFontFamily) {
    if (m_appFontFamily != newAppFontFamily) {
        HANDLE_CONFIG;
        WRITE_CONFIG(APP_FONTFAMILY, newAppFontFamily);
        m_appFontFamily = newAppFontFamily;
    }
}

bool SettingManager::editorShowHeader() const { return m_editorShowHeader; }

void SettingManager::setEditorShowHeader(bool newEditorShowAddr) {
    if (m_editorShowHeader != newEditorShowAddr) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_SHOW_ADDR, newEditorShowAddr);
        m_editorShowHeader = newEditorShowAddr;
    }
}

bool SettingManager::enablePlugin() const { return m_enablePlugin; }

void SettingManager::setEnablePlugin(bool newEnablePlugin) {
    if (m_enablePlugin != newEnablePlugin) {
        HANDLE_CONFIG;
        WRITE_CONFIG(PLUGIN_ENABLE, newEnablePlugin);
        m_enablePlugin = newEnablePlugin;
    }
}

QList<RecentFileManager::RecentInfo> SettingManager::recentHexFiles() const {
    return m_recentHexFiles;
}

void SettingManager::setRecentFiles(
    const QList<RecentFileManager::RecentInfo> &newRecentFiles) {
    if (m_recentHexFiles != newRecentFiles) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_RECENTFILES, getVarList(newRecentFiles));
        m_recentHexFiles = newRecentFiles;
    }
}

Qt::WindowState SettingManager::defaultWinState() const {
    return m_defaultWinState;
}

void SettingManager::setDefaultWinState(Qt::WindowState newDefaultWinState) {
    switch (newDefaultWinState) {
    case Qt::WindowNoState:
    case Qt::WindowMaximized:
    case Qt::WindowFullScreen:
        break;
    default:
        newDefaultWinState = Qt::WindowMaximized;
        break;
    }
    if (m_defaultWinState != newDefaultWinState) {
        HANDLE_CONFIG;
        WRITE_CONFIG(APP_WINDOWSIZE, newDefaultWinState);
        m_defaultWinState = newDefaultWinState;
    }
}

void SettingManager::reset(SETTINGS cat) {
    __reset(cat);
    load();
}

void SettingManager::__reset(SETTINGS cat) {
    HANDLE_CONFIG;
    if (cat.testFlag(SETTING::APP)) {
        WRITE_CONFIG(SKIN_THEME, 0);
        WRITE_CONFIG(APP_LANGUAGE, QString());
        WRITE_CONFIG(APP_FONTFAMILY, _defaultFont.family());
        WRITE_CONFIG(APP_FONTSIZE, _defaultFont.pointSize());
        WRITE_CONFIG(APP_WINDOWSIZE, Qt::WindowMaximized);
    }
    if (cat.testFlag(SETTING::PLUGIN)) {
        WRITE_CONFIG(PLUGIN_ENABLE, true);
        WRITE_CONFIG(PLUGIN_ENABLE_ROOT, false);
        WRITE_CONFIG(PLUGIN_ENABLE_MANAGER, true);
        WRITE_CONFIG(PLUGIN_ENABLE_HEXEXT, true);
        WRITE_CONFIG(PLUGIN_ENABLEDPLUGINS_DEV, {});
        WRITE_CONFIG(PLUGIN_ENABLEDPLUGINS_EXT, {});
    }
    if (cat.testFlag(SETTING::EDITOR)) {
        WRITE_CONFIG(EDITOR_FONTSIZE, _defaultFont.pointSize());
        WRITE_CONFIG(EDITOR_SHOW_ADDR, true);
        WRITE_CONFIG(EDITOR_SHOW_COL, true);
        WRITE_CONFIG(EDITOR_SHOW_TEXT, true);
        WRITE_CONFIG(EDITOR_FIND_MAXCOUNT, 100);
        WRITE_CONFIG(EDITOR_COPY_LIMIT, 100);
        WRITE_CONFIG(EDITOR_DECSTRLIMIT, 10);
    }
    if (cat.testFlag(SETTING::SCRIPT)) {
        WRITE_CONFIG(SCRIPT_ENABLE, true);
        WRITE_CONFIG(SCRIPT_TIMEOUT, 10);
        WRITE_CONFIG(SCRIPT_ALLOW_USRSCRIPT_INROOT, false);
        WRITE_CONFIG(SCRIPT_USRHIDECATS, QStringList());
        WRITE_CONFIG(SCRIPT_SYSHIDECATS, QStringList());
    }
    if (cat.testFlag(SETTING::OTHER)) {
        WRITE_CONFIG(OTHER_USESYS_FILEDIALOG, true);
#ifdef WINGHEX_USE_FRAMELESS
        WRITE_CONFIG(OTHER_USE_NATIVE_TITLEBAR, false);
#endif
        WRITE_CONFIG(OTHER_DONT_USE_SPLASH, false);
        WRITE_CONFIG(OTHER_CHECK_UPDATE, false);
        WRITE_CONFIG(OTHER_LOG_LEVEL, Logger::defaultLevel());
        WRITE_CONFIG(OTHER_LOG_COUNT, 20);
    }
}

qsizetype SettingManager::decodeStrlimit() const { return m_decodeStrlimit; }

void SettingManager::setDecodeStrlimit(qsizetype newDecodeStrlimit) {
    newDecodeStrlimit =
        qBound(qsizetype(100), newDecodeStrlimit, qsizetype(1024));
    if (m_decodeStrlimit != newDecodeStrlimit) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_DECSTRLIMIT, newDecodeStrlimit);
        m_decodeStrlimit = newDecodeStrlimit;
        Q_EMIT sigDecodeStrlimitChanged(m_decodeStrlimit);
    }
}

qsizetype SettingManager::copylimit() const { return m_copylimit; }

void SettingManager::setCopylimit(qsizetype newCopylimit) {
    newCopylimit = qBound(qsizetype(100), newCopylimit, qsizetype(1024));
    if (m_copylimit != newCopylimit) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_COPY_LIMIT, newCopylimit);
        m_copylimit = newCopylimit;
        Q_EMIT sigDecodeStrlimitChanged(m_copylimit);
    }
}

bool SettingManager::editorShowtext() const { return m_editorShowtext; }

void SettingManager::setEditorShowtext(bool newEditorShowtext) {
    if (m_editorShowtext != newEditorShowtext) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_SHOW_TEXT, newEditorShowtext);
        m_editorShowtext = newEditorShowtext;
    }
}

bool SettingManager::editorShowcol() const { return m_editorShowcol; }

void SettingManager::setEditorShowcol(bool newEditorShowcol) {
    if (m_editorShowcol != newEditorShowcol) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_SHOW_COL, newEditorShowcol);
        m_editorShowcol = newEditorShowcol;
    }
}

int SettingManager::editorfontSize() const { return m_editorfontSize; }

void SettingManager::setEditorfontSize(int newEditorfontSize) {
    if (m_editorfontSize != newEditorfontSize) {
        HANDLE_CONFIG;
        WRITE_CONFIG(EDITOR_FONTSIZE, newEditorfontSize);
        m_editorfontSize = newEditorfontSize;
        Q_EMIT sigEditorfontSizeChanged(newEditorfontSize);
    }
}

int SettingManager::appfontSize() const { return m_appfontSize; }

void SettingManager::setAppfontSize(int newAppfontSize) {
    if (m_appfontSize != newAppfontSize) {
        HANDLE_CONFIG;
        WRITE_CONFIG(APP_FONTSIZE, newAppfontSize);
        m_appfontSize = newAppfontSize;
    }
}

bool SettingManager::enablePlgInRoot() const { return m_enablePlgInRoot; }

void SettingManager::setEnablePlgInRoot(bool newEnablePlgInRoot) {
    if (m_enablePlgInRoot != newEnablePlgInRoot) {
        HANDLE_CONFIG;
        WRITE_CONFIG(PLUGIN_ENABLE_ROOT, newEnablePlgInRoot);
        m_enablePlgInRoot = newEnablePlgInRoot;
    }
}

QString SettingManager::defaultLang() const { return m_defaultLang; }

void SettingManager::setDefaultLang(const QString &newDefaultLang) {
    if (m_defaultLang != newDefaultLang) {
        HANDLE_CONFIG;
        WRITE_CONFIG(APP_LANGUAGE, newDefaultLang);
        m_defaultLang = newDefaultLang;
    }
}

SettingManager &SettingManager::instance() {
    static SettingManager ins;
    return ins;
}

SettingManager::~SettingManager() {}

QByteArray SettingManager::dockLayout() const { return m_dockLayout; }

void SettingManager::setDockLayout(const QByteArray &newDockLayout) {
    if (m_dockLayout != newDockLayout) {
        HANDLE_CONFIG;
        WRITE_CONFIG(DOCK_LAYOUT, newDockLayout);
        m_dockLayout = newDockLayout;
    }
}

int SettingManager::themeID() const { return m_themeID; }

void SettingManager::setThemeID(int newThemeID) {
    if (m_themeID != newThemeID) {
        HANDLE_CONFIG;
        WRITE_CONFIG(SKIN_THEME, newThemeID);
        m_themeID = newThemeID;
    }
}
