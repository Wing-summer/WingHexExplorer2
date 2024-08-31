#include "settingmanager.h"
#include "setting.h"

#include "class/skinmanager.h"
#include "utilities.h"
#include <QFileInfo>
#include <QMetaEnum>

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

const auto SCRIPT_USRDISPLAYCATS = QStringLiteral("script.usrDisplayCats");
const auto SCRIPT_SYSDISPLAYCATS = QStringLiteral("script.sysDisplayCats");

SettingManager::SettingManager() {
    _defaultFont = qApp->font();
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
    m_recentHexFiles =
        READ_CONFIG(EDITOR_RECENTFILES,
                    QVariant::fromValue(QList<RecentFileManager::RecentInfo>()))
            .value<QList<RecentFileManager::RecentInfo>>();
    m_recentScriptFiles =
        READ_CONFIG(SCRIPT_RECENTFILES,
                    QVariant::fromValue(QList<RecentFileManager::RecentInfo>()))
            .value<QList<RecentFileManager::RecentInfo>>();
    m_usrDisplayCats =
        READ_CONFIG(SCRIPT_USRDISPLAYCATS, QStringList()).toStringList();
    m_sysDisplayCats =
        READ_CONFIG(SCRIPT_SYSDISPLAYCATS, QStringList()).toStringList();

    m_lastUsedPath = READ_CONFIG(APP_LASTUSED_PATH, QString()).toString();
    if (!m_lastUsedPath.isEmpty()) {
        QFileInfo info(m_lastUsedPath);
        if (info.exists() && info.isDir()) {
        } else {
            m_lastUsedPath.clear();
        }
    }

    emit sigAppFontFamilyChanged(m_appFontFamily);
    emit sigAppfontSizeChanged(m_appfontSize);
    emit sigEditorfontSizeChanged(m_editorfontSize);
    emit sigCopylimitChanged(m_copylimit);
    emit sigDecodeStrlimitChanged(m_decodeStrlimit);
}

QByteArray SettingManager::scriptDockLayout() const {
    return m_scriptDockLayout;
}

void SettingManager::setScriptDockLayout(
    const QByteArray &newScriptDockLayout) {
    m_scriptDockLayout = newScriptDockLayout;
}

QStringList SettingManager::sysDisplayCats() const { return m_sysDisplayCats; }

QStringList SettingManager::usrDisplayCats() const { return m_usrDisplayCats; }

QList<RecentFileManager::RecentInfo> SettingManager::recentScriptFiles() const {
    return m_recentScriptFiles;
}

void SettingManager::setRecentScriptFiles(
    const QList<RecentFileManager::RecentInfo> &newRecentScriptFiles) {
    m_recentScriptFiles = newRecentScriptFiles;
}

QString SettingManager::appFontFamily() const { return m_appFontFamily; }

void SettingManager::setAppFontFamily(const QString &newAppFontFamily) {
    if (m_appFontFamily != newAppFontFamily) {
        m_appFontFamily = newAppFontFamily;
        emit sigAppFontFamilyChanged(newAppFontFamily);
    }
}

bool SettingManager::editorShowHeader() const { return m_editorShowHeader; }

void SettingManager::setEditorShowHeader(bool newEditorShowAddr) {
    m_editorShowHeader = newEditorShowAddr;
}

bool SettingManager::enablePlugin() const { return m_enablePlugin; }

void SettingManager::setEnablePlugin(bool newEnablePlugin) {
    m_enablePlugin = newEnablePlugin;
}

QList<RecentFileManager::RecentInfo> SettingManager::recentHexFiles() const {
    return m_recentHexFiles;
}

void SettingManager::setRecentFiles(
    const QList<RecentFileManager::RecentInfo> &newRecentFiles) {
    m_recentHexFiles = newRecentFiles;
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
    m_defaultWinState = newDefaultWinState;
}

void SettingManager::save(SETTINGS cat) {
    HANDLE_CONFIG;
    WRITE_CONFIG(DOCK_LAYOUT, m_dockLayout);
    WRITE_CONFIG(SCRIPT_DOCK_LAYOUT, m_scriptDockLayout);
    WRITE_CONFIG(EDITOR_RECENTFILES, QVariant::fromValue(m_recentHexFiles));
    WRITE_CONFIG(SCRIPT_RECENTFILES, QVariant::fromValue(m_recentScriptFiles));
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
    load();
}

qsizetype SettingManager::decodeStrlimit() const { return m_decodeStrlimit; }

void SettingManager::setDecodeStrlimit(qsizetype newDecodeStrlimit) {
    if (m_decodeStrlimit != newDecodeStrlimit) {
        m_decodeStrlimit =
            qBound(qsizetype(100), newDecodeStrlimit, qsizetype(1024));
        emit sigDecodeStrlimitChanged(m_decodeStrlimit);
    }
}

qsizetype SettingManager::copylimit() const { return m_copylimit; }

void SettingManager::setCopylimit(qsizetype newCopylimit) {
    if (m_copylimit != newCopylimit) {
        m_copylimit = qBound(qsizetype(100), newCopylimit, qsizetype(1024));
        emit sigDecodeStrlimitChanged(m_copylimit);
    }
}

QString SettingManager::editorEncoding() const { return m_editorEncoding; }

void SettingManager::setEditorEncoding(const QString &newEditorEncoding) {
    m_editorEncoding = newEditorEncoding;
}

bool SettingManager::editorShowtext() const { return m_editorShowtext; }

void SettingManager::setEditorShowtext(bool newEditorShowtext) {
    m_editorShowtext = newEditorShowtext;
}

bool SettingManager::editorShowcol() const { return m_editorShowcol; }

void SettingManager::setEditorShowcol(bool newEditorShowcol) {
    m_editorShowcol = newEditorShowcol;
}

int SettingManager::editorfontSize() const { return m_editorfontSize; }

void SettingManager::setEditorfontSize(int newEditorfontSize) {
    if (m_editorfontSize != newEditorfontSize) {
        m_editorfontSize = newEditorfontSize;
        emit sigEditorfontSizeChanged(newEditorfontSize);
    }
}

int SettingManager::appfontSize() const { return m_appfontSize; }

void SettingManager::setAppfontSize(int newAppfontSize) {
    if (m_appfontSize != newAppfontSize) {
        m_appfontSize = newAppfontSize;
        emit sigAppfontSizeChanged(newAppfontSize);
    }
}

bool SettingManager::enablePlgInRoot() const { return m_enablePlgInRoot; }

void SettingManager::setEnablePlgInRoot(bool newEnablePlgInRoot) {
    m_enablePlgInRoot = newEnablePlgInRoot;
}

QString SettingManager::defaultLang() const { return m_defaultLang; }

void SettingManager::setDefaultLang(const QString &newDefaultLang) {
    m_defaultLang = newDefaultLang;
}

SettingManager &SettingManager::instance() {
    static SettingManager ins;
    return ins;
}

SettingManager::~SettingManager() {}

QByteArray SettingManager::dockLayout() const { return m_dockLayout; }

void SettingManager::setDockLayout(const QByteArray &newDockLayout) {
    m_dockLayout = newDockLayout;
}

int SettingManager::themeID() const { return m_themeID; }

void SettingManager::setThemeID(int newThemeID) { m_themeID = newThemeID; }
