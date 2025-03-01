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
#include <QQueue>
#include <QReadWriteLock>
#include <QTimer>
#include <QTimerEvent>
#include <QToolButton>
#include <QVariant>

#include "WingPlugin/iwingdevice.h"
#include "class/clickcallback.h"
#include "class/wingangelapi.h"
#include "control/editorview.h"

using namespace WingHex;

class MainWindow;
class asCScriptEngine;

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

    void destory();

    const QList<IWingPlugin *> &plugins() const;
    IWingPlugin *plugin(qsizetype index) const;

    const QList<IWingDevice *> &devices() const;
    IWingDevice *device(qsizetype index) const;

    WingAngelAPI *angelApi() const;

    void cleanUpEditorViewHandle(EditorView *view);

    bool dispatchEvent(IWingPlugin::RegisteredEvent event,
                       const QVariantList &params);

    IWingDevice *ext2Device(const QString &ext);

    QStringList scriptMarcos() const;

public:
    void scriptPragmaBegin();

    qsizetype pluginAPICount() const;

public:
    PluginInfo getPluginInfo(IWingPluginBase *plg) const;

    QString getPluginID(IWingPluginBase *plg) const;

    static QString getPUID(IWingPluginBase *p);

    static QString type2AngelScriptString(IWingPlugin::MetaType type,
                                          bool isArg, bool noModifier = false);

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

    static bool equalCompareHandle(const SharedUniqueId &id, int handle);

    static int getUIDHandle(const SharedUniqueId &id);

private:
    PluginInfo parsePluginMetadata(const QJsonObject &meta);

    PluginStatus checkPluginMetadata(const PluginInfo &meta, bool isPlg);

    static bool isValidIdentifier(const QString &str);

    void retranslateMetadata(IWingPluginBase *plg, PluginInfo &meta);

private:
    void registerFns(IWingPlugin *plg);
    void registerUnSafeFns(IWingPlugin *plg);
    void registerEnums(IWingPlugin *plg);
    void registerMarcos(IWingPlugin *plg);
    void registerEvents(IWingPlugin *plg);

    void applyFunctionTables(IWingPluginBase *plg, const CallTable &fns);

    static QString getScriptFnSig(const QString &fnName,
                                  const IWingPlugin::ScriptFnInfo &fninfo);

    bool isPluginLoaded(const WingDependency &d);

    bool isPluginLoaded(const QString &id);

    bool checkThreadAff();

    static QString packLogMessage(const char *header, const QString &msg);

    EditorView *pluginCurrentEditor(IWingPlugin *plg) const;

    QSharedPointer<PluginFileContext> pluginContextById(IWingPlugin *plg,
                                                        int fid) const;

    QUndoCommand *pluginCurrentUndoCmd(IWingPlugin *plg) const;

private:
    void loadPlugin(IWingPlugin *p, PluginInfo &meta,
                    const std::optional<QDir> &setdir);
    void loadPlugin(IWingDevice *p, PluginInfo &meta,
                    const std::optional<QDir> &setdir);

private:
    void registerMarcoDevice(IWingDevice *plg);

private:
    void registerPluginDockWidgets(IWingPluginBase *p);
    void registerPluginPages(IWingPluginBase *p);

public:
    bool updateTextList_API(const QStringList &data, const QString &title,
                            const ClickCallBack &click,
                            const ClickCallBack &dblclick);
    bool updateTextTree_API(const QString &json, const QString &title,
                            const ClickCallBack &click,
                            const ClickCallBack &dblclick);
    bool updateTextTable_API(const QString &json, const QStringList &headers,
                             const QStringList &headerNames,
                             const QString &title, const ClickCallBack &click,
                             const ClickCallBack &dblclick);

public:
    // fpr crash checking
    QString currentLoadingPlugin() const;

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

    void initCheckingEngine();

    void finalizeCheckingEngine();

    // IWingPluginBase API
public:
    WING_SERVICE void toast(WingHex::IWingPluginBase *sender,
                            const QPixmap &icon, const QString &message);

    // logging
    WING_SERVICE void logTrace(WingHex::IWingPluginBase *sender,
                               const QString &message);
    WING_SERVICE void logDebug(WingHex::IWingPluginBase *sender,
                               const QString &message);
    WING_SERVICE void logInfo(WingHex::IWingPluginBase *sender,
                              const QString &message);
    WING_SERVICE void logWarn(WingHex::IWingPluginBase *sender,
                              const QString &message);
    WING_SERVICE void logError(WingHex::IWingPluginBase *sender,
                               const QString &message);
    WING_SERVICE bool raiseDockWidget(WingHex::IWingPluginBase *sender,
                                      QWidget *w);
    WING_SERVICE WingHex::AppTheme
    currentAppTheme(WingHex::IWingPluginBase *sender);
    WING_SERVICE QDialog *createDialog(WingHex::IWingPluginBase *sender,
                                       QWidget *content);

public:
    WING_SERVICE void msgAboutQt(WingHex::IWingPluginBase *sender,
                                 QWidget *parent, const QString &title);
    WING_SERVICE QMessageBox::StandardButton
    msgInformation(WingHex::IWingPluginBase *sender, QWidget *parent,
                   const QString &title, const QString &text,
                   QMessageBox::StandardButtons buttons,
                   QMessageBox::StandardButton defaultButton);
    WING_SERVICE QMessageBox::StandardButton
    msgQuestion(WingHex::IWingPluginBase *sender, QWidget *parent,
                const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    WING_SERVICE QMessageBox::StandardButton
    msgWarning(WingHex::IWingPluginBase *sender, QWidget *parent,
               const QString &title, const QString &text,
               QMessageBox::StandardButtons buttons,
               QMessageBox::StandardButton defaultButton);
    WING_SERVICE QMessageBox::StandardButton
    msgCritical(WingHex::IWingPluginBase *sender, QWidget *parent,
                const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    WING_SERVICE void msgAbout(WingHex::IWingPluginBase *sender,
                               QWidget *parent, const QString &title,
                               const QString &text);
    WING_SERVICE QMessageBox::StandardButton
    msgbox(WingHex::IWingPluginBase *sender, QWidget *parent,
           QMessageBox::Icon icon, const QString &title, const QString &text,
           QMessageBox::StandardButtons buttons,
           QMessageBox::StandardButton defaultButton);

public:
    WING_SERVICE QString dlgGetText(WingHex::IWingPluginBase *sender,
                                    QWidget *parent, const QString &title,
                                    const QString &label,
                                    QLineEdit::EchoMode echo,
                                    const QString &text, bool *ok,
                                    Qt::InputMethodHints inputMethodHints);
    WING_SERVICE QString dlgGetMultiLineText(
        WingHex::IWingPluginBase *sender, QWidget *parent, const QString &title,
        const QString &label, const QString &text, bool *ok,
        Qt::InputMethodHints inputMethodHints);
    WING_SERVICE QString dlgGetItem(WingHex::IWingPluginBase *sender,
                                    QWidget *parent, const QString &title,
                                    const QString &label,
                                    const QStringList &items, int current,
                                    bool editable, bool *ok,
                                    Qt::InputMethodHints inputMethodHints);
    WING_SERVICE int dlgGetInt(WingHex::IWingPluginBase *sender,
                               QWidget *parent, const QString &title,
                               const QString &label, int value, int minValue,
                               int maxValue, int step, bool *ok);
    WING_SERVICE double dlgGetDouble(WingHex::IWingPluginBase *sender,
                                     QWidget *parent, const QString &title,
                                     const QString &label, double value,
                                     double minValue, double maxValue,
                                     int decimals, bool *ok, double step);

public:
    WING_SERVICE QString
    dlgGetExistingDirectory(WingHex::IWingPluginBase *sender, QWidget *parent,
                            const QString &caption, const QString &dir,
                            QFileDialog::Options options);
    WING_SERVICE QString dlgGetOpenFileName(
        WingHex::IWingPluginBase *sender, QWidget *parent,
        const QString &caption, const QString &dir, const QString &filter,
        QString *selectedFilter, QFileDialog::Options options);
    WING_SERVICE QStringList dlgGetOpenFileNames(
        WingHex::IWingPluginBase *sender, QWidget *parent,
        const QString &caption, const QString &dir, const QString &filter,
        QString *selectedFilter, QFileDialog::Options options);
    WING_SERVICE QString dlgGetSaveFileName(
        WingHex::IWingPluginBase *sender, QWidget *parent,
        const QString &caption, const QString &dir, const QString &filter,
        QString *selectedFilter, QFileDialog::Options options);

    WING_SERVICE QColor dlgGetColor(WingHex::IWingPluginBase *sender,
                                    const QString &caption, QWidget *parent);

    // IWingPlugin API
public:
    WING_SERVICE bool existsServiceHost(WingHex::IWingPluginBase *sender,
                                        const QString &puid);

    WING_SERVICE bool
    invokeService(WingHex::IWingPluginBase *sender, const QString &puid,
                  const char *method, Qt::ConnectionType type,
                  QGenericReturnArgument ret, QGenericArgument val0,
                  QGenericArgument val1, QGenericArgument val2,
                  QGenericArgument val3, QGenericArgument val4);

    WING_SERVICE bool isCurrentDocEditing(WingHex::IWingPluginBase *sender);

    WING_SERVICE QString currentDocFilename(WingHex::IWingPluginBase *sender);

    // document
    WING_SERVICE bool isReadOnly(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool isInsertionMode(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool isKeepSize(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool isLocked(WingHex::IWingPluginBase *sender);

    WING_SERVICE qsizetype documentLines(WingHex::IWingPluginBase *sender);

    WING_SERVICE qsizetype documentBytes(WingHex::IWingPluginBase *sender);

    WING_SERVICE WingHex::HexPosition
    currentPos(WingHex::IWingPluginBase *sender);

    WING_SERVICE qsizetype currentRow(WingHex::IWingPluginBase *sender);

    WING_SERVICE qsizetype currentColumn(WingHex::IWingPluginBase *sender);

    WING_SERVICE qsizetype currentOffset(WingHex::IWingPluginBase *sender);

    WING_SERVICE qsizetype selectedLength(WingHex::IWingPluginBase *sender);

    WING_SERVICE QByteArray selectedBytes(WingHex::IWingPluginBase *sender,
                                          qsizetype index);

    WING_SERVICE QByteArrayList
    selectionBytes(WingHex::IWingPluginBase *sender);

    WING_SERVICE WingHex::HexPosition
    selectionStart(WingHex::IWingPluginBase *sender, qsizetype index);

    WING_SERVICE WingHex::HexPosition
    selectionEnd(WingHex::IWingPluginBase *sender, qsizetype index);

    WING_SERVICE qsizetype selectionLength(WingHex::IWingPluginBase *sender,
                                           qsizetype index);

    WING_SERVICE qsizetype selectionCount(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool stringVisible(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool addressVisible(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool headerVisible(WingHex::IWingPluginBase *sender);

    WING_SERVICE quintptr addressBase(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool isModified(WingHex::IWingPluginBase *sender);

    WING_SERVICE qint8 readInt8(WingHex::IWingPluginBase *sender,
                                qsizetype offset);

    WING_SERVICE qint16 readInt16(WingHex::IWingPluginBase *sender,
                                  qsizetype offset);

    WING_SERVICE qint32 readInt32(WingHex::IWingPluginBase *sender,
                                  qsizetype offset);

    WING_SERVICE qint64 readInt64(WingHex::IWingPluginBase *sender,
                                  qsizetype offset);

    WING_SERVICE quint8 readUInt8(WingHex::IWingPluginBase *sender,
                                  qsizetype offset);

    WING_SERVICE quint16 readUInt16(WingHex::IWingPluginBase *sender,
                                    qsizetype offset);

    WING_SERVICE quint32 readUInt32(WingHex::IWingPluginBase *sender,
                                    qsizetype offset);

    WING_SERVICE quint64 readUInt64(WingHex::IWingPluginBase *sender,
                                    qsizetype offset);

    WING_SERVICE float readFloat(WingHex::IWingPluginBase *sender,
                                 qsizetype offset);

    WING_SERVICE double readDouble(WingHex::IWingPluginBase *sender,
                                   qsizetype offset);

    WING_SERVICE QString readString(WingHex::IWingPluginBase *sender,
                                    qsizetype offset, const QString &encoding);

    WING_SERVICE QByteArray readBytes(WingHex::IWingPluginBase *sender,
                                      qsizetype offset, qsizetype count);

    WING_SERVICE qsizetype findNext(WingHex::IWingPluginBase *sender,
                                    qsizetype begin, const QByteArray &ba);

    WING_SERVICE qsizetype findPrevious(WingHex::IWingPluginBase *sender,
                                        qsizetype begin, const QByteArray &ba);

    WING_SERVICE QString bookMarkComment(WingHex::IWingPluginBase *sender,
                                         qsizetype pos);

    WING_SERVICE bool existBookMark(WingHex::IWingPluginBase *sender,
                                    qsizetype pos);

    // document
    WING_SERVICE bool switchDocument(WingHex::IWingPluginBase *sender,
                                     int handle);

    WING_SERVICE bool raiseDocument(WingHex::IWingPluginBase *sender,
                                    int handle);

    WING_SERVICE bool setLockedFile(WingHex::IWingPluginBase *sender, bool b);

    WING_SERVICE bool setKeepSize(WingHex::IWingPluginBase *sender, bool b);

    WING_SERVICE bool setStringVisible(WingHex::IWingPluginBase *sender,
                                       bool b);

    WING_SERVICE bool setAddressVisible(WingHex::IWingPluginBase *sender,
                                        bool b);

    WING_SERVICE bool setHeaderVisible(WingHex::IWingPluginBase *sender,
                                       bool b);

    WING_SERVICE bool setAddressBase(WingHex::IWingPluginBase *sender,
                                     quintptr base);

    WING_SERVICE bool beginMarco(WingHex::IWingPluginBase *sender,
                                 const QString &txt);

    WING_SERVICE bool endMarco(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool writeInt8(WingHex::IWingPluginBase *sender,
                                qsizetype offset, qint8 value);

    WING_SERVICE bool writeInt16(WingHex::IWingPluginBase *sender,
                                 qsizetype offset, qint16 value);

    WING_SERVICE bool writeInt32(WingHex::IWingPluginBase *sender,
                                 qsizetype offset, qint32 value);

    WING_SERVICE bool writeInt64(WingHex::IWingPluginBase *sender,
                                 qsizetype offset, qint64 value);

    WING_SERVICE bool writeUInt8(WingHex::IWingPluginBase *sender,
                                 qsizetype offset, quint8 value);

    WING_SERVICE bool writeUInt16(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, quint16 value);

    WING_SERVICE bool writeUInt32(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, quint32 value);

    WING_SERVICE bool writeUInt64(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, quint64 value);

    WING_SERVICE bool writeFloat(WingHex::IWingPluginBase *sender,
                                 qsizetype offset, float value);

    WING_SERVICE bool writeDouble(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, double value);

    WING_SERVICE bool writeString(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, const QString &value,
                                  const QString &encoding);

    WING_SERVICE bool writeBytes(WingHex::IWingPluginBase *sender,
                                 qsizetype offset, const QByteArray &data);

    WING_SERVICE bool insertInt8(WingHex::IWingPluginBase *sender,
                                 qsizetype offset, qint8 value);

    WING_SERVICE bool insertInt16(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, qint16 value);

    WING_SERVICE bool insertInt32(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, qint32 value);

    WING_SERVICE bool insertInt64(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, qint64 value);

    WING_SERVICE bool insertUInt8(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, quint8 value);

    WING_SERVICE bool insertUInt16(WingHex::IWingPluginBase *sender,
                                   qsizetype offset, quint16 value);

    WING_SERVICE bool insertUInt32(WingHex::IWingPluginBase *sender,
                                   qsizetype offset, quint32 value);

    WING_SERVICE bool insertUInt64(WingHex::IWingPluginBase *sender,
                                   qsizetype offset, quint64 value);

    WING_SERVICE bool insertFloat(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, float value);

    WING_SERVICE bool insertDouble(WingHex::IWingPluginBase *sender,
                                   qsizetype offset, double value);

    WING_SERVICE bool insertString(WingHex::IWingPluginBase *sender,
                                   qsizetype offset, const QString &value,
                                   const QString &encoding);

    WING_SERVICE bool insertBytes(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, const QByteArray &data);

    WING_SERVICE bool appendInt8(WingHex::IWingPluginBase *sender, qint8 value);

    WING_SERVICE bool appendInt16(WingHex::IWingPluginBase *sender,
                                  qint16 value);

    WING_SERVICE bool appendInt32(WingHex::IWingPluginBase *sender,
                                  qint32 value);

    WING_SERVICE bool appendInt64(WingHex::IWingPluginBase *sender,
                                  qint64 value);

    WING_SERVICE bool appendUInt8(WingHex::IWingPluginBase *sender,
                                  quint8 value);

    WING_SERVICE bool appendUInt16(WingHex::IWingPluginBase *sender,
                                   quint16 value);

    WING_SERVICE bool appendUInt32(WingHex::IWingPluginBase *sender,
                                   quint32 value);

    WING_SERVICE bool appendUInt64(WingHex::IWingPluginBase *sender,
                                   quint64 value);

    WING_SERVICE bool appendFloat(WingHex::IWingPluginBase *sender,
                                  float value);

    WING_SERVICE bool appendDouble(WingHex::IWingPluginBase *sender,
                                   double value);

    WING_SERVICE bool appendString(WingHex::IWingPluginBase *sender,
                                   const QString &value,
                                   const QString &encoding);

    WING_SERVICE bool appendBytes(WingHex::IWingPluginBase *sender,
                                  const QByteArray &data);

    WING_SERVICE bool removeBytes(WingHex::IWingPluginBase *sender,
                                  qsizetype offset, qsizetype len);

    // cursor
    WING_SERVICE bool moveTo(WingHex::IWingPluginBase *sender, qsizetype line,
                             qsizetype column, int nibbleindex,
                             bool clearSelection);

    WING_SERVICE bool moveTo(WingHex::IWingPluginBase *sender, qsizetype offset,
                             bool clearSelection);

    WING_SERVICE bool select(WingHex::IWingPluginBase *sender, qsizetype offset,
                             qsizetype length, SelectionMode mode);

    WING_SERVICE bool setInsertionMode(WingHex::IWingPluginBase *sender,
                                       bool isinsert);

    WING_SERVICE bool metadata(WingHex::IWingPluginBase *sender,
                               qsizetype begin, qsizetype length,
                               const QColor &fgcolor, const QColor &bgcolor,
                               const QString &comment);

    WING_SERVICE bool removeMetadata(WingHex::IWingPluginBase *sender,
                                     qsizetype offset);

    WING_SERVICE bool clearMetadata(WingHex::IWingPluginBase *sender);

    WING_SERVICE bool setMetaVisible(WingHex::IWingPluginBase *sender, bool b);

    WING_SERVICE bool setMetafgVisible(WingHex::IWingPluginBase *sender,
                                       bool b);

    WING_SERVICE bool setMetabgVisible(WingHex::IWingPluginBase *sender,
                                       bool b);

    WING_SERVICE bool setMetaCommentVisible(WingHex::IWingPluginBase *sender,
                                            bool b);

    // mainwindow
    WING_SERVICE WingHex::ErrFile newFile(WingHex::IWingPluginBase *sender);

    WING_SERVICE WingHex::ErrFile openFile(WingHex::IWingPluginBase *sender,
                                           const QString &filename);

    WING_SERVICE WingHex::ErrFile openExtFile(WingHex::IWingPluginBase *sender,
                                              const QString &ext,
                                              const QString &file);

    WING_SERVICE WingHex::ErrFile
    openWorkSpace(WingHex::IWingPluginBase *sender, const QString &filename);

    WING_SERVICE WingHex::ErrFile closeHandle(WingHex::IWingPluginBase *sender,
                                              int handle);

    WING_SERVICE WingHex::ErrFile closeFile(WingHex::IWingPluginBase *sender,
                                            int handle, bool force);

    WING_SERVICE WingHex::ErrFile saveFile(WingHex::IWingPluginBase *sender,
                                           int handle);

    WING_SERVICE WingHex::ErrFile exportFile(WingHex::IWingPluginBase *sender,
                                             int handle,
                                             const QString &savename);

    WING_SERVICE WingHex::ErrFile saveAsFile(WingHex::IWingPluginBase *sender,
                                             int handle,
                                             const QString &savename);

    WING_SERVICE WingHex::ErrFile openCurrent(WingHex::IWingPluginBase *sender);

    WING_SERVICE WingHex::ErrFile closeCurrent(WingHex::IWingPluginBase *sender,
                                               bool force);

    WING_SERVICE WingHex::ErrFile saveCurrent(WingHex::IWingPluginBase *sender);

    WING_SERVICE WingHex::ErrFile
    exportCurrent(WingHex::IWingPluginBase *sender, const QString &savename);

    WING_SERVICE WingHex::ErrFile
    saveAsCurrent(WingHex::IWingPluginBase *sender, const QString &savename);

    // bookmark
    WING_SERVICE bool addBookMark(WingHex::IWingPluginBase *sender,
                                  qsizetype pos, const QString &comment);

    WING_SERVICE bool modBookMark(WingHex::IWingPluginBase *sender,
                                  qsizetype pos, const QString &comment);

    WING_SERVICE bool removeBookMark(WingHex::IWingPluginBase *sender,
                                     qsizetype pos);

    WING_SERVICE bool clearBookMark(WingHex::IWingPluginBase *sender);

    // extension
    WING_SERVICE bool closeAllFiles(WingHex::IWingPluginBase *sender);

public:
    WING_SERVICE bool dataVisualText(WingHex::IWingPluginBase *sender,
                                     const QString &data, const QString &title);

    WING_SERVICE bool dataVisualTextList(WingHex::IWingPluginBase *sender,
                                         const QStringList &data,
                                         const QString &title,
                                         WingHex::ClickedCallBack clicked,
                                         WingHex::ClickedCallBack dblClicked);

    WING_SERVICE bool dataVisualTextTree(WingHex::IWingPluginBase *sender,
                                         const QString &json,
                                         const QString &title,
                                         WingHex::ClickedCallBack clicked,
                                         WingHex::ClickedCallBack dblClicked);

    WING_SERVICE bool dataVisualTextTable(WingHex::IWingPluginBase *sender,
                                          const QString &json,
                                          const QStringList &headers,
                                          const QStringList &headerNames,
                                          const QString &title,
                                          WingHex::ClickedCallBack clicked,
                                          WingHex::ClickedCallBack dblClicked);

    // API for Qt Plugin Only
    WING_SERVICE bool
    dataVisualTextListByModel(WingHex::IWingPluginBase *sender,
                              QAbstractItemModel *model, const QString &title,
                              WingHex::ClickedCallBack clicked,
                              WingHex::ClickedCallBack dblClicked);

    WING_SERVICE bool
    dataVisualTextTableByModel(WingHex::IWingPluginBase *sender,
                               QAbstractItemModel *model, const QString &title,
                               WingHex::ClickedCallBack clicked,
                               WingHex::ClickedCallBack dblClicked);

    WING_SERVICE bool
    dataVisualTextTreeByModel(WingHex::IWingPluginBase *sender,
                              QAbstractItemModel *model, const QString &title,
                              WingHex::ClickedCallBack clicked,
                              WingHex::ClickedCallBack dblClicked);

private:
    WingHex::IWingPlugin *checkPluginAndReport(WingHex::IWingPluginBase *sender,
                                               const char *func);

    WingHex::IWingDevice *checkBaseAndReport(WingHex::IWingPluginBase *sender,
                                             const char *func);

    bool checkErrAllAllowAndReport(WingHex::IWingPluginBase *sender,
                                   const char *func);

private:
    CallTable _plgFns;

private:
    MainWindow *_win = nullptr;
    QHash<IWingPluginBase *, PluginInfo> _pinfos;
    QList<IWingPlugin *> _loadedplgs;
    QHash<QWidget *, ads::CDockWidget *> _raisedw;
    QStringList _lazyplgs;

    QList<IWingDevice *> _loadeddevs;

    QMap<IWingPlugin::RegisteredEvent, QList<IWingPlugin *>> _evplgs;

    QHash<IWingPlugin *, QVector<QSharedPointer<PluginFileContext>>>
        m_plgviewMap;
    QHash<IWingPlugin *, int> m_plgCurrentfid; // fid
    QHash<EditorView *, QList<IWingPlugin *>> m_viewBindings;

    UniqueIdGenerator m_idGen;

    QHash<QString, QHash<QString, WingAngelAPI::ScriptFnInfo>> _scfns;

    WingAngelAPI *_angelplg = nullptr;
    asCScriptEngine *_engine = nullptr;

    QStringList _scriptMarcos;
    QList<IWingPlugin *> _pragmaedPlg;

    QReadWriteLock _rwlock;

private:
    QString _curLoadingPlg;

    Q_DISABLE_COPY_MOVE(PluginSystem)
};

#endif // PLUGINSYSTEM_H
