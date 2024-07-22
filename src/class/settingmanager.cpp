#include "settingmanager.h"
#include "setting.h"

#include "src/class/skinmanager.h"
#include "src/utilities.h"
#include <QMetaEnum>

const auto DOCK_LAYOUT = QStringLiteral("dock.layout");

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

void SettingManager::load() {
    auto defaultFontSize = _defaultFont.pointSize();
    Q_ASSERT(defaultFontSize > 0);
    HANDLE_CONFIG;
    READ_CONFIG_INT(m_themeID, SKIN_THEME, 0);
    m_themeID = qBound(0, m_themeID,
                       QMetaEnum::fromType<SkinManager::Theme>().keyCount());
    m_defaultLang = READ_CONFIG(APP_LANGUAGE, QString()).toString();
    m_dockLayout = READ_CONFIG(DOCK_LAYOUT, QByteArray()).toByteArray();
    m_appFontFamily =
        READ_CONFIG(APP_FONTFAMILY, _defaultFont.family()).toString();
    READ_CONFIG_INT_POSITIVE(m_appfontSize, APP_FONTSIZE, defaultFontSize);
    m_defaultWinState = READ_CONFIG(APP_WINDOWSIZE, Qt::WindowMaximized)
                            .value<Qt::WindowState>();
    READ_CONFIG_BOOL(m_enablePlugin, PLUGIN_ENABLE, true);
    READ_CONFIG_BOOL(m_enablePlgInRoot, PLUGIN_ENABLE_ROOT, false);
    READ_CONFIG_INT_POSITIVE(m_editorfontSize, EDITOR_FONTSIZE,
                             defaultFontSize);
    READ_CONFIG_BOOL(m_editorShowAddr, EDITOR_SHOW_ADDR, true);
    READ_CONFIG_BOOL(m_editorShowcol, EDITOR_SHOW_COL, true);
    READ_CONFIG_BOOL(m_editorShowtext, EDITOR_SHOW_TEXT, true);
    m_editorEncoding =
        READ_CONFIG(EDITOR_ENCODING, QStringLiteral("ASCII")).toString();
    auto encodings = Utilities::getEncodings();
    if (!encodings.contains(m_editorEncoding)) {
        m_editorEncoding = QStringLiteral("ASCII");
    }
    READ_CONFIG_QSIZETYPE(m_findmaxcount, EDITOR_FIND_MAXCOUNT, 100);
    READ_CONFIG_QSIZETYPE(m_copylimit, EDITOR_COPY_LIMIT, 100);
    READ_CONFIG_QSIZETYPE(m_decodeStrlimit, EDITOR_DECSTRLIMIT, 10);
    m_recentHexFiles =
        READ_CONFIG(EDITOR_RECENTFILES, QStringList()).toStringList();
    m_recentScriptFiles =
        READ_CONFIG(SCRIPT_RECENTFILES, QStringList()).toStringList();
    m_usrDisplayCats =
        READ_CONFIG(SCRIPT_USRDISPLAYCATS, QStringList()).toStringList();
    m_sysDisplayCats =
        READ_CONFIG(SCRIPT_SYSDISPLAYCATS, QStringList()).toStringList();
}

QStringList SettingManager::sysDisplayCats() const { return m_sysDisplayCats; }

QStringList SettingManager::usrDisplayCats() const { return m_usrDisplayCats; }

QStringList SettingManager::recentScriptFiles() const {
    return m_recentScriptFiles;
}

void SettingManager::setRecentScriptFiles(
    const QStringList &newRecentScriptFiles) {
    m_recentScriptFiles = newRecentScriptFiles;
}

QString SettingManager::appFontFamily() const { return m_appFontFamily; }

void SettingManager::setAppFontFamily(const QString &newAppFontFamily) {
    m_appFontFamily = newAppFontFamily;
}

bool SettingManager::editorShowAddr() const { return m_editorShowAddr; }

void SettingManager::setEditorShowAddr(bool newEditorShowAddr) {
    m_editorShowAddr = newEditorShowAddr;
}

bool SettingManager::enablePlugin() const { return m_enablePlugin; }

void SettingManager::setEnablePlugin(bool newEnablePlugin) {
    m_enablePlugin = newEnablePlugin;
}

QStringList SettingManager::recentHexFiles() const { return m_recentHexFiles; }

void SettingManager::setRecentFiles(const QStringList &newRecentFiles) {
    m_recentHexFiles = newRecentFiles;
}

Qt::WindowState SettingManager::defaultWinState() const {
    return m_defaultWinState;
}

void SettingManager::setDefaultWinState(Qt::WindowState newDefaultWinState) {
    m_defaultWinState = newDefaultWinState;
}

void SettingManager::save(SETTINGS cat) {
    HANDLE_CONFIG;
    WRITE_CONFIG(DOCK_LAYOUT, m_dockLayout);
    WRITE_CONFIG(EDITOR_RECENTFILES, m_recentHexFiles);
    WRITE_CONFIG(SCRIPT_RECENTFILES, m_recentScriptFiles);
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
        WRITE_CONFIG(EDITOR_SHOW_ADDR, m_editorShowAddr);
        WRITE_CONFIG(EDITOR_SHOW_COL, m_editorShowcol);
        WRITE_CONFIG(EDITOR_SHOW_TEXT, m_editorShowtext);
        WRITE_CONFIG(EDITOR_ENCODING, m_editorEncoding);
        WRITE_CONFIG(EDITOR_FIND_MAXCOUNT, m_findmaxcount);
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
    m_decodeStrlimit = newDecodeStrlimit;
}

qsizetype SettingManager::copylimit() const { return m_copylimit; }

void SettingManager::setCopylimit(qsizetype newCopylimit) {
    m_copylimit = newCopylimit;
}

qsizetype SettingManager::findmaxcount() const { return m_findmaxcount; }

void SettingManager::setFindmaxcount(qsizetype newFindmaxcount) {
    m_findmaxcount = newFindmaxcount;
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
    m_editorfontSize = newEditorfontSize;
}

int SettingManager::appfontSize() const { return m_appfontSize; }

void SettingManager::setAppfontSize(int newAppfontSize) {
    m_appfontSize = newAppfontSize;
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

SettingManager::~SettingManager() { save(); }

QByteArray SettingManager::dockLayout() const { return m_dockLayout; }

void SettingManager::setDockLayout(const QByteArray &newDockLayout) {
    m_dockLayout = newDockLayout;
}

int SettingManager::themeID() const { return m_themeID; }

void SettingManager::setThemeID(int newThemeID) { m_themeID = newThemeID; }
