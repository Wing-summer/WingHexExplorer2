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
#include "plugin/iwingdevice.h"

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
#ifdef QT_DEBUG
                qDebug() << "[UniqueId] alloced: " << id;
#endif
            }

            ~UniqueId() {
                if (_gen) {
                    _gen->release(_id);
                }
#ifdef QT_DEBUG
                qDebug() << "[UniqueId] freed: " << _id;
#endif
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

    using SharedUniqueId =
        QExplicitlySharedDataPointer<UniqueIdGenerator::UniqueId>;

public:
    struct PluginInfo {
        QString id;
        QString name;
        QVersionNumber version;
        QString vendor;
        QList<WingDependency> dependencies;
        QString author;
        QString license;
        QString url;
    };

    enum class PluginStatus {
        Valid,
        BrokenVersion,
        InvalidID,
        DupID,
        LackDependencies
    };
    Q_ENUM(PluginStatus)

private:
    struct PluginFileContext {
        SharedUniqueId fid;
        EditorView *view = nullptr;
        IWingPlugin *linkedplg = nullptr;
        QUndoCommand *cmd = nullptr;

        ~PluginFileContext() {
            if (cmd) {
                delete cmd;
            }
        }
    };

public:
    static PluginSystem &instance();

    void setMainWindow(MainWindow *win);
    QWidget *mainWindow() const;

    void loadAllPlugin();
    void unloadAllPlugin();

    const QList<IWingPlugin *> &plugins() const;
    IWingPlugin *plugin(qsizetype index) const;

    const QList<IWingDevice *> &devices() const;
    IWingDevice *device(qsizetype index) const;

    WingAngelAPI *angelApi() const;

    void cleanUpEditorViewHandle(EditorView *view);

    bool dispatchEvent(IWingPlugin::RegisteredEvent event,
                       const QVariantList &params);

    IWingDevice *ext2Device(const QString &ext);

public:
    PluginInfo getPluginInfo(IWingPluginBase *plg) const;

    QString getPluginID(IWingPluginBase *plg) const;

private:
    void loadExtPlugin();

    void loadDevicePlugin();

    void checkDirRootSafe(const QDir &dir);

    template <typename T>
    std::optional<PluginInfo> loadPlugin(const QFileInfo &filename,
                                         const QDir &setdir);

    bool closeEditor(IWingPlugin *plg, int handle, bool force);

    bool closeHandle(IWingPlugin *plg, int handle);

    bool checkPluginCanOpenedFile(IWingPlugin *plg);

    bool checkPluginHasAlreadyOpened(IWingPlugin *plg, EditorView *view);

    EditorView *getCurrentPluginView(IWingPlugin *plg);

    EditorView *handle2EditorView(IWingPlugin *plg, int handle);

    SharedUniqueId assginHandleForPluginView(IWingPlugin *plg,
                                             EditorView *view);

private:
    PluginInfo parsePluginMetadata(const QJsonObject &meta);

    PluginStatus checkPluginMetadata(const PluginInfo &meta, bool isPlg);

private:
    void registerFns(IWingPlugin *plg);
    void registerEnums(IWingPlugin *plg);
    void registerEvents(IWingPlugin *plg);

    static QString type2AngelScriptString(IWingPlugin::MetaType type,
                                          bool isArg);

    static QString getScriptFnSig(const QString &fnName,
                                  const IWingPlugin::ScriptFnInfo &fninfo);

    static QString getPUID(IWingPluginBase *p);

    bool isPluginLoaded(const WingDependency &d);

    bool isPluginLoaded(const QString &id);

    bool checkThreadAff();

    static QString packLogMessage(const char *header, const QString &msg);

    EditorView *pluginCurrentEditor(IWingPlugin *plg) const;

    std::optional<PluginSystem::PluginFileContext>
    pluginContextById(IWingPlugin *plg, int fid) const;

    std::optional<QVector<PluginFileContext>::iterator>
    pluginContextByIdIt(IWingPlugin *plg, int fid);

    QUndoCommand *pluginCurrentUndoCmd(IWingPlugin *plg) const;

private:
    void loadPlugin(IWingPlugin *p, PluginInfo &meta,
                    const std::optional<QDir> &setdir);
    void loadPlugin(IWingDevice *p, PluginInfo &meta,
                    const std::optional<QDir> &setdir);

private:
    void connectInterface(IWingPlugin *plg);
    void connectLoadingInterface(IWingPlugin *plg);
    void connectBaseInterface(IWingPlugin *plg);
    void connectReaderInterface(IWingPlugin *plg);
    void connectControllerInterface(IWingPlugin *plg);
    void connectUIInterface(IWingPlugin *plg);

private:
    void connectInterface(IWingDevice *plg);
    void connectLoadingInterface(IWingDevice *plg);

private:
    void connectBaseInterface(IWingPluginBase *plg);
    void connectUIInterface(IWingPluginBase *plg);
    void registerPluginDockWidgets(IWingPluginBase *p);
    void registerPluginPages(IWingPluginBase *p);

private:
    template <typename T>
    T readBasicTypeContent(IWingPlugin *plg, qsizetype offset) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = pluginCurrentEditor(plg);
        if (e) {
            _rwlock.lockForRead();
            auto buffer = e->hexEditor()->document()->read(offset, sizeof(T));
            if (buffer.size() == sizeof(T)) {
                auto pb = reinterpret_cast<const T *>(buffer.constData());
                _rwlock.unlock();
                return *pb;
            }
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
        auto e = getCurrentPluginView(plg);
        if (e) {
            auto editor = e->hexEditor();
            auto doc = editor->document();
            auto buffer = reinterpret_cast<const char *>(&value);
            auto uc = pluginCurrentUndoCmd(plg);
            auto cmd = doc->MakeInsert(uc, editor->cursor(), offset,
                                       QByteArray(buffer, sizeof(T)));
            if (uc == nullptr && cmd) {
                _rwlock.lockForWrite();
                doc->pushMakeUndo(cmd);
                _rwlock.unlock();
                return true;
            }
        }
        return false;
    }

    template <typename T>
    bool writeBasicTypeContent(IWingPlugin *plg, qsizetype offset,
                               const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = getCurrentPluginView(plg);
        if (e) {
            auto editor = e->hexEditor();
            auto doc = editor->document();
            auto buffer = reinterpret_cast<const char *>(&value);
            auto uc = pluginCurrentUndoCmd(plg);
            auto cmd = doc->MakeReplace(uc, editor->cursor(), offset,
                                        QByteArray(buffer, sizeof(T)));
            if (uc == nullptr && cmd) {
                _rwlock.lockForWrite();
                doc->pushMakeUndo(cmd);
                _rwlock.unlock();
                return true;
            }
        }
        return false;
    }

    template <typename T>
    bool appendBasicTypeContent(IWingPlugin *plg, const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = getCurrentPluginView(plg);
        if (e) {
            auto editor = e->hexEditor();
            auto doc = editor->document();
            auto buffer = reinterpret_cast<const char *>(&value);
            auto uc = pluginCurrentUndoCmd(plg);
            auto cmd = doc->MakeAppend(uc, editor->cursor(),
                                       QByteArray(buffer, sizeof(T)));
            if (uc == nullptr && cmd) {
                _rwlock.lockForWrite();
                doc->pushMakeUndo(cmd);
                _rwlock.unlock();
                return true;
            }
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
    QHash<IWingPluginBase *, PluginInfo> _pinfos;
    QList<IWingPlugin *> _loadedplgs;
    QHash<QWidget *, ads::CDockWidget *> _raisedw;
    QStringList _lazyplgs;

    QList<IWingDevice *> _loadeddevs;

    QMap<IWingPlugin::RegisteredEvent, QList<IWingPlugin *>> _evplgs;

    QHash<IWingPlugin *, QVector<PluginFileContext>> m_plgviewMap;
    QHash<IWingPlugin *, int> m_plgCurrentfid; // fid
    QHash<EditorView *, QList<IWingPlugin *>> m_viewBindings;

    UniqueIdGenerator m_idGen;

    QHash<QString, QHash<QString, WingAngelAPI::ScriptFnInfo>> _scfns;

    WingAngelAPI *_angelplg = nullptr;

    QReadWriteLock _rwlock;
};

#endif // PLUGINSYSTEM_H
