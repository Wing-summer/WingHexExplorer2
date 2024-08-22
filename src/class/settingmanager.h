#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QByteArray>
#include <QFont>
#include <QObject>
#include <QString>
#include <QStringList>

class SettingManager : public QObject {
    Q_OBJECT

public:
    enum SETTING {
        APP = 1,
        PLUGIN = 2,
        EDITOR = 4,
        SCRIPT = 8,
        ALL = APP | PLUGIN | EDITOR | SCRIPT
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

    QStringList recentHexFiles() const;
    void setRecentFiles(const QStringList &newRecentFiles);

    bool enablePlugin() const;
    void setEnablePlugin(bool newEnablePlugin);

    bool editorShowHeader() const;
    void setEditorShowHeader(bool newEditorShowAddr);

    QString appFontFamily() const;
    void setAppFontFamily(const QString &newAppFontFamily);

    QStringList recentScriptFiles() const;
    void setRecentScriptFiles(const QStringList &newRecentScriptFiles);

    QStringList usrDisplayCats() const;

    QStringList sysDisplayCats() const;

    QString lastUsedPath() const;
    void setLastUsedPath(const QString &newLastUsedPath);

signals:
    void sigEditorfontSizeChanged(int v);
    void sigDecodeStrlimitChanged(int v);
    void sigCopylimitChanged(int v);

    void sigAppfontSizeChanged(int v);
    void sigAppFontFamilyChanged(const QString &font);

private:
    SettingManager();

    void load();

    Q_DISABLE_COPY(SettingManager)
private:
    int m_themeID = 0;
    int m_appfontSize = 18;
    int m_editorfontSize = 18;
    bool m_enablePlugin = true;
    bool m_enablePlgInRoot = false;
    QString m_defaultLang;
    QByteArray m_dockLayout;
    QString m_appFontFamily;

    bool m_editorShowHeader = true;
    bool m_editorShowcol = true;
    bool m_editorShowtext = true;
    QString m_editorEncoding;

    qsizetype m_copylimit = 100;
    qsizetype m_decodeStrlimit = 10;

    QStringList m_recentHexFiles;
    QStringList m_recentScriptFiles;
    Qt::WindowState m_defaultWinState = Qt::WindowMaximized;

    QStringList m_usrDisplayCats;
    QStringList m_sysDisplayCats;

    QString m_lastUsedPath;

private:
    QFont _defaultFont;
};

#endif // SETTINGMANAGER_H
