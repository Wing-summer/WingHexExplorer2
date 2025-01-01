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
#include "QJsonModel/include/QJsonModel.hpp"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "class/languagemanager.h"
#include "class/logger.h"
#include "class/settingmanager.h"
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
    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("plgset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        qApp->exit(int(CrashCode::PluginSetting));
    }

    for (auto &item : _loadedplgs) {
        auto set = std::make_unique<QSettings>(
            udir.absoluteFilePath(item->metaObject()->className()),
            QSettings::Format::IniFormat);
        item->unload(set);
        delete item;
    }
}

const QList<IWingPlugin *> &PluginSystem::plugins() const {
    return _loadedplgs;
}

const IWingPlugin *PluginSystem::plugin(qsizetype index) const {
    return _loadedplgs.at(index);
}

void PluginSystem::loadPlugin(const QFileInfo &fileinfo, const QDir &setdir) {
    Q_ASSERT(_win);

    if (fileinfo.exists()) {
        auto fileName = fileinfo.absoluteFilePath();
        QPluginLoader loader(fileName, this);
        Logger::info(tr("LoadingPlugin") + fileinfo.fileName());
        auto p = qobject_cast<IWingPlugin *>(loader.instance());
        if (Q_UNLIKELY(p == nullptr)) {
            Logger::critical(loader.errorString());
        } else {
            loadPlugin(p, fileName, setdir);
        }
        Logger::_log("");
    }
}

WingAngelAPI *PluginSystem::angelApi() const { return _angelplg; }

EditorView *PluginSystem::getCurrentPluginView(IWingPlugin *plg) {
    if (plg == nullptr) {
        return nullptr;
    }
    if (!m_plgviewMap.contains(plg)) {
        return nullptr;
    }
    auto view = m_plgviewMap.value(plg);
    if (view == nullptr) {
        view = _win->m_curEditor;
    }
    return view;
}

EditorView *PluginSystem::handle2EditorView(IWingPlugin *plg, int handle) {
    if (handle < 0) {
        return getCurrentPluginView(plg);
    }

    auto handles = m_plgHandles.value(plg);
    auto r = std::find_if(handles.begin(), handles.end(),
                          [handle](const QPair<int, EditorView *> &d) {
                              return d.first == handle;
                          });
    if (r != handles.end()) {
        return r->second;
    }
    return nullptr;
}

PluginSystem::SharedUniqueId
PluginSystem::assginHandleForPluginView(IWingPlugin *plg, EditorView *view) {
    if (plg == nullptr || view == nullptr) {
        return {};
    }

    auto id = m_idGen.get();
    m_plgHandles[plg].append(qMakePair(id, view));
    m_viewBindings[view].append(plg);
    return id;
}

bool PluginSystem::checkPluginCanOpenedFile(IWingPlugin *plg) {
    if (plg == nullptr) {
        return false;
    }
    return m_plgHandles[plg].size() <= UCHAR_MAX;
}

bool PluginSystem::checkPluginHasAlreadyOpened(IWingPlugin *plg,
                                               EditorView *view) {
    auto &maps = m_plgHandles[plg];
    auto ret = std::find_if(
        maps.begin(), maps.end(),
        [view](const QPair<SharedUniqueId, EditorView *> &content) {
            return content.second == view;
        });
    return ret != maps.end();
}

void PluginSystem::cleanUpEditorViewHandle(EditorView *view) {
    if (m_viewBindings.contains(view)) {
        auto v = m_viewBindings.value(view);

        // clean up
        for (auto &plg : v) {
            auto handles = m_plgHandles.value(plg);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            handles.erase(
                std::remove_if(handles.begin(), handles.end(),
                               [view](const QPair<int, EditorView *> &v) {
                                   return v.second == view;
                               }));

#else
            handles.removeIf([view](const QPair<int, EditorView *> &v) {
                return v.second == view;
            });
#endif
        }
        m_viewBindings.remove(view);
    }
}

bool PluginSystem::closeEditor(IWingPlugin *plg, int handle, bool force) {
    if (handle >= 0) {
        auto &handles = m_plgHandles[plg];
        auto r = std::find_if(
            handles.begin(), handles.end(),
            [handle](const QPair<SharedUniqueId, EditorView *> &d) {
                return (*d.first) == handle;
            });
        if (r == handles.end()) {
            return false;
        }

        auto &bind = m_viewBindings[r->second];
        bind.removeOne(plg);
        if (bind.isEmpty()) {
            _win->closeEditor(r->second, force);
        }
    } else {
        if (_win->m_curEditor == nullptr) {
            return false;
        }

        auto &bind = m_viewBindings[_win->m_curEditor];
        bind.removeOne(plg);
        if (bind.isEmpty()) {
            _win->closeEditor(_win->m_curEditor, force);
        }
    }

    m_plgviewMap.remove(plg);
    m_plgHandles.remove(plg);

    return true;
}

void PluginSystem::dispatchEvent(IWingPlugin::RegisteredEvent event,
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
        Q_ASSERT(params.size() == 1);
        auto fileName = params.first().toString();
        Q_ASSERT(!fileName.isEmpty());
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Opened, fileName,
                                 {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileSaved: {
        Q_ASSERT(params.size() == 2);
        auto newFileName = params.at(0).toString();
        auto oldFileName = params.at(1).toString();
        Q_ASSERT(!newFileName.isEmpty() && !oldFileName.isEmpty());
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Saved,
                                 oldFileName, newFileName);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileSwitched: {
        Q_ASSERT(params.size() == 2);
        auto newFileName = params.at(0).toString();
        auto oldFileName = params.at(1).toString();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Switched,
                                 oldFileName, newFileName);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::AppReady: {
        Q_ASSERT(params.isEmpty());
        for (auto &plg : _evplgs[event]) {
            plg->eventReady();
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileClosed: {
        Q_ASSERT(params.size() == 1);
        auto fileName = params.first().toString();
        Q_ASSERT(!fileName.isEmpty());
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Closed, fileName,
                                 {});
        }
    } break;
    default:
        break;
    }
}

void PluginSystem::registerFns(IWingPlugin *plg) {
    Q_ASSERT(plg);
    auto fns = plg->registeredScriptFns();
    if (fns.isEmpty()) {
        return;
    }

    // <signatures, std::function>
    QHash<QString, IWingPlugin::ScriptFnInfo> rfns;
    for (auto p = fns.constKeyValueBegin(); p != fns.constKeyValueEnd(); ++p) {
        auto &fn = p->second;
        auto sig = getScriptFnSig(p->first, fn);
        if (rfns.find(sig) != rfns.end()) {
            Logger::warning(tr("RegisteredFnDup"));
            continue;
        }
        rfns.insert(sig, p->second);
    }

    Q_ASSERT(_angelplg);
    _angelplg->registerScriptFns(plg->metaObject()->className(), rfns);
}

void PluginSystem::registerEnums(IWingPlugin *plg) {
    Q_ASSERT(plg);
    auto objs = plg->registeredScriptEnums();
    if (objs.isEmpty()) {
        return;
    }

    Q_ASSERT(_angelplg);
    _angelplg->registerScriptEnums(plg->metaObject()->className(), objs);
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

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileClosed)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileClosed].append(plg);
    }
}

QString PluginSystem::type2AngelScriptString(IWingPlugin::MetaType type,
                                             bool isArg) {
    bool isArray = type & WingHex::IWingPlugin::Array;
    bool isMap = type & WingHex::IWingPlugin::Map;
    bool isRef = type & WingHex::IWingPlugin::Ref;

    QString retype;

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
    case WingHex::IWingPlugin::Color:
        retype = QStringLiteral("color");
        break;
    default:
        return {};
    }

    if (isMap && isArray) {
        return {};
    }

    if (isArray) {
        retype = QStringLiteral("array<") + retype + QStringLiteral(">");
    }

    if (isMap) {
        retype = QStringLiteral("dictionary<") + retype + QStringLiteral(">");
    }

    if (isRef) {
        if (isArg) {
            retype += QStringLiteral(" @out");
        } else {
            retype += QStringLiteral("@");
        }
    }

    return retype;
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
        params << type2AngelScriptString(param.first, true) +
                      QStringLiteral(" ") + param.second;
    }

    return sig + params.join(',') + QStringLiteral(")");
}

QString PluginSystem::getPUID(IWingPlugin *p) {
    if (p) {
        constexpr auto puid_limit = 36; // same as uuid length, so enough
        auto prop = p->property("puid").toString().trimmed();
        if (prop.length() > puid_limit) {
            return {};
        }
        auto pid = QString(p->metaObject()->className());
        if (pid.length() > puid_limit) {
            return {};
        }
        return prop.isEmpty() ? pid : prop;
    } else {
        return {};
    }
}

bool PluginSystem::isPluginLoaded(const WingDependency &d) {
    for (auto &info : _loadedplginfo) {
        if (info.version >= d.version && info.puid == d.puid) {
            if (d.md5.isEmpty()) {
                return true;
            } else {
                return d.md5.compare(info.md5, Qt::CaseInsensitive) == 0;
            }
        }
    }
    return false;
}

void PluginSystem::loadPlugin(IWingPlugin *p, const QString &fileName,
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

        auto puid = getPUID(p);
        if (puid.isEmpty()) {
            throw tr("ErrLoadInvalidPUID");
        }

        for (auto &uid : _loadedplginfo) {
            if (uid.puid == puid) {
                throw tr("ErrLoadLoadedPlugin");
            }
        }

        // check dependencise
        auto dps = p->dependencies();
        if (!dps.isEmpty()) {
            for (auto &d : dps) {
                if (!isPluginLoaded(d)) {
                    _lazyplgs.append(qMakePair(p, fileName));
                    return;
                }
            }
        }

        emit pluginLoading(p->pluginName());

        auto pid = QString(p->metaObject()->className());
        p_tr = LanguageManager::instance().try2LoadPluginLang(pid);

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
                throw tr("ErrLoadInitPlugin");
            }
        }

        _loadedplgs.append(p);

        WingDependency de;
        de.puid = puid;
        de.version = p->pluginVersion();
        de.md5 = Utilities::getMd5(fileName);

        _loadedplginfo.append(de);

        Logger::warning(tr("PluginName :") + p->pluginName());
        Logger::warning(tr("PluginAuthor :") + p->pluginAuthor());
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
                    Logger::warning(tr("InvalidNullDockWidget:") %
                                    QStringLiteral("{ ") % p->pluginName() %
                                    QStringLiteral(" } ") % widgetName %
                                    QStringLiteral(" ( ") % displayName %
                                    QStringLiteral(" )"));
                    continue;
                }

                auto dw = _win->buildDockWidget(_win->m_dock, widgetName,
                                                displayName, info.widget,
                                                MainWindow::PLUGIN_VIEWS);
                switch (info.area) {
                case Qt::LeftDockWidgetArea: {
                    if (_win->m_leftViewArea == nullptr) {
                        _win->m_leftViewArea = _win->m_dock->addDockWidget(
                            ads::LeftDockWidgetArea, dw);
                    } else {
                        _win->m_leftViewArea = _win->m_dock->addDockWidget(
                            ads::CenterDockWidgetArea, dw,
                            _win->m_leftViewArea);
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

        {
            auto rp = p->registeredPages();
            if (!rp.isEmpty()) {
                for (auto &page : rp) {
                    page->setProperty("__plg__", QVariant::fromValue(p));
                }
                _win->m_plgPages.append(rp);
            }
        }

        registerFns(p);
        registerEnums(p);
        registerEvents(p);
        connectInterface(p);

        m_plgviewMap.insert(p, nullptr);
    } catch (const QString &error) {
        Logger::critical(error);
        if (p_tr) {
            p_tr->deleteLater();
        }
    }
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
    connect(plg, &IWingPlugin::toast, this,
            [=](const QPixmap &icon, const QString &message) {
                if (!checkThreadAff()) {
                    return;
                }
                if (message.isEmpty()) {
                    return;
                }
                Toast::toast(_win, icon, message);
            });
    connect(plg, &IWingPlugin::trace, this, [=](const QString &message) {
        Logger::trace(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::debug, this, [=](const QString &message) {
        Logger::debug(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::info, this, [=](const QString &message) {
        Logger::info(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::warn, this, [=](const QString &message) {
        Logger::warning(
            packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::error, this, [=](const QString &message) {
        Logger::critical(
            packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::createDialog, this,
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
                    auto render = hexeditor->renderer();
                    auto pos = doc->searchForward(offset, QByteArray(1, 0));
                    if (pos < 0) {
                        pos = doc->searchForward(offset, QByteArray(1, '\n'));
                        if (pos < 0) {
                            return QString();
                        }
                    }
                    auto buffer = doc->read(offset, int(pos - offset));

                    QString enco = encoding;
                    if (!enco.length()) {
                        enco = render->encoding();
                    }

                    _rwlock.unlock();
                    return Utilities::decodingString(buffer, enco);
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
    connect(preader, &WingPlugin::Reader::currentEncoding, _win,
            [=]() -> QString {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->renderer()->encoding();
                }
                return {};
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
                    m_plgviewMap[plg] = nullptr;
                } else {
                    auto handles = m_plgHandles.value(plg);
                    auto ret = std::find_if(
                        handles.begin(), handles.end(),
                        [handle](const QPair<int, EditorView *> &v) {
                            return v.first == handle;
                        });
                    if (ret == handles.end()) {
                        return false;
                    }
                    m_plgviewMap[plg] = ret->second;
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
                    } else {
                        return false;
                    }
                } else {
                    auto handles = m_plgHandles.value(plg);
                    auto ret = std::find_if(
                        handles.begin(), handles.end(),
                        [handle](const QPair<int, EditorView *> &v) {
                            return v.first == handle;
                        });
                    if (ret == handles.end()) {
                        return false;
                    }
                    ret->second->raise();
                }
                return true;
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
    connect(pctl, &WingPlugin::Controller::insertBytes, _win,
            [=](qsizetype offset, const QByteArray &data) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    _rwlock.lockForWrite();
                    auto ret = e->hexEditor()->document()->insert(offset, data);
                    _rwlock.unlock();
                    return ret;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::writeBytes, _win,
            [=](qsizetype offset, const QByteArray &data) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    _rwlock.lockForWrite();
                    auto ret =
                        e->hexEditor()->document()->replace(offset, data);
                    _rwlock.unlock();
                    return ret;
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
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto render = editor->renderer();

                    QString enco = encoding;
                    if (!enco.length()) {
                        enco = render->encoding();
                    }

                    auto unicode = Utilities::encodingString(value, enco);

                    _rwlock.lockForWrite();
                    auto ret = doc->insert(offset, unicode);
                    _rwlock.unlock();
                    return ret;
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
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto render = editor->renderer();

                    QString enco = encoding;
                    if (!enco.length()) {
                        enco = render->encoding();
                    }

                    auto unicode = Utilities::encodingString(value, enco);
                    _rwlock.lockForWrite();
                    auto ret = doc->replace(offset, unicode);
                    _rwlock.unlock();
                    return ret;
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
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto render = editor->renderer();
                    auto offset = doc->length();

                    QString enco = encoding;
                    if (!enco.length()) {
                        enco = render->encoding();
                    }

                    auto unicode = Utilities::encodingString(value, enco);
                    _rwlock.lockForWrite();
                    auto ret = doc->insert(offset, unicode);
                    _rwlock.unlock();
                    return ret;
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::remove, _win,
            [=](qsizetype offset, qsizetype len) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    _rwlock.lockForWrite();
                    auto ret = doc->remove(offset, len);
                    _rwlock.unlock();
                    return ret;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::removeAllBytes, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            auto len = doc->length();
            _rwlock.lockForWrite();
            auto ret = doc->remove(0, len);
            _rwlock.unlock();
            return ret;
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
                    return doc->metadata()->metadata(begin, begin + length,
                                                     fgcolor, bgcolor, comment);
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
                    return doc->metadata()->metadata(begin, begin + length,
                                                     fgcolor, bgcolor, comment);
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
                    doc->metadata()->removeMetadata(offset);
                    return true;
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
            doc->metadata()->clear();
            return true;
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
                    return doc->metadata()->comment(begin, begin + length,
                                                    comment);
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
                return doc->metadata()->foreground(begin, begin + length,
                                                   fgcolor);
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
                return doc->metadata()->background(begin, begin + length,
                                                   bgcolor);
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
    connect(pctl, &WingPlugin::Controller::setCurrentEncoding, _win,
            [=](const QString &encoding) -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->renderer()->setEncoding(encoding);
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
                    return doc->addBookMark(pos, comment);
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
                    return doc->modBookMark(pos, comment);
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
                    return doc->removeBookMark(pos);
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
            return doc->clearBookMark();
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
            m_plgviewMap[plg] = view;
            return ErrFile(int(*id));
        } else {
            return ErrFile::Error;
        }
    });
    connect(pctl, &WingPlugin::Controller::openWorkSpace, _win,
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
                    m_plgviewMap[plg] = view;
                    return ErrFile(int(*id));
                } else {
                    return ret;
                }
            });
    connect(pctl, &WingPlugin::Controller::openFile, _win,
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
                    m_plgviewMap[plg] = view;
                    return ErrFile(int(*id));
                } else {
                    return ret;
                }
            });
    connect(pctl, &WingPlugin::Controller::openRegionFile, _win,
            [=](const QString &filename, qsizetype start,
                qsizetype length) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                EditorView *view = nullptr;
                if (!checkPluginCanOpenedFile(plg)) {
                    return ErrFile::TooManyOpenedFile;
                }
                auto ret = _win->openRegionFile(filename, &view, start, length);
                if (view) {
                    if (ret == ErrFile::AlreadyOpened &&
                        checkPluginHasAlreadyOpened(plg, view)) {
                        return ErrFile::AlreadyOpened;
                    }
                    auto id = assginHandleForPluginView(plg, view);
                    m_plgviewMap[plg] = view;
                    return ErrFile(int(*id));
                } else {
                    return ret;
                }
            });
    connect(pctl, &WingPlugin::Controller::openDriver, _win,
            [=](const QString &driver) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                EditorView *view = nullptr;
                auto ret = _win->openDriver(driver, &view);
                if (!checkPluginCanOpenedFile(plg)) {
                    return ErrFile::TooManyOpenedFile;
                }
                if (view) {
                    if (ret == ErrFile::AlreadyOpened &&
                        checkPluginHasAlreadyOpened(plg, view)) {
                        return ErrFile::AlreadyOpened;
                    }
                    auto id = assginHandleForPluginView(plg, view);
                    m_plgviewMap[plg] = view;
                    return ErrFile(int(*id));
                } else {
                    return ret;
                }
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
            [=](int handle, bool ignoreMd5) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                auto view = handle2EditorView(plg, handle);
                if (view) {
                    _win->saveEditor(view, {}, ignoreMd5);
                    return ErrFile::Success;
                }
                return ErrFile::NotExist;
            });
    connect(
        pctl, &WingPlugin::Controller::exportFile, _win,
        [=](int handle, const QString &savename, bool ignoreMd5) -> ErrFile {
            if (!checkThreadAff()) {
                return ErrFile::NotAllowedInNoneGUIThread;
            }
            auto view = handle2EditorView(plg, handle);
            if (view) {
                _win->saveEditor(view, savename, ignoreMd5, true);
                return ErrFile::Success;
            }
            return ErrFile::NotExist;
        });

    connect(
        pctl, &WingPlugin::Controller::saveAsFile, _win,
        [=](int handle, const QString &savename, bool ignoreMd5) -> ErrFile {
            if (!checkThreadAff()) {
                return ErrFile::NotAllowedInNoneGUIThread;
            }
            auto view = handle2EditorView(plg, handle);
            if (view) {
                _win->saveEditor(view, savename, ignoreMd5);
                return ErrFile::Success;
            }
            return ErrFile::NotExist;
        });

    connect(pctl, &WingPlugin::Controller::closeCurrentFile, _win,
            [=](bool force) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                auto view = getCurrentPluginView(plg);
                if (view == nullptr) {
                    return ErrFile::NotExist;
                }

                return _win->closeEditor(view, force);
            });
    connect(pctl, &WingPlugin::Controller::saveCurrentFile, _win,
            [=](bool ignoreMd5) -> ErrFile {
                if (!checkThreadAff()) {
                    return ErrFile::NotAllowedInNoneGUIThread;
                }
                auto view = getCurrentPluginView(plg);
                if (view) {
                    auto ws = _win->m_views.value(view);
                    return view->save(
                        ws, {}, ignoreMd5, false,
                        EditorView::SaveWorkSpaceAttr::AutoWorkSpace);
                }
                return ErrFile::Error;
            });

    connect(pctl, &WingPlugin::Controller::closeAllPluginFiles, _win,
            [=]() -> bool {
                if (!checkThreadAff()) {
                    return false;
                }
                auto &maps = m_plgHandles[plg];
                for (auto &item : maps) {
                    auto &bind = m_viewBindings[item.second];
                    bind.removeOne(plg);
                    if (bind.isEmpty()) {
                        _win->closeEditor(item.second, true);
                    }
                }
                m_plgviewMap.remove(plg);
                m_plgHandles.remove(plg);
                return true;
            });
}

void PluginSystem::connectUIInterface(IWingPlugin *plg) {
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
            auto oldmodel = _win->m_infolist->model();
            if (oldmodel) {
                oldmodel->deleteLater();
            }
            _win->m_infolist->setProperty("__TITLE__", title);
            auto model = new QStringListModel(data);
            _win->m_infolist->setModel(model);
            _win->m_infoclickfn = clicked;
            _win->m_infodblclickfn = dblClicked;
            return true;
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
            auto oldmodel = _win->m_infotree->model();
            if (oldmodel) {
                oldmodel->deleteLater();
            }
            _win->m_infotree->setProperty("__TITLE__", title);
            auto model = new QJsonModel;
            if (model->loadJson(json.toUtf8())) {
                _win->m_infotree->setModel(model);
                _win->m_infotreeclickfn = clicked;
                _win->m_infotreedblclickfn = dblClicked;
                return true;
            }
            return false;
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
                for (auto p = headers.cbegin(); p != headers.cend();
                     ++p, ++np) {
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
            _win->m_infotableclickfn = clicked;
            _win->m_infotabledblclickfn = dblClicked;
            return true;
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

void PluginSystem::LoadPlugin() {
    Q_ASSERT(_win);

    auto &set = SettingManager::instance();
    if (set.scriptEnabled()) {
        _angelplg = new WingAngelAPI;

        loadPlugin(_angelplg, {}, std::nullopt);
    }

    bool ok;
    auto displg = qEnvironmentVariableIntValue("WING_DISABLE_PLUGIN", &ok);
    if (displg > 0 || !set.enablePlugin()) {
        return;
    }

#ifdef QT_DEBUG
    QDir plugindir(QCoreApplication::applicationDirPath() + QDir::separator() +
                   QStringLiteral("plugin"));
    // 这是我的插件调试目录，如果调试插件，请更换路径
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

    if (Utilities::isRoot()) {
        if (!set.enablePlgInRoot()) {
            return;
        }
    } else {
        auto testFileName = plugindir.absoluteFilePath(
            QUuid::createUuid().toString(QUuid::Id128));

        QFile f(testFileName);
        if (f.open(QFile::WriteOnly)) {
            f.close();
            f.remove();
            Logger::warning(QStringLiteral("<i><u>") % tr("UnsafePluginDir") %
                            QStringLiteral("</u></i>"));
        }
    }

    auto plgs = plugindir.entryInfoList();
    Logger::info(tr("FoundPluginCount") + QString::number(plgs.count()));

    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("plgset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        throw CrashCode::PluginSetting;
    }

    for (auto &item : plgs) {
        loadPlugin(item, udir);
    }

    if (!_lazyplgs.isEmpty()) {
        decltype(_lazyplgs) lazyplgs;
        lazyplgs.swap(_lazyplgs);

        for (auto &item : lazyplgs) {
            loadPlugin(item.first, item.second, udir);
        }
    }

    if (!_lazyplgs.isEmpty()) {
        Logger::critical(tr("PluginLoadingFailedSummary"));
        Logger::_log({});
        for (auto &lplg : _lazyplgs) {
            auto plg = lplg.first;
            Logger::critical(tr("- PluginName:") + plg->pluginName());
            Logger::critical(tr("- Dependencies:"));
            for (auto &d : plg->dependencies()) {
                Logger::critical(QString(4, ' ') + tr("PUID:") + d.puid);
                Logger::critical(QString(4, ' ') + tr("Version:") +
                                 QString::number(d.version));
                Logger::critical(QString(4, ' ') + tr("MD5:") + d.md5);
            }
            plg->deleteLater();
        }
        _lazyplgs.clear();
    }

    Logger::info(tr("PluginLoadingFinished"));
}

EditorView *PluginSystem::pluginCurrentEditor(IWingPlugin *sender) const {
    if (sender) {
        auto editor = m_plgviewMap.value(sender);
        if (editor) {
            return editor;
        } else {
            return _win->currentEditor();
        }
    }
    return nullptr;
}
