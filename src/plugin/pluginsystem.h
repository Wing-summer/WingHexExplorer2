#ifndef PLUGINSYSTEM_H
#define PLUGINSYSTEM_H

#include "../plugin/iwingplugin.h"
#include <QDockWidget>
#include <QHash>
#include <QList>
#include <QMenu>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <QTimerEvent>
#include <QToolButton>
#include <QVariant>

class PluginSystem : public QObject {
    Q_OBJECT
public:
    static PluginSystem &instance();

    void LoadPlugin();
    void UnloadPlugin();
    QList<IWingPlugin *> plugins();
    void raiseDispatch(HookIndex hookindex, QList<QVariant> params);

    void loadPlugin(QFileInfo filename);

private:
    const QList<QVariant> emptyparam;

signals:
    void sigPluginContextMenuNeedAdd(QHash<QString, QMenu *> contextMenus);
    void sigPluginToolBarAdd(
        QHash<QString, QPair<QString, QList<QToolButton *>>> toolbars);

    void
    PluginDockWidgetAdd(const QString &pluginname,
                        const QHash<QDockWidget *, Qt::DockWidgetArea> &rdw);
    void PluginToolButtonAdd(QToolButton *btn);

    void ConnectBase(const IWingPlugin *plugin);

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

private:
    QStringList loadedpuid;
    QList<IWingPlugin *> loadedplgs;
    QMap<HookIndex, QList<IWingPlugin *>> dispatcher;
    QMutex mutex;
};

#endif // PLUGINSYSTEM_H
