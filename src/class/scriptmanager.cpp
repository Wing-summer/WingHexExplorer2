#include "scriptmanager.h"

#include "../dbghelper.h"

class callback_streambuf : public std::streambuf {
public:
    callback_streambuf(
        std::function<void(char const *, std::streamsize)> callback);

protected:
    std::streamsize xsputn(char_type const *s, std::streamsize count) override;

private:
    std::function<void(char const *, std::streamsize)> callback;
};

callback_streambuf::callback_streambuf(
    std::function<void(const char *, std::streamsize)> callback)
    : callback(callback) {}

std::streamsize callback_streambuf::xsputn(const char_type *s,
                                           std::streamsize count) {
    callback(s, count);
    return count;
}

//==================================================================

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

ScriptManager &ScriptManager::instance() {
    static ScriptManager ins;
    return ins;
}

QString ScriptManager::userScriptPath() const { return m_usrScriptsPath; }

QString ScriptManager::systemScriptPath() const { return m_sysScriptsPath; }

ScriptManager::ScriptManager() : QObject() {
    ASSERT_SINGLETON;

    // init script directory
    m_sysScriptsPath = qApp->applicationDirPath() + QDir::separator() +
                       QStringLiteral("scripts");
    m_usrScriptsPath =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
        QDir::separator() + QStringLiteral("scripts");

    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
            [=](const QString &path) {

            });

    QDir sysScriptDir(m_sysScriptsPath);
    if (sysScriptDir.exists()) {
        for (auto &info :
             sysScriptDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            m_sysScriptsDbCats << info.fileName();
        }
        m_watcher->addPath(m_sysScriptsPath);
    }

    QDir scriptDir(m_usrScriptsPath);
    if (!scriptDir.exists()) {
        QDir().mkpath(m_usrScriptsPath);
    } else {
        for (auto &info :
             scriptDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            m_usrScriptsDbCats << info.fileName();
        }
    }

    m_watcher->addPath(m_usrScriptsPath);

    // redirect output
    sout = new callback_streambuf(
        std::bind(&ScriptManager::messageCallBack, this, STD_OUTPUT::STD_OUT,
                  std::placeholders::_1, std::placeholders::_2));
    serr = new callback_streambuf(
        std::bind(&ScriptManager::messageCallBack, this, STD_OUTPUT::STD_ERROR,
                  std::placeholders::_1, std::placeholders::_2));

    std_out = std::cout.rdbuf(sout);
    std_err = std::cerr.rdbuf(serr);
}

ScriptManager::~ScriptManager() {
    std::cout.rdbuf(std_out);
    std::cerr.rdbuf(std_err);
    delete sout;
    delete serr;
}

void ScriptManager::messageCallBack(STD_OUTPUT io, const char *str,
                                    std::streamsize size) {
    switch (io) {
    case STD_OUTPUT::STD_OUT:
        std_out->sputn(str, size);
        break;
    case STD_OUTPUT::STD_ERROR:
        std_err->sputn(str, size);
        break;
    }
    emit messageOut(io, QString::fromStdString(
                            std::string(str, std::string::size_type(size))));
}

QStringList ScriptManager::getScriptFileNames(const QDir &dir) const {
    if (!dir.exists()) {
        return {};
    }
    QStringList ret;
    for (auto &info : dir.entryInfoList({"*.as"}, QDir::Files)) {
        ret << info.absoluteFilePath();
    }
    return ret;
}

QStringList ScriptManager::sysScriptsDbCats() const {
    return m_sysScriptsDbCats;
}

QStringList ScriptManager::getUsrScriptFileNames(const QString &cat) const {
    QDir scriptDir(m_usrScriptsPath);
    if (!scriptDir.exists()) {
        return {};
    }
    scriptDir.cd(cat);
    return getScriptFileNames(scriptDir);
}

QStringList ScriptManager::getSysScriptFileNames(const QString &cat) const {
    QDir scriptDir(m_sysScriptsPath);
    if (!scriptDir.exists()) {
        return {};
    }
    scriptDir.cd(cat);
    return getScriptFileNames(scriptDir);
}

void ScriptManager::refresh() {}

void ScriptManager::runScript(const QString &filename) {}

QStringList ScriptManager::usrScriptsDbCats() const {
    return m_usrScriptsDbCats;
}
