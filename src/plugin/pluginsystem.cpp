#include "pluginsystem.h"
#include "../class/logger.h"
#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QPluginLoader>
#include <QtCore>

PluginSystem::PluginSystem(QObject *parent) : QObject(parent) {
    // init plugin dispathcer
#define InitDispathcer(hookindex)                                              \
    dispatcher.insert(hookindex, QList<IWingPlugin *>());

    InitDispathcer(HookIndex::OpenFileBegin);
    InitDispathcer(HookIndex::OpenFileEnd);
    InitDispathcer(HookIndex::CloseFileBegin);
    InitDispathcer(HookIndex::CloseFileEnd);
    InitDispathcer(HookIndex::NewFileBegin);
    InitDispathcer(HookIndex::NewFileEnd);
    InitDispathcer(HookIndex::DocumentSwitched);
}

PluginSystem::~PluginSystem() {
    for (auto &item : loadedplgs) {
        item->plugin2MessagePipe(WingPluginMessage::PluginUnLoading,
                                 emptyparam);
        item->controller.disconnect();
        item->reader.disconnect();
        item->plugin2MessagePipe(WingPluginMessage::PluginUnLoaded, emptyparam);
        item->unload();
        item->deleteLater();
    }
}

QList<IWingPlugin *> PluginSystem::plugins() { return loadedplgs; }

void PluginSystem::raiseDispatch(HookIndex hookindex, QList<QVariant> params) {
    auto dispatch = dispatcher[hookindex];
    params.push_front(hookindex);
    for (auto item : dispatch) {
        item->plugin2MessagePipe(WingPluginMessage::HookMessage, params);
    }
}

void PluginSystem::loadPlugin(QFileInfo fileinfo) {
    if (fileinfo.exists()) {
        QPluginLoader loader(fileinfo.absoluteFilePath());
        Logger::info(tr("LoadingPlugin") + fileinfo.fileName());
        QList<WingPluginInfo> loadedplginfos;
        try {
            auto p = qobject_cast<IWingPlugin *>(loader.instance());
            if (p) {
                if (p->signature() != WINGSUMMER) {
                    throw tr("ErrLoadPluginSign");
                }

                if (p->sdkVersion() != SDKVERSION) {
                    throw tr("ErrLoadPluginSDKVersion");
                }

                if (!p->pluginName().trimmed().length()) {
                    throw tr("ErrLoadPluginNoName");
                }

                auto puid = IWingPlugin::GetPUID(p);
                if (puid != p->puid()) {
                    throw tr("ErrLoadPluginPUID");
                }

                if (loadedpuid.contains(puid)) {
                    throw tr("ErrLoadLoadedPlugin");
                }

                p->plugin2MessagePipe(WingPluginMessage::PluginLoading,
                                      emptyparam);

                if (!p->init(loadedplginfos)) {
                    throw tr("ErrLoadInitPlugin");
                }

                WingPluginInfo info;
                info.puid = p->puid();
                info.pluginName = p->pluginName();
                info.pluginAuthor = p->pluginAuthor();
                info.pluginComment = p->pluginComment();
                info.pluginVersion = p->pluginVersion();

                loadedplginfos.push_back(info);
                loadedplgs.push_back(p);
                loadedpuid << puid;

                Logger::warning(tr("PluginName :") + info.pluginName);
                Logger::warning(tr("PluginAuthor :") + info.pluginAuthor);
                Logger::warning(tr("PluginWidgetRegister"));

                auto menu = p->registeredContextMenu();
                if (!menu.isEmpty()) {
                    emit this->sigPluginContextMenuNeedAdd(menu);
                }

                auto tb = p->registerToolBars();
                if (!tb.isEmpty()) {
                    emit this->sigPluginToolBarAdd(tb);
                }

                // TODO
                // QHash<QDockWidget *, Qt::DockWidgetArea> dws;
                // p->registerDockWidget(dws);
                // if (dws.count()) {
                //     emit this->PluginDockWidgetAdd(p->pluginName(), dws);
                // }

                emit ConnectBase(p);

                auto sub = p->getHookSubscribe();

#define INSERTSUBSCRIBE(HOOK)                                                  \
    if (sub & HOOK)                                                            \
        dispatcher[HOOK].push_back(p);

                INSERTSUBSCRIBE(HookIndex::OpenFileBegin);
                INSERTSUBSCRIBE(HookIndex::OpenFileEnd);
                INSERTSUBSCRIBE(HookIndex::CloseFileBegin);
                INSERTSUBSCRIBE(HookIndex::CloseFileEnd);
                INSERTSUBSCRIBE(HookIndex::NewFileBegin);
                INSERTSUBSCRIBE(HookIndex::NewFileEnd);
                INSERTSUBSCRIBE(HookIndex::DocumentSwitched);

                p->plugin2MessagePipe(WingPluginMessage::PluginLoaded,
                                      emptyparam);

            } else {
                throw loader.errorString();
            }
        } catch (const QString &error) {
            Logger::critical(error);
            loader.unload();
        }

        Logger::_log("");
    }
}

PluginSystem &PluginSystem::instance() {
    static PluginSystem ins;
    return ins;
}

void PluginSystem::LoadPlugin() {
#ifdef QT_DEBUG
    QDir plugindir(QCoreApplication::applicationDirPath() +
                   QStringLiteral("/plugin"));
    //这是我的插件调试目录，如果调试插件，请更换路径
#ifdef Q_OS_WIN
    plugindir.setNameFilters({"*.dll"});
#else
    plugindir.setNameFilters({"*.so"});
#endif
#else
    QDir plugindir(QCoreApplication::applicationDirPath() +
                   QStringLiteral("/plugin"));
    plugindir.setNameFilters({"*.wingplg"});
#endif
    auto plgs = plugindir.entryInfoList();
    Logger::info(tr("FoundPluginCount") + QString::number(plgs.count()));

    for (auto &item : plgs) {
        loadPlugin(item);
    }

    Logger::info(tr("PluginLoadingFinished"));
}
