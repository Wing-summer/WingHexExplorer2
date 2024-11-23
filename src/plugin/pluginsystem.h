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
private:
    class UniqueIdGenerator {
    public:
        class UniqueId : public QSharedData {
        public:
            UniqueId() : _id(-1), _gen(nullptr) {}

            UniqueId(UniqueIdGenerator *gen, int id) : _id(id), _gen(gen) {
                Q_ASSERT(gen);
                Q_ASSERT(id >= 0);
            }

            ~UniqueId() {
                if (_gen) {
                    _gen->release(_id);
                }
            }

            operator int() { return _id; }

        private:
            int _id;
            UniqueIdGenerator *_gen;
        };

    public:
        UniqueIdGenerator() : currentId(0) {}

        QExplicitlySharedDataPointer<UniqueId> get() {
            if (!releasedIds.isEmpty()) {
                int id = releasedIds.dequeue();
                return QExplicitlySharedDataPointer<UniqueId>(
                    new UniqueId(this, id));
            } else {
                return QExplicitlySharedDataPointer<UniqueId>(
                    new UniqueId(this, currentId++));
            }
        }

        void release(int id) {
            if (id < currentId && !releasedIds.contains(id)) {
                releasedIds.enqueue(id);
            }
        }

    private:
        int currentId;
        QQueue<int> releasedIds;
    };

    using UniqueId = QExplicitlySharedDataPointer<UniqueIdGenerator::UniqueId>;

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

    EditorView *getCurrentPluginView(IWingPlugin *plg);

    EditorView *handle2EditorView(IWingPlugin *plg, int handle);

    UniqueId assginHandleForPluginView(IWingPlugin *plg, EditorView *view);

    bool checkPluginCanOpenedFile(IWingPlugin *plg);

    void cleanUpEditorViewHandle(EditorView *view);

private:
    void registerFns(IWingPlugin *plg);

    QString type2AngelScriptString(IWingPlugin::MetaType type);

    QString getScriptFnSig(const QString &fnName,
                           const IWingPlugin::ScriptFnInfo &fninfo);

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

signals:
    void pluginLoading(const QString &plgName);

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

private:
    MainWindow *_win = nullptr;
    QStringList loadedpuid;
    QList<IWingPlugin *> loadedplgs;

    QMap<IWingPlugin *, EditorView *> m_plgviewMap;
    QMap<IWingPlugin *, QList<QPair<UniqueId, EditorView *>>> m_plgHandles;
    QMap<EditorView *, QList<IWingPlugin *>> m_viewBindings;

    UniqueIdGenerator m_idGen;

    QHash<QString, QHash<QString, WingAngelAPI::ScriptFnInfo>> _scfns;

    WingAngelAPI *_angelplg = nullptr;
};

#endif // PLUGINSYSTEM_H
