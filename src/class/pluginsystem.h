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
    // fpr crash checking
    QString currentLoadingPlugin() const;

private:
    template <typename T>
    T readBasicTypeContent(IWingPlugin *plg, qsizetype offset) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return EditorView::readBasicTypeContent<T>(e, offset, _rwlock);
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
            return EditorView::insertBasicTypeContent<T>(
                e, offset, value, pluginCurrentUndoCmd(plg), _rwlock);
        }
        return false;
    }

    template <typename T>
    bool writeBasicTypeContent(IWingPlugin *plg, qsizetype offset,
                               const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = getCurrentPluginView(plg);
        if (e) {
            return EditorView::writeBasicTypeContent<T>(
                e, offset, value, pluginCurrentUndoCmd(plg), _rwlock);
        }
        return false;
    }

    template <typename T>
    bool appendBasicTypeContent(IWingPlugin *plg, const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = getCurrentPluginView(plg);
        if (e) {
            return EditorView::appendBasicTypeContent<T>(
                e, value, pluginCurrentUndoCmd(plg), _rwlock);
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
    WING_SERVICE void toast(QObject *sender, const QPixmap &icon,
                            const QString &message);

    // logging
    WING_SERVICE void logTrace(QObject *sender, const QString &message);
    WING_SERVICE void logDebug(QObject *sender, const QString &message);
    WING_SERVICE void logInfo(QObject *sender, const QString &message);
    WING_SERVICE void logWarn(QObject *sender, const QString &message);
    WING_SERVICE void logError(QObject *sender, const QString &message);
    WING_SERVICE bool raiseDockWidget(QObject *sender, QWidget *w);

    WING_SERVICE WingHex::AppTheme currentAppTheme(QObject *sender);
    WING_SERVICE QDialog *createDialog(QObject *sender, QWidget *content);

public:
    WING_SERVICE void msgAboutQt(QObject *sender, QWidget *parent,
                                 const QString &title);
    WING_SERVICE QMessageBox::StandardButton
    msgInformation(QObject *sender, QWidget *parent, const QString &title,
                   const QString &text, QMessageBox::StandardButtons buttons,
                   QMessageBox::StandardButton defaultButton);
    WING_SERVICE QMessageBox::StandardButton
    msgQuestion(QObject *sender, QWidget *parent, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    WING_SERVICE QMessageBox::StandardButton
    msgWarning(QObject *sender, QWidget *parent, const QString &title,
               const QString &text, QMessageBox::StandardButtons buttons,
               QMessageBox::StandardButton defaultButton);
    WING_SERVICE QMessageBox::StandardButton
    msgCritical(QObject *sender, QWidget *parent, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    WING_SERVICE void msgAbout(QObject *sender, QWidget *parent,
                               const QString &title, const QString &text);
    WING_SERVICE QMessageBox::StandardButton
    msgbox(QObject *sender, QWidget *parent, QMessageBox::Icon icon,
           const QString &title, const QString &text,
           QMessageBox::StandardButtons buttons,
           QMessageBox::StandardButton defaultButton);

public:
    WING_SERVICE QString dlgGetText(QObject *sender, QWidget *parent,
                                    const QString &title, const QString &label,
                                    QLineEdit::EchoMode echo,
                                    const QString &text, bool *ok,
                                    Qt::InputMethodHints inputMethodHints);
    WING_SERVICE QString
    dlgGetMultiLineText(QObject *sender, QWidget *parent, const QString &title,
                        const QString &label, const QString &text, bool *ok,
                        Qt::InputMethodHints inputMethodHints);
    WING_SERVICE QString dlgGetItem(QObject *sender, QWidget *parent,
                                    const QString &title, const QString &label,
                                    const QStringList &items, int current,
                                    bool editable, bool *ok,
                                    Qt::InputMethodHints inputMethodHints);
    WING_SERVICE int dlgGetInt(QObject *sender, QWidget *parent,
                               const QString &title, const QString &label,
                               int value, int minValue, int maxValue, int step,
                               bool *ok);
    WING_SERVICE double dlgGetDouble(QObject *sender, QWidget *parent,
                                     const QString &title, const QString &label,
                                     double value, double minValue,
                                     double maxValue, int decimals, bool *ok,
                                     double step);

public:
    WING_SERVICE QString dlgGetExistingDirectory(QObject *sender,
                                                 QWidget *parent,
                                                 const QString &caption,
                                                 const QString &dir,
                                                 QFileDialog::Options options);
    WING_SERVICE QString dlgGetOpenFileName(QObject *sender, QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            const QString &filter,
                                            QString *selectedFilter,
                                            QFileDialog::Options options);
    WING_SERVICE QStringList dlgGetOpenFileNames(
        QObject *sender, QWidget *parent, const QString &caption,
        const QString &dir, const QString &filter, QString *selectedFilter,
        QFileDialog::Options options);
    WING_SERVICE QString dlgGetSaveFileName(QObject *sender, QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            const QString &filter,
                                            QString *selectedFilter,
                                            QFileDialog::Options options);

    WING_SERVICE QColor dlgGetColor(QObject *sender, const QString &caption,
                                    QWidget *parent);

    // IWingPlugin API
public:
    WING_SERVICE bool existsServiceHost(QObject *sender, const QString &puid);

    WING_SERVICE bool
    invokeService(QObject *sender, const QString &puid, const char *method,
                  Qt::ConnectionType type, QGenericReturnArgument ret,
                  QGenericArgument val0, QGenericArgument val1,
                  QGenericArgument val2, QGenericArgument val3,
                  QGenericArgument val4);
    bool invokeService(QObject *sender, const QString &selfpuid,
                       const QString &puid, const char *method,
                       Qt::ConnectionType type, QGenericReturnArgument ret,
                       QGenericArgument val0, QGenericArgument val1,
                       QGenericArgument val2, QGenericArgument val3,
                       QGenericArgument val4);

    WING_SERVICE bool isCurrentDocEditing(QObject *sender);

    WING_SERVICE QString currentDocFilename(QObject *sender);

    // document
    WING_SERVICE bool isReadOnly(QObject *sender);

    WING_SERVICE bool isInsertionMode(QObject *sender);

    WING_SERVICE bool isKeepSize(QObject *sender);

    WING_SERVICE bool isLocked(QObject *sender);

    WING_SERVICE qsizetype documentLines(QObject *sender);

    WING_SERVICE qsizetype documentBytes(QObject *sender);

    WING_SERVICE WingHex::HexPosition currentPos(QObject *sender);

    WING_SERVICE qsizetype currentRow(QObject *sender);

    WING_SERVICE qsizetype currentColumn(QObject *sender);

    WING_SERVICE qsizetype currentOffset(QObject *sender);

    WING_SERVICE qsizetype selectedLength(QObject *sender);

    WING_SERVICE QByteArray selectedBytes(QObject *sender, qsizetype index);

    WING_SERVICE QByteArrayList selectionBytes(QObject *sender);

    WING_SERVICE WingHex::HexPosition selectionStart(QObject *sender,
                                                     qsizetype index);

    WING_SERVICE WingHex::HexPosition selectionEnd(QObject *sender,
                                                   qsizetype index);

    WING_SERVICE qsizetype selectionLength(QObject *sender, qsizetype index);

    WING_SERVICE qsizetype selectionCount(QObject *sender);

    WING_SERVICE bool stringVisible(QObject *sender);

    WING_SERVICE bool addressVisible(QObject *sender);

    WING_SERVICE bool headerVisible(QObject *sender);

    WING_SERVICE quintptr addressBase(QObject *sender);

    WING_SERVICE bool isModified(QObject *sender);

    WING_SERVICE qint8 readInt8(QObject *sender, qsizetype offset);

    WING_SERVICE qint16 readInt16(QObject *sender, qsizetype offset);

    WING_SERVICE qint32 readInt32(QObject *sender, qsizetype offset);

    WING_SERVICE qint64 readInt64(QObject *sender, qsizetype offset);

    WING_SERVICE quint8 readUInt8(QObject *sender, qsizetype offset);

    WING_SERVICE quint16 readUInt16(QObject *sender, qsizetype offset);

    WING_SERVICE quint32 readUInt32(QObject *sender, qsizetype offset);

    WING_SERVICE quint64 readUInt64(QObject *sender, qsizetype offset);

    WING_SERVICE float readFloat(QObject *sender, qsizetype offset);

    WING_SERVICE double readDouble(QObject *sender, qsizetype offset);

    WING_SERVICE QString readString(QObject *sender, qsizetype offset,
                                    const QString &encoding);

    WING_SERVICE QByteArray readBytes(QObject *sender, qsizetype offset,
                                      qsizetype count);

    WING_SERVICE qsizetype findNext(QObject *sender, qsizetype begin,
                                    const QByteArray &ba);

    WING_SERVICE qsizetype findPrevious(QObject *sender, qsizetype begin,
                                        const QByteArray &ba);

    WING_SERVICE QString bookMarkComment(QObject *sender, qsizetype pos);

    WING_SERVICE bool existBookMark(QObject *sender, qsizetype pos);

    // document
    WING_SERVICE bool switchDocument(QObject *sender, int handle);

    WING_SERVICE bool raiseDocument(QObject *sender, int handle);

    WING_SERVICE bool setLockedFile(QObject *sender, bool b);

    WING_SERVICE bool setKeepSize(QObject *sender, bool b);

    WING_SERVICE bool setStringVisible(QObject *sender, bool b);

    WING_SERVICE bool setAddressVisible(QObject *sender, bool b);

    WING_SERVICE bool setHeaderVisible(QObject *sender, bool b);

    WING_SERVICE bool setAddressBase(QObject *sender, quintptr base);

    WING_SERVICE bool beginMarco(QObject *sender, const QString &txt);

    WING_SERVICE bool endMarco(QObject *sender);

    WING_SERVICE bool writeInt8(QObject *sender, qsizetype offset, qint8 value);

    WING_SERVICE bool writeInt16(QObject *sender, qsizetype offset,
                                 qint16 value);

    WING_SERVICE bool writeInt32(QObject *sender, qsizetype offset,
                                 qint32 value);

    WING_SERVICE bool writeInt64(QObject *sender, qsizetype offset,
                                 qint64 value);

    WING_SERVICE bool writeUInt8(QObject *sender, qsizetype offset,
                                 quint8 value);

    WING_SERVICE bool writeUInt16(QObject *sender, qsizetype offset,
                                  quint16 value);

    WING_SERVICE bool writeUInt32(QObject *sender, qsizetype offset,
                                  quint32 value);

    WING_SERVICE bool writeUInt64(QObject *sender, qsizetype offset,
                                  quint64 value);

    WING_SERVICE bool writeFloat(QObject *sender, qsizetype offset,
                                 float value);

    WING_SERVICE bool writeDouble(QObject *sender, qsizetype offset,
                                  double value);

    WING_SERVICE bool writeString(QObject *sender, qsizetype offset,
                                  const QString &value,
                                  const QString &encoding);

    WING_SERVICE bool writeBytes(QObject *sender, qsizetype offset,
                                 const QByteArray &data);

    WING_SERVICE bool insertInt8(QObject *sender, qsizetype offset,
                                 qint8 value);

    WING_SERVICE bool insertInt16(QObject *sender, qsizetype offset,
                                  qint16 value);

    WING_SERVICE bool insertInt32(QObject *sender, qsizetype offset,
                                  qint32 value);

    WING_SERVICE bool insertInt64(QObject *sender, qsizetype offset,
                                  qint64 value);

    WING_SERVICE bool insertUInt8(QObject *sender, qsizetype offset,
                                  quint8 value);

    WING_SERVICE bool insertUInt16(QObject *sender, qsizetype offset,
                                   quint16 value);

    WING_SERVICE bool insertUInt32(QObject *sender, qsizetype offset,
                                   quint32 value);

    WING_SERVICE bool insertUInt64(QObject *sender, qsizetype offset,
                                   quint64 value);

    WING_SERVICE bool insertFloat(QObject *sender, qsizetype offset,
                                  float value);

    WING_SERVICE bool insertDouble(QObject *sender, qsizetype offset,
                                   double value);

    WING_SERVICE bool insertString(QObject *sender, qsizetype offset,
                                   const QString &value,
                                   const QString &encoding);

    WING_SERVICE bool insertBytes(QObject *sender, qsizetype offset,
                                  const QByteArray &data);

    WING_SERVICE bool appendInt8(QObject *sender, qint8 value);

    WING_SERVICE bool appendInt16(QObject *sender, qint16 value);

    WING_SERVICE bool appendInt32(QObject *sender, qint32 value);

    WING_SERVICE bool appendInt64(QObject *sender, qint64 value);

    WING_SERVICE bool appendUInt8(QObject *sender, quint8 value);

    WING_SERVICE bool appendUInt16(QObject *sender, quint16 value);

    WING_SERVICE bool appendUInt32(QObject *sender, quint32 value);

    WING_SERVICE bool appendUInt64(QObject *sender, quint64 value);

    WING_SERVICE bool appendFloat(QObject *sender, float value);

    WING_SERVICE bool appendDouble(QObject *sender, double value);

    WING_SERVICE bool appendString(QObject *sender, const QString &value,
                                   const QString &encoding);

    WING_SERVICE bool appendBytes(QObject *sender, const QByteArray &data);

    WING_SERVICE bool removeBytes(QObject *sender, qsizetype offset,
                                  qsizetype len);

    // cursor
    WING_SERVICE bool moveTo(QObject *sender, qsizetype line, qsizetype column,
                             int nibbleindex, bool clearSelection);

    WING_SERVICE bool moveTo(QObject *sender, qsizetype offset,
                             bool clearSelection);

    WING_SERVICE bool select(QObject *sender, qsizetype offset,
                             qsizetype length, SelectionMode mode);

    WING_SERVICE bool setInsertionMode(QObject *sender, bool isinsert);

    WING_SERVICE bool metadata(QObject *sender, qsizetype begin,
                               qsizetype length, const QColor &fgcolor,
                               const QColor &bgcolor, const QString &comment);

    WING_SERVICE bool removeMetadata(QObject *sender, qsizetype offset);

    WING_SERVICE bool clearMetadata(QObject *sender);

    WING_SERVICE bool setMetaVisible(QObject *sender, bool b);

    WING_SERVICE bool setMetafgVisible(QObject *sender, bool b);

    WING_SERVICE bool setMetabgVisible(QObject *sender, bool b);

    WING_SERVICE bool setMetaCommentVisible(QObject *sender, bool b);

    // mainwindow
    WING_SERVICE WingHex::ErrFile newFile(QObject *sender);

    WING_SERVICE WingHex::ErrFile openFile(QObject *sender,
                                           const QString &filename);

    WING_SERVICE WingHex::ErrFile
    openExtFile(QObject *sender, const QString &ext, const QString &file);

    WING_SERVICE WingHex::ErrFile openWorkSpace(QObject *sender,
                                                const QString &filename);

    WING_SERVICE WingHex::ErrFile closeHandle(QObject *sender, int handle);

    WING_SERVICE WingHex::ErrFile closeFile(QObject *sender, int handle,
                                            bool force);

    WING_SERVICE WingHex::ErrFile saveFile(QObject *sender, int handle);

    WING_SERVICE WingHex::ErrFile exportFile(QObject *sender, int handle,
                                             const QString &savename);

    WING_SERVICE WingHex::ErrFile saveAsFile(QObject *sender, int handle,
                                             const QString &savename);

    WING_SERVICE WingHex::ErrFile openCurrent(QObject *sender);

    WING_SERVICE WingHex::ErrFile closeCurrent(QObject *sender, bool force);

    WING_SERVICE WingHex::ErrFile saveCurrent(QObject *sender);

    WING_SERVICE WingHex::ErrFile exportCurrent(QObject *sender,
                                                const QString &savename);

    WING_SERVICE WingHex::ErrFile saveAsCurrent(QObject *sender,
                                                const QString &savename);

    // bookmark
    WING_SERVICE bool addBookMark(QObject *sender, qsizetype pos,
                                  const QString &comment);

    WING_SERVICE bool modBookMark(QObject *sender, qsizetype pos,
                                  const QString &comment);

    WING_SERVICE bool removeBookMark(QObject *sender, qsizetype pos);

    WING_SERVICE bool clearBookMark(QObject *sender);

    // extension
    WING_SERVICE bool closeAllFiles(QObject *sender);

private:
    WingHex::IWingPlugin *checkPluginAndReport(QObject *sender,
                                               const char *func);

    WingHex::IWingDevice *checkBaseAndReport(QObject *sender, const char *func);

    bool checkErrAllAllowAndReport(QObject *sender, const char *func);

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
