#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QDir>
#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>

#include "control/scriptingconsole.h"

class ScriptManager : public QObject {
    Q_OBJECT

public:
    struct ScriptDirMeta {
        QString rawName; // a flag
        QString name;
        QString author;
        QString license;
        QString homepage;
        QString comment;
        bool isSys; // a flag
    };

public:
    static ScriptManager &instance();

    QString userScriptPath() const;

    QString systemScriptPath() const;

    QStringList usrScriptsDbCats() const;

    QStringList sysScriptsDbCats() const;

    QStringList getUsrScriptFileNames(const QString &cat) const;

    QStringList getSysScriptFileNames(const QString &cat) const;

    void refresh();
    void refreshUsrScriptsDbCats();
    void refreshSysScriptsDbCats();

    void attach(ScriptingConsole *console);
    void detach();

    ScriptDirMeta usrDirMeta(const QString &cat) const;
    ScriptDirMeta sysDirMeta(const QString &cat) const;

public slots:
    void runScript(const QString &filename);

private:
    explicit ScriptManager();
    virtual ~ScriptManager();

    Q_DISABLE_COPY(ScriptManager)

    ScriptDirMeta ensureDirMeta(const QFileInfo &info);

    QStringList getScriptFileNames(const QDir &dir) const;

    QString readJsonObjString(const QJsonObject &jobj, const QString &key);

private:
    QString m_sysScriptsPath;
    QString m_usrScriptsPath;
    QStringList m_usrScriptsDbCats;
    QStringList m_sysScriptsDbCats;

    QHash<QString, ScriptDirMeta> _usrDirMetas;
    QHash<QString, ScriptDirMeta> _sysDirMetas;

    ScriptingConsole *_console = nullptr;
};

Q_DECLARE_METATYPE(ScriptManager::ScriptDirMeta)

#endif // SCRIPTMANAGER_H
