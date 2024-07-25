#ifndef PLUGINSYSTEM_H
#define PLUGINSYSTEM_H

#include "src/define.h"
#include "src/plugin/iwingplugin.h"
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

using namespace WingHex;

class MainWindow;
class EditorView;

class PluginSystem : public QObject {
    Q_OBJECT
public:
    static PluginSystem &instance();

    void setMainWindow(MainWindow *win);

    void LoadPlugin();
    void UnloadPlugin();
    const QList<IWingPlugin *> &plugins() const;
    const IWingPlugin *plugin(qindextype index) const;
    void raiseDispatch(HookIndex hookindex, QList<QVariant> params);

    void loadPlugin(QFileInfo filename);

private:
    void subscribeDispatcher(IWingPlugin *plg, HookIndex hookIndex);

    void connectInterface(IWingPlugin *plg);
    void connectBaseInterface(IWingPlugin *plg);
    void connectReaderInterface(IWingPlugin *plg);
    void connectControllerInterface(IWingPlugin *plg);
    void connectUIInterface(IWingPlugin *plg);

    static bool checkSender(QObject *sender);

    EditorView *pluginCurrentEditor(QObject *sender) const;

private:
    const QList<QVariant> emptyparam;

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

private:
    MainWindow *_win = nullptr;
    QThread *_plgThread = nullptr;
    QStringList loadedpuid;
    QList<IWingPlugin *> loadedplgs;
    QMap<HookIndex, QList<IWingPlugin *>> dispatcher;
    QMutex mutex;

    QMap<QObject *, EditorView *> m_plgviewMap;
};

#endif // PLUGINSYSTEM_H
