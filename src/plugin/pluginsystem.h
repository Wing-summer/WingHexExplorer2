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
#include "control/editorview.h"

using namespace WingHex;

class MainWindow;

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
    template <typename T>
    T readBasicTypeContent(IWingPlugin *plg, qsizetype offset) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto buffer = e->hexEditor()->document()->read(offset, sizeof(T));
            auto pb = reinterpret_cast<const T *>(buffer.constData());
            return *pb;
        }
        if constexpr (std::is_floating_point_v<T>) {
            return qQNaN();
        } else {
            return T(0);
        }
    }

    template <typename T>
    bool insertBasicTypeContent(IWingPlugin *plg, qsizetype offset,
                                const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            auto buffer = reinterpret_cast<const char *>(&value);
            return doc->insert(offset, QByteArray(buffer, sizeof(T)));
        }
        return false;
    }

    template <typename T>
    bool writeBasicTypeContent(IWingPlugin *plg, qsizetype offset,
                               const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            auto buffer = reinterpret_cast<const char *>(&value);
            return doc->replace(offset, QByteArray(buffer, sizeof(T)));
        }
        return false;
    }

    template <typename T>
    bool appendBasicTypeContent(IWingPlugin *plg, const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            auto offset = doc->length();
            auto buffer = reinterpret_cast<const char *>(&value);
            QByteArray data(buffer, sizeof(T));
            return doc->insert(offset, data);
        }
        return false;
    }

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
