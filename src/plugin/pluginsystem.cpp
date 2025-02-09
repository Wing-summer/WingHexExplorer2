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

#include "pluginsystem.h"
#include "AngelScript/sdk/angelscript/source/as_builder.h"
#include "QJsonModel/include/QJsonModel.hpp"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "class/languagemanager.h"
#include "class/logger.h"
#include "class/settingmanager.h"
#include "class/skinmanager.h"
#include "class/wingcstruct.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "define.h"
#include "dialog/colorpickerdialog.h"
#include "dialog/framelessdialogbase.h"
#include "dialog/mainwindow.h"
#include "model/qjsontablemodel.h"

#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QPluginLoader>
#include <QtCore>

PluginSystem::PluginSystem(QObject *parent) : QObject(parent) {}

PluginSystem::~PluginSystem() {
    Q_ASSERT(_loadedplgs.isEmpty());
    Q_ASSERT(_loadeddevs.isEmpty());
}

void PluginSystem::initCheckingEngine() {
    _engine = dynamic_cast<asCScriptEngine *>(asCreateScriptEngine());
    Q_ASSERT(_engine);

    // basic checking environment
    ScriptMachine::registerEngineAddon(_engine);
    _angelplg->installBasicTypes(_engine);
}

void PluginSystem::finalizeCheckingEngine() { _engine->ShutDownAndRelease(); }

const QList<IWingPlugin *> &PluginSystem::plugins() const {
    return _loadedplgs;
}

IWingPlugin *PluginSystem::plugin(qsizetype index) const {
    return _loadedplgs.at(index);
}

const QList<IWingDevice *> &PluginSystem::devices() const {
    return _loadeddevs;
}

IWingDevice *PluginSystem::device(qsizetype index) const {
    return _loadeddevs.at(index);
}

template <typename T>
std::optional<PluginSystem::PluginInfo>
PluginSystem::loadPlugin(const QFileInfo &fileinfo, const QDir &setdir) {
    Q_ASSERT(_win);

    if (fileinfo.exists()) {
        auto fileName = fileinfo.absoluteFilePath();
        QPluginLoader loader(fileName, this);
        Logger::info(tr("LoadingPlugin") + fileinfo.fileName());

        auto lmeta = loader.metaData();
        PluginStatus cret;
        std::optional<PluginSystem::PluginInfo> meta;
        if (lmeta.contains("MetaData")) {
            auto m = parsePluginMetadata(lmeta["MetaData"].toObject());
            cret = checkPluginMetadata(m, std::is_same_v<T, IWingPlugin>);
            meta = m;
        } else {
            cret = PluginStatus::BrokenVersion;
        }

        switch (cret) {
        case PluginStatus::Valid:
            // OK and success
            break;
        case PluginStatus::BrokenVersion:
            Logger::critical(tr("InvalidPluginBrokenInfo"));
            return std::nullopt;
        case PluginStatus::LackDependencies: {
            _lazyplgs.append(fileName);
            return meta;
        } break;
        case PluginStatus::InvalidID:
            Logger::critical(tr("InvalidPluginID"));
            return std::nullopt;
        case PluginStatus::DupID:
            Logger::critical(tr("InvalidDupPlugin"));
            return std::nullopt;
        }

        auto p = qobject_cast<T *>(loader.instance());
        if (Q_UNLIKELY(p == nullptr)) {
            Logger::critical(loader.errorString());
        } else {
            auto m = meta.value();
            retranslateMetadata(p, m);
            loadPlugin(p, m, setdir);
        }
        Logger::newLine();
    }

    return std::nullopt;
}

WingAngelAPI *PluginSystem::angelApi() const { return _angelplg; }

EditorView *PluginSystem::getCurrentPluginView(IWingPlugin *plg) {
    if (plg == nullptr) {
        return nullptr;
    }
    if (!m_plgCurrentfid.contains(plg)) {
        m_plgCurrentfid.insert(plg, -1);
    }

    auto fid = m_plgCurrentfid[plg];
    auto ctx = pluginContextById(plg, fid);
    if (ctx) {
        return ctx->view;
    }
    return _win->m_curEditor;
}

EditorView *PluginSystem::handle2EditorView(IWingPlugin *plg, int handle) {
    if (handle < 0) {
        return getCurrentPluginView(plg);
    }

    auto ctx = pluginContextById(plg, handle);
    if (ctx) {
        return ctx->view;
    }

    return nullptr;
}

bool PluginSystem::equalCompareHandle(const SharedUniqueId &id, int handle) {
    return (*id) == handle;
}

int PluginSystem::getUIDHandle(const SharedUniqueId &id) {
    Q_ASSERT(id.data());
    return int(*id);
}

void PluginSystem::retranslateMetadata(IWingPluginBase *plg, PluginInfo &meta) {
    meta.author = plg->retranslate(meta.author);
    meta.vendor = plg->retranslate(meta.vendor);
    meta.license = plg->retranslate(meta.license);
}

PluginSystem::SharedUniqueId
PluginSystem::assginHandleForPluginView(IWingPlugin *plg, EditorView *view) {
    if (plg == nullptr || view == nullptr) {
        return {};
    }

    auto id = m_idGen.get();

    PluginFileContext context;
    context.fid = id;
    context.linkedplg = plg;
    context.view = view;

    m_plgviewMap[plg].append(context);
    m_viewBindings[view].append(plg);
    return id;
}

PluginSystem::PluginInfo
PluginSystem::parsePluginMetadata(const QJsonObject &meta) {
    PluginSystem::PluginInfo info;

    info.id = meta["Id"].toString().trimmed();
    info.version =
        QVersionNumber::fromString(meta["Version"].toString().trimmed());

    info.vendor = meta["Vendor"].toString().trimmed();
    info.author = meta["Author"].toString().trimmed();
    info.license = meta["License"].toString().trimmed();
    info.url = meta["Url"].toString().trimmed();

    auto dependsObj = meta["Dependencies"];
    if (!dependsObj.isNull()) {
        if (dependsObj.isArray()) {
            auto arr = dependsObj.toArray();
            for (int i = 0; i < arr.size(); ++i) {
                auto d = arr.at(i);
                if (d.isObject()) {
                    auto id = d["Id"].toString().trimmed();
                    if (id.isEmpty()) {
                        continue;
                    }
                    auto version = QVersionNumber::fromString(
                        d["Version"].toString().trimmed());
                    WingDependency dp;
                    dp.puid = id;
                    dp.version = version;
                    info.dependencies.append(dp);
                }
            }
        }
    }

    return info;
}

PluginSystem::PluginStatus
PluginSystem::checkPluginMetadata(const PluginInfo &meta, bool isPlg) {
    constexpr auto puid_limit = 36; // same as uuid length, so enough

    if (meta.id.isEmpty() && meta.id.length() > puid_limit) {
        return PluginStatus::InvalidID;
    }

    if (meta.id.front().isDigit()) {
        return PluginStatus::InvalidID;
    }
    auto r = std::find_if(meta.id.begin(), meta.id.end(), [](const QChar &ch) {
        return !ch.isLetterOrNumber() && ch != '_';
    });
    if (r != meta.id.end()) {
        return PluginStatus::InvalidID;
    }

    if (isPluginLoaded(meta.id)) {
        return PluginStatus::DupID;
    }
    if (meta.version.isNull()) {
        return PluginStatus::BrokenVersion;
    }
    if (isPlg) {
        for (auto &d : meta.dependencies) {
            if (!isPluginLoaded(d)) {
                return PluginStatus::LackDependencies;
            }
        }
    }
    return PluginStatus::Valid;
}

bool PluginSystem::checkPluginCanOpenedFile(IWingPlugin *plg) {
    if (plg == nullptr) {
        return false;
    }
    return m_plgviewMap[plg].size() <= UCHAR_MAX;
}

bool PluginSystem::checkPluginHasAlreadyOpened(IWingPlugin *plg,
                                               EditorView *view) {
    auto &maps = m_plgviewMap[plg];
    auto ret = std::find_if(maps.begin(), maps.end(),
                            [view](const PluginFileContext &content) {
                                return content.view == view;
                            });
    return ret != maps.end();
}

void PluginSystem::cleanUpEditorViewHandle(EditorView *view) {
    if (m_viewBindings.contains(view)) {
        auto v = m_viewBindings[view];

        // clean up
        for (auto &plg : v) {
            auto &handles = m_plgviewMap[plg];
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            handles.erase(std::remove_if(
                handles.begin(), handles.end(),
                [view, this](const PluginFileContext &v) {
                    if (v.view == view) {
                        if (equalCompareHandle(v.fid,
                                               m_plgCurrentfid[v.linkedplg])) {
                            m_plgCurrentfid[v.linkedplg] = -1;
                        }
                        dispatchEvent(
                            IWingPlugin::RegisteredEvent::PluginFileClosed,
                            {quintptr(v.linkedplg), v.view->fileName(),
                             int(v.fid),
                             QVariant::fromValue(
                                 _win->getEditorViewFileType(view))});
                        return true;
                    }
                    return false;
                }));

#else
            handles.removeIf([view, this](const PluginFileContext &v) {
                if (v.view == view) {
                    if (equalCompareHandle(v.fid,
                                           m_plgCurrentfid[v.linkedplg])) {
                        m_plgCurrentfid[v.linkedplg] = -1;
                    }
                    dispatchEvent(
                        IWingPlugin::RegisteredEvent::PluginFileClosed,
                        {quintptr(v.linkedplg), v.view->fileName(),
                         getUIDHandle(v.fid),
                         QVariant::fromValue(
                             _win->getEditorViewFileType(view))});
                    return true;
                }
                return false;
            });
#endif
        }
        m_viewBindings.remove(view);
    }
}

bool PluginSystem::closeEditor(IWingPlugin *plg, int handle, bool force) {
    if (handle >= 0) {
        auto &handles = m_plgviewMap[plg];
        auto r = std::find_if(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            handles.cbegin(), handles.cend(),
#else
            handles.begin(), handles.end(),
#endif
            [handle](const PluginFileContext &d) {
                return equalCompareHandle(d.fid, handle);
            });
        if (r == handles.end()) {
            return false;
        }

        auto &bind = m_viewBindings[r->view];
        bind.removeOne(plg);
        if (bind.isEmpty()) {
            _win->closeEditor(r->view, force);
            m_viewBindings.remove(r->view);
        }

        if (m_plgCurrentfid[plg] == handle) {
            m_plgCurrentfid[plg] = -1;
        }

        handles.erase(r);
    } else {
        return false;
    }

    return true;
}

bool PluginSystem::closeHandle(IWingPlugin *plg, int handle) {
    if (handle < 0) {
        return false;
    }
    auto &handles = m_plgviewMap[plg];
    auto r = std::find_if(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        handles.cbegin(), handles.cend(),
#else
        handles.begin(), handles.end(),
#endif
        [handle](const PluginFileContext &d) {
            return equalCompareHandle(d.fid, handle);
        });
    if (r == handles.end()) {
        return false;
    }

    auto &bind = m_viewBindings[r->view];
    bind.removeOne(plg);
    if (bind.isEmpty()) {
        m_viewBindings.remove(r->view);
    }

    if (m_plgCurrentfid[plg] == handle) {
        m_plgCurrentfid[plg] = -1;
    }
    handles.erase(r);
    return true;
}

bool PluginSystem::dispatchEvent(IWingPlugin::RegisteredEvent event,
                                 const QVariantList &params) {
    switch (event) {
    case WingHex::IWingPlugin::RegisteredEvent::SelectionChanged: {
        Q_ASSERT(params.size() == 2 &&
                 params.at(0).canConvert<QByteArrayList>() &&
                 params.at(1).canConvert<bool>());
        auto buffers = params.first().value<QByteArrayList>();
        auto isPreview = params.at(1).toBool();
        for (auto &plg : _evplgs[event]) {
            plg->eventSelectionChanged(buffers, isPreview);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::CursorPositionChanged: {
        Q_ASSERT(params.size() == 1 && params.at(0).canConvert<QHexPosition>());
        auto cursor = params.at(0).value<QHexPosition>();
        HexPosition pos;
        pos.line = cursor.line;
        pos.column = cursor.column;
        pos.nibbleindex = cursor.nibbleindex;
        pos.lineWidth = cursor.lineWidth;
        for (auto &plg : _evplgs[event]) {
            plg->eventCursorPositionChanged(pos);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileOpened: {
        Q_ASSERT(params.size() == 2);
        auto fileName = params.at(0).toString();
        auto fileType = params.at(1).value<WingHex::IWingPlugin::FileType>();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Opened, fileType,
                                 fileName, -1, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileSaved: {
        Q_ASSERT(params.size() == 4);
        auto newFileName = params.at(0).toString();
        auto oldFileName = params.at(1).toString();
        auto isExported = params.at(2).toBool();
        auto fileType = params.at(3).value<WingHex::IWingPlugin::FileType>();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(isExported
                                     ? IWingPlugin::PluginFileEvent::Exported
                                     : IWingPlugin::PluginFileEvent::Saved,
                                 fileType, oldFileName, -1, newFileName);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileSwitched: {
        Q_ASSERT(params.size() == 2);
        auto newFileName = params.at(0).toString();
        auto oldFileName = params.at(1).toString();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Switched,
                                 IWingPlugin::FileType::Invalid, oldFileName,
                                 -1, newFileName);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::AppReady: {
        Q_ASSERT(params.isEmpty());
        for (auto &plg : _evplgs[event]) {
            plg->eventReady();
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileClosed: {
        Q_ASSERT(params.size() == 2);
        auto fileName = params.first().toString();
        auto fileType = params.at(1).value<WingHex::IWingPlugin::FileType>();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Closed, fileType,
                                 fileName, -1, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::ScriptPragma: {
        Q_ASSERT(params.size() == 4);
        auto section = params.at(0).toString();
        auto plgID = params.at(1).toString();
        auto &es = _evplgs[event];
        auto r = std::find_if(
            es.constBegin(), es.constEnd(),
            [plgID](IWingPlugin *p) { return getPUID(p) == plgID; });
        if (r == es.constEnd()) {
            return false;
        }
        return (*r)->eventOnScriptPragma(section, params.at(3).toStringList());
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::ScriptPragmaInit: {
        Q_ASSERT(params.isEmpty());
        for (auto &plg : _evplgs[event]) {
            plg->eventOnScriptPragmaInit();
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::PluginFileOpened: {
        Q_ASSERT(params.size() == 4);
        auto plg =
            reinterpret_cast<IWingPlugin *>(params.at(0).value<quintptr>());
        auto fileName = params.at(1).toString();
        auto id = params.at(2).toInt();
        auto fileType = params.at(3).value<WingHex::IWingPlugin::FileType>();
        if (_evplgs[event].contains(plg)) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::PluginOpened,
                                 fileType, fileName, id, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::PluginFileClosed: {
        Q_ASSERT(params.size() == 4);
        auto plg =
            reinterpret_cast<IWingPlugin *>(params.at(0).value<quintptr>());
        auto fileName = params.at(1).toString();
        auto id = params.at(2).toInt();
        auto fileType = params.at(3).value<WingHex::IWingPlugin::FileType>();
        if (_evplgs[event].contains(plg)) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::PluginClosed,
                                 fileType, fileName, id, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::AppClosing:
        for (auto &plg : _evplgs[event]) {
            auto ret = plg->eventClosing();
            if (!ret) {
                Logger::warning(tr("AppClosingCanceled:") + plg->pluginName() +
                                QStringLiteral(" (") + getPluginID(plg) +
                                QStringLiteral(")"));
                return ret;
            }
        }
        break;
    default:
        return false;
    }
    return true;
}

IWingDevice *PluginSystem::ext2Device(const QString &ext) {
    auto r =
        std::find_if(_loadeddevs.begin(), _loadeddevs.end(),
                     [=](IWingDevice *dev) { return _pinfos[dev].id == ext; });
    if (r == _loadeddevs.end()) {
        return nullptr;
    }
    return *r;
}

PluginSystem::PluginInfo
PluginSystem::getPluginInfo(IWingPluginBase *plg) const {
    Q_ASSERT(plg);
    return _pinfos.value(plg);
}

QString PluginSystem::getPluginID(IWingPluginBase *plg) const {
    Q_ASSERT(plg);
    return _pinfos.value(plg).id;
}

void PluginSystem::loadExtPlugin() {
#ifdef QT_DEBUG
    QDir plugindir(QCoreApplication::applicationDirPath() + QDir::separator() +
                   QStringLiteral("plugin"));
#ifdef Q_OS_WIN
    plugindir.setNameFilters({"*.dll", "*.wingplg"});
#else
    plugindir.setNameFilters({"*.so", "*.wingplg"});
#endif
#else
    QDir plugindir(QCoreApplication::applicationDirPath() + QDir::separator() +
                   QStringLiteral("plugin"));
    plugindir.setNameFilters({"*.wingplg"});
#endif

    Logger::newLine();
    checkDirRootSafe(plugindir);

    auto plgs = plugindir.entryInfoList();
    Logger::info(tr("FoundPluginCount") + QString::number(plgs.count()));

    if (!plgs.isEmpty()) {
        Logger::newLine();
    }

    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("plgset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        throw CrashCode::PluginSetting;
    }

    for (auto &item : plgs) {
        loadPlugin<IWingPlugin>(item, udir);
    }

    QList<PluginSystem::PluginInfo> errorplg;
    if (!_lazyplgs.isEmpty()) {
        QStringList lazyplgs;
        lazyplgs.swap(_lazyplgs);

        for (auto &item : lazyplgs) {
            auto r = loadPlugin<IWingPlugin>(QFileInfo(item), udir);
            if (r) {
                errorplg.append(r.value());
            }
        }
    }

    if (!_lazyplgs.isEmpty()) {
        Logger::critical(tr("PluginLoadingFailedSummary"));
        Logger::newLine();

        for (auto &lplg : errorplg) {
            Logger::critical(tr("- PluginID:") + lplg.id);
            Logger::critical(tr("- Dependencies:"));
            for (auto &d : lplg.dependencies) {
                Logger::critical(QString(4, ' ') + tr("PUID:") + d.puid);
                Logger::critical(QString(4, ' ') + tr("Version:") +
                                 d.version.toString());
            }
        }
        _lazyplgs.clear();
    }

    Logger::info(tr("PluginLoadingFinished"));
}

void PluginSystem::loadDevicePlugin() {
#ifdef QT_DEBUG
    QDir devdir(QCoreApplication::applicationDirPath() + QDir::separator() +
                QStringLiteral("devdrv"));
#ifdef Q_OS_WIN
    devdir.setNameFilters({"*.dll", "*.wingdrv"});
#else
    devdir.setNameFilters({"*.so", "*.wingdrv"});
#endif
#else
    QDir devdir(QCoreApplication::applicationDirPath() + QDir::separator() +
                QStringLiteral("devdrv"));
    devdir.setNameFilters({"*.wingdrv"});
#endif

    Logger::newLine();
    checkDirRootSafe(devdir);

    auto plgs = devdir.entryInfoList();
    Logger::info(tr("FoundDrvPluginCount") + QString::number(plgs.count()));
    if (!plgs.isEmpty()) {
        Logger::newLine();
    }

    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("drvset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        throw CrashCode::PluginSetting;
    }

    for (auto &item : plgs) {
        loadPlugin<IWingDevice>(item, udir);
    }
}

void PluginSystem::checkDirRootSafe(const QDir &dir) {
    auto testFileName =
        dir.absoluteFilePath(QUuid::createUuid().toString(QUuid::Id128));

    QFile f(testFileName);
    if (f.open(QFile::WriteOnly)) {
        f.close();
        f.remove();
        Logger::warning(QStringLiteral("<i><u>") % tr("UnsafePluginDir") %
                        QStringLiteral("</u></i>"));
    }
}

void PluginSystem::registerFns(IWingPlugin *plg) {
    Q_ASSERT(plg);
    auto fns = plg->registeredScriptFns();
    if (fns.isEmpty()) {
        return;
    }

    // checking functions
    asCBuilder c(_engine, nullptr);

    auto id = getPluginID(plg);
    auto rawID = id.toUtf8();

    QList<asCScriptFunction *> fnlist;

    auto r = _engine->SetDefaultNamespace(rawID);
    if (Q_UNLIKELY(r < 0)) {
        _engine->configFailed = false;
        return;
    }

    // <signatures, std::function>
    QHash<QString, IWingPlugin::ScriptFnInfo> rfns;
    for (auto p = fns.constKeyValueBegin(); p != fns.constKeyValueEnd(); ++p) {
        auto func = new asCScriptFunction(_engine, nullptr, asFUNC_SYSTEM);

        auto &fn = p->second;
        auto sig = getScriptFnSig(p->first, fn);
        if (sig.isEmpty()) {
            Logger::critical(tr("RegisterScriptFnUnSupportedTypes:") + id +
                             QStringLiteral("::") + p->first);
            func->Release();
            continue;
        }

        auto r = c.ParseFunctionDeclaration(nullptr, sig.toLatin1(), func, true,
                                            nullptr, nullptr,
                                            _engine->defaultNamespace);
        if (r < 0) {
            Logger::critical(tr("RegisterScriptFnInvalidSig:") + sig);
            func->Release();
            continue;
        }

        r = c.CheckNameConflict(func->name.AddressOf(), 0, 0,
                                _engine->defaultNamespace, false, false, false);
        if (r < 0) {
            Logger::critical(tr("RegisterScriptFnConflitSig:") + sig);
            func->Release();
            continue;
        }

        fnlist.append(func);

        // register functions with internal hacking
        func->id = _engine->GetNextScriptFunctionId();
        _engine->AddScriptFunction(func);
        rfns.insert(sig, p->second);
    }

    // clear the internal hacking functions
    for (auto &fn : fnlist) {
        _engine->RemoveScriptFunction(fn);
    }
    _engine->SetDefaultNamespace("");

    fnlist.clear();

    Q_ASSERT(_angelplg);
    _angelplg->registerScriptFns(_pinfos.value(plg).id, rfns);
}

void PluginSystem::registerUnSafeFns(IWingPlugin *plg) {
    Q_ASSERT(plg);

    auto fns = plg->registeredScriptUnsafeFns();
    if (fns.isEmpty()) {
        return;
    }

    // checking functions
    asCBuilder c(_engine, nullptr);

    auto id = getPluginID(plg);
    auto rawID = id.toUtf8();

    QList<asCScriptFunction *> fnlist;

    auto r = _engine->SetDefaultNamespace(rawID);
    if (Q_UNLIKELY(r < 0)) {
        _engine->configFailed = false;
        return;
    }

    // <signatures, std::function>
    QHash<QString, IWingPlugin::UNSAFE_SCFNPTR> rfns;
    for (auto p = fns.constKeyValueBegin(); p != fns.constKeyValueEnd(); ++p) {
        auto func = new asCScriptFunction(_engine, nullptr, asFUNC_SYSTEM);
        auto sig = p->first;
        auto r = c.ParseFunctionDeclaration(nullptr, sig.toLatin1(), func, true,
                                            nullptr, nullptr,
                                            _engine->defaultNamespace);
        if (r < 0) {
            Logger::critical(tr("RegisterScriptFnInvalidSig:") + sig);
            func->funcType = asFUNC_DUMMY;
            continue;
        }

        r = c.CheckNameConflict(func->name.AddressOf(), 0, 0,
                                _engine->defaultNamespace, false, false, false);
        if (r < 0) {
            Logger::critical(tr("RegisterScriptFnConflitSig:") + sig);
            func->funcType = asFUNC_DUMMY;
            continue;
        }

        fnlist.append(func);

        // register functions with internal hacking
        func->id = _engine->GetNextScriptFunctionId();
        _engine->AddScriptFunction(func);
        rfns.insert(sig, p->second);
    }

    // clear the internal hacking functions
    for (auto &fn : fnlist) {
        _engine->RemoveScriptFunction(fn);
    }
    _engine->SetDefaultNamespace("");

    fnlist.clear();

    Q_ASSERT(_angelplg);
    _angelplg->registerUnSafeScriptFns(_pinfos.value(plg).id, rfns);
}

void PluginSystem::registerEnums(IWingPlugin *plg) {
    Q_ASSERT(plg);
    auto objs = plg->registeredScriptEnums();
    if (objs.isEmpty()) {
        return;
    }

    decltype(objs) passedEnums;

    // checking enums
    // Here we just register the enum in real action.
    // If failed, we can hack and restore the engine.
    auto id = getPluginID(plg);
    auto r = _engine->SetDefaultNamespace(id.toUtf8());
    if (Q_UNLIKELY(r < 0)) {
        _engine->configFailed = false;
        return;
    }

    // not neccssary to really restore when failed, 'cause of unique namespace
    for (auto el = objs.constKeyValueBegin(); el != objs.constKeyValueEnd();
         ++el) {
        auto ename = el->first.toUtf8();
        auto r = _engine->RegisterEnum(ename);
        if (r < 0) {
            Logger::critical(tr("InvalidEnumName:") + el->first);
            _engine->configFailed = false;
            continue;
        }

        bool hasNoError = true;
        for (auto &ev : el->second) {
            r = _engine->RegisterEnumValue(ename, ev.first.toUtf8(), ev.second);
            if (r < 0) {
                Logger::critical(
                    tr("InvalidEnumValue:") % el->first % QStringLiteral("::") %
                    ev.first % QStringLiteral(" (") %
                    QString::number(ev.second) % QStringLiteral(")"));
                _engine->configFailed = false;
                hasNoError = false;
                break;
            }
        }

        if (hasNoError) {
            passedEnums.insert(el->first, el->second);
        }
    }

    Q_ASSERT(_angelplg);
    _angelplg->registerScriptEnums(id, passedEnums);
}

void PluginSystem::registerEvents(IWingPlugin *plg) {
    Q_ASSERT(plg);
    auto evs = plg->registeredEvents();
    using FlagInt = decltype(evs)::Int;
    if (FlagInt(evs) == FlagInt(IWingPlugin::RegisteredEvent::None)) {
        return;
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::SelectionChanged)) {
        _evplgs[IWingPlugin::RegisteredEvent::SelectionChanged].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::CursorPositionChanged)) {
        _evplgs[IWingPlugin::RegisteredEvent::CursorPositionChanged].append(
            plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileOpened)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileOpened].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileSaved)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileSaved].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileSwitched)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileSwitched].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::AppReady)) {
        _evplgs[IWingPlugin::RegisteredEvent::AppReady].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::AppClosing)) {
        _evplgs[IWingPlugin::RegisteredEvent::AppClosing].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileClosed)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileClosed].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::ScriptPragma)) {
        _evplgs[IWingPlugin::RegisteredEvent::ScriptPragma].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::ScriptPragmaInit)) {
        _evplgs[IWingPlugin::RegisteredEvent::ScriptPragmaInit].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::PluginFileOpened)) {
        _evplgs[IWingPlugin::RegisteredEvent::PluginFileOpened].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::PluginFileClosed)) {
        _evplgs[IWingPlugin::RegisteredEvent::PluginFileClosed].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::ScriptUnSafeFnRegistering)) {
        registerUnSafeFns(plg);
    }
}

QString PluginSystem::type2AngelScriptString(IWingPlugin::MetaType type,
                                             bool isArg, bool noModifier) {
    auto isArray = !!(type & WingHex::IWingPlugin::Array);
    auto isList = !!(type & WingHex::IWingPlugin::List);
    auto isContainer = isArray || isList;
    if (isContainer) {
        if (isArray && isList) {
            return {};
        }
    }

    QString retype;
    bool complexType = false;

    type = IWingPlugin::MetaType(type & WingHex::IWingPlugin::MetaTypeMask);
    switch (type) {
    case WingHex::IWingPlugin::Void:
        retype = QStringLiteral("void");
        break;
    case WingHex::IWingPlugin::Bool:
        retype = QStringLiteral("bool");
        break;
    case WingHex::IWingPlugin::Int:
        retype = QStringLiteral("int");
        break;
    case WingHex::IWingPlugin::UInt:
        retype = QStringLiteral("uint");
        break;
    case WingHex::IWingPlugin::Int8:
        retype = QStringLiteral("int8");
        break;
    case WingHex::IWingPlugin::UInt8:
        retype = QStringLiteral("uint8");
        break;
    case WingHex::IWingPlugin::Int16:
        retype = QStringLiteral("int16");
        break;
    case WingHex::IWingPlugin::UInt16:
        retype = QStringLiteral("uint16");
        break;
    case WingHex::IWingPlugin::Int64:
        retype = QStringLiteral("int64");
        break;
    case WingHex::IWingPlugin::UInt64:
        retype = QStringLiteral("uint64");
        break;
    case WingHex::IWingPlugin::Float:
        retype = QStringLiteral("float");
        break;
    case WingHex::IWingPlugin::Double:
        retype = QStringLiteral("double");
        break;
    case WingHex::IWingPlugin::String:
        retype = QStringLiteral("string");
        break;
    case WingHex::IWingPlugin::Char:
        retype = QStringLiteral("char");
        break;
    case WingHex::IWingPlugin::Byte:
        retype = QStringLiteral("byte");
        break;
    case WingHex::IWingPlugin::Color:
        retype = QStringLiteral("color");
        complexType = true;
        break;
    case WingHex::IWingPlugin::Map:
    case WingHex::IWingPlugin::Hash:
        retype = QStringLiteral("dictionary");
        complexType = true;
        break;
    default:
        return {};
    }

    if (isArray || isList) {
        retype.append(QStringLiteral("[]"));
    }

    if (isArg) {
        if (!noModifier && (isContainer || complexType)) {
            retype.append(QStringLiteral(" &in"))
                .prepend(QStringLiteral("const "));
        }
    } else {
        if (!noModifier) {
            // if it's a return type, only array<byte> and array<string> are
            // supported in AngelScript
            // ( array<byte> -> QByteArray , array<string> -> QStringList ),
            // other array types are not suported. PRs are welcomed !!!
            // IT'S TOO COMPLEX TO SUPPORT QVARIANTLIST !!!

            // You can use unsafe registering to support
            // extensive scripting system.
            // It will be faster and flexible but not easy to implement.
            if (isContainer) {
                if (type != WingHex::IWingPlugin::Byte &&
                    type != WingHex::IWingPlugin::String) {
                    return {};
                }

                retype.append(QStringLiteral("@"));
            }

            if (complexType) {
                retype.append(QStringLiteral("@"));
            }
        }
    }

    return retype.trimmed();
}

QString PluginSystem::getScriptFnSig(const QString &fnName,
                                     const IWingPlugin::ScriptFnInfo &fninfo) {
    if (fnName.isEmpty()) {
        return {};
    }

    QString sig;

    auto ret = type2AngelScriptString(fninfo.ret, false);
    if (ret.isEmpty()) {
        return {};
    }

    sig += ret + QStringLiteral(" ") + fnName + QStringLiteral("(");

    QStringList params;
    for (auto &param : fninfo.params) {
        auto ret = type2AngelScriptString(param.first, true);
        if (ret.isEmpty()) {
            return {};
        }

        params << ret + QStringLiteral(" ") + param.second;
    }

    return sig + params.join(',') + QStringLiteral(")");
}

QString PluginSystem::getPUID(IWingPluginBase *p) {
    if (p) {
        return PluginSystem::instance().getPluginID(p);
    } else {
        return {};
    }
}

bool PluginSystem::isPluginLoaded(const WingDependency &d) {
    for (auto &info : _pinfos) {
        if ((d.version.isNull() || info.version >= d.version) &&
            info.id == d.puid) {
            return true;
        }
    }
    return false;
}

bool PluginSystem::isPluginLoaded(const QString &id) {
    for (auto &info : _pinfos) {
        if (info.id == id) {
            return true;
        }
    }
    return false;
}

void PluginSystem::loadPlugin(IWingPlugin *p, PluginInfo &meta,
                              const std::optional<QDir> &setdir) {
    QTranslator *p_tr = nullptr;

    try {
        if (p->signature() != WINGSUMMER) {
            throw tr("ErrLoadPluginSign");
        }

        if (p->sdkVersion() != SDKVERSION) {
            throw tr("ErrLoadPluginSDKVersion");
        }

        if (!p->pluginName().trimmed().length()) {
            throw tr("ErrLoadPluginNoName");
        }

        // dependencies had been checked

        emit pluginLoading(p->pluginName());

        p_tr = LanguageManager::instance().try2LoadPluginLang(meta.id);

        connectLoadingInterface(p);

        {
            std::unique_ptr<QSettings> setp(nullptr);
            if (setdir.has_value()) {
                setp = std::make_unique<QSettings>(
                    setdir->absoluteFilePath(meta.id),
                    QSettings::Format::IniFormat);
            }

            if (!p->init(setp)) {
                setp->deleteLater();
                throw tr("ErrLoadInitPlugin");
            }
        }

        _loadedplgs.append(p);
        _pinfos.insert(p, meta);

        Logger::warning(tr("PluginName :") + p->pluginName());
        Logger::warning(tr("PluginAuthor :") + meta.author);
        Logger::warning(tr("PluginWidgetRegister"));

        // ensure call only once
        auto ribbonToolboxInfos = p->registeredRibbonTools();
        if (!ribbonToolboxInfos.isEmpty()) {
            for (auto &item : ribbonToolboxInfos) {
                if (_win->m_ribbonMaps.contains(item.catagory)) {
                    if (!item.toolboxs.isEmpty()) {
                        auto &cat = _win->m_ribbonMaps[item.catagory];
                        for (auto &group : item.toolboxs) {
                            if (group.tools.isEmpty()) {
                                continue;
                            }
                            auto g = cat->addGroup(group.name);
                            for (auto &tool : group.tools) {
                                g->addButton(tool);
                            }
                        }
                    }
                } else {
                    if (!item.toolboxs.isEmpty()) {
                        bool isSys = false;
                        RibbonTabContent *cat;
                        if (_win->m_ribbonMaps.contains(item.catagory)) {
                            isSys = true;
                            cat = _win->m_ribbonMaps.value(item.catagory);
                        } else {
                            cat = new RibbonTabContent;
                        }
                        bool hasAdded = false;
                        for (auto &group : item.toolboxs) {
                            if (group.tools.isEmpty()) {
                                continue;
                            }
                            auto g = cat->addGroup(group.name);
                            for (auto &tool : group.tools) {
                                g->addButton(tool);
                            }
                            hasAdded = true;
                        }
                        if (!isSys) {
                            if (hasAdded) {
                                _win->m_ribbon->addTab(cat, item.displayName);
                                _win->m_ribbonMaps[item.catagory] = cat;
                            } else {
                                delete cat;
                            }
                        }
                    }
                }
            }
        }

        registerPluginDockWidgets(p);

        {
            auto menu = p->registeredHexContextMenu();
            if (menu) {
                _win->m_hexContextMenu.append(menu);
            }
        }

        {
            auto vieww = p->registeredEditorViewWidgets();
            if (!vieww.isEmpty()) {
                _win->m_editorViewWidgets.insert(p, vieww);
            }
        }

        {
            auto sp = p->registeredSettingPages();
            if (!sp.isEmpty()) {
                for (auto page = sp.keyBegin(); page != sp.keyEnd(); ++page) {
                    auto pp = *page;
                    pp->setProperty("__plg__", QVariant::fromValue(p));
                }
                _win->m_settingPages.insert(sp);
            }
        }

        registerPluginPages(p);

        registerFns(p);
        registerEnums(p);
        registerEvents(p);
        connectInterface(p);

        // prepare for file contenxt
        m_plgviewMap.insert(p, {});
        m_plgCurrentfid.insert(p, -1);
    } catch (const QString &error) {
        Logger::critical(error);
        if (p_tr) {
            p_tr->deleteLater();
        }
    }
}

void PluginSystem::loadPlugin(IWingDevice *p, PluginInfo &meta,
                              const std::optional<QDir> &setdir) {
    QTranslator *p_tr = nullptr;

    try {
        if (p->signature() != WINGSUMMER) {
            throw tr("ErrLoadExtPluginSign");
        }

        if (p->sdkVersion() != SDKVERSION) {
            throw tr("ErrLoadExtPluginSDKVersion");
        }

        Logger::warning(tr("ExtPluginAuthor :") + meta.author);
        Logger::warning(tr("ExtPluginWidgetRegister"));

        p_tr = LanguageManager::instance().try2LoadPluginLang(meta.id);

        connectLoadingInterface(p);

        {
            std::unique_ptr<QSettings> setp(nullptr);
            if (setdir.has_value()) {
                setp = std::make_unique<QSettings>(
                    setdir->absoluteFilePath(p->metaObject()->className()),
                    QSettings::Format::IniFormat);
            }

            if (!p->init(setp)) {
                setp->deleteLater();
                throw tr("ErrLoadInitExtPlugin");
            }
        }

        _loadeddevs.append(p);
        _pinfos.insert(p, meta);

        registerPluginPages(p);
        connectInterface(p);

        // ok register into menu open
        auto menu =
            _win->m_toolBtneditors[MainWindow::ToolButtonIndex::OPEN_EXT]
                ->menu();
        menu->addAction(_win->newAction(
            p->supportedFileIcon(), p->supportedFileExtDisplayName(),
            [p, this]() {
                auto file = p->onOpenFileBegin();
                if (file.isNull()) {
                    // common dialog
                    file = WingFileDialog::getOpenFileName(
                        _win, tr("ChooseFile"), _win->m_lastusedpath);
                }

                if (file.isEmpty()) {
                    return;
                }

                EditorView *view = nullptr;
                auto res = _win->openExtFile(getPluginID(p), file, &view);

                if (res == ErrFile::NotExist) {
                    WingMessageBox::critical(_win, tr("Error"),
                                             tr("FileNotExist"));
                    return;
                }
                if (res == ErrFile::Permission) {
                    WingMessageBox::critical(_win, tr("Error"),
                                             tr("FilePermission"));
                    return;
                }

                if (res == ErrFile::AlreadyOpened) {
                    Q_ASSERT(view);
                    view->raise();
                    view->setFocus();
                    return;
                }
            }));

    } catch (const QString &error) {
        Logger::critical(error);
        if (p_tr) {
            p_tr->deleteLater();
        }
    }
}

void PluginSystem::registerPluginDockWidgets(IWingPluginBase *p) {
    auto dockWidgets = p->registeredDockWidgets();
    if (!dockWidgets.isEmpty()) {
        for (auto &info : dockWidgets) {
            auto widgetName = info.widgetName.trimmed();
            auto displayName = info.displayName.trimmed();
            if (displayName.isEmpty()) {
                displayName = widgetName;
            }
            if (widgetName.isEmpty()) {
                Logger::warning(tr("EmptyNameDockWidget:") %
                                QStringLiteral("{ ") % p->pluginName() %
                                QStringLiteral(" }"));
                continue;
            }

            auto inch = std::find_if_not(
                widgetName.begin(), widgetName.end(),
                [](const QChar &ch) { return ch.isLetterOrNumber(); });
            if (inch != widgetName.end()) {
                Logger::warning(tr("InvalidNameDockWidget:") %
                                QStringLiteral("{ ") % p->pluginName() %
                                QStringLiteral(" } ") % widgetName);
                continue;
            }

            if (info.widget == nullptr) {
                Logger::warning(
                    tr("InvalidNullDockWidget:") % QStringLiteral("{ ") %
                    p->pluginName() % QStringLiteral(" } ") % widgetName %
                    QStringLiteral(" ( ") % displayName % QStringLiteral(" )"));
                continue;
            }

            auto dw =
                _win->buildDockWidget(_win->m_dock, widgetName, displayName,
                                      info.widget, MainWindow::PLUGIN_VIEWS);
            _raisedw.insert(info.widget, dw);

            switch (info.area) {
            case Qt::LeftDockWidgetArea: {
                if (_win->m_leftViewArea == nullptr) {
                    _win->m_leftViewArea = _win->m_dock->addDockWidget(
                        ads::LeftDockWidgetArea, dw);
                } else {
                    _win->m_leftViewArea = _win->m_dock->addDockWidget(
                        ads::CenterDockWidgetArea, dw, _win->m_leftViewArea);
                }
            } break;
            case Qt::RightDockWidgetArea:
                _win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                            _win->m_rightViewArea);
                break;
            case Qt::TopDockWidgetArea: {
                if (_win->m_topViewArea == nullptr) {
                    _win->m_topViewArea = _win->m_dock->addDockWidget(
                        ads::LeftDockWidgetArea, dw);
                } else {
                    _win->m_topViewArea = _win->m_dock->addDockWidget(
                        ads::CenterDockWidgetArea, dw, _win->m_topViewArea);
                }
            } break;
            case Qt::BottomDockWidgetArea:
                _win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                            _win->m_bottomViewArea);
                break;
            case Qt::DockWidgetArea_Mask:
            case Qt::NoDockWidgetArea:
                _win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                            _win->m_rightViewArea);
                dw->hide();
                break;
            }
        }
    }
}

void PluginSystem::registerPluginPages(IWingPluginBase *p) {
    auto rp = p->registeredPages();
    if (!rp.isEmpty()) {
        for (auto &page : rp) {
            page->setProperty("__plg__", QVariant::fromValue(p));
        }
        _win->m_plgPages.append(rp);
    }
}

bool PluginSystem::updateTextList_API(const QStringList &data,
                                      const QString &title,
                                      const ClickCallBack &click,
                                      const ClickCallBack &dblclick) {
    auto oldmodel = _win->m_infolist->model();
    if (oldmodel) {
        oldmodel->deleteLater();
    }
    _win->m_infolist->setProperty("__TITLE__", title);
    auto model = new QStringListModel(data);
    _win->m_infolist->setModel(model);
    _win->m_infoclickfn = click;
    _win->m_infodblclickfn = dblclick;
    return true;
}

bool PluginSystem::updateTextTree_API(const QString &json, const QString &title,
                                      const ClickCallBack &click,
                                      const ClickCallBack &dblclick) {
    auto oldmodel = _win->m_infotree->model();
    if (oldmodel) {
        oldmodel->deleteLater();
    }
    _win->m_infotree->setProperty("__TITLE__", title);
    auto model = new QJsonModel;
    if (model->loadJson(json.toUtf8())) {
        _win->m_infotree->setModel(model);
        _win->m_infotreeclickfn = click;
        _win->m_infotreedblclickfn = dblclick;
        return true;
    }
    return false;
}

bool PluginSystem::updateTextTable_API(const QString &json,
                                       const QStringList &headers,
                                       const QStringList &headerNames,
                                       const QString &title,
                                       const ClickCallBack &click,
                                       const ClickCallBack &dblclick) {
    auto oldmodel = _win->m_infotable->model();
    if (oldmodel) {
        oldmodel->deleteLater();
    }

    QJsonTableModel::Header header;
    if (headers.size() > headerNames.size()) {
        for (auto &name : headers) {
            QJsonTableModel::Heading heading;
            heading["index"] = name;
            heading["title"] = name;
            header.append(heading);
        }
    } else {
        auto np = headerNames.cbegin();
        for (auto p = headers.cbegin(); p != headers.cend(); ++p, ++np) {
            QJsonTableModel::Heading heading;
            heading["index"] = *p;
            heading["title"] = *np;
            header.append(heading);
        }
    }
    _win->m_infotable->setProperty("__TITLE__", title);
    auto model = new QJsonTableModel(header);
    model->setJson(QJsonDocument::fromJson(json.toUtf8()));
    _win->m_infotable->setModel(model);
    _win->m_infotableclickfn = click;
    _win->m_infotabledblclickfn = dblclick;
    return true;
}

void PluginSystem::connectInterface(IWingPlugin *plg) {
    connectReaderInterface(plg);
    connectControllerInterface(plg);
}

void PluginSystem::connectLoadingInterface(IWingPlugin *plg) {
    connectBaseInterface(plg);
    connectUIInterface(plg);
}

void PluginSystem::connectBaseInterface(IWingPlugin *plg) {
    connectBaseInterface(dynamic_cast<IWingPluginBase *>(plg));
    connect(plg, &IWingPlugin::existsServiceHost, this,
            [this](const QString &puid) {
                return std::find_if(_loadedplgs.begin(), _loadedplgs.end(),
                                    [puid, this](IWingPlugin *plg) {
                                        return getPluginID(plg) == puid;
                                    }) != _loadedplgs.end();
            });
    connect(
        plg,
        QOverload<const QString &, const char *, Qt::ConnectionType,
                  QGenericReturnArgument, QGenericArgument, QGenericArgument,
                  QGenericArgument, QGenericArgument, QGenericArgument,
                  QGenericArgument, QGenericArgument, QGenericArgument,
                  QGenericArgument>::of(&IWingPlugin::invokeService),
        this,
        [=](const QString &puid, const char *method, Qt::ConnectionType type,
            QGenericReturnArgument ret, QGenericArgument val0,
            QGenericArgument val1, QGenericArgument val2, QGenericArgument val3,
            QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
            QGenericArgument val7, QGenericArgument val8) -> bool {
            auto r = std::find_if(
                _loadedplgs.begin(), _loadedplgs.end(),
                [=](IWingPlugin *plg) { return getPUID(plg) == puid; });
            if (r == _loadedplgs.end()) {
                return false;
            }

            auto obj = *r;
            auto meta = obj->metaObject();

            // filter the evil call and report to log
            QVarLengthArray<char, 512> sig;
            int len = qstrlen(method);
            if (len <= 0)
                return false;
            sig.append(method, len);
            sig.append('(');

            auto sname = QMetaType::fromType<WingHex::SenderInfo>().name();

            const char *typeNames[] = {ret.name(),  sname,       val0.name(),
                                       val1.name(), val2.name(), val3.name(),
                                       val4.name(), val5.name(), val6.name(),
                                       val7.name(), val8.name()};
            size_t paramCount;
            constexpr auto maxParamCount =
                sizeof(typeNames) / sizeof(const char *);
            for (paramCount = 1; paramCount < maxParamCount; ++paramCount) {
                len = qstrlen(typeNames[paramCount]);
                if (len <= 0)
                    break;
                sig.append(typeNames[paramCount], len);
                sig.append(',');
            }
            if (paramCount == 1)
                sig.append(')'); // no parameters
            else
                sig[sig.size() - 1] = ')';
            sig.append('\0');

            // checking
            auto midx = meta->indexOfMethod(sig.constData());
            if (midx < 0) {
                auto norm = QMetaObject::normalizedSignature(sig.constData());
                midx = meta->indexOfMethod(norm.constData());
                if (midx < 0) {
                    return false;
                }
            }

            auto m = meta->method(midx);

            if (m.methodType() == QMetaMethod::Signal) {
                // report
                Logger::warning(packLogMessage(
                    plg->metaObject()->className(),
                    tr("[EvilCall]") %
                        QString::fromLatin1(sig.data(), sig.length())));
                return false;
            }

            meta = plg->metaObject();
            SenderInfo info;
            info.plgcls = meta->className();
            info.puid = getPUID(plg);

            auto meta_name = "WING_META";
            // property first
            auto var = plg->property(meta_name);
            if (var.isValid()) {
                info.meta = var;
            } else {
                auto iidx = meta->indexOfClassInfo(meta_name);
                if (iidx >= 0) {
                    info.meta = QString(meta->classInfo(iidx).value());
                }
            }

            return m.invoke(obj, type, ret,
                            WINGAPI_ARG(WingHex::SenderInfo, info), val0, val1,
                            val2, val3, val4, val5, val6, val7, val8);
        });
}

void PluginSystem::connectReaderInterface(IWingPlugin *plg) {
    auto preader = &plg->reader;

    connect(preader, &WingPlugin::Reader::isCurrentDocEditing, _win,
            [=]() -> bool { return pluginCurrentEditor(plg); });
    connect(preader, &WingPlugin::Reader::currentDocFilename, _win,
            [=]() -> QString {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->fileName();
                }
                return QString();
            });
    connect(preader, &WingPlugin::Reader::isInsertionMode, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->cursor()->insertionMode() ==
                   QHexCursor::InsertMode;
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::isLocked, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->isLocked();
        }
        return true;
    });
    connect(preader, &WingPlugin::Reader::isKeepSize, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->isKeepSize();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::isModified, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->document()->isDocSaved();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::isReadOnly, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->isReadOnly();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::documentLines, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->renderer()->documentLines();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::documentBytes, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->documentBytes();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::currentPos, _win,
            [=]() -> HexPosition {
                HexPosition pos;
                memset(&pos, 0, sizeof(HexPosition));

                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto cursor = e->hexEditor()->cursor();
                    pos.line = cursor->currentLine();
                    pos.column = cursor->currentColumn();
                    pos.lineWidth = e->hexEditor()->document()->hexLineWidth();
                    pos.nibbleindex = cursor->currentNibble();
                }
                return pos;
            });
    connect(preader, &WingPlugin::Reader::currentRow, _win, [=]() -> qsizetype {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->currentRow();
        }
        return 0;
    });
    connect(preader, &WingPlugin::Reader::currentColumn, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->currentColumn();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::currentOffset, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->cursor()->position().offset();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::selectedLength, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->cursor()->currentSelectionLength();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::selectedBytes, _win,
            [=](qsizetype index) -> QByteArray {
                auto e = pluginCurrentEditor(plg);
                if (e && index >= 0 &&
                    index < e->hexEditor()->cursor()->selectionCount()) {
                    return e->hexEditor()->selectedBytes(index);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::selectionBytes, _win,
            [=]() -> QByteArrayList {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->selectedBytes();
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::selectionStart, _win,
            [=](qsizetype index) -> HexPosition {
                HexPosition pos;
                memset(&pos, 0, sizeof(HexPosition));

                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto cursor = e->hexEditor()->cursor();
                    if (index >= 0 && index < cursor->selectionCount()) {
                        auto qpos = cursor->selectionStart(index);
                        pos.line = qpos.line;
                        pos.column = qpos.column;
                        pos.lineWidth = qpos.lineWidth;
                        pos.nibbleindex = qpos.nibbleindex;
                    }
                }
                return pos;
            });
    connect(preader, &WingPlugin::Reader::selectionEnd, _win,
            [=](qsizetype index) -> HexPosition {
                HexPosition pos;
                memset(&pos, 0, sizeof(HexPosition));

                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto cursor = e->hexEditor()->cursor();
                    if (index >= 0 && index < cursor->selectionCount()) {
                        auto qpos = cursor->selectionEnd(index);
                        pos.line = qpos.line;
                        pos.column = qpos.column;
                        pos.lineWidth = qpos.lineWidth;
                        pos.nibbleindex = qpos.nibbleindex;
                    }
                }
                return pos;
            });
    connect(preader, &WingPlugin::Reader::selectionLength, _win,
            [=](qsizetype index) -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto cursor = e->hexEditor()->cursor();
                    if (index >= 0 && index < cursor->selectionCount()) {
                        return cursor->selectionLength(index);
                    }
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::selectionCount, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto cursor = e->hexEditor()->cursor();
                    return cursor->selectionCount();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::stringVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->asciiVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::headerVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->headerVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::addressVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->addressVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::addressBase, _win, [=]() -> quintptr {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->addressBase();
        }
        return 0;
    });
    connect(preader, &WingPlugin::Reader::readBytes, _win,
            [=](qsizetype offset, qsizetype len) -> QByteArray {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    _rwlock.lockForRead();
                    auto ret = e->hexEditor()->document()->read(offset, len);
                    return ret;
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::readInt8, _win,
            [=](qsizetype offset) -> qint8 {
                return readBasicTypeContent<qint8>(plg, offset);
            });
    connect(preader, &WingPlugin::Reader::readInt16, _win,
            [=](qsizetype offset) -> qint16 {
                return readBasicTypeContent<qint16>(plg, offset);
            });
    connect(preader, &WingPlugin::Reader::readInt32, _win,
            [=](qsizetype offset) -> qint32 {
                return readBasicTypeContent<qint32>(plg, offset);
            });
    connect(preader, &WingPlugin::Reader::readInt64, _win,
            [=](qsizetype offset) -> qint64 {
                return readBasicTypeContent<qint64>(plg, offset);
            });
    connect(preader, &WingPlugin::Reader::readFloat, _win,
            [=](qsizetype offset) -> float {
                return readBasicTypeContent<float>(plg, offset);
            });
    connect(preader, &WingPlugin::Reader::readDouble, _win,
            [=](qsizetype offset) -> double {
                return readBasicTypeContent<double>(plg, offset);
            });

    connect(preader, &WingPlugin::Reader::readString, _win,
            [=](qsizetype offset, const QString &encoding) -> QString {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    _rwlock.lockForRead();
                    auto hexeditor = e->hexEditor();
                    auto doc = hexeditor->document();
                    auto pos = doc->searchForward(offset, QByteArray(1, 0));
                    if (pos < 0) {
                        pos = doc->searchForward(offset, QByteArray(1, '\n'));
                        if (pos < 0) {
                            return QString();
                        }
                    }
                    auto buffer = doc->read(offset, int(pos - offset));
                    _rwlock.unlock();
                    return Utilities::decodingString(buffer, encoding);
                }
                return QString();
            });

    connect(preader, &WingPlugin::Reader::findAllBytes, _win,
            [=](qsizetype begin, qsizetype end,
                const QByteArray &b) -> QList<qsizetype> {
                QList<qsizetype> results;
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->document()->findAllBytes(begin, end, b,
                                                             results);
                }
                return results;
            });
    connect(preader, &WingPlugin::Reader::searchForward, _win,
            [=](qsizetype begin, const QByteArray &ba) -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->searchForward(begin, ba);
                }
                return qsizetype(-1);
            });
    connect(preader, &WingPlugin::Reader::searchBackward, _win,
            [=](qsizetype begin, const QByteArray &ba) -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->searchBackward(begin,
                                                                      ba);
                }
                return qsizetype(-1);
            });
    connect(
        preader, &WingPlugin::Reader::lineHasMetadata, _win,
        [=](qsizetype line) -> bool {
            auto e = pluginCurrentEditor(plg);
            if (e) {
                return e->hexEditor()->document()->metadata()->lineHasMetadata(
                    line);
            }
            return false;
        });
    connect(preader, &WingPlugin::Reader::lineHasBookMark, _win,
            [=](qsizetype line) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->lineHasBookMark(line);
                }
                return false;
            });
    connect(preader, &WingPlugin::Reader::getsBookmarkPos, _win,
            [=](qsizetype line) -> QList<qsizetype> {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->getLineBookmarksPos(
                        line);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::bookMarkComment, _win,
            [=](qsizetype pos) -> QString {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->bookMark(pos);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::existBookMark, _win,
            [=](qsizetype pos) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->existBookMark(pos);
                }
                return false;
            });
    connect(preader, &WingPlugin::Reader::getSupportedEncodings, _win,
            [=]() -> QStringList { return Utilities::getEncodings(); });
    connect(preader, &WingPlugin::Reader::getStorageDrivers, _win,
            [=]() -> QStringList {
                QStringList drivers;
                auto sdns = QStorageInfo::mountedVolumes();
                for (auto &item : sdns) {
                    drivers.append(item.device());
                }
                return drivers;
            });
}

void PluginSystem::connectControllerInterface(IWingPlugin *plg) {
    Q_ASSERT(plg);

    auto pctl = &plg->controller;
    connect(pctl, &WingPlugin::Controller::switchDocument, _win,
            [=](int handle) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                if (handle < 0) {
                    m_plgCurrentfid[plg] = -1;
                } else {
                    auto ctx = pluginContextById(plg, handle);
                    if (ctx) {
                        m_plgCurrentfid[plg] = handle;
                    }
                }
                return true;
            });
    connect(pctl, &WingPlugin::Controller::raiseDocument, _win,
            [=](int handle) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }

                if (handle < 0) {
                    auto cur = _win->m_curEditor;
                    if (cur) {
                        cur->raise();
                        return true;
                    }
                } else {
                    auto ctx = pluginContextById(plg, handle);
                    if (ctx) {
                        ctx->view->raise();
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setLockedFile, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->setLockedFile(b);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setKeepSize, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->setKeepSize(b);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setStringVisible, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setAsciiVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setHeaderVisible, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setHeaderVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setAddressVisible, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setAddressVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setAddressBase, _win,
            [=](quintptr base) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setAddressBase(base);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::beginMarco, _win,
            [=](const QString &txt) -> bool {
                auto fid = m_plgCurrentfid[plg];
                if (fid < 0) {
                    return false;
                }

                auto r = pluginContextByIdIt(plg, fid);
                if (r) {
                    (*r)->cmd = new QUndoCommand(txt);
                }
                return true;
            });
    connect(pctl, &WingPlugin::Controller::endMarco, _win, [=]() -> bool {
        auto fid = m_plgCurrentfid[plg];
        if (fid < 0) {
            return false;
        }

        auto &info = m_plgviewMap[plg];
        auto &e = info[fid];
        if (e.cmd) {
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::insertBytes, _win,
            [=](qsizetype offset, const QByteArray &data) -> bool {
                auto e = getCurrentPluginView(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd =
                        doc->MakeInsert(uc, editor->cursor(), offset, data);
                    if (uc == nullptr && cmd) {
                        _rwlock.lockForWrite();
                        doc->pushMakeUndo(cmd);
                        _rwlock.unlock();
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::writeBytes, _win,
            [=](qsizetype offset, const QByteArray &data) -> bool {
                auto e = getCurrentPluginView(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd =
                        doc->MakeReplace(uc, editor->cursor(), offset, data);
                    if (uc == nullptr && cmd) {
                        _rwlock.lockForWrite();
                        doc->pushMakeUndo(cmd);
                        _rwlock.unlock();
                        return true;
                    }
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::insertInt8, _win,
            [=](qsizetype offset, qint8 value) -> bool {
                return insertBasicTypeContent(plg, offset, value);
            });

    connect(pctl, &WingPlugin::Controller::insertInt16, _win,
            [=](qsizetype offset, qint16 value) -> bool {
                return insertBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::insertInt32, _win,
            [=](qsizetype offset, qint32 value) -> bool {
                return insertBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::insertInt64, _win,
            [=](qsizetype offset, qint64 value) -> bool {
                return insertBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::insertFloat, _win,
            [=](qsizetype offset, float value) -> bool {
                return insertBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::insertDouble, _win,
            [=](qsizetype offset, double value) -> bool {
                return insertBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::insertString, _win,
            [=](qsizetype offset, const QString &value,
                const QString &encoding) -> bool {
                auto e = getCurrentPluginView(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();

                    auto unicode = Utilities::encodingString(value, encoding);
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd =
                        doc->MakeInsert(uc, editor->cursor(), offset, unicode);
                    if (uc == nullptr && cmd) {
                        _rwlock.lockForWrite();
                        doc->pushMakeUndo(cmd);
                        _rwlock.unlock();
                        return true;
                    }
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::writeInt8, _win,
            [=](qsizetype offset, qint8 value) -> bool {
                return writeBasicTypeContent(plg, offset, value);
            });

    connect(pctl, &WingPlugin::Controller::writeInt16, _win,
            [=](qsizetype offset, qint16 value) -> bool {
                return writeBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::writeInt32, _win,
            [=](qsizetype offset, qint32 value) -> bool {
                return writeBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::writeInt64, _win,
            [=](qsizetype offset, qint64 value) -> bool {
                return writeBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::writeFloat, _win,
            [=](qsizetype offset, float value) -> bool {
                return writeBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::writeDouble, _win,
            [=](qsizetype offset, double value) -> bool {
                return writeBasicTypeContent(plg, offset, value);
            });
    connect(pctl, &WingPlugin::Controller::writeString, _win,
            [=](qsizetype offset, const QString &value,
                const QString &encoding) -> bool {
                auto e = getCurrentPluginView(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();

                    auto unicode = Utilities::encodingString(value, encoding);
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd =
                        doc->MakeReplace(uc, editor->cursor(), offset, unicode);
                    if (uc == nullptr && cmd) {
                        _rwlock.lockForWrite();
                        doc->pushMakeUndo(cmd);
                        _rwlock.unlock();
                        return true;
                    }
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::appendInt8, _win,
            [=](qint8 value) -> bool {
                return appendBasicTypeContent(plg, value);
            });
    connect(pctl, &WingPlugin::Controller::appendInt16, _win,
            [=](qint16 value) -> bool {
                return appendBasicTypeContent(plg, value);
            });
    connect(pctl, &WingPlugin::Controller::appendInt32, _win,
            [=](qint32 value) -> bool {
                return appendBasicTypeContent(plg, value);
            });
    connect(pctl, &WingPlugin::Controller::appendInt64, _win,
            [=](qint64 value) -> bool {
                return appendBasicTypeContent(plg, value);
            });
    connect(pctl, &WingPlugin::Controller::appendFloat, _win,
            [=](qint64 value) -> bool {
                return appendBasicTypeContent(plg, value);
            });
    connect(pctl, &WingPlugin::Controller::appendDouble, _win,
            [=](qint64 value) -> bool {
                return appendBasicTypeContent(plg, value);
            });
    connect(pctl, &WingPlugin::Controller::appendString, _win,
            [=](const QString &value, const QString &encoding) -> bool {
                auto e = getCurrentPluginView(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto offset = doc->length();

                    auto unicode = Utilities::encodingString(value, encoding);
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd =
                        doc->MakeInsert(uc, editor->cursor(), offset, unicode);
                    if (uc == nullptr && cmd) {
                        _rwlock.lockForWrite();
                        doc->pushMakeUndo(cmd);
                        _rwlock.unlock();
                        return true;
                    }
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::remove, _win,
            [=](qsizetype offset, qsizetype len) -> bool {
                auto e = getCurrentPluginView(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();

                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd =
                        doc->MakeRemove(uc, editor->cursor(), offset, len);
                    if (uc == nullptr && cmd) {
                        _rwlock.lockForWrite();
                        doc->pushMakeUndo(cmd);
                        _rwlock.unlock();
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::removeAllBytes, _win, [=]() -> bool {
        auto e = getCurrentPluginView(plg);
        if (e) {
            auto editor = e->hexEditor();
            auto doc = editor->document();
            auto len = doc->length();

            auto uc = pluginCurrentUndoCmd(plg);
            auto cmd = doc->MakeRemove(uc, editor->cursor(), 0, len);
            if (uc == nullptr && cmd) {
                _rwlock.lockForWrite();
                doc->pushMakeUndo(cmd);
                _rwlock.unlock();
                return true;
            }
        }
        return false;
    });
    connect(pctl,
            QOverload<qsizetype, qsizetype, int, bool>::of(
                &WingPlugin::Controller::moveTo),
            _win,
            [=](qsizetype line, qsizetype column, int nibbleindex,
                bool clearSelection) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->cursor()->moveTo(line, column, nibbleindex,
                                                     clearSelection);
                    return true;
                }
                return false;
            });
    connect(pctl,
            QOverload<qsizetype, bool>::of(&WingPlugin::Controller::moveTo),
            _win, [=](qsizetype offset, bool clearSelection) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->cursor()->moveTo(offset, clearSelection);
                    return true;
                }
                return false;
            });
    connect(
        pctl, &WingPlugin::Controller::select, _win,
        [=](qsizetype offset, qsizetype length, SelectionMode mode) -> bool {
            if (!checkThreadAff()) {
                return false;
            }
            auto e = pluginCurrentEditor(plg);
            if (e) {
                auto cursor = e->hexEditor()->cursor();
                cursor->moveTo(offset);
                QHexCursor::SelectionMode smode;
                switch (mode) {
                case WingHex::SelectionMode::Add:
                    smode = QHexCursor::SelectionAdd;
                    break;
                case WingHex::SelectionMode::Remove:
                    smode = QHexCursor::SelectionRemove;
                    break;
                case WingHex::SelectionMode::Single:
                    smode = QHexCursor::SelectionNormal;
                    break;
                }
                cursor->select(length, smode);
                return true;
            }
            return false;
        });
    connect(pctl, &WingPlugin::Controller::setInsertionMode, _win,
            [=](bool isinsert) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->cursor()->setInsertionMode(
                        isinsert ? QHexCursor::InsertMode
                                 : QHexCursor::OverwriteMode);
                    return true;
                }
                return false;
            });
    connect(pctl,
            QOverload<qsizetype, qusizetype, const QColor &, const QColor &,
                      const QString &>::of(&WingPlugin::Controller::metadata),
            _win,
            [=](qsizetype begin, qusizetype length, const QColor &fgcolor,
                const QColor &bgcolor, const QString &comment) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();

                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd = doc->metadata()->MakeMetadata(
                        uc, begin, begin + length, fgcolor, bgcolor, comment);
                    if (uc == nullptr && cmd) {
                        doc->pushMakeUndo(cmd);
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::removeMetadata, _win,
            [=](qsizetype offset) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd = doc->metadata()->MakeRemoveMetadata(uc, offset);
                    if (uc == nullptr && cmd) {
                        doc->pushMakeUndo(cmd);
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::clearMetadata, _win, [=]() -> bool {
        if (!checkThreadAff()) {
            return false;
        }
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            auto uc = pluginCurrentUndoCmd(plg);
            auto cmd = doc->metadata()->MakeClear(uc);
            if (uc == nullptr && cmd) {
                doc->pushMakeUndo(cmd);
                return true;
            }
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::comment, _win,
            [=](qsizetype begin, qusizetype length,
                const QString &comment) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd = doc->metadata()->MakeComment(
                        uc, begin, begin + length, comment);
                    if (uc == nullptr && cmd) {
                        doc->pushMakeUndo(cmd);
                        return true;
                    }
                }
                return false;
            });
    connect(
        pctl, &WingPlugin::Controller::foreground, _win,
        [=](qsizetype begin, qusizetype length, const QColor &fgcolor) -> bool {
            if (!checkThreadAff()) {
                return false;
            }
            auto e = pluginCurrentEditor(plg);
            if (e) {
                auto doc = e->hexEditor()->document();
                auto uc = pluginCurrentUndoCmd(plg);
                auto cmd = doc->metadata()->MakeForeground(
                    uc, begin, begin + length, fgcolor);
                if (uc == nullptr && cmd) {
                    doc->pushMakeUndo(cmd);
                    return true;
                }
            }
            return false;
        });
    connect(
        pctl, &WingPlugin::Controller::background, _win,
        [=](qsizetype begin, qusizetype length, const QColor &bgcolor) -> bool {
            if (!checkThreadAff()) {
                return false;
            }
            auto e = pluginCurrentEditor(plg);
            if (e) {
                auto doc = e->hexEditor()->document();
                auto uc = pluginCurrentUndoCmd(plg);
                auto cmd = doc->metadata()->MakeBackground(
                    uc, begin, begin + length, bgcolor);
                if (uc == nullptr && cmd) {
                    doc->pushMakeUndo(cmd);
                    return true;
                }
            }
            return false;
        });
    connect(pctl, &WingPlugin::Controller::setMetaVisible, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetafgVisible(b);
                    doc->setMetabgVisible(b);
                    doc->setMetaCommentVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setMetafgVisible, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetafgVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setMetabgVisible, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetabgVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setMetaCommentVisible, _win,
            [=](bool b) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetaCommentVisible(b);
                    return true;
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::addBookMark, _win,
            [=](qsizetype pos, const QString &comment) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd = doc->MakeAddBookMark(uc, pos, comment);
                    if (uc == nullptr && cmd) {
                        doc->pushMakeUndo(cmd);
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::modBookMark, _win,
            [=](qsizetype pos, const QString &comment) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd = doc->MakeModBookMark(uc, pos, comment);
                    if (uc == nullptr && cmd) {
                        doc->pushMakeUndo(cmd);
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::removeBookMark, _win,
            [=](qsizetype pos) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto uc = pluginCurrentUndoCmd(plg);
                    auto cmd = doc->MakeRemoveBookMark(uc, pos);
                    if (uc == nullptr && cmd) {
                        doc->pushMakeUndo(cmd);
                        return true;
                    }
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::clearBookMark, _win, [=]() -> bool {
        if (!checkThreadAff()) {
            return false;
        }
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            auto uc = pluginCurrentUndoCmd(plg);
            auto cmd = doc->MakeClearBookMark(uc);
            if (uc == nullptr && cmd) {
                doc->pushMakeUndo(cmd);
                return true;
            }
        }
        return false;
    });

    // mainwindow
    connect(pctl, &WingPlugin::Controller::newFile, _win, [=]() -> ErrFile {
        if (!checkThreadAff()) {
            return ErrFile::NotAllowedInNoneGUIThread;
        }
        if (!checkPluginCanOpenedFile(plg)) {
            return ErrFile::TooManyOpenedFile;
        }
        auto view = _win->newfileGUI();
        if (view) {
            auto id = assginHandleForPluginView(plg, view);
            m_plgCurrentfid[plg] = id;
            auto handle = getUIDHandle(id);

            PluginSystem::instance().dispatchEvent(
                IWingPlugin::RegisteredEvent::PluginFileOpened,
                {quintptr(plg),
                 {},
                 handle,
                 QVariant::fromValue(_win->getEditorViewFileType(view))});

            return ErrFile(handle);
        } else {
            return ErrFile::Error;
        }
    });
    connect(
        pctl, &WingPlugin::Controller::openWorkSpace, _win,
        [=](const QString &filename) -> ErrFile {
            if (!checkThreadAff()) {
                return ErrFile::NotAllowedInNoneGUIThread;
            }
            EditorView *view = nullptr;
            if (!checkPluginCanOpenedFile(plg)) {
                return ErrFile::TooManyOpenedFile;
            }
            auto ret = _win->openWorkSpace(filename, &view);
            if (view) {
                if (ret == ErrFile::AlreadyOpened &&
                    checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::AlreadyOpened;
                }
                auto id = assginHandleForPluginView(plg, view);
                m_plgCurrentfid[plg] = id;
                auto handle = getUIDHandle(id);

                PluginSystem::instance().dispatchEvent(
                    IWingPlugin::RegisteredEvent::PluginFileOpened,
                    {quintptr(plg), filename, handle,
                     QVariant::fromValue(_win->getEditorViewFileType(view))});

                return ErrFile(handle);
            } else {
                return ret;
            }
        });
    connect(
        pctl, &WingPlugin::Controller::openFile, _win,
        [=](const QString &filename) -> ErrFile {
            if (!checkThreadAff()) {
                return ErrFile::NotAllowedInNoneGUIThread;
            }
            EditorView *view = nullptr;
            if (!checkPluginCanOpenedFile(plg)) {
                return ErrFile::TooManyOpenedFile;
            }
            auto ret = _win->openFile(filename, &view);
            if (view) {
                if (ret == ErrFile::AlreadyOpened &&
                    checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::AlreadyOpened;
                }
                auto id = assginHandleForPluginView(plg, view);
                m_plgCurrentfid[plg] = id;
                auto handle = getUIDHandle(id);
                PluginSystem::instance().dispatchEvent(
                    IWingPlugin::RegisteredEvent::PluginFileOpened,
                    {quintptr(plg), filename, handle,
                     QVariant::fromValue(_win->getEditorViewFileType(view))});

                return ErrFile(handle);
            } else {
                return ret;
            }
        });
    connect(
        pctl, &WingPlugin::Controller::openExtFile, _win,
        [=](const QString &ext, const QString &file) {
            if (!checkThreadAff()) {
                return ErrFile::NotAllowedInNoneGUIThread;
            }
            EditorView *view = nullptr;
            if (!checkPluginCanOpenedFile(plg)) {
                return ErrFile::TooManyOpenedFile;
            }
            auto ret = _win->openExtFile(ext, file, &view);
            if (view) {
                if (ret == ErrFile::AlreadyOpened &&
                    checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::AlreadyOpened;
                }
                auto id = assginHandleForPluginView(plg, view);
                m_plgCurrentfid[plg] = id;

                auto handle = getUIDHandle(id);
                PluginSystem::instance().dispatchEvent(
                    IWingPlugin::RegisteredEvent::PluginFileOpened,
                    {quintptr(plg), file, handle,
                     QVariant::fromValue(_win->getEditorViewFileType(view))});

                return ErrFile(handle);
            } else {
                return ret;
            }
        });
    connect(pctl, &WingPlugin::Controller::closeHandle, _win,
            [=](int handle) -> WingHex::ErrFile {
                if (closeHandle(plg, handle)) {
                    return WingHex::ErrFile::Success;
                }
                return WingHex::ErrFile::NotExist;
            });
    connect(pctl, &WingPlugin::Controller::closeFile, _win,
            [=](int handle, bool force) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                if (closeEditor(plg, handle, force)) {
                    return ErrFile::Success;
                }
                return ErrFile::NotExist;
            });
    connect(pctl, &WingPlugin::Controller::saveFile, _win,
            [=](int handle) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                auto view = handle2EditorView(plg, handle);
                if (!checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::Error;
                }

                if (view) {
                    _win->saveEditor(view, {});
                    return ErrFile::Success;
                }
                return ErrFile::NotExist;
            });
    connect(pctl, &WingPlugin::Controller::exportFile, _win,
            [=](int handle, const QString &savename) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }

                auto view = handle2EditorView(plg, handle);
                if (!checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::Error;
                }

                if (view) {
                    _win->saveEditor(view, savename, true);
                    return ErrFile::Success;
                }
                return ErrFile::NotExist;
            });

    connect(pctl, &WingPlugin::Controller::saveAsFile, _win,
            [=](int handle, const QString &savename) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                auto view = handle2EditorView(plg, handle);
                if (!checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::Error;
                }

                if (view) {
                    _win->saveEditor(view, savename);
                    return ErrFile::Success;
                }
                return ErrFile::NotExist;
            });
    connect(pctl, &WingPlugin::Controller::openCurrent, _win, [=]() -> ErrFile {
        if (!checkThreadAff()) {
            return ErrFile::NotAllowedInNoneGUIThread;
        }

        auto view = getCurrentPluginView(plg);
        if (view) {
            if (checkPluginHasAlreadyOpened(plg, view)) {
                return ErrFile::AlreadyOpened;
            }

            auto id = assginHandleForPluginView(plg, view);
            m_plgCurrentfid[plg] = id;

            return ErrFile(int(*id));
        }
        return ErrFile::Error;
    });
    connect(pctl, &WingPlugin::Controller::closeCurrent, _win,
            [=](bool force) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                auto view = getCurrentPluginView(plg);
                if (!checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::Error;
                }

                if (view == nullptr) {
                    return ErrFile::NotExist;
                }

                if (!checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::Error;
                }

                return _win->closeEditor(view, force);
            });
    connect(pctl, &WingPlugin::Controller::saveCurrent, _win, [=]() -> ErrFile {
        if (!checkThreadAff()) {
            return ErrFile::NotAllowedInNoneGUIThread;
        }
        auto view = getCurrentPluginView(plg);
        if (!checkPluginHasAlreadyOpened(plg, view)) {
            return ErrFile::Error;
        }

        if (view) {
            auto ws = _win->m_views.value(view);
            return view->save(ws, {}, false,
                              EditorView::SaveWorkSpaceAttr::AutoWorkSpace);
        }
        return ErrFile::Error;
    });
    connect(pctl, &WingPlugin::Controller::saveAsCurrent, _win,
            [=](const QString &savename) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                auto view = getCurrentPluginView(plg);
                if (!checkPluginHasAlreadyOpened(plg, view)) {
                    return ErrFile::Error;
                }

                if (view) {
                    _win->saveEditor(view, savename, true);
                    return ErrFile::Success;
                }
                return ErrFile::Error;
            });

    connect(pctl, &WingPlugin::Controller::closeAll, _win, [=]() -> bool {
        if (!checkThreadAff()) {
            return false;
        }
        auto &maps = m_plgviewMap[plg];
        for (auto &item : maps) {
            closeEditor(plg, getUIDHandle(item.fid), true);
        }
        m_plgCurrentfid[plg] = -1;
        maps.clear();
        return true;
    });
}

void PluginSystem::connectUIInterface(IWingPlugin *plg) {
    connectUIInterface(static_cast<IWingPluginBase *>(plg));

    auto visual = &plg->visual;
    connect(visual, &WingPlugin::DataVisual::updateText, _win,
            [=](const QString &txt, const QString &title) -> bool {
                _win->m_infotxt->setProperty("__TITLE__", title);
                _win->m_infotxt->setText(txt);
                return true;
            });
    connect(
        visual, &WingPlugin::DataVisual::updateTextList, _win,
        [=](const QStringList &data, const QString &title,
            WingHex::WingPlugin::DataVisual::ClickedCallBack clicked,
            WingHex::WingPlugin::DataVisual::DoubleClickedCallBack dblClicked)
            -> bool {
            return updateTextList_API(data, title, clicked, dblClicked);
        });
    connect(
        visual, &WingPlugin::DataVisual::updateTextListByModel, _win,
        [=](QAbstractItemModel *model, const QString &title,
            WingHex::WingPlugin::DataVisual::ClickedCallBack clicked,
            WingHex::WingPlugin::DataVisual::DoubleClickedCallBack dblClicked)
            -> bool {
            if (model) {
                auto oldmodel = _win->m_infolist->model();
                if (oldmodel) {
                    oldmodel->deleteLater();
                }
                _win->m_infolist->setProperty("__TITLE__", title);
                _win->m_infolist->setModel(model);
                _win->m_infoclickfn = clicked;
                _win->m_infodblclickfn = dblClicked;
                return true;
            }
            return false;
        });
    connect(
        visual, &WingPlugin::DataVisual::updateTextTree, _win,
        [=](const QString &json, const QString &title,
            WingHex::WingPlugin::DataVisual::ClickedCallBack clicked,
            WingHex::WingPlugin::DataVisual::DoubleClickedCallBack dblClicked)
            -> bool {
            return updateTextTree_API(json, title, clicked, dblClicked);
        });
    connect(
        visual, &WingPlugin::DataVisual::updateTextTreeByModel, _win,
        [=](QAbstractItemModel *model, const QString &title,
            WingHex::WingPlugin::DataVisual::ClickedCallBack clicked,
            WingHex::WingPlugin::DataVisual::DoubleClickedCallBack dblClicked)
            -> bool {
            if (model) {
                auto oldmodel = _win->m_infotree->model();
                if (oldmodel) {
                    oldmodel->deleteLater();
                }
                _win->m_infotree->setProperty("__TITLE__", title);
                _win->m_infotree->setModel(model);
                _win->m_infotreeclickfn = clicked;
                _win->m_infotreedblclickfn = dblClicked;
                return true;
            }
            return false;
        });
    connect(
        visual, &WingPlugin::DataVisual::updateTextTable, _win,
        [=](const QString &json, const QStringList &headers,
            const QStringList &headerNames, const QString &title,
            WingHex::WingPlugin::DataVisual::ClickedCallBack clicked,
            WingHex::WingPlugin::DataVisual::DoubleClickedCallBack dblClicked)
            -> bool {
            return updateTextTable_API(json, headers, headerNames, title,
                                       clicked, dblClicked);
        });
    connect(
        visual, &WingPlugin::DataVisual::updateTextTableByModel, _win,
        [=](QAbstractItemModel *model, const QString &title,
            WingHex::WingPlugin::DataVisual::ClickedCallBack clicked,
            WingHex::WingPlugin::DataVisual::DoubleClickedCallBack dblClicked)
            -> bool {
            if (model) {
                auto oldmodel = _win->m_infotable->model();
                if (oldmodel) {
                    oldmodel->deleteLater();
                }
                _win->m_infotable->setProperty("__TITLE__", title);
                _win->m_infotable->setModel(model);
                _win->m_infotableclickfn = clicked;
                _win->m_infotabledblclickfn = dblClicked;
                return true;
            }
            return false;
        });
}

void PluginSystem::connectInterface(IWingDevice *plg) {
    Q_UNUSED(plg);
    // do nothing
}

void PluginSystem::connectLoadingInterface(IWingDevice *plg) {
    connectBaseInterface(plg);
    connectUIInterface(plg);
}

bool PluginSystem::checkThreadAff() {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(tr("Not allowed operation in non-UI thread"));
        return false;
    }
    return true;
}

QString PluginSystem::packLogMessage(const char *header, const QString &msg) {
    return QStringLiteral("{") + header + QStringLiteral("} ") + msg;
}

PluginSystem &PluginSystem::instance() {
    static PluginSystem ins;
    return ins;
}

void PluginSystem::setMainWindow(MainWindow *win) { _win = win; }

QWidget *PluginSystem::mainWindow() const { return _win; }

void PluginSystem::loadAllPlugin() {
    Q_ASSERT(_win);

    auto &set = SettingManager::instance();
    if (set.scriptEnabled()) {
        _angelplg = new WingAngelAPI;

        QFile angeljson(QStringLiteral(
            ":/com.wingsummer.winghex/src/class/WingAngelAPI.json"));
        auto ret = angeljson.open(QFile::ReadOnly);
        Q_ASSERT(ret);
        Q_UNUSED(ret);
        auto angelapi = angeljson.readAll();
        angeljson.close();

        QJsonDocument doc = QJsonDocument::fromJson(angelapi);
        auto meta = parsePluginMetadata(doc.object());
        retranslateMetadata(_angelplg, meta);
        loadPlugin(_angelplg, meta, std::nullopt);
    }

    Logger::newLine();

    // ok, setup checking engine
    initCheckingEngine();

    {
        auto cstructplg = new WingCStruct;
        QFile cstructjson(QStringLiteral(
            ":/com.wingsummer.winghex/src/class/WingCStruct.json"));
        auto ret = cstructjson.open(QFile::ReadOnly);
        Q_ASSERT(ret);
        Q_UNUSED(ret);
        auto cstruct = cstructjson.readAll();
        cstructjson.close();

        QJsonDocument doc = QJsonDocument::fromJson(cstruct);
        auto meta = parsePluginMetadata(doc.object());

        QDir setd(Utilities::getAppDataPath());
        auto plgset = QStringLiteral("plgset");
        setd.mkdir(plgset);
        retranslateMetadata(cstructplg, meta);
        loadPlugin(cstructplg, meta, setd);
    }

    bool ok;

    auto disAll =
        qEnvironmentVariableIntValue("WING_DISABLE_PLUGIN_SYSTEM", &ok);
    if (!ok || (ok && disAll == 0)) {
        bool enabledrv = true, enableplg = true;
        auto disdrv = qEnvironmentVariableIntValue("WING_DISABLE_EXTDRV", &ok);
        if (ok && disdrv != 0) {
            enabledrv = false;
        }

        if (enabledrv) {
            loadDevicePlugin();
        }

        auto displg = qEnvironmentVariableIntValue("WING_DISABLE_PLUGIN", &ok);
        if ((ok && displg != 0) || !set.enablePlugin()) {
            enableplg = false;
        }

        if (Utilities::isRoot()) {
            if (!set.enablePlgInRoot()) {
                enableplg = false;
            }
        }

        if (enableplg) {
            loadExtPlugin();
        }
    }

    // loading finished, delete the checking engine
    finalizeCheckingEngine();
}

void PluginSystem::destory() {
    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("plgset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        qApp->exit(int(CrashCode::PluginSetting));
    }

    for (auto &item : _loadedplgs) {
        auto set =
            std::make_unique<QSettings>(udir.absoluteFilePath(_pinfos[item].id),
                                        QSettings::Format::IniFormat);
        item->unload(set);
        delete item;
    }
    _loadedplgs.clear();

    for (auto &item : _loadeddevs) {
        auto set =
            std::make_unique<QSettings>(udir.absoluteFilePath(_pinfos[item].id),
                                        QSettings::Format::IniFormat);
        item->unload(set);
        delete item;
    }
    _loadeddevs.clear();
}

EditorView *PluginSystem::pluginCurrentEditor(IWingPlugin *plg) const {
    if (plg) {
        auto fid = m_plgCurrentfid[plg];
        if (fid < 0) {
            return _win->currentEditor();
        }

        auto ctx = pluginContextById(plg, fid);
        if (ctx) {
            return ctx->view;
        }
    }
    return nullptr;
}

std::optional<PluginSystem::PluginFileContext>
PluginSystem::pluginContextById(IWingPlugin *plg, int fid) const {
    if (fid < 0) {
        return std::nullopt;
    }
    auto &handles = m_plgviewMap[plg];
    auto r = std::find_if(handles.begin(), handles.end(),
                          [fid](const PluginFileContext &d) {
                              return equalCompareHandle(d.fid, fid);
                          });
    if (r == handles.end()) {
        return std::nullopt;
    }
    return *r;
}

std::optional<QVector<PluginSystem::PluginFileContext>::iterator>
PluginSystem::pluginContextByIdIt(IWingPlugin *plg, int fid) {
    auto &handles = m_plgviewMap[plg];
    if (fid < 0) {
        return std::nullopt;
    }
    auto r = std::find_if(handles.begin(), handles.end(),
                          [fid](const PluginFileContext &d) {
                              return equalCompareHandle(d.fid, fid);
                          });
    if (r == handles.end()) {
        return std::nullopt;
    }
    return r;
}

QUndoCommand *PluginSystem::pluginCurrentUndoCmd(IWingPlugin *plg) const {
    if (plg) {
        auto fid = m_plgCurrentfid[plg];
        if (fid < 0) {
            return nullptr;
        }

        auto ctx = pluginContextById(plg, fid);
        if (ctx) {
            return ctx->cmd;
        }
    }
    return nullptr;
}

void PluginSystem::connectBaseInterface(IWingPluginBase *plg) {
    Q_ASSERT(plg);

    connect(plg, &IWingPluginBase::toast, this,
            [=](const QPixmap &icon, const QString &message) {
                if (!checkThreadAff()) {
                    return;
                }
                if (message.isEmpty()) {
                    return;
                }
                Toast::toast(_win, icon, message);
            });
    connect(plg, &IWingPluginBase::trace, this, [=](const QString &message) {
        Logger::trace(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPluginBase::debug, this, [=](const QString &message) {
        Logger::debug(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPluginBase::info, this, [=](const QString &message) {
        Logger::info(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPluginBase::warn, this, [=](const QString &message) {
        Logger::warning(
            packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPluginBase::error, this, [=](const QString &message) {
        Logger::critical(
            packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPluginBase::currentAppTheme, this,
            []() -> WingHex::AppTheme {
                auto theme = SkinManager::instance().currentTheme();
                switch (theme) {
                case SkinManager::Theme::Dark:
                    return WingHex::AppTheme::Dark;
                case SkinManager::Theme::Light:
                    return WingHex::AppTheme::Light;
                }
                return WingHex::AppTheme::Dark; // fallback to default theme
            });
    connect(plg, &IWingPluginBase::createDialog, this,
            [=](QWidget *w) -> QDialog * {
                if (!checkThreadAff()) {
                    return nullptr;
                }
                if (w) {
                    auto d = new FramelessDialogBase;
                    d->buildUpContent(w);
                    d->setWindowTitle(w->windowTitle());
                    d->setWindowIcon(w->windowIcon());
                    return d;
                } else {
                    return nullptr;
                }
            });
    connect(plg, &IWingPluginBase::raiseDockWidget, this,
            [=](QWidget *w) -> bool {
                if (_raisedw.contains(w)) {
                    _raisedw.value(w)->raise();
                    return true;
                }
                return false;
            });
}

void PluginSystem::connectUIInterface(IWingPluginBase *plg) {
    Q_ASSERT(plg);

    auto msgbox = &plg->msgbox;
    connect(msgbox, &WingPlugin::MessageBox::aboutQt, _win,
            [this](QWidget *parent, const QString &title) -> void {
                if (checkThreadAff()) {
                    WingMessageBox::aboutQt(parent, title);
                }
            });
    connect(msgbox, &WingPlugin::MessageBox::information, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::information(parent, title, text,
                                                       buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::question, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::question(parent, title, text,
                                                    buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::warning, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::warning(parent, title, text, buttons,
                                                   defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::critical, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::critical(parent, title, text,
                                                    buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::about, _win,
            [=](QWidget *parent, const QString &title,
                const QString &text) -> void {
                if (checkThreadAff()) {
                    WingMessageBox::about(parent, title, text);
                }
            });
    connect(msgbox, &WingPlugin::MessageBox::msgbox, _win,
            [=](QWidget *parent, QMessageBox::Icon icon, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::msgbox(parent, icon, title, text,
                                                  buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });

    auto inputbox = &plg->inputbox;
    connect(inputbox, &WingPlugin::InputBox::getText, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                QLineEdit::EchoMode echo, const QString &text, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getText(parent, title, label, echo,
                                                    text, ok, inputMethodHints);
                }
                return {};
            });
    connect(inputbox, &WingPlugin::InputBox::getMultiLineText, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                const QString &text, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getMultiLineText(
                        parent, title, label, text, ok, inputMethodHints);
                }
                return {};
            });
    connect(inputbox, &WingPlugin::InputBox::getItem, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                const QStringList &items, int current, bool editable, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getItem(parent, title, label, items,
                                                    current, editable, ok,
                                                    inputMethodHints);
                }
                return {};
            });
    connect(
        inputbox, &WingPlugin::InputBox::getInt, _win,
        [=](QWidget *parent, const QString &title, const QString &label,
            int value, int minValue, int maxValue, int step, bool *ok) -> int {
            if (checkThreadAff()) {
                return WingInputDialog::getInt(parent, title, label, value,
                                               minValue, maxValue, step, ok);
            }
            return 0;
        });
    connect(inputbox, &WingPlugin::InputBox::getDouble, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                double value, double minValue, double maxValue, int decimals,
                bool *ok, double step) -> double {
                if (checkThreadAff()) {
                    return WingInputDialog::getDouble(parent, title, label,
                                                      value, minValue, maxValue,
                                                      decimals, ok, step);
                }
                return qQNaN();
            });

    auto filedlg = &plg->filedlg;
    connect(filedlg, &WingPlugin::FileDialog::getExistingDirectory, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                QFileDialog::Options options) -> QString {
                if (checkThreadAff()) {
                    return WingFileDialog::getExistingDirectory(parent, caption,
                                                                dir, options);
                }
                return {};
            });
    connect(filedlg, &WingPlugin::FileDialog::getOpenFileName, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                const QString &filter, QString *selectedFilter,
                QFileDialog::Options options) -> QString {
                if (checkThreadAff()) {
                    return WingFileDialog::getOpenFileName(
                        parent, caption, dir, filter, selectedFilter, options);
                }
                return {};
            });
    connect(filedlg, &WingPlugin::FileDialog::getOpenFileNames, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                const QString &filter, QString *selectedFilter,
                QFileDialog::Options options) -> QStringList {
                if (checkThreadAff()) {
                    return WingFileDialog::getOpenFileNames(
                        parent, caption, dir, filter, selectedFilter, options);
                }
                return {};
            });
    connect(filedlg, &WingPlugin::FileDialog::getSaveFileName, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                const QString &filter, QString *selectedFilter,
                QFileDialog::Options options) -> QString {
                if (checkThreadAff()) {
                    return WingFileDialog::getSaveFileName(
                        parent, caption, dir, filter, selectedFilter, options);
                }
                return {};
            });

    auto colordlg = &plg->colordlg;
    connect(colordlg, &WingPlugin::ColorDialog::getColor, _win,
            [=](const QString &caption, QWidget *parent) -> QColor {
                if (checkThreadAff()) {
                    ColorPickerDialog d(parent);
                    d.setWindowTitle(caption);
                    if (d.exec()) {
                        return d.color();
                    }
                }
                return {};
            });
}
