// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "WingPlugin/iwingplugin.h"
#include "WingPlugin/winghexdocument.h"
#include "WingPlugin/wingplugincalls_p.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QSharedPointer>
#include <QTest>
#include <thread>
#include <type_traits>

using namespace WingHex;

class Plugin final : public IWingPlugin {
public:
    bool init(const std::unique_ptr<QSettings> &) override { return true; }
    void unload(std::unique_ptr<QSettings> &) override {}
    const QString pluginName() const override {
        return QStringLiteral("probe");
    }
    const QString pluginComment() const override { return {}; }
};

// A transport fixture: real SDK calls go through its Qt meta-method table.
// Host filesystem/driver integration is exercised separately by host_probe.py.
class Host final : public QObject {
    Q_OBJECT
public:
    struct State {
        QUrl url;
        QByteArray bytes = QByteArrayLiteral("abcdefgh");
        bool modified = false;
        bool closed = false;
        qsizetype selectionBegin = 0;
        qsizetype selectionSize = 0;
        QMap<qsizetype, QString> bookmarks;
        MetadataInfo annotation{-1, -1, {}, {}, {}};
    };
    struct Handle {
        const QObject *owner;
        QSharedPointer<State> state;
    };
    QHash<int, Handle> handles;
    QHash<QUrl, QSharedPointer<State>> files;
    int next = 0;
    int active = -1;
    int switches = 0;
    QHash<const QObject *, int> current;
    bool denyClose = false;
    bool denySwitch = false;

    void attach(Plugin &plugin) {
        WingPluginCallsCorePrivate data;
        data._fnCaller = this;
        for (int i = metaObject()->methodOffset();
             i < metaObject()->methodCount(); ++i) {
            auto method = metaObject()->method(i);
            if (method.methodType() != QMetaMethod::Slot) {
                continue;
            }
            FunctionSig sig;
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
            sig.fnName = method.nameView();
            sig.types.fill(0);
            sig.typesCount = method.parameterCount() - 1;
#else
            sig.fnName = method.name();
            sig.types.resize(method.parameterCount() - 1);
#endif
            for (int p = 1; p < method.parameterCount(); ++p) {
                sig.types[p - 1] = method.parameterType(p);
            }
            data._fnTable.insert(sig, method);
        }
        CallTableEvent event(&data);
        QCoreApplication::sendEvent(&plugin, &event);
    }
    QSharedPointer<State> state(const QObject *owner, int handle) {
        auto it = handles.constFind(handle);
        return it != handles.cend() && it->owner == owner && !it->state->closed
                   ? it->state
                   : QSharedPointer<State>();
    }
public slots:
    bool isFileOpened(const QObject *owner, int handle) {
        return bool(state(owner, handle));
    }
    bool select(const QObject *owner, qsizetype offset, qsizetype length,
                SelectionMode) {
        auto target = state(owner, current.value(owner, -1));
        if (!target) {
            return false;
        }
        target->selectionBegin = offset;
        target->selectionSize = length;
        return true;
    }
    QByteArray selectedBytes(const QObject *owner, qsizetype index) {
        auto target = state(owner, current.value(owner, -1));
        return target && index == 0 ? target->bytes.mid(target->selectionBegin,
                                                        target->selectionSize)
                                    : QByteArray{};
    }
    bool addBookMark(const QObject *owner, qsizetype offset,
                     const QString &comment) {
        auto target = state(owner, current.value(owner, -1));
        if (!target) {
            return false;
        }
        target->bookmarks.insert(offset, comment);
        return true;
    }
    QString bookMarkComment(const QObject *owner, qsizetype offset) {
        auto target = state(owner, current.value(owner, -1));
        return target ? target->bookmarks.value(offset) : QString{};
    }
    bool metadata(const QObject *owner, qsizetype begin, qsizetype length,
                  const QColor &foreground, const QColor &background,
                  const QString &comment) {
        auto target = state(owner, current.value(owner, -1));
        if (!target) {
            return false;
        }
        target->annotation = {begin, begin + length, foreground, background,
                              comment};
        return true;
    }
    MetadataInfo metadataInfo(const QObject *owner, qsizetype) {
        auto target = state(owner, current.value(owner, -1));
        return target ? target->annotation : MetadataInfo{-1, -1, {}, {}, {}};
    }
    bool switchDocument(const QObject *owner, int handle) {
        if (QThread::currentThread() != thread()) {
            return false;
        }
        if (denySwitch || !state(owner, handle)) {
            return false;
        }
        ++switches;
        current[owner] = handle;
        return true;
    }
    QByteArray readBytes(const QObject *owner, qsizetype offset,
                         qsizetype count) {
        auto target = state(owner, current.value(owner, -1));
        return target ? target->bytes.mid(offset, count) : QByteArray{};
    }
    bool writeBytes(const QObject *owner, qsizetype offset,
                    const QByteArray &bytes) {
        if (QThread::currentThread() != thread()) {
            return false;
        }
        auto target = state(owner, current.value(owner, -1));
        if (!target) {
            return false;
        }
        target->bytes.replace(offset, bytes.size(), bytes);
        target->modified = true;
        return true;
    }
    bool moveTo(const QObject *owner, qsizetype, bool) {
        return bool(state(owner, current.value(owner, -1)));
    }
    bool moveTo(const QObject *owner, qsizetype, qsizetype, int, bool) {
        return bool(state(owner, current.value(owner, -1)));
    }

    int openFile(const QObject *owner, const QUrl &url) {
        if (!url.isValid() || url.isEmpty()) {
            return ErrFile::InvalidFormat;
        }
        if (!files.contains(url) || files[url]->closed) {
            files[url] = QSharedPointer<State>::create();
            files[url]->url = url;
        }
        const auto id = next++;
        handles.insert(id, {owner, files[url]});
        return id;
    }
    int closeFileHandle(const QObject *owner, int handle) {
        if (QThread::currentThread() != thread()) {
            return ErrFile::NotAllowedInNoneGUIThread;
        }
        if (!state(owner, handle)) {
            return ErrFile::NotExist;
        }
        handles.remove(handle);
        return ErrFile::Success;
    }
    int closeFile(const QObject *owner, int handle, bool force) {
        if (QThread::currentThread() != thread()) {
            return ErrFile::NotAllowedInNoneGUIThread;
        }
        auto target = state(owner, handle);
        if (!target) {
            return ErrFile::NotExist;
        }
        if (denyClose) {
            return ErrFile::Permission;
        }
        if (target->modified && !force) {
            return ErrFile::UnSaved;
        }
        target->closed = true;
        handles.removeIf(
            [&](const auto &entry) { return entry.value().state == target; });
        return ErrFile::Success;
    }
    DocumentInfo documentInfo(const QObject *owner, int handle) {
        auto target = state(owner, handle);
        DocumentInfo info;
        if (target) {
            info.fileNameUrl = target->url;
            info.documentType = target->url.scheme() == QStringLiteral("wdrv")
                                    ? DocumentType::Extension
                                    : DocumentType::File;
            if (target->url.scheme() == QStringLiteral("wingpro")) {
                info.workSpaceName = target->url.toLocalFile();
                if (info.workSpaceName.isEmpty()) {
                    info.workSpaceName = target->url.path();
                }
            }
            info.isSaved = !target->modified;
        }
        return info;
    }
    bool raiseDocument(const QObject *owner, int handle) {
        if (!state(owner, handle)) {
            return false;
        }
        active = handle;
        return true;
    }
};

template <typename T, typename = void>
struct HasUrlOpen : std::false_type {};
template <typename T>
struct HasUrlOpen<T, std::void_t<decltype(std::declval<T &>().openFile(
                         std::declval<QUrl>()))>> : std::true_type {};
static_assert(!HasUrlOpen<WingHexDocument>::value);
static_assert(!std::is_copy_constructible_v<WingHexDocument>);
static_assert(std::is_nothrow_move_constructible_v<WingHexDocument>);

static_assert(std::is_base_of_v<IWingPluginCallsOp, WingHexDocument>);
static_assert(!std::is_copy_constructible_v<WingHexDocument::UsingEnv>);
static_assert(std::is_nothrow_move_constructible_v<WingHexDocument::UsingEnv>);

class DocumentTest final : public QObject {
    Q_OBJECT
private slots:
    void usageScope() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument a(&plugin), b(&plugin);
        QCOMPARE(a.openFile(QStringLiteral("scope-a.bin")),
                 int(ErrFile::Success));
        QCOMPARE(b.openFile(QStringLiteral("scope-b.bin")),
                 int(ErrFile::Success));
        QVERIFY(a.readBytes(0, 3).isEmpty());
        QVERIFY(!a.writeBytes(0, QByteArrayLiteral("bad")));
        QVERIFY(!a.select(0, 1));
        QVERIFY(!a.addBookMark(0, QStringLiteral("outside")));
        QVERIFY(
            !a.metadata(0, 1, Qt::red, Qt::blue, QStringLiteral("outside")));
        {
            auto usage = a.use();
            QVERIFY(usage);
            QCOMPARE(host.switches, 1);
            QCOMPARE(a.readBytes(0, 3), QByteArrayLiteral("abc"));
            QVERIFY(a.writeBytes(0, QByteArrayLiteral("A")));
            QVERIFY(a.moveTo(qsizetype(0), false));
            QVERIFY(a.moveTo(qsizetype(0), qsizetype(0), 0, false));
            QVERIFY(a.select(0, 2));
            QCOMPARE(a.selectedBytes(0), QByteArrayLiteral("Ab"));
            QVERIFY(a.addBookMark(1, QStringLiteral("bookmark")));
            QCOMPARE(a.bookMarkComment(1), QStringLiteral("bookmark"));
            QVERIFY(
                a.metadata(0, 2, Qt::red, Qt::blue, QStringLiteral("note")));
            QCOMPARE(a.metadataInfo(0).comment, QStringLiteral("note"));
            QCOMPARE(host.switches, 1);
            auto nested = a.use();
            QVERIFY(nested);
            QCOMPARE(host.switches, 1);
            QVERIFY(!b.use());
            QVERIFY(b.readBytes(0, 3).isEmpty());
            auto moved = std::move(nested);
            QVERIFY(!nested);
            QVERIFY(moved);
            WingHexDocument movedDocument(std::move(a));
            QCOMPARE(movedDocument.readBytes(0, 1), QByteArrayLiteral("A"));
            QVERIFY(a.readBytes(0, 1).isEmpty());
            std::thread reader([&] {
                QCOMPARE(movedDocument.readBytes(0, 3),
                         QByteArrayLiteral("Abc"));
                auto nestedReader = movedDocument.use();
                QVERIFY(nestedReader);
                QVERIFY(!movedDocument.writeBytes(0, QByteArrayLiteral("bad")));
            });
            reader.join();
            QCOMPARE(movedDocument.closeHandle(), int(ErrFile::Success));
            QVERIFY(!usage);
            QVERIFY(!moved);
            QVERIFY(movedDocument.readBytes(0, 1).isEmpty());
        }
        {
            auto usage = b.use();
            QVERIFY(usage);
            QCOMPARE(host.switches, 2);
            QCOMPARE(b.readBytes(0, 3), QByteArrayLiteral("abc"));
        }
        QVERIFY(b.readBytes(0, 3).isEmpty());
        auto again = b.use();
        QVERIFY(again);
        QCOMPARE(host.switches, 3);
        auto expired = [&] {
            WingHexDocument temporary(&plugin);
            if (temporary.openFile(QStringLiteral("temporary.bin")) !=
                ErrFile::Success) {
                qFatal("Cannot prepare temporary document");
            }
            // A different document cannot overlap the live b scope.
            return temporary.use();
        }();
        QVERIFY(!expired);
        QCOMPARE(b.closeHandle(), int(ErrFile::Success));
        QVERIFY(!again);
        QCOMPARE(b.openFile(QStringLiteral("scope-b.bin")),
                 int(ErrFile::Success));
        QVERIFY(b.readBytes(0, 1).isEmpty());
        auto fresh = b.use();
        QVERIFY(fresh);
        QCOMPARE(host.switches, 4);
        auto inner = b.use();
        fresh = std::move(inner);
        QVERIFY(fresh);
        QVERIFY(!inner);
        QCOMPARE(host.switches, 4);
    }
    void invalidDocumentCannotEnterScope() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument document(&plugin);
        QVERIFY(!document.use());
        QCOMPARE(document.openFile(QStringLiteral("scope.bin")),
                 ErrFile::Success);
        WingHexDocument moved(std::move(document));
        QVERIFY(!document.use());
        auto scope = moved.use();
        QVERIFY(scope);
        const auto handle = host.handles.cbegin().key();
        QCOMPARE(host.closeFile(&plugin, handle, false), ErrFile::Success);
        QVERIFY(!moved.use());
        QVERIFY(moved.readBytes(0, 1).isEmpty());
    }
    void failedScopeEntryCanRetry() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument a(&plugin), b(&plugin);
        QCOMPARE(a.openFile(QStringLiteral("scope-a.bin")), ErrFile::Success);
        QCOMPARE(b.openFile(QStringLiteral("scope-b.bin")), ErrFile::Success);

        host.denySwitch = true;
        QVERIFY(!a.use());
        QCOMPARE(host.switches, 0);
        QVERIFY(a.readBytes(0, 1).isEmpty());
        host.denySwitch = false;
        auto scope = b.use();
        QVERIFY(scope);
        QCOMPARE(host.switches, 1);
        QVERIFY(!a.use());
        QCOMPARE(b.readBytes(0, 3), QByteArrayLiteral("abc"));
    }
    void lastScopeReleasedOnWorker() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument a(&plugin), b(&plugin);
        QCOMPARE(a.openFile(QStringLiteral("scope-a.bin")), ErrFile::Success);
        QCOMPARE(b.openFile(QStringLiteral("scope-b.bin")), ErrFile::Success);
        auto scope = a.use();
        QVERIFY(scope);
        std::thread worker([owned = std::move(scope)] {});
        worker.join();
        QVERIFY(!scope);
        QVERIFY(a.readBytes(0, 1).isEmpty());
        {
            auto next = b.use();
            QVERIFY(next);
            QCOMPARE(host.switches, 2);
            QVERIFY(!a.use());
        }
        auto again = a.use();
        QVERIFY(again);
        QCOMPARE(host.switches, 3);
    }
    void invalidatedScopeDoesNotBlockReentry() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument a(&plugin), b(&plugin);
        QCOMPARE(a.openFile(QStringLiteral("scope-a.bin")), ErrFile::Success);
        QCOMPARE(b.openFile(QStringLiteral("scope-b.bin")), ErrFile::Success);
        auto old = a.use();
        QVERIFY(old);
        QCOMPARE(a.closeHandle(), ErrFile::Success);
        QVERIFY(!old);
        {
            auto next = b.use();
            QVERIFY(next);
            QCOMPARE(a.openFile(QStringLiteral("scope-a.bin")),
                     ErrFile::Success);
            QVERIFY(!a.use());
            QVERIFY(!old);
        }
        auto fresh = a.use();
        QVERIFY(fresh);
        QVERIFY(!old);
        QCOMPARE(host.switches, 3);
    }
    void closeWithUsingEnv() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument document(&plugin);
        QCOMPARE(document.openFile(QStringLiteral("close-scope.bin")),
                 int(ErrFile::Success));
        auto env = document.use();
        QVERIFY(env);
        QVERIFY(document.writeBytes(0, QByteArrayLiteral("A")));
        QCOMPARE(document.close(), int(ErrFile::UnSaved));
        QVERIFY(env);
        QCOMPARE(document.readBytes(0, 1), QByteArrayLiteral("A"));
        QCOMPARE(document.close(true), int(ErrFile::Success));
        QVERIFY(!env);
        QVERIFY(!document.writeBytes(0, QByteArrayLiteral("bad")));
    }
    void independentPluginScopes() {
        Host host;
        Plugin first, second;
        host.attach(first);
        host.attach(second);
        WingHexDocument a(&first), b(&second);
        QCOMPARE(a.openFile(QStringLiteral("first-plugin.bin")),
                 int(ErrFile::Success));
        QCOMPARE(b.openFile(QStringLiteral("second-plugin.bin")),
                 int(ErrFile::Success));
        auto useA = a.use();
        auto useB = b.use();
        QVERIFY(useA);
        QVERIFY(useB);
        QCOMPARE(host.switches, 2);
        QCOMPARE(a.readBytes(0, 3), QByteArrayLiteral("abc"));
        QCOMPARE(b.readBytes(0, 3), QByteArrayLiteral("abc"));
    }
    void guardOutlivesPlugin() {
        Host host;
        auto plugin = std::make_unique<Plugin>();
        host.attach(*plugin);
        WingHexDocument document(plugin.get());
        QCOMPARE(document.openFile(QStringLiteral("plugin.bin")),
                 int(ErrFile::Success));
        auto usage = document.use();
        QVERIFY(usage);
        plugin.reset();
        QVERIFY(!usage);
        QVERIFY(document.readBytes(0, 1).isEmpty());
    }
    void guardOutlivesDocument() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        auto usage = [&] {
            WingHexDocument document(&plugin);
            if (document.openFile(QStringLiteral("temporary.bin")) !=
                ErrFile::Success) {
                qFatal("Cannot prepare document");
            }
            return document.use();
        }();
        QVERIFY(!usage);
        QVERIFY(host.handles.isEmpty());
    }
    void urls() {
        const auto path = QDir::tempPath() + QStringLiteral("/中文 # %.bin");
        QCOMPARE(WingHex::fileUrl(path).toLocalFile(), path);
        auto workspace = WingHex::workspaceUrl(path);
        QCOMPARE(workspace.scheme(), QStringLiteral("wingpro"));
        workspace.setScheme(QStringLiteral("file"));
        QCOMPARE(workspace.toLocalFile(), path);
        const auto device = QStringLiteral("/中文 # %?x/a");
        const auto url = WingHex::deviceUrl(QStringLiteral("driver"), device);
        QCOMPARE(url.scheme(), QStringLiteral("wdrv"));
        QCOMPARE(url.path().mid(1), device);
        QVERIFY(!url.hasQuery());
        QVERIFY(!url.hasFragment());
        QCOMPARE(QUrl(url.toEncoded()).path().mid(1), device);
        QVERIFY(WingHex::deviceUrl({}, device).isEmpty());
        QCOMPARE(
            WingHex::deviceUrl(QStringLiteral("ShareMemoryDrv"), device).host(),
            QStringLiteral("sharememorydrv"));
        QVERIFY(WingHex::fileUrl({}).isEmpty());
        QVERIFY(WingHex::workspaceUrl({}).isEmpty());
        QVERIFY(QDir::isAbsolutePath(
            WingHex::fileUrl(QStringLiteral("relative.bin")).toLocalFile()));
    }
    void validityUsesOwnedHandleWithoutScope() {
        Host host;
        Plugin plugin, other;
        host.attach(plugin);
        host.attach(other);
        WingHexDocument document(&plugin);
        QVERIFY(!document.isValid());
        QCOMPARE(document.openFile(QStringLiteral("query.bin")),
                 ErrFile::Success);
        QVERIFY(document.isValid());
        QCOMPARE(host.switches, 0);
        const auto handle = host.handles.cbegin().key();
        QVERIFY(plugin.isFileOpened(handle));
        QVERIFY(!other.isFileOpened(handle));
        QVERIFY(!plugin.isFileOpened(-1));
        bool workerValid = false;
        std::thread worker([&] { workerValid = document.isValid(); });
        worker.join();
        QVERIFY(workerValid);
        QCOMPARE(document.closeHandle(), ErrFile::Success);
        QVERIFY(!document.isValid());
        QVERIFY(!plugin.isFileOpened(handle));
        QCOMPARE(document.openFile(QStringLiteral("query.bin")),
                 ErrFile::Success);
        WingHexDocument moved(std::move(document));
        QVERIFY(!document.isValid());
        QVERIFY(moved.isValid());
        const auto next = host.handles.cbegin().key();
        QCOMPARE(host.closeFile(&plugin, next, true), ErrFile::Success);
        QVERIFY(!moved.isValid());
    }

    void documentInfoSnapshot() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument document(&plugin);
        QCOMPARE(document.openFile(QStringLiteral("properties.bin")),
                 int(ErrFile::Success));
        const auto url = WingHex::fileUrl(QStringLiteral("properties.bin"));
        const auto snapshot = document.documentInfo();
        QCOMPARE(snapshot.fileNameUrl, url);
        QCOMPARE(snapshot.documentType, DocumentType::File);
        QVERIFY(snapshot.isSaved);
        host.files[url]->modified = true;
        QVERIFY(!document.documentInfo().isSaved);
        QVERIFY(snapshot.isSaved);
        QCOMPARE(document.close(true), int(ErrFile::Success));
        QCOMPARE(document.openWorkSpace(QStringLiteral("properties.wingpro")),
                 int(ErrFile::Success));
        QCOMPARE(document.documentInfo().workSpaceName,
                 WingHex::fileUrl(QStringLiteral("properties.wingpro"))
                     .toLocalFile());
    }
    void releaseHandleExplicitly() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument document(&plugin);
        QCOMPARE(document.openFile(QStringLiteral("release.bin")),
                 ErrFile::Success);
        QCOMPARE(document.closeHandle(), int(ErrFile::Success));
        QVERIFY(host.handles.isEmpty());
        QVERIFY(!host.files.cbegin().value()->closed);
        QCOMPARE(document.closeHandle(), int(ErrFile::NotExist));
        QCOMPARE(document.openFile(QStringLiteral("release.bin")),
                 ErrFile::Success);
    }
    void openingAndClosing() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        {
            WingHexDocument document(&plugin);
            QCOMPARE(document.openFile(QStringLiteral("first.bin")),
                     int(ErrFile::Success));
            QCOMPARE(document.openFile(QStringLiteral("other.bin")),
                     int(ErrFile::Success));
            QCOMPARE(host.handles.size(), 1);
            QVERIFY(!host.files[WingHex::fileUrl(QStringLiteral("first.bin"))]
                         ->closed);
            QVERIFY(document.activate());
            host.files[WingHex::fileUrl(QStringLiteral("other.bin"))]
                ->modified = true;
            QCOMPARE(document.close(), int(ErrFile::UnSaved));
            host.denyClose = true;
            QCOMPARE(document.close(true), int(ErrFile::Permission));
            host.denyClose = false;
            QCOMPARE(document.close(true), int(ErrFile::Success));
        }
        WingHexDocument device(&plugin);
        QCOMPARE(
            device.openExt(QStringLiteral("driver"), QStringLiteral("a#b")),
            int(ErrFile::Success));
        QCOMPARE(device.documentInfo().documentType, DocumentType::Extension);
        WingHexDocument workspace(&plugin);
        QCOMPARE(workspace.openWorkSpace(QStringLiteral("a.wingpro")),
                 int(ErrFile::Success));
        QVERIFY(!workspace.documentInfo().workSpaceName.isEmpty());
    }
    void moveAndExternalClose() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        WingHexDocument a(&plugin), b(&plugin);
        QCOMPARE(a.openFile(QStringLiteral("a.bin")), int(ErrFile::Success));
        QCOMPARE(b.openFile(QStringLiteral("b.bin")), int(ErrFile::Success));
        a = std::move(b);
        QCOMPARE(b.closeHandle(), int(ErrFile::NotExist));
        QCOMPARE(host.handles.size(), 1);
        const auto handle = host.handles.cbegin().key();
        QCOMPARE(host.closeFile(&plugin, handle, false), int(ErrFile::Success));
        QCOMPARE(a.closeHandle(), int(ErrFile::NotExist));
        QCOMPARE(a.openFile(QStringLiteral("again.bin")),
                 int(ErrFile::Success));
    }
    void workerDestruction() {
        Host host;
        Plugin plugin;
        host.attach(plugin);
        auto document = std::make_unique<WingHexDocument>(&plugin);
        QCOMPARE(document->openFile(QStringLiteral("a.bin")), ErrFile::Success);
        std::thread worker(
            [owned = std::move(document)]() mutable { owned.reset(); });
        worker.join();
        QCOMPARE(host.handles.size(), 1);
        QCoreApplication::sendPostedEvents(&plugin, QEvent::MetaCall);
        QCOMPARE(host.handles.size(), 0);
        QVERIFY(!host.files.cbegin().value()->closed);
    }
    void contextAndThread() {
        Host host;
        auto plugin = std::make_unique<Plugin>();
        host.attach(*plugin);
        WingHexDocument document(plugin.get());
        QCOMPARE(document.openFile(QStringLiteral("a.bin")), ErrFile::Success);
        std::thread worker([&] {
            QCOMPARE(document.documentInfo().fileNameUrl,
                     WingHex::fileUrl(QStringLiteral("a.bin")));
            QCOMPARE(document.documentInfo().documentType, DocumentType::File);
            QVERIFY(!document.use());
            QCOMPARE(document.close(), ErrFile::NotAllowedInNoneGUIThread);
        });
        worker.join();
        plugin.reset();
        QVERIFY(!document.use());
    }
};

QTEST_MAIN(DocumentTest)
#include "tst_plugindocument.moc"
