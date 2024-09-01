#ifndef PLUGINSYSTEM_H
#define PLUGINSYSTEM_H

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

#include "class/wingangelapi.h"

using namespace WingHex;

class MainWindow;
class EditorView;

class PluginSystem : public QObject {
    Q_OBJECT
public:
    static PluginSystem &instance();

    void setMainWindow(MainWindow *win);
    QWidget *mainWindow() const;

    void LoadPlugin();
    void UnloadPlugin();
    const QList<IWingPlugin *> &plugins() const;
    const IWingPlugin *plugin(qsizetype index) const;

    void loadPlugin(QFileInfo filename);

    WingAngelAPI *angelApi() const;

    void cleanUpEditorViewHandle(EditorView *view);

private:
    bool loadPlugin(IWingPlugin *p);

    void connectInterface(IWingPlugin *plg);
    void connectBaseInterface(IWingPlugin *plg);
    void connectReaderInterface(IWingPlugin *plg);
    void connectControllerInterface(IWingPlugin *plg);
    void connectUIInterface(IWingPlugin *plg);

    bool checkThreadAff();
    static QString packLogMessage(const char *header, const QString &msg);

    EditorView *pluginCurrentEditor(IWingPlugin *sender) const;

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

private:
    MainWindow *_win = nullptr;
    QStringList loadedpuid;
    QList<IWingPlugin *> loadedplgs;

    QMap<IWingPlugin *, EditorView *> m_plgviewMap;

    QMap<IWingPlugin *, QList<QPair<int, EditorView *>>> m_plgHandles;
    QMap<EditorView *, QList<IWingPlugin *>> m_viewBindings;

    WingAngelAPI *_angelplg = nullptr;
};

#endif // PLUGINSYSTEM_H
