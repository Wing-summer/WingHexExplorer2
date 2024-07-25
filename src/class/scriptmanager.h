#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QDir>
#include <QFileSystemWatcher>
#include <QObject>
#include <iostream>

class callback_streambuf;

class ScriptManager : public QObject {
    Q_OBJECT

public:
    static ScriptManager &instance();

    QString userScriptPath() const;

    QString systemScriptPath() const;

    QStringList usrScriptsDbCats() const;

    QStringList sysScriptsDbCats() const;

    QStringList getUsrScriptFileNames(const QString &cat) const;

    QStringList getSysScriptFileNames(const QString &cat) const;

    void refresh();

public slots:
    void runScript(const QString &filename);

public:
    enum class STD_OUTPUT { STD_OUT, STD_ERROR };
    Q_ENUM(STD_OUTPUT)

signals:
    void messageOut(STD_OUTPUT io, QString message);

private:
    explicit ScriptManager();
    virtual ~ScriptManager();

    Q_DISABLE_COPY(ScriptManager)

    void messageCallBack(ScriptManager::STD_OUTPUT io, char const *str,
                         std::streamsize size);

    QStringList getScriptFileNames(const QDir &dir) const;

private:
    callback_streambuf *sout = nullptr;
    callback_streambuf *serr = nullptr;

    std::streambuf *std_out = nullptr;
    std::streambuf *std_err = nullptr;

    QFileSystemWatcher *m_watcher;
    QString m_sysScriptsPath;
    QString m_usrScriptsPath;
    QStringList m_usrScriptsDbCats;
    QStringList m_sysScriptsDbCats;
};

#endif // SCRIPTMANAGER_H
