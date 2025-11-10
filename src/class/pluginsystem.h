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
#include "WingPlugin/iwinghexeditorplugin.h"
#include "WingPlugin/iwingmanager.h"
#include "class/wingangelapi.h"
#include "control/editorview.h"

using namespace WingHex;

class MainWindow;
class asCScriptEngine;

// only plugin api can be declared with `public slot`

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
    enum class PluginStatus {
        Valid,
        SDKVersion,
        BrokenVersion,
        InvalidID,
        DupID,
        LackDependencies
    };

    enum class BlockReason { Disabled, BlockedByManager };

private:
    struct PluginFileContext {
        SharedUniqueId fid;
        EditorView *view = nullptr;
        IWingPlugin *linkedplg = nullptr;
    };

    struct PluginFile {
        QVector<QSharedPointer<PluginFileContext>> contexts;
        int currentFID = -1;
    };

    struct ViewBind {
        QList<IWingPlugin *> linkedplg;
        QList<QPair<QUndoCommand *, IWingPlugin *>> undoStackPlg;

        ~ViewBind() {
            if (!undoStackPlg.isEmpty()) {
                delete undoStackPlg.first().first;
            }
            undoStackPlg.clear();
        }
    };

public:
    static PluginSystem &instance();

    void setMainWindow(MainWindow *win);
    QWidget *mainWindow() const;

    void loadAllPlugins();
    void unloadAllPlugins();

    void doneRegisterScriptObj();

    void destory();

    const QList<IWingPlugin *> &plugins() const;
    IWingPlugin *plugin(qsizetype index) const;

    const QList<IWingDevice *> &devices() const;
    IWingDevice *device(qsizetype index) const;

    WingAngelAPI *angelApi() const;

    void cleanUpEditorViewHandle(EditorView *view);

    bool dispatchEvent(IWingPlugin::RegisteredEvent event,
                       const QVariantList &params);

    std::optional<WingHex::PragmaResult>
    processPragma(const QString &section, const QString &plgId,
                  const QStringList &params);

    IWingDevice *ext2Device(const QString &ext);

    QStringList scriptMarcos() const;

public:
    void scriptPragmaBegin();

    qsizetype pluginAPICount() const;

public:
    PluginInfo getPluginInfo(IWingPluginBase *plg) const;

    QString getPluginID(IWingPluginBase *plg) const;

    static QString getPUID(IWingPluginBase *p);

private:
    void loadExtPlugin();

    void loadDevicePlugin();

    void checkDirRootSafe(const QDir &dir);

    void try2LoadManagerPlugin();

    void try2LoadHexExtPlugin();

    template <typename T>
    std::optional<PluginInfo> loadPlugin(const QFileInfo &filename,
                                         const QDir &setdir);

    bool closeEditor(IWingPlugin *plg, int handle, bool force);

    bool closeHandle(IWingPlugin *plg, int handle);

    bool checkPluginCanOpenedFile(IWingPlugin *plg);

    bool checkPluginHasAlreadyOpened(IWingPlugin *plg, EditorView *view);

    EditorView *getCurrentPluginView(IWingPlugin *plg);

    EditorView *handle2EditorView(IWingPlugin *plg, int handle);

    int assginHandleForOpenPluginView(IWingPlugin *plg, EditorView *view);

    static bool equalCompareHandle(const SharedUniqueId &id, int handle);

    static int getUIDHandle(const SharedUniqueId &id);

private:
    PluginInfo parsePluginMetadata(const QJsonObject &meta);

    PluginStatus checkPluginMetadata(const PluginInfo &meta, bool isPlg);

    void retranslateMetadata(IWingPluginBase *plg, PluginInfo &meta);

private:
    void registerEvents(IWingPlugin *plg);

    void registerHexContextMenu(IWingHexEditorInterface *inter);

    void applyFunctionTables(QObject *plg, const CallTable &fns);

    bool isPluginLoaded(const WingDependency &d);

    bool isPluginLoaded(const QString &id);

    bool checkThreadAff();

    static QString packLogMessage(const char *header, const QString &msg);

    EditorView *pluginCurrentEditor(IWingPlugin *plg) const;

    QSharedPointer<PluginFileContext> pluginContextById(IWingPlugin *plg,
                                                        int fid) const;

    QUndoCommand *currentUndoCmd(EditorView *view);

private:
    void loadPlugin(IWingPlugin *p, PluginInfo &meta,
                    const std::optional<QDir> &setdir);
    void loadPlugin(IWingDevice *p, PluginInfo &meta,
                    const std::optional<QDir> &setdir);

private:
    void registerPluginDetectMarco(const QString &id);

private:
    void registerRibbonTools(const QList<WingRibbonToolBoxInfo> &tools);
    void registeredSettingPages(const QVariant &itptr,
                                const QList<SettingPage *> &pages);
    void registerPluginDockWidgets(IWingPluginBase *p);

public:
    // fpr crash checking
    QString currentLoadingPlugin() const;

    IWingManager *monitorManager() const;

    IWingHexEditorPlugin *hexEditorExtension() const;

    QMap<BlockReason, QList<PluginInfo>> blockedPlugins() const;

    QMap<BlockReason, QList<PluginInfo>> blockedDevPlugins() const;

    const std::optional<PluginInfo> &monitorManagerInfo() const;

    const std::optional<PluginInfo> &hexEditorExtensionInfo() const;

signals:
    void pluginLoading(const QString &plgName);

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

    // IWingPluginBase API
public slots:
    void toast(const QObject *sender, const QPixmap &icon,
               const QString &message);

    // logging
    void logTrace(const QObject *sender, const QString &message);
    void logDebug(const QObject *sender, const QString &message);
    void logInfo(const QObject *sender, const QString &message);
    void logWarn(const QObject *sender, const QString &message);
    void logError(const QObject *sender, const QString &message);
    bool raiseDockWidget(const QObject *sender, QWidget *w);

    WingHex::AppTheme currentAppTheme(const QObject *sender);
    QDialog *createDialog(const QObject *sender, QWidget *content);

public slots:
    void msgAboutQt(const QObject *sender, QWidget *parent,
                    const QString &title);
    QMessageBox::StandardButton
    msgInformation(const QObject *sender, QWidget *parent, const QString &title,
                   const QString &text, QMessageBox::StandardButtons buttons,
                   QMessageBox::StandardButton defaultButton);
    QMessageBox::StandardButton
    msgQuestion(const QObject *sender, QWidget *parent, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    QMessageBox::StandardButton
    msgWarning(const QObject *sender, QWidget *parent, const QString &title,
               const QString &text, QMessageBox::StandardButtons buttons,
               QMessageBox::StandardButton defaultButton);
    QMessageBox::StandardButton
    msgCritical(const QObject *sender, QWidget *parent, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    void msgAbout(const QObject *sender, QWidget *parent, const QString &title,
                  const QString &text);
    QMessageBox::StandardButton
    msgbox(const QObject *sender, QWidget *parent, QMessageBox::Icon icon,
           const QString &title, const QString &text,
           QMessageBox::StandardButtons buttons,
           QMessageBox::StandardButton defaultButton);

public slots:
    QString dlgGetText(const QObject *sender, QWidget *parent,
                       const QString &title, const QString &label,
                       QLineEdit::EchoMode echo, const QString &text, bool *ok,
                       Qt::InputMethodHints inputMethodHints);
    QString dlgGetMultiLineText(const QObject *sender, QWidget *parent,
                                const QString &title, const QString &label,
                                const QString &text, bool *ok,
                                Qt::InputMethodHints inputMethodHints);
    QString dlgGetItem(const QObject *sender, QWidget *parent,
                       const QString &title, const QString &label,
                       const QStringList &items, int current, bool editable,
                       bool *ok, Qt::InputMethodHints inputMethodHints);
    int dlgGetInt(const QObject *sender, QWidget *parent, const QString &title,
                  const QString &label, int value, int minValue, int maxValue,
                  int step, bool *ok);
    double dlgGetDouble(const QObject *sender, QWidget *parent,
                        const QString &title, const QString &label,
                        double value, double minValue, double maxValue,
                        int decimals, bool *ok, double step);

public slots:
    QString dlgGetExistingDirectory(const QObject *sender, QWidget *parent,
                                    const QString &caption, const QString &dir,
                                    QFileDialog::Options options);
    QString dlgGetOpenFileName(const QObject *sender, QWidget *parent,
                               const QString &caption, const QString &dir,
                               const QString &filter, QString *selectedFilter,
                               QFileDialog::Options options);
    QStringList dlgGetOpenFileNames(const QObject *sender, QWidget *parent,
                                    const QString &caption, const QString &dir,
                                    const QString &filter,
                                    QString *selectedFilter,
                                    QFileDialog::Options options);
    QString dlgGetSaveFileName(const QObject *sender, QWidget *parent,
                               const QString &caption, const QString &dir,
                               const QString &filter, QString *selectedFilter,
                               QFileDialog::Options options);

    QColor dlgGetColor(const QObject *sender, const QString &caption,
                       QWidget *parent);

    // IWingPlugin API
public slots:
    bool existsServiceHost(const QObject *sender, const QString &puid);

    bool invokeServiceImpl(const QObject *sender, const QString &puid,
                           const WingHex::MetaCallInfo &infos);

    bool isCurrentDocEditing(const QObject *sender);

    QString currentDocFile(const QObject *sender);

public slots:
    // document
    bool isReadOnly(const QObject *sender);

    bool isInsertionMode(const QObject *sender);

    bool isKeepSize(const QObject *sender);

    bool isLocked(const QObject *sender);

    qsizetype documentLines(const QObject *sender);

    qsizetype documentBytes(const QObject *sender);

    WingHex::HexPosition currentPos(const QObject *sender);

    qsizetype currentRow(const QObject *sender);

    qsizetype currentColumn(const QObject *sender);

    qsizetype currentOffset(const QObject *sender);

    qsizetype selectedLength(const QObject *sender);

    QByteArray selectedBytes(const QObject *sender, qsizetype index);

    QByteArrayList selectionBytes(const QObject *sender);

    WingHex::HexPosition selectionStart(const QObject *sender, qsizetype index);

    WingHex::HexPosition selectionEnd(const QObject *sender, qsizetype index);

    qsizetype selectionLength(const QObject *sender, qsizetype index);

    qsizetype selectionCount(const QObject *sender);

    bool stringVisible(const QObject *sender);

    bool addressVisible(const QObject *sender);

    bool headerVisible(const QObject *sender);

    quintptr addressBase(const QObject *sender);

    bool isModified(const QObject *sender);

    qint8 readInt8(const QObject *sender, qsizetype offset);

    qint16 readInt16(const QObject *sender, qsizetype offset);

    qint32 readInt32(const QObject *sender, qsizetype offset);

    qint64 readInt64(const QObject *sender, qsizetype offset);

    quint8 readUInt8(const QObject *sender, qsizetype offset);

    quint16 readUInt16(const QObject *sender, qsizetype offset);

    quint32 readUInt32(const QObject *sender, qsizetype offset);

    quint64 readUInt64(const QObject *sender, qsizetype offset);

    float readFloat(const QObject *sender, qsizetype offset);

    double readDouble(const QObject *sender, qsizetype offset);

    QString readString(const QObject *sender, qsizetype offset,
                       const QString &encoding);

    QByteArray readBytes(const QObject *sender, qsizetype offset,
                         qsizetype count);

    qsizetype findNext(const QObject *sender, qsizetype begin,
                       const QByteArray &ba);

    qsizetype findPrevious(const QObject *sender, qsizetype begin,
                           const QByteArray &ba);

    QString bookMarkComment(const QObject *sender, qsizetype pos);

    bool existBookMark(const QObject *sender, qsizetype pos);

    // document
    bool switchDocument(const QObject *sender, int handle);

    bool raiseDocument(const QObject *sender, int handle);

    bool setLockedFile(const QObject *sender, bool b);

    bool setKeepSize(const QObject *sender, bool b);

    bool setStringVisible(const QObject *sender, bool b);

    bool setAddressVisible(const QObject *sender, bool b);

    bool setHeaderVisible(const QObject *sender, bool b);

    bool setAddressBase(const QObject *sender, quintptr base);

    bool beginMarco(const QObject *sender, const QString &txt);

    bool endMarco(const QObject *sender);

    bool isMacroEmpty(const QObject *sender);

    bool resetMarco(const QObject *sender);

    bool writeInt8(const QObject *sender, qsizetype offset, qint8 value);

    bool writeInt16(const QObject *sender, qsizetype offset, qint16 value);

    bool writeInt32(const QObject *sender, qsizetype offset, qint32 value);

    bool writeInt64(const QObject *sender, qsizetype offset, qint64 value);

    bool writeUInt8(const QObject *sender, qsizetype offset, quint8 value);

    bool writeUInt16(const QObject *sender, qsizetype offset, quint16 value);

    bool writeUInt32(const QObject *sender, qsizetype offset, quint32 value);

    bool writeUInt64(const QObject *sender, qsizetype offset, quint64 value);

    bool writeFloat(const QObject *sender, qsizetype offset, float value);

    bool writeDouble(const QObject *sender, qsizetype offset, double value);

    bool writeString(const QObject *sender, qsizetype offset,
                     const QString &value, const QString &encoding);

    bool writeBytes(const QObject *sender, qsizetype offset,
                    const QByteArray &data);

    bool insertInt8(const QObject *sender, qsizetype offset, qint8 value);

    bool insertInt16(const QObject *sender, qsizetype offset, qint16 value);

    bool insertInt32(const QObject *sender, qsizetype offset, qint32 value);

    bool insertInt64(const QObject *sender, qsizetype offset, qint64 value);

    bool insertUInt8(const QObject *sender, qsizetype offset, quint8 value);

    bool insertUInt16(const QObject *sender, qsizetype offset, quint16 value);

    bool insertUInt32(const QObject *sender, qsizetype offset, quint32 value);

    bool insertUInt64(const QObject *sender, qsizetype offset, quint64 value);

    bool insertFloat(const QObject *sender, qsizetype offset, float value);

    bool insertDouble(const QObject *sender, qsizetype offset, double value);

    bool insertString(const QObject *sender, qsizetype offset,
                      const QString &value, const QString &encoding);

    bool insertBytes(const QObject *sender, qsizetype offset,
                     const QByteArray &data);

    bool appendInt8(const QObject *sender, qint8 value);

    bool appendInt16(const QObject *sender, qint16 value);

    bool appendInt32(const QObject *sender, qint32 value);

    bool appendInt64(const QObject *sender, qint64 value);

    bool appendUInt8(const QObject *sender, quint8 value);

    bool appendUInt16(const QObject *sender, quint16 value);

    bool appendUInt32(const QObject *sender, quint32 value);

    bool appendUInt64(const QObject *sender, quint64 value);

    bool appendFloat(const QObject *sender, float value);

    bool appendDouble(const QObject *sender, double value);

    bool appendString(const QObject *sender, const QString &value,
                      const QString &encoding);

    bool appendBytes(const QObject *sender, const QByteArray &data);

    bool removeBytes(const QObject *sender, qsizetype offset, qsizetype len);

    // cursor
    bool moveTo(const QObject *sender, qsizetype line, qsizetype column,
                int nibbleindex, bool clearSelection);

    bool moveTo(const QObject *sender, qsizetype offset, bool clearSelection);

    bool select(const QObject *sender, qsizetype offset, qsizetype length,
                WingHex::SelectionMode mode);

    bool setInsertionMode(const QObject *sender, bool isinsert);

    bool metadata(const QObject *sender, qsizetype begin, qsizetype length,
                  const QColor &fgcolor, const QColor &bgcolor,
                  const QString &comment);

    bool removeMetadata(const QObject *sender, qsizetype offset);

    bool clearMetadata(const QObject *sender);

    bool setMetaVisible(const QObject *sender, bool b);

    bool setMetafgVisible(const QObject *sender, bool b);

    bool setMetabgVisible(const QObject *sender, bool b);

    bool setMetaCommentVisible(const QObject *sender, bool b);

public slots:
    // mainwindow
    int newFile(const QObject *sender);

    int openFile(const QObject *sender, const QString &filename);

    int openExtFile(const QObject *sender, const QString &ext,
                    const QString &file);

    int openWorkSpace(const QObject *sender, const QString &filename);

    WingHex::ErrFile closeHandle(const QObject *sender, int handle);

    WingHex::ErrFile closeFile(const QObject *sender, int handle, bool force);

    WingHex::ErrFile saveFile(const QObject *sender, int handle);

    WingHex::ErrFile exportFile(const QObject *sender, int handle,
                                const QString &savename);

    WingHex::ErrFile saveAsFile(const QObject *sender, int handle,
                                const QString &savename);

    int openCurrent(const QObject *sender);

    WingHex::ErrFile closeCurrent(const QObject *sender, bool force);

    WingHex::ErrFile saveCurrent(const QObject *sender);

    WingHex::ErrFile exportCurrent(const QObject *sender,
                                   const QString &savename);

    WingHex::ErrFile saveAsCurrent(const QObject *sender,
                                   const QString &savename);

public slots:
    // bookmark
    bool addBookMark(const QObject *sender, qsizetype pos,
                     const QString &comment);

    bool modBookMark(const QObject *sender, qsizetype pos,
                     const QString &comment);

    bool removeBookMark(const QObject *sender, qsizetype pos);

    bool clearBookMark(const QObject *sender);

    // extension
    bool closeAllFiles(const QObject *sender);

    // generic call support
    WingHex::IWingGeneric *__createParamContext(const QObject *sender,
                                                void *ctx);

    void __raiseContextException(const QObject *sender,
                                 const QString &exception, bool allowCatch);

private:
    WingHex::IWingPlugin *checkPluginAndReport(const QObject *sender,
                                               const char *func);

public:
    inline static QVariant to_variant(const QVariant &v) { return v; }

    template <typename T>
    inline static QVariant to_variant(T &&v) {
        return QVariant::fromValue(std::forward<T>(v));
    }

    template <typename... Args>
    inline static QVariantList makeVariantList(Args &&...args) {
        QVariantList list;
        list.reserve(sizeof...(Args));
        (list.append(to_variant(std::forward<Args>(args))), ...);
        return list;
    }

    template <typename FuncT, typename... GivenArgs>
    inline bool passByFailedGuard(const QObject *sender, const char *func,
                                  GivenArgs &&...givenArgs) {
        using FnInfo = QtPrivate::FunctionPointer<FuncT>;
        using GivenArgsList =
            QtPrivate::List<typename QtPrivate::RemoveRef<GivenArgs>::Type...>;
        static_assert(FnInfo::ArgumentCount - 1 == GivenArgsList::size,
                      "argument count mismatch");
        static_assert(
            QtPrivate::CheckCompatibleArguments<typename FnInfo::Arguments::Cdr,
                                                GivenArgsList>::value,
            "argument types are not compatible with function signature");

        auto params = makeVariantList(std::forward<GivenArgs>(givenArgs)...);
        return __passByFailedGuard(sender, func, params);
    }

    bool __passByFailedGuard(const QObject *sender, const char *func,
                             const QVariantList &params);

private:
    CallTable _plgFns;

private:
    MainWindow *_win = nullptr;
    QHash<IWingPluginBase *, PluginInfo> _pinfos;
    QList<IWingPlugin *> _loadedplgs;
    QHash<QWidget *, ads::CDockWidget *> _raisedw;
    QStringList _lazyplgs;

    QList<IWingDevice *> _loadeddevs;

    QStringList _enabledExtIDs;
    QStringList _enabledDevIDs;
    QMap<BlockReason, QList<PluginInfo>> _blkplgs;
    QMap<BlockReason, QList<PluginInfo>> _blkdevs;

    QMap<IWingPlugin::RegisteredEvent, QList<IWingPlugin *>> _evplgs;

    QHash<IWingPlugin *, PluginFile> m_plgviewMap;
    QHash<EditorView *, ViewBind> m_viewBindings;

    UniqueIdGenerator m_idGen;

    IWingManager *_manager = nullptr;
    std::optional<PluginInfo> _manInfo;

    IWingHexEditorPlugin *_hexExt = nullptr;
    std::optional<PluginInfo> _manHexInfo;

    WingAngelAPI *_angelplg = nullptr;

    QStringList _scriptMarcos;
    QList<IWingPlugin *> _pragmaedPlg;

private:
    QString _curLoadingPlg;

    Q_DISABLE_COPY_MOVE(PluginSystem)
};

#endif // PLUGINSYSTEM_H
