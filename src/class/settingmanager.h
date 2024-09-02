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

    QString editorEncoding() const;
    void setEditorEncoding(const QString &newEditorEncoding);

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

    QStringList usrDisplayCats() const;

    QStringList sysDisplayCats() const;

    QString lastUsedPath() const;
    void setLastUsedPath(const QString &newLastUsedPath);

    QByteArray scriptDockLayout() const;
    void setScriptDockLayout(const QByteArray &newScriptDockLayout);

    bool useNativeFileDialog() const;
    void setUseNativeFileDialog(bool newUseNativeFileDialog);

    bool useNativeTitleBar() const;
    void setUseNativeTitleBar(bool newUseNativeTitleBar);

signals:
    void sigEditorfontSizeChanged(int v);
    void sigDecodeStrlimitChanged(int v);
    void sigCopylimitChanged(int v);

    void sigAppfontSizeChanged(int v);
    void sigAppFontFamilyChanged(const QString &font);

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
    QString m_editorEncoding;

    qsizetype m_copylimit = 100;
    qsizetype m_decodeStrlimit = 10;

    QList<RecentFileManager::RecentInfo> m_recentHexFiles;
    QList<RecentFileManager::RecentInfo> m_recentScriptFiles;
    Qt::WindowState m_defaultWinState = Qt::WindowMaximized;

    QStringList m_usrDisplayCats;
    QStringList m_sysDisplayCats;

    QString m_lastUsedPath;

    bool m_useNativeFileDialog = true;
    bool m_useNativeTitleBar = false;

private:
    QFont _defaultFont;
};

#endif // SETTINGMANAGER_H
