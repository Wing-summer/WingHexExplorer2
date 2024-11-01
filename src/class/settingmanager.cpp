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
#include "utilities.h"
#include <QFileInfo>
#include <QMetaEnum>
#include <QSettings>

#define HANDLE_CONFIG                                                          \
    QSettings set(QStringLiteral(APP_ORG), QStringLiteral(APP_NAME))

#define CONFIG set

#define WRITE_CONFIG(config, dvalue)                                           \
    if (this->_setUnsaved.testFlag(SETTING_ITEM::config)) {                    \
        set.setValue(config, dvalue);                                          \
        _setUnsaved.setFlag(SettingManager::SETTING_ITEM::config, false);      \
    }

#define READ_CONFIG(config, dvalue) set.value(config, dvalue)

#define READ_CONFIG_SAFE(var, config, dvalue, func)                            \
    {                                                                          \
        auto b = false;                                                        \
        var = READ_CONFIG(config, dvalue).func(&b);                            \
        if (!b) {                                                              \
            var = dvalue;                                                      \
        }                                                                      \
    }

#define READ_CONFIG_INT(var, config, dvalue)                                   \
    READ_CONFIG_SAFE(var, config, dvalue, toInt)

#define READ_CONFIG_BOOL(var, config, dvalue)                                  \
    var = READ_CONFIG(config, dvalue).toBool()

#define READ_CONFIG_QSIZETYPE(var, config, dvalue)                             \
    READ_CONFIG_SAFE(var, config, dvalue, toLongLong)

#define READ_CONFIG_INT_POSITIVE(var, config, dvalue)                          \
    {                                                                          \
        Q_ASSERT(dvalue > 0);                                                  \
        READ_CONFIG_SAFE(var, config, dvalue, toInt);                          \
        if (var <= 0) {                                                        \
            var = dvalue;                                                      \
        }                                                                      \
    }

#define READ_CONFIG_DOUBLE_POSITIVE(var, config, dvalue)                       \
    {                                                                          \
        Q_ASSERT(dvalue > 0);                                                  \
        READ_CONFIG_SAFE(var, config, dvalue, toDouble);                       \
        if (var <= 0) {                                                        \
            var = dvalue;                                                      \
        }                                                                      \
    }

const auto DOCK_LAYOUT = QStringLiteral("dock.layout");
const auto SCRIPT_DOCK_LAYOUT = QStringLiteral("script.layout");
const auto APP_LASTUSED_PATH = QStringLiteral("app.lastusedpath");

const auto SKIN_THEME = QStringLiteral("skin.theme");
const auto APP_FONTFAMILY = QStringLiteral("app.fontfamily");
const auto APP_FONTSIZE = QStringLiteral("app.fontsize");
const auto APP_WINDOWSIZE = QStringLiteral("app.windowsize");
const auto APP_LANGUAGE = QStringLiteral("app.lang");

const auto PLUGIN_ENABLE = QStringLiteral("plugin.enableplugin");
const auto PLUGIN_ENABLE_ROOT = QStringLiteral("plugin.rootenableplugin");

const auto EDITOR_FONTSIZE = QStringLiteral("editor.fontsize");
const auto EDITOR_SHOW_ADDR = QStringLiteral("editor.showaddr");
const auto EDITOR_SHOW_COL = QStringLiteral("editor.showcol");
const auto EDITOR_SHOW_TEXT = QStringLiteral("editor.showtext");
const auto EDITOR_ENCODING = QStringLiteral("editor.encoding");
const auto EDITOR_FIND_MAXCOUNT = QStringLiteral("editor.findmaxcount");
const auto EDITOR_COPY_LIMIT = QStringLiteral("editor.copylimit");
const auto EDITOR_DECSTRLIMIT = QStringLiteral("editor.decstrlimit");
const auto EDITOR_RECENTFILES = QStringLiteral("editor.recentfiles");
const auto SCRIPT_RECENTFILES = QStringLiteral("script.recentfiles");

const auto SCRIPT_ALLOW_USRSCRIPT_INROOT =
    QStringLiteral("script.allowUsrScriptRoot");
const auto SCRIPT_USRHIDECATS = QStringLiteral("script.usrHideCats");
const auto SCRIPT_SYSHIDECATS = QStringLiteral("script.sysHideCats");

const auto OTHER_USESYS_FILEDIALOG = QStringLiteral("sys.nativeDialog");
const auto OTHER_USE_NATIVE_TITLEBAR = QStringLiteral("sys.nativeTitleBar");
const auto OTHER_LOG_LEVEL = QStringLiteral("sys.loglevel");

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
    m_lastUsedPath = newLastUsedPath;
}

void SettingManager::load() {
    auto defaultFontSize = _defaultFont.pointSize();
    Q_ASSERT(defaultFontSize > 0);
    HANDLE_CONFIG;
    READ_CONFIG_INT(m_themeID, SKIN_THEME, 0);
    m_themeID = qBound(0, m_themeID,
                       QMetaEnum::fromType<SkinManager::Theme>().keyCount());
    m_defaultLang = READ_CONFIG(APP_LANGUAGE, QString()).toString();
    m_dockLayout = READ_CONFIG(DOCK_LAYOUT, QByteArray()).toByteArray();
    m_scriptDockLayout =
        READ_CONFIG(SCRIPT_DOCK_LAYOUT, QByteArray()).toByteArray();
    m_appFontFamily =
        READ_CONFIG(APP_FONTFAMILY, _defaultFont.family()).toString();
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
    READ_CONFIG_INT_POSITIVE(m_editorfontSize, EDITOR_FONTSIZE,
                             defaultFontSize);
    READ_CONFIG_BOOL(m_editorShowHeader, EDITOR_SHOW_ADDR, true);
    READ_CONFIG_BOOL(m_editorShowcol, EDITOR_SHOW_COL, true);
    READ_CONFIG_BOOL(m_editorShowtext, EDITOR_SHOW_TEXT, true);
    READ_CONFIG_BOOL(m_useNativeFileDialog, OTHER_USESYS_FILEDIALOG, true);
#ifdef WINGHEX_USE_FRAMELESS
    READ_CONFIG_BOOL(m_useNativeTitleBar, OTHER_USE_NATIVE_TITLEBAR, false);
#else
    m_useNativeTitleBar = true;
#endif
    READ_CONFIG_INT_POSITIVE(m_logLevel, OTHER_LOG_LEVEL,
                             Logger::defaultLevel());
    m_logLevel =
        qBound(int(Logger::LEVEL_BEGIN), m_logLevel, int(Logger::LEVEL_LAST));

    m_editorEncoding =
        READ_CONFIG(EDITOR_ENCODING, QStringLiteral("ASCII")).toString();
    auto encodings = Utilities::getEncodings();
    if (!encodings.contains(m_editorEncoding)) {
        m_editorEncoding = QStringLiteral("ASCII");
    }
    READ_CONFIG_QSIZETYPE(m_copylimit, EDITOR_COPY_LIMIT, 100);
    m_copylimit = qBound(qsizetype(100), m_copylimit, qsizetype(1024));
    READ_CONFIG_QSIZETYPE(m_decodeStrlimit, EDITOR_DECSTRLIMIT, 100);
    m_decodeStrlimit =
        qBound(qsizetype(100), m_decodeStrlimit, qsizetype(1024));
    m_recentHexFiles = getDataFromVarList(
        READ_CONFIG(EDITOR_RECENTFILES, QVariantList()).toList());
    m_recentScriptFiles = getDataFromVarList(
        READ_CONFIG(SCRIPT_RECENTFILES, QVariantList()).toList());

    READ_CONFIG_BOOL(m_allowUsrScriptInRoot, SCRIPT_ALLOW_USRSCRIPT_INROOT,
                     false);
    m_usrHideCats =
        READ_CONFIG(SCRIPT_USRHIDECATS, QStringList()).toStringList();
    m_sysHideCats =
        READ_CONFIG(SCRIPT_SYSHIDECATS, QStringList()).toStringList();

    m_lastUsedPath = READ_CONFIG(APP_LASTUSED_PATH, QString()).toString();
    if (!m_lastUsedPath.isEmpty()) {
        QFileInfo info(m_lastUsedPath);
        if (info.exists() && info.isDir()) {
        } else {
            m_lastUsedPath.clear();
        }
    }

    emit sigEditorfontSizeChanged(m_editorfontSize);
    emit sigCopylimitChanged(m_copylimit);
    emit sigDecodeStrlimitChanged(m_decodeStrlimit);
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

bool SettingManager::allowUsrScriptInRoot() const {
    return m_allowUsrScriptInRoot;
}

void SettingManager::setAllowUsrScriptInRoot(bool newAllowUsrScriptInRoot) {
    if (m_allowUsrScriptInRoot != newAllowUsrScriptInRoot) {
        m_allowUsrScriptInRoot = newAllowUsrScriptInRoot;
        _setUnsaved.setFlag(SETTING_ITEM::SCRIPT_ALLOW_USRSCRIPT_INROOT);
    }
}

void SettingManager::setUsrHideCats(const QStringList &newUsrHideCats) {
    if (m_usrHideCats != newUsrHideCats) {
        m_usrHideCats = newUsrHideCats;
        _setUnsaved.setFlag(SETTING_ITEM::SCRIPT_USRHIDECATS);
    }
}

void SettingManager::setSysHideCats(const QStringList &newSysHideCats) {
    if (m_sysHideCats != newSysHideCats) {
        m_sysHideCats = newSysHideCats;
        _setUnsaved.setFlag(SETTING_ITEM::SCRIPT_SYSHIDECATS);
    }
}

int SettingManager::logLevel() const { return m_logLevel; }

void SettingManager::setLogLevel(int newLogLevel) {
    if (m_logLevel != newLogLevel) {
        m_logLevel = newLogLevel;
        _setUnsaved.setFlag(SETTING_ITEM::OTHER_LOG_LEVEL);
    }
}

bool SettingManager::useNativeTitleBar() const { return m_useNativeTitleBar; }

void SettingManager::setUseNativeTitleBar(bool newUseNativeTitleBar) {
    if (m_useNativeTitleBar != newUseNativeTitleBar) {
        m_useNativeTitleBar = newUseNativeTitleBar;
        _setUnsaved.setFlag(SETTING_ITEM::OTHER_USE_NATIVE_TITLEBAR);
    }
}

bool SettingManager::useNativeFileDialog() const {
    return m_useNativeFileDialog;
}

void SettingManager::setUseNativeFileDialog(bool newUseNativeFileDialog) {
    if (m_useNativeFileDialog != newUseNativeFileDialog) {
        m_useNativeFileDialog = newUseNativeFileDialog;
        _setUnsaved.setFlag(SETTING_ITEM::OTHER_USESYS_FILEDIALOG);
    }
}

QByteArray SettingManager::scriptDockLayout() const {
    return m_scriptDockLayout;
}

void SettingManager::setScriptDockLayout(
    const QByteArray &newScriptDockLayout) {
    if (m_scriptDockLayout != newScriptDockLayout) {
        m_scriptDockLayout = newScriptDockLayout;
        _setUnsaved.setFlag(SETTING_ITEM::SCRIPT_DOCK_LAYOUT);
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
        m_recentScriptFiles = newRecentScriptFiles;
        _setUnsaved.setFlag(SETTING_ITEM::SCRIPT_RECENTFILES);
    }
}

QString SettingManager::appFontFamily() const { return m_appFontFamily; }

void SettingManager::setAppFontFamily(const QString &newAppFontFamily) {
    if (m_appFontFamily != newAppFontFamily) {
        m_appFontFamily = newAppFontFamily;
        _setUnsaved.setFlag(SETTING_ITEM::APP_FONTFAMILY);
    }
}

bool SettingManager::editorShowHeader() const { return m_editorShowHeader; }

void SettingManager::setEditorShowHeader(bool newEditorShowAddr) {
    if (m_editorShowHeader != newEditorShowAddr) {
        m_editorShowHeader = newEditorShowAddr;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_SHOW_ADDR);
    }
}

bool SettingManager::enablePlugin() const { return m_enablePlugin; }

void SettingManager::setEnablePlugin(bool newEnablePlugin) {
    if (m_enablePlugin != newEnablePlugin) {
        m_enablePlugin = newEnablePlugin;
        _setUnsaved.setFlag(SETTING_ITEM::PLUGIN_ENABLE);
    }
}

QList<RecentFileManager::RecentInfo> SettingManager::recentHexFiles() const {
    return m_recentHexFiles;
}

void SettingManager::setRecentFiles(
    const QList<RecentFileManager::RecentInfo> &newRecentFiles) {
    if (m_recentHexFiles != newRecentFiles) {
        m_recentHexFiles = newRecentFiles;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_RECENTFILES);
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
        m_defaultWinState = newDefaultWinState;
        _setUnsaved.setFlag(SETTING_ITEM::APP_WINDOWSIZE);
    }
}

void SettingManager::save(SETTINGS cat) {
    HANDLE_CONFIG;
    WRITE_CONFIG(DOCK_LAYOUT, m_dockLayout);
    WRITE_CONFIG(SCRIPT_DOCK_LAYOUT, m_scriptDockLayout);
    WRITE_CONFIG(EDITOR_RECENTFILES, getVarList(m_recentHexFiles));
    WRITE_CONFIG(SCRIPT_RECENTFILES, getVarList(m_recentScriptFiles));
    WRITE_CONFIG(APP_LASTUSED_PATH, m_lastUsedPath);
    if (cat.testFlag(SETTING::APP)) {
        WRITE_CONFIG(SKIN_THEME, m_themeID);
        WRITE_CONFIG(APP_LANGUAGE, m_defaultLang);
        WRITE_CONFIG(APP_FONTFAMILY, m_appFontFamily);
        WRITE_CONFIG(APP_FONTSIZE, m_appfontSize);
        WRITE_CONFIG(APP_WINDOWSIZE, m_defaultWinState);
    }
    if (cat.testFlag(SETTING::PLUGIN)) {
        WRITE_CONFIG(PLUGIN_ENABLE, m_enablePlugin);
        WRITE_CONFIG(PLUGIN_ENABLE_ROOT, m_enablePlgInRoot);
    }
    if (cat.testFlag(SETTING::EDITOR)) {
        WRITE_CONFIG(EDITOR_FONTSIZE, m_editorfontSize);
        WRITE_CONFIG(EDITOR_SHOW_ADDR, m_editorShowHeader);
        WRITE_CONFIG(EDITOR_SHOW_COL, m_editorShowcol);
        WRITE_CONFIG(EDITOR_SHOW_TEXT, m_editorShowtext);
        WRITE_CONFIG(EDITOR_ENCODING, m_editorEncoding);
        WRITE_CONFIG(EDITOR_COPY_LIMIT, m_copylimit);
        WRITE_CONFIG(EDITOR_DECSTRLIMIT, m_decodeStrlimit);
    }
    if (cat.testFlag(SETTING::SCRIPT)) {
        WRITE_CONFIG(SCRIPT_ALLOW_USRSCRIPT_INROOT, m_allowUsrScriptInRoot);
        WRITE_CONFIG(SCRIPT_USRHIDECATS, m_usrHideCats);
        WRITE_CONFIG(SCRIPT_SYSHIDECATS, m_sysHideCats);
    }
    if (cat.testFlag(SETTING::OTHER)) {
        WRITE_CONFIG(OTHER_USESYS_FILEDIALOG, m_useNativeFileDialog);
#ifdef WINGHEX_USE_FRAMELESS
        WRITE_CONFIG(OTHER_USE_NATIVE_TITLEBAR, m_useNativeTitleBar);
#endif
        WRITE_CONFIG(OTHER_LOG_LEVEL, m_logLevel);
    }
}

void SettingManager::reset(SETTINGS cat) {
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
    }
    if (cat.testFlag(SETTING::EDITOR)) {
        WRITE_CONFIG(EDITOR_FONTSIZE, _defaultFont.pointSize());
        WRITE_CONFIG(EDITOR_SHOW_ADDR, true);
        WRITE_CONFIG(EDITOR_SHOW_COL, true);
        WRITE_CONFIG(EDITOR_SHOW_TEXT, true);
        WRITE_CONFIG(EDITOR_ENCODING, QStringLiteral("ASCII"));
        WRITE_CONFIG(EDITOR_FIND_MAXCOUNT, 100);
        WRITE_CONFIG(EDITOR_COPY_LIMIT, 100);
        WRITE_CONFIG(EDITOR_DECSTRLIMIT, 10);
    }
    if (cat.testFlag(SETTING::SCRIPT)) {
        WRITE_CONFIG(SCRIPT_ALLOW_USRSCRIPT_INROOT, false);
        WRITE_CONFIG(SCRIPT_USRHIDECATS, QStringList());
        WRITE_CONFIG(SCRIPT_SYSHIDECATS, QStringList());
    }
    if (cat.testFlag(SETTING::OTHER)) {
        WRITE_CONFIG(OTHER_USESYS_FILEDIALOG, true);
#ifdef WINGHEX_USE_FRAMELESS
        WRITE_CONFIG(OTHER_USE_NATIVE_TITLEBAR, false);
#endif
        WRITE_CONFIG(OTHER_LOG_LEVEL, Logger::defaultLevel());
    }
    load();
}

qsizetype SettingManager::decodeStrlimit() const { return m_decodeStrlimit; }

void SettingManager::setDecodeStrlimit(qsizetype newDecodeStrlimit) {
    newDecodeStrlimit =
        qBound(qsizetype(100), newDecodeStrlimit, qsizetype(1024));
    if (m_decodeStrlimit != newDecodeStrlimit) {
        m_decodeStrlimit = newDecodeStrlimit;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_DECSTRLIMIT);
        emit sigDecodeStrlimitChanged(m_decodeStrlimit);
    }
}

qsizetype SettingManager::copylimit() const { return m_copylimit; }

void SettingManager::setCopylimit(qsizetype newCopylimit) {
    newCopylimit = qBound(qsizetype(100), newCopylimit, qsizetype(1024));
    if (m_copylimit != newCopylimit) {
        m_copylimit = newCopylimit;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_COPY_LIMIT);
        emit sigDecodeStrlimitChanged(m_copylimit);
    }
}

QString SettingManager::editorEncoding() const { return m_editorEncoding; }

void SettingManager::setEditorEncoding(const QString &newEditorEncoding) {
    if (m_editorEncoding != newEditorEncoding) {
        m_editorEncoding = newEditorEncoding;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_ENCODING);
    }
}

bool SettingManager::editorShowtext() const { return m_editorShowtext; }

void SettingManager::setEditorShowtext(bool newEditorShowtext) {
    if (m_editorShowtext != newEditorShowtext) {
        m_editorShowtext = newEditorShowtext;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_SHOW_TEXT);
    }
}

bool SettingManager::editorShowcol() const { return m_editorShowcol; }

void SettingManager::setEditorShowcol(bool newEditorShowcol) {
    if (m_editorShowcol != newEditorShowcol) {
        m_editorShowcol = newEditorShowcol;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_SHOW_COL);
    }
}

int SettingManager::editorfontSize() const { return m_editorfontSize; }

void SettingManager::setEditorfontSize(int newEditorfontSize) {
    if (m_editorfontSize != newEditorfontSize) {
        m_editorfontSize = newEditorfontSize;
        _setUnsaved.setFlag(SETTING_ITEM::EDITOR_FONTSIZE);
        emit sigEditorfontSizeChanged(newEditorfontSize);
    }
}

int SettingManager::appfontSize() const { return m_appfontSize; }

void SettingManager::setAppfontSize(int newAppfontSize) {
    if (m_appfontSize != newAppfontSize) {
        m_appfontSize = newAppfontSize;
        _setUnsaved.setFlag(SETTING_ITEM::APP_FONTSIZE);
    }
}

bool SettingManager::enablePlgInRoot() const { return m_enablePlgInRoot; }

void SettingManager::setEnablePlgInRoot(bool newEnablePlgInRoot) {
    if (m_enablePlgInRoot != newEnablePlgInRoot) {
        m_enablePlgInRoot = newEnablePlgInRoot;
        _setUnsaved.setFlag(SETTING_ITEM::PLUGIN_ENABLE_ROOT);
    }
}

QString SettingManager::defaultLang() const { return m_defaultLang; }

void SettingManager::setDefaultLang(const QString &newDefaultLang) {
    if (m_defaultLang != newDefaultLang) {
        m_defaultLang = newDefaultLang;
        _setUnsaved.setFlag(SETTING_ITEM::APP_LANGUAGE);
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
        m_dockLayout = newDockLayout;
        _setUnsaved.setFlag(SETTING_ITEM::DOCK_LAYOUT);
    }
}

int SettingManager::themeID() const { return m_themeID; }

void SettingManager::setThemeID(int newThemeID) {
    if (m_themeID != newThemeID) {
        m_themeID = newThemeID;
        _setUnsaved.setFlag(SETTING_ITEM::SKIN_THEME);
    }
}
