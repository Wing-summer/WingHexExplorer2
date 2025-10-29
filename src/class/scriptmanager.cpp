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

#include "scriptmanager.h"

#include "DockWidgetTab.h"
#include "dbghelper.h"

#include <QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>

#include "scriptmachine.h"
#include "settingmanager.h"
#include "utilities.h"
#include "wingmessagebox.h"

ScriptManager &ScriptManager::instance() {
    static ScriptManager ins;
    return ins;
}

QString ScriptManager::userScriptPath() const { return m_usrScriptsPath; }

QString ScriptManager::systemScriptPath() const { return m_sysScriptsPath; }

ScriptManager::ScriptManager() {
    ASSERT_SINGLETON;

    // init script directory
    m_sysScriptsPath = qApp->applicationDirPath() + QDir::separator() +
                       QStringLiteral("scripts");
    m_usrScriptsPath = Utilities::getAppDataPath() + QDir::separator() +
                       QStringLiteral("scripts");

    refreshSysScriptsDbCats();
    refreshUsrScriptsDbCats();
}

ScriptManager::~ScriptManager() {}

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

QString ScriptManager::readJsonObjString(const QJsonObject &jobj,
                                         const QString &key) {
    auto t = jobj.value(key);
    if (!t.isUndefined() && t.isString()) {
        return t.toString();
    }
    return {};
}

bool ScriptManager::readJsonObjBool(const QJsonObject &jobj,
                                    const QString &key) {
    return jobj.value(key).toBool();
}

QMenu *ScriptManager::buildUpScriptDirMenu(QWidget *parent,
                                           const QStringList &files,
                                           bool isSys) {
    auto menu = new QMenu(parent);
    for (auto &file : files) {
        menu->addAction(
            ICONRES(QStringLiteral("script")), QFileInfo(file).fileName(),
            parent, [=] {
                if (Utilities::isRoot() && !isSys &&
                    !SettingManager::instance().allowUsrScriptInRoot()) {
                    WingMessageBox::critical(nullptr, tr("RunScript"),
                                             tr("CanNotRunUsrScriptForPolicy"));
                    return;
                }
                ScriptManager::instance().runScript(file);
            });
    }
    return menu;
}

ads::CDockWidgetTab *ScriptManager::indicator() const { return m_indicator; }

void ScriptManager::setIndicator(ads::CDockWidgetTab *newIndicator) {
    m_indicator = newIndicator;
}

bool ScriptManager::isScriptFile(const QString &file) {
    QFileInfo info(file);
    auto suffix = info.suffix();
    return info.exists() && Utilities::isTextFile(info) &&
           (suffix.compare(QStringLiteral("as"), Qt::CaseInsensitive) == 0 ||
            suffix.compare(QStringLiteral("anglescript"),
                           Qt::CaseInsensitive) == 0);
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

void ScriptManager::refreshUsrScriptsDbCats() {
    m_usrScriptsDbCats.clear();

    QDir scriptDir(m_usrScriptsPath);
    if (!scriptDir.exists()) {
        QDir().mkpath(m_usrScriptsPath);
    } else {
        for (auto &info :
             scriptDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QDir dir(info.absoluteFilePath());
            auto files = dir.entryList({"*.as"}, QDir::Files);
            m_usrScriptsDbCats << info.fileName();
            auto meta = ensureDirMeta(info);
            meta.isSys = false;
            meta.isEmptyDir = files.isEmpty();
            _usrDirMetas.insert(info.fileName(), meta);
        }
    }
}

void ScriptManager::refreshSysScriptsDbCats() {
    m_sysScriptsDbCats.clear();

    QDir sysScriptDir(m_sysScriptsPath);
    if (sysScriptDir.exists()) {
        for (auto &info :
             sysScriptDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QDir dir(info.absoluteFilePath());
            auto files = dir.entryList({"*.as"}, QDir::Files);
            m_sysScriptsDbCats << info.fileName();
            auto meta = ensureDirMeta(info);
            meta.isSys = true;
            meta.isEmptyDir = files.isEmpty();
            _sysDirMetas.insert(info.fileName(), meta);
        }
    }
}

ScriptManager::ScriptDirMeta
ScriptManager::ensureDirMeta(const QFileInfo &info) {
    ScriptDirMeta meta;

    if (!info.isDir()) {
        return meta;
    }
    QDir base(info.absoluteFilePath());
    if (!base.exists(QStringLiteral(".wingasmeta"))) {
        QJsonObject jobj;
        jobj.insert(QStringLiteral("name"), info.fileName());
        jobj.insert(QStringLiteral("author"), QLatin1String());
        jobj.insert(QStringLiteral("license"), QLatin1String());
        jobj.insert(QStringLiteral("homepage"), QLatin1String());
        jobj.insert(QStringLiteral("comment"), QLatin1String());
        jobj.insert(QStringLiteral("hexmenu"), false);
        QFile f(base.absoluteFilePath(QStringLiteral(".wingasmeta")));
        if (f.open(QFile::WriteOnly | QFile::Text)) {
            QJsonDocument jdoc(jobj);
            if (f.write(jdoc.toJson(QJsonDocument::JsonFormat::Indented)) >=
                0) {
                f.close();
            }
        }
        meta.name = info.fileName();
    } else {
        QFile f(base.absoluteFilePath(QStringLiteral(".wingasmeta")));
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
            if (err.error == QJsonParseError::NoError) {
                auto jobj = doc.object();
                auto name = readJsonObjString(jobj, QStringLiteral("name"));
                if (name.trimmed().isEmpty()) {
                    name = info.fileName();
                }
                meta.name = name;
                meta.author = readJsonObjString(jobj, QStringLiteral("author"));
                meta.license =
                    readJsonObjString(jobj, QStringLiteral("license"));
                meta.homepage =
                    readJsonObjString(jobj, QStringLiteral("homepage"));
                meta.comment =
                    readJsonObjString(jobj, QStringLiteral("comment"));
                meta.isContextMenu =
                    readJsonObjBool(jobj, QStringLiteral("hexmenu"));
            } else {
                meta.name = info.fileName();
            }
        } else {
            meta.name = info.fileName();
        }
    }

    meta.rawName = info.fileName();
    return meta;
}

ScriptManager::ScriptDirMeta
ScriptManager::usrDirMeta(const QString &cat) const {
    return _usrDirMetas.value(cat);
}

ScriptManager::ScriptDirMeta
ScriptManager::sysDirMeta(const QString &cat) const {
    return _sysDirMetas.value(cat);
}

QList<QMenu *>
ScriptManager::buildUpScriptRunnerContext(RibbonButtonGroup *group,
                                          QWidget *parent) {
    QList<QMenu *> maps;

    auto &sm = ScriptManager::instance();
    auto &stm = SettingManager::instance();

    auto hideCats = stm.sysHideCats();
    for (auto &cat : sm.sysScriptsDbCats()) {
        if (hideCats.contains(cat)) {
            continue;
        }

        auto meta = sm.sysDirMeta(cat);
        auto files = sm.getSysScriptFileNames(cat);
        if (!files.isEmpty()) {
            addPannelAction(group, ICONRES(QStringLiteral("scriptfolder")),
                            meta.name,
                            buildUpScriptDirMenu(group, files, true));
            if (meta.isContextMenu) {
                auto m = buildUpScriptDirMenu(parent, files, true);
                m->setTitle(meta.name);
                m->setIcon(ICONRES(QStringLiteral("scriptfolder")));
                maps << m;
            }
        }
    }

    hideCats = stm.usrHideCats();
    for (auto &cat : sm.usrScriptsDbCats()) {
        if (hideCats.contains(cat)) {
            continue;
        }

        auto meta = sm.usrDirMeta(cat);
        auto files = sm.getUsrScriptFileNames(cat);
        if (!files.isEmpty()) {
            addPannelAction(group, ICONRES(QStringLiteral("scriptfolderusr")),
                            meta.name,
                            buildUpScriptDirMenu(group, files, false));

            if (meta.isContextMenu) {
                auto m = buildUpScriptDirMenu(parent, files, true);
                m->setTitle(meta.name);
                m->setIcon(ICONRES(QStringLiteral("scriptfolderusr")));
                maps << m;
            }
        }
    }

    return maps;
}

void ScriptManager::runScript(const QString &filename) {
    auto &ins = ScriptMachine::instance();
    if (ins.isRunning(ScriptMachine::Background)) {
        auto ret = QMessageBox::question(nullptr, tr("ScriptRunning"),
                                         tr("ScriptRunningRequestLastStop?"));
        if (ret == QMessageBox::Yes) {
            ins.abortScript(ScriptMachine::Background);
        } else {
            return;
        }
    }

    Q_ASSERT(m_indicator);
    m_indicator->setIcon(ICONRES(QStringLiteral("dbgrun")));
    ins.executeScript(ScriptMachine::Background, filename);
    m_indicator->setIcon({});
}

QStringList ScriptManager::usrScriptsDbCats() const {
    return m_usrScriptsDbCats;
}
