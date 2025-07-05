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
#include "WingPlugin/iwingmanager.h"
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
    enum class PluginStatus {
        Valid,
        SDKVersion,
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

    void loadAllPlugin();
    void unloadAllPlugin();

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

    const std::optional<PluginInfo> &monitorManagerInfo() const;

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
                e, offset, value, currentUndoCmd(e), _rwlock);
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
                e, offset, value, currentUndoCmd(e), _rwlock);
        }
        return false;
    }

    template <typename T>
    bool appendBasicTypeContent(IWingPlugin *plg, const T &value) {
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);
        auto e = getCurrentPluginView(plg);
        if (e) {
            return EditorView::appendBasicTypeContent<T>(
                e, value, currentUndoCmd(e), _rwlock);
        }
        return false;
    }

signals:
    void pluginLoading(const QString &plgName);

private:
    PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

    // IWingPluginBase API
public slots:
    WING_API void toast(const QObject *sender, const QPixmap &icon,
                        const QString &message);

    // logging
    WING_API void logTrace(const QObject *sender, const QString &message);
    WING_API void logDebug(const QObject *sender, const QString &message);
    WING_API void logInfo(const QObject *sender, const QString &message);
    WING_API void logWarn(const QObject *sender, const QString &message);
    WING_API void logError(const QObject *sender, const QString &message);
    WING_API bool raiseDockWidget(const QObject *sender, QWidget *w);

    WING_API WingHex::AppTheme currentAppTheme(const QObject *sender);
    WING_API QDialog *createDialog(const QObject *sender, QWidget *content);

public slots:
    WING_API void msgAboutQt(const QObject *sender, QWidget *parent,
                             const QString &title);
    WING_API QMessageBox::StandardButton
    msgInformation(const QObject *sender, QWidget *parent, const QString &title,
                   const QString &text, QMessageBox::StandardButtons buttons,
                   QMessageBox::StandardButton defaultButton);
    WING_API QMessageBox::StandardButton
    msgQuestion(const QObject *sender, QWidget *parent, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    WING_API QMessageBox::StandardButton
    msgWarning(const QObject *sender, QWidget *parent, const QString &title,
               const QString &text, QMessageBox::StandardButtons buttons,
               QMessageBox::StandardButton defaultButton);
    WING_API QMessageBox::StandardButton
    msgCritical(const QObject *sender, QWidget *parent, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton);
    WING_API void msgAbout(const QObject *sender, QWidget *parent,
                           const QString &title, const QString &text);
    WING_API QMessageBox::StandardButton
    msgbox(const QObject *sender, QWidget *parent, QMessageBox::Icon icon,
           const QString &title, const QString &text,
           QMessageBox::StandardButtons buttons,
           QMessageBox::StandardButton defaultButton);

public slots:
    WING_API QString dlgGetText(const QObject *sender, QWidget *parent,
                                const QString &title, const QString &label,
                                QLineEdit::EchoMode echo, const QString &text,
                                bool *ok,
                                Qt::InputMethodHints inputMethodHints);
    WING_API QString dlgGetMultiLineText(const QObject *sender, QWidget *parent,
                                         const QString &title,
                                         const QString &label,
                                         const QString &text, bool *ok,
                                         Qt::InputMethodHints inputMethodHints);
    WING_API QString dlgGetItem(const QObject *sender, QWidget *parent,
                                const QString &title, const QString &label,
                                const QStringList &items, int current,
                                bool editable, bool *ok,
                                Qt::InputMethodHints inputMethodHints);
    WING_API int dlgGetInt(const QObject *sender, QWidget *parent,
                           const QString &title, const QString &label,
                           int value, int minValue, int maxValue, int step,
                           bool *ok);
    WING_API double dlgGetDouble(const QObject *sender, QWidget *parent,
                                 const QString &title, const QString &label,
                                 double value, double minValue, double maxValue,
                                 int decimals, bool *ok, double step);

public slots:
    WING_API QString dlgGetExistingDirectory(const QObject *sender,
                                             QWidget *parent,
                                             const QString &caption,
                                             const QString &dir,
                                             QFileDialog::Options options);
    WING_API QString dlgGetOpenFileName(const QObject *sender, QWidget *parent,
                                        const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options);
    WING_API QStringList dlgGetOpenFileNames(
        const QObject *sender, QWidget *parent, const QString &caption,
        const QString &dir, const QString &filter, QString *selectedFilter,
        QFileDialog::Options options);
    WING_API QString dlgGetSaveFileName(const QObject *sender, QWidget *parent,
                                        const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options);

    WING_API QColor dlgGetColor(const QObject *sender, const QString &caption,
                                QWidget *parent);

    // IWingPlugin API
public slots:
    WING_API bool existsServiceHost(const QObject *sender, const QString &puid);

    WING_API bool invokeServiceImpl(const QObject *sender, const QString &puid,
                                    const WingHex::MetaCallInfo &infos);

    WING_API bool isCurrentDocEditing(const QObject *sender);

    WING_API QString currentDocFilename(const QObject *sender);

    // document
    WING_API bool isReadOnly(const QObject *sender);

    WING_API bool isInsertionMode(const QObject *sender);

    WING_API bool isKeepSize(const QObject *sender);

    WING_API bool isLocked(const QObject *sender);

    WING_API qsizetype documentLines(const QObject *sender);

    WING_API qsizetype documentBytes(const QObject *sender);

    WING_API WingHex::HexPosition currentPos(const QObject *sender);

    WING_API qsizetype currentRow(const QObject *sender);

    WING_API qsizetype currentColumn(const QObject *sender);

    WING_API qsizetype currentOffset(const QObject *sender);

    WING_API qsizetype selectedLength(const QObject *sender);

    WING_API QByteArray selectedBytes(const QObject *sender, qsizetype index);

    WING_API QByteArrayList selectionBytes(const QObject *sender);

    WING_API WingHex::HexPosition selectionStart(const QObject *sender,
                                                 qsizetype index);

    WING_API WingHex::HexPosition selectionEnd(const QObject *sender,
                                               qsizetype index);

    WING_API qsizetype selectionLength(const QObject *sender, qsizetype index);

    WING_API qsizetype selectionCount(const QObject *sender);

    WING_API bool stringVisible(const QObject *sender);

    WING_API bool addressVisible(const QObject *sender);

    WING_API bool headerVisible(const QObject *sender);

    WING_API quintptr addressBase(const QObject *sender);

    WING_API bool isModified(const QObject *sender);

    WING_API qint8 readInt8(const QObject *sender, qsizetype offset);

    WING_API qint16 readInt16(const QObject *sender, qsizetype offset);

    WING_API qint32 readInt32(const QObject *sender, qsizetype offset);

    WING_API qint64 readInt64(const QObject *sender, qsizetype offset);

    WING_API quint8 readUInt8(const QObject *sender, qsizetype offset);

    WING_API quint16 readUInt16(const QObject *sender, qsizetype offset);

    WING_API quint32 readUInt32(const QObject *sender, qsizetype offset);

    WING_API quint64 readUInt64(const QObject *sender, qsizetype offset);

    WING_API float readFloat(const QObject *sender, qsizetype offset);

    WING_API double readDouble(const QObject *sender, qsizetype offset);

    WING_API QString readString(const QObject *sender, qsizetype offset,
                                const QString &encoding);

    WING_API QByteArray readBytes(const QObject *sender, qsizetype offset,
                                  qsizetype count);

    WING_API qsizetype findNext(const QObject *sender, qsizetype begin,
                                const QByteArray &ba);

    WING_API qsizetype findPrevious(const QObject *sender, qsizetype begin,
                                    const QByteArray &ba);

    WING_API QString bookMarkComment(const QObject *sender, qsizetype pos);

    WING_API bool existBookMark(const QObject *sender, qsizetype pos);

    // document
    WING_API bool switchDocument(const QObject *sender, int handle);

    WING_API bool raiseDocument(const QObject *sender, int handle);

    WING_API bool setLockedFile(const QObject *sender, bool b);

    WING_API bool setKeepSize(const QObject *sender, bool b);

    WING_API bool setStringVisible(const QObject *sender, bool b);

    WING_API bool setAddressVisible(const QObject *sender, bool b);

    WING_API bool setHeaderVisible(const QObject *sender, bool b);

    WING_API bool setAddressBase(const QObject *sender, quintptr base);

    WING_API bool beginMarco(const QObject *sender, const QString &txt);

    WING_API bool endMarco(const QObject *sender);

    WING_API bool isMacroEmpty(const QObject *sender);

    WING_API bool resetMarco(const QObject *sender);

    WING_API bool writeInt8(const QObject *sender, qsizetype offset,
                            qint8 value);

    WING_API bool writeInt16(const QObject *sender, qsizetype offset,
                             qint16 value);

    WING_API bool writeInt32(const QObject *sender, qsizetype offset,
                             qint32 value);

    WING_API bool writeInt64(const QObject *sender, qsizetype offset,
                             qint64 value);

    WING_API bool writeUInt8(const QObject *sender, qsizetype offset,
                             quint8 value);

    WING_API bool writeUInt16(const QObject *sender, qsizetype offset,
                              quint16 value);

    WING_API bool writeUInt32(const QObject *sender, qsizetype offset,
                              quint32 value);

    WING_API bool writeUInt64(const QObject *sender, qsizetype offset,
                              quint64 value);

    WING_API bool writeFloat(const QObject *sender, qsizetype offset,
                             float value);

    WING_API bool writeDouble(const QObject *sender, qsizetype offset,
                              double value);

    WING_API bool writeString(const QObject *sender, qsizetype offset,
                              const QString &value, const QString &encoding);

    WING_API bool writeBytes(const QObject *sender, qsizetype offset,
                             const QByteArray &data);

    WING_API bool insertInt8(const QObject *sender, qsizetype offset,
                             qint8 value);

    WING_API bool insertInt16(const QObject *sender, qsizetype offset,
                              qint16 value);

    WING_API bool insertInt32(const QObject *sender, qsizetype offset,
                              qint32 value);

    WING_API bool insertInt64(const QObject *sender, qsizetype offset,
                              qint64 value);

    WING_API bool insertUInt8(const QObject *sender, qsizetype offset,
                              quint8 value);

    WING_API bool insertUInt16(const QObject *sender, qsizetype offset,
                               quint16 value);

    WING_API bool insertUInt32(const QObject *sender, qsizetype offset,
                               quint32 value);

    WING_API bool insertUInt64(const QObject *sender, qsizetype offset,
                               quint64 value);

    WING_API bool insertFloat(const QObject *sender, qsizetype offset,
                              float value);

    WING_API bool insertDouble(const QObject *sender, qsizetype offset,
                               double value);

    WING_API bool insertString(const QObject *sender, qsizetype offset,
                               const QString &value, const QString &encoding);

    WING_API bool insertBytes(const QObject *sender, qsizetype offset,
                              const QByteArray &data);

    WING_API bool appendInt8(const QObject *sender, qint8 value);

    WING_API bool appendInt16(const QObject *sender, qint16 value);

    WING_API bool appendInt32(const QObject *sender, qint32 value);

    WING_API bool appendInt64(const QObject *sender, qint64 value);

    WING_API bool appendUInt8(const QObject *sender, quint8 value);

    WING_API bool appendUInt16(const QObject *sender, quint16 value);

    WING_API bool appendUInt32(const QObject *sender, quint32 value);

    WING_API bool appendUInt64(const QObject *sender, quint64 value);

    WING_API bool appendFloat(const QObject *sender, float value);

    WING_API bool appendDouble(const QObject *sender, double value);

    WING_API bool appendString(const QObject *sender, const QString &value,
                               const QString &encoding);

    WING_API bool appendBytes(const QObject *sender, const QByteArray &data);

    WING_API bool removeBytes(const QObject *sender, qsizetype offset,
                              qsizetype len);

    // cursor
    WING_API bool moveTo(const QObject *sender, qsizetype line,
                         qsizetype column, int nibbleindex,
                         bool clearSelection);

    WING_API bool moveTo(const QObject *sender, qsizetype offset,
                         bool clearSelection);

    WING_API bool select(const QObject *sender, qsizetype offset,
                         qsizetype length, WingHex::SelectionMode mode);

    WING_API bool setInsertionMode(const QObject *sender, bool isinsert);

    WING_API bool metadata(const QObject *sender, qsizetype begin,
                           qsizetype length, const QColor &fgcolor,
                           const QColor &bgcolor, const QString &comment);

    WING_API bool removeMetadata(const QObject *sender, qsizetype offset);

    WING_API bool clearMetadata(const QObject *sender);

    WING_API bool setMetaVisible(const QObject *sender, bool b);

    WING_API bool setMetafgVisible(const QObject *sender, bool b);

    WING_API bool setMetabgVisible(const QObject *sender, bool b);

    WING_API bool setMetaCommentVisible(const QObject *sender, bool b);

    // mainwindow
    WING_API int newFile(const QObject *sender);

    WING_API int openFile(const QObject *sender, const QString &filename);

    WING_API int openExtFile(const QObject *sender, const QString &ext,
                             const QString &file);

    WING_API int openWorkSpace(const QObject *sender, const QString &filename);

    WING_API WingHex::ErrFile closeHandle(const QObject *sender, int handle);

    WING_API WingHex::ErrFile closeFile(const QObject *sender, int handle,
                                        bool force);

    WING_API WingHex::ErrFile saveFile(const QObject *sender, int handle);

    WING_API WingHex::ErrFile exportFile(const QObject *sender, int handle,
                                         const QString &savename);

    WING_API WingHex::ErrFile saveAsFile(const QObject *sender, int handle,
                                         const QString &savename);

    WING_API int openCurrent(const QObject *sender);

    WING_API WingHex::ErrFile closeCurrent(const QObject *sender, bool force);

    WING_API WingHex::ErrFile saveCurrent(const QObject *sender);

    WING_API WingHex::ErrFile exportCurrent(const QObject *sender,
                                            const QString &savename);

    WING_API WingHex::ErrFile saveAsCurrent(const QObject *sender,
                                            const QString &savename);

    // bookmark
    WING_API bool addBookMark(const QObject *sender, qsizetype pos,
                              const QString &comment);

    WING_API bool modBookMark(const QObject *sender, qsizetype pos,
                              const QString &comment);

    WING_API bool removeBookMark(const QObject *sender, qsizetype pos);

    WING_API bool clearBookMark(const QObject *sender);

    // extension
    WING_API bool closeAllFiles(const QObject *sender);

    // generic call support
    WING_API WingHex::IWingGeneric *__createParamContext(const QObject *sender,
                                                         void *ctx);

    WING_API void __raiseContextException(const QObject *sender,
                                          const QString &exception,
                                          bool allowCatch = true);

private:
    WingHex::IWingPlugin *checkPluginAndReport(const QObject *sender,
                                               const char *func);

    bool passByFailedGuard(const QObject *sender, const char *func,
                           const QVariantList &params);

    bool checkErrAllAllowAndReport(const QObject *sender, const char *func);

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

    QHash<IWingPlugin *, PluginFile> m_plgviewMap;
    QHash<EditorView *, ViewBind> m_viewBindings;

    UniqueIdGenerator m_idGen;

    IWingManager *_manager = nullptr;
    std::optional<PluginInfo> _manInfo;

    WingAngelAPI *_angelplg = nullptr;

    QStringList _scriptMarcos;
    QList<IWingPlugin *> _pragmaedPlg;

    QReadWriteLock _rwlock;

private:
    QString _curLoadingPlg;

    Q_DISABLE_COPY_MOVE(PluginSystem)
};

#endif // PLUGINSYSTEM_H
