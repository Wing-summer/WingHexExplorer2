// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-or-later
#include "WingPlugin/winghexdocument.h"
#include "class/file/workspacemanager.h"
#include "class/plugin/pluginsystem.h"
#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "dialog/mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTemporaryDir>
#include <atomic>
#include <cstring>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

using namespace WingHex;
#define REQUIRE(expression)                                                    \
    do {                                                                       \
        if (!(expression)) {                                                   \
            std::cerr << "FAIL line " << __LINE__ << ": " << #expression       \
                      << '\n';                                                 \
            return false;                                                      \
        }                                                                      \
    } while (false)

class ProbePlugin final : public IWingPlugin {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingPlugin)
public:
    int acquired = 0;
    int released = 0;
    bool init(const std::unique_ptr<QSettings> &) override { return true; }
    void unload(std::unique_ptr<QSettings> &) override {}
    const QString pluginName() const override {
        return QStringLiteral("probe");
    }
    const QString pluginComment() const override { return {}; }
    RegisteredEvents registeredEvents() const override {
        return RegisteredEvents(RegisteredEvent::PluginFileOpened) |
               RegisteredEvent::PluginFileClosed;
    }
    void eventPluginFile(PluginFileEvent event, FileType, const QUrl &, int,
                         const QUrl &) override {
        acquired += event == PluginFileEvent::PluginOpened;
        released += event == PluginFileEvent::PluginClosed;
    }
};

class ProbeDevice final : public WingIODevice {
public:
    QByteArray bytes = QByteArrayLiteral("device-data");
    qint64 size() const override { return bytes.size(); }
    bool isSequential() const override { return false; }
    bool seek(qint64 offset) override {
        return offset >= 0 && offset <= size() && QIODevice::seek(offset);
    }
    bool defaultIsLocked() const override { return false; }

protected:
    qint64 readData(char *data, qint64 length) override {
        const auto count = std::min(length, size() - pos());
        std::memcpy(data, bytes.constData() + pos(), size_t(count));
        return count;
    }
    qint64 writeData(const char *data, qint64 length) override {
        const auto count = std::min(length, size() - pos());
        std::memcpy(bytes.data() + pos(), data, size_t(count));
        return count;
    }
};

class ProbeDriver final : public IWingDevice {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingDevice)
public:
    bool allowClose = true;
    int opens = 0;
    int closes = 0;
    QString lastPath;
    bool init(const std::unique_ptr<QSettings> &) override { return true; }
    void unload(std::unique_ptr<QSettings> &) override {}
    const QString pluginName() const override {
        return QStringLiteral("driver");
    }
    const QString pluginComment() const override { return {}; }
    QString supportedFileExtDisplayName() const override {
        return QStringLiteral("driver");
    }
    WingIODevice *onOpenFile(const QString &path) override {
        ++opens;
        lastPath = path;
        return new ProbeDevice;
    }
    bool onCloseFile(WingIODevice *) override {
        ++closes;
        return allowClose;
    }
};

class PluginDocumentHostProbe {
    static EditorView *findView(const QUrl &url) {
        for (auto view : EditorView::instances()) {
            if (!view->isClosed() && view->fileNameUrl() == url) {
                return view;
            }
        }
        return nullptr;
    }

    static bool checkHandlePool() {
        using Pool = PluginSystem::DocumentHandlePool;
        using Context = PluginSystem::PluginFileContext;
        auto pool = QSharedPointer<Pool>::create();
        auto first = QSharedPointer<Context>::create(pool);
        auto retained = first;
        auto second = QSharedPointer<Context>::create(pool);
        const int releasedId = first->fid;
        REQUIRE(releasedId >= 0 && second->fid != releasedId);
        first.clear();
        auto third = QSharedPointer<Context>::create(pool);
        REQUIRE(third->fid != releasedId && third->fid != second->fid);
        retained.clear();
        auto recycled = QSharedPointer<Context>::create(pool);
        REQUIRE(recycled->fid == releasedId);

        const auto weakPool = pool.toWeakRef();
        pool.clear();
        REQUIRE(!weakPool.isNull());
        second.clear();
        third.clear();
        recycled.clear();
        REQUIRE(weakPool.isNull());

        pool = QSharedPointer<Pool>::create();
        pool->nextId = std::numeric_limits<int>::max() - 1;
        auto last = QSharedPointer<Context>::create(pool);
        REQUIRE(last->fid == std::numeric_limits<int>::max() - 1);
        auto exhausted = QSharedPointer<Context>::create(pool);
        REQUIRE(exhausted->fid < 0);
        exhausted.clear();
        const int lastId = last->fid;
        last.clear();
        auto reusedAtLimit = QSharedPointer<Context>::create(pool);
        REQUIRE(reusedAtLimit->fid == lastId);
        REQUIRE(QSharedPointer<Context>::create(pool)->fid < 0);
        return true;
    }

public:
    static bool run() {
        REQUIRE(checkHandlePool());
        auto &settings = SettingManager::instance();
        settings.setScriptEnabled(false);
        settings.setCheckUpdate(false);
        ThemeManager::instance();
        MainWindow window(nullptr);
        auto &host = PluginSystem::instance();
        const auto meta = host.metaObject();
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "toast(const QObject*,const QPixmap&,const QString&)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "logTrace(const QObject*,const QString&)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "logDebug(const QObject*,const QString&)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "logInfo(const QObject*,const QString&)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "logWarn(const QObject*,const QString&)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "logError(const QObject*,const QString&)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "raiseDockWidget(const QObject*,QWidget*)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentAppTheme(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "standardIcon(const QObject*,WingHex::StandardIcon)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appAuthorPixmap(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "standardPixmap(const QObject*,WingHex::StandardIcon)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "standardIconData(const QObject*,WingHex::StandardIcon)")
                    .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "createDialog(const QObject*,QWidget*)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "createMainWindow(const QObject*,QWidget*)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "msgAboutQt(const QObject*,QWidget*,const QString&)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "msgInformation(const "
                                      "QObject*,QWidget*,const QString&,const "
                                      "QString&,QMessageBox::StandardButtons,"
                                      "QMessageBox::StandardButton)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "msgQuestion(const QObject*,QWidget*,const QString&,const "
                    "QString&,QMessageBox::StandardButtons,QMessageBox::"
                    "StandardButton)")
                    .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "msgWarning(const QObject*,QWidget*,const QString&,const "
                    "QString&,QMessageBox::StandardButtons,QMessageBox::"
                    "StandardButton)")
                    .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "msgCritical(const QObject*,QWidget*,const QString&,const "
                    "QString&,QMessageBox::StandardButtons,QMessageBox::"
                    "StandardButton)")
                    .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "msgAbout(const QObject*,QWidget*,const "
                                      "QString&,const QString&)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "msgbox(const QObject*,QWidget*,QMessageBox::Icon,const "
                    "QString&,const "
                    "QString&,QMessageBox::StandardButtons,QMessageBox::"
                    "StandardButton)")
                    .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetText(const QObject*,QWidget*,const "
                        "QString&,const QString&,QLineEdit::EchoMode,const "
                        "QString&,bool*,Qt::InputMethodHints)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetMultiLineText(const QObject*,QWidget*,const "
                        "QString&,const QString&,const "
                        "QString&,bool*,Qt::InputMethodHints)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetItem(const QObject*,QWidget*,const "
                        "QString&,const QString&,const "
                        "QStringList&,int,bool,bool*,Qt::InputMethodHints)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetInt(const QObject*,QWidget*,const "
                        "QString&,const QString&,int,int,int,int,bool*)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "dlgGetDouble(const QObject*,QWidget*,const QString&,const "
                    "QString&,double,double,double,int,bool*,double)")
                    .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetExistingDirectory(const QObject*,QWidget*,const "
                        "QString&,const QString&,QFileDialog::Options)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetOpenFileName(const QObject*,QWidget*,const "
                        "QString&,const QString&,const "
                        "QString&,QString*,QFileDialog::Options)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetOpenFileNames(const QObject*,QWidget*,const "
                        "QString&,const QString&,const "
                        "QString&,QString*,QFileDialog::Options)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetSaveFileName(const QObject*,QWidget*,const "
                        "QString&,const QString&,const "
                        "QString&,QString*,QFileDialog::Options)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "dlgGetColor(const QObject*,const QString&,QWidget*)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "existsServiceHost(const QObject*,const QString&)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "invokeServiceImpl(const QObject*,const "
                                      "QString&,const WingHex::MetaCallInfo&)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "isCurrentDocEditing(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentDocFile(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentDocFileName(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentDocWorkSpace(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentDocWorkSpaceName(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "isReadOnly(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "isInsertionMode(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "isKeepSize(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature("isLocked(const QObject*)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "documentLines(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "documentBytes(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentPos(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentRow(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentColumn(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "currentOffset(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "selectedLength(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "selectedBytes(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "selectionBytes(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "selectionStart(const QObject*,qsizetype)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "selectionEnd(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "selectionLength(const QObject*,qsizetype)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "selectionCount(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "stringVisible(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "addressVisible(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "headerVisible(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "addressBase(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "isModified(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readInt8(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readInt16(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readInt32(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readInt64(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readUInt8(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readUInt16(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readUInt32(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readUInt64(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readFloat(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "readDouble(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "readString(const QObject*,qsizetype,const QString&)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "readBytes(const QObject*,qsizetype,qsizetype)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "findNext(const QObject*,qsizetype,const QByteArray&)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "findPrevious(const QObject*,qsizetype,const QByteArray&)")
                    .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "metadataInfo(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "metadataInfoByIndex(const QObject*,qsizetype)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "bookMarkCount(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "bookMarkPos(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "metadataCount(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "bookMarkComment(const QObject*,qsizetype)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "existBookMark(const QObject*,qsizetype)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "switchDocument(const QObject*,int)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "raiseDocument(const QObject*,int)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setLockedFile(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setKeepSize(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setStringVisible(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setAddressVisible(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setHeaderVisible(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setAddressBase(const QObject*,quintptr)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "beginMarco(const QObject*,const QString&)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature("endMarco(const QObject*)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "isMacroEmpty(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "resetMarco(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "writeInt8(const QObject*,qsizetype,qint8)")
                                  .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "writeInt16(const QObject*,qsizetype,qint16)")
                                  .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "writeInt32(const QObject*,qsizetype,qint32)")
                                  .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "writeInt64(const QObject*,qsizetype,qint64)")
                                  .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "writeUInt8(const QObject*,qsizetype,quint8)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "writeUInt16(const QObject*,qsizetype,quint16)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "writeUInt32(const QObject*,qsizetype,quint32)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "writeUInt64(const QObject*,qsizetype,quint64)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "writeFloat(const QObject*,qsizetype,float)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "writeDouble(const QObject*,qsizetype,double)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "writeString(const QObject*,qsizetype,const "
                                  "QString&,const QString&)")
                                  .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "writeBytes(const QObject*,qsizetype,const QByteArray&)")
                    .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "insertInt8(const QObject*,qsizetype,qint8)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertInt16(const QObject*,qsizetype,qint16)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertInt32(const QObject*,qsizetype,qint32)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertInt64(const QObject*,qsizetype,qint64)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertUInt8(const QObject*,qsizetype,quint8)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertUInt16(const QObject*,qsizetype,quint16)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertUInt32(const QObject*,qsizetype,quint32)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertUInt64(const QObject*,qsizetype,quint64)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "insertFloat(const QObject*,qsizetype,float)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "insertDouble(const QObject*,qsizetype,double)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "insertString(const QObject*,qsizetype,const "
                                  "QString&,const QString&)")
                                  .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(
                QMetaObject::normalizedSignature(
                    "insertBytes(const QObject*,qsizetype,const QByteArray&)")
                    .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendInt8(const QObject*,qint8)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendInt16(const QObject*,qint16)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendInt32(const QObject*,qint32)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendInt64(const QObject*,qint64)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendUInt8(const QObject*,quint8)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendUInt16(const QObject*,quint16)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendUInt32(const QObject*,quint32)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendUInt64(const QObject*,quint64)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendFloat(const QObject*,float)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendDouble(const QObject*,double)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "appendString(const QObject*,const "
                                      "QString&,const QString&)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "appendBytes(const QObject*,const QByteArray&)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "removeBytes(const QObject*,qsizetype,qsizetype)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "moveTo(const QObject*,qsizetype,qsizetype,int,bool)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "moveTo(const QObject*,qsizetype,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "select(const "
                        "QObject*,qsizetype,qsizetype,WingHex::SelectionMode)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setInsertionMode(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "metadata(const QObject*,qsizetype,qsizetype,const "
                        "QColor&,const QColor&,const QString&)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "removeMetadata(const QObject*,qsizetype)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "clearMetadata(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setMetaVisible(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setMetafgVisible(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "setMetabgVisible(const QObject*,bool)")
                                      .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "setMetaCommentVisible(const QObject*,bool)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "openFile(const QObject*,const QUrl&)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "closeFile(const QObject*,int,bool)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "openCurrent(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "closeCurrent(const QObject*)")
                                      .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "addBookMark(const QObject*,qsizetype,const QString&)")
                        .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(
                    QMetaObject::normalizedSignature(
                        "modBookMark(const QObject*,qsizetype,const QString&)")
                        .constData()) >= 0);
        REQUIRE(
            meta->indexOfSlot(QMetaObject::normalizedSignature(
                                  "removeBookMark(const QObject*,qsizetype)")
                                  .constData()) >= 0);
        REQUIRE(meta->indexOfSlot(QMetaObject::normalizedSignature(
                                      "clearBookMark(const QObject*)")
                                      .constData()) >= 0);
        ProbePlugin plugin, other;
        host.applyFunctionTables(&plugin);
        host.applyFunctionTables(&other);
        host.registerEvents(&plugin);
        ProbeDriver driver;
        PluginInfo driverInfo;
        driverInfo.id = QStringLiteral("probedriver");
        host._loadeddevs.append(&driver);
        host._pinfos.insert(&driver, driverInfo);
        QTemporaryDir directory;
        REQUIRE(directory.isValid());
        const auto first = directory.filePath(QStringLiteral("中文 # %.bin"));
        const auto second = directory.filePath(QStringLiteral("other.bin"));
        for (const auto &path : {first, second}) {
            QFile file(path);
            REQUIRE(file.open(QIODevice::WriteOnly));
            REQUIRE(file.write("abcdefgh") == 8);
        }
        const auto initialViews = EditorView::instances().size();
        {
            WingHexDocument a(&plugin), b(&plugin), alias(&plugin);
            REQUIRE(a.openFile(first) == ErrFile::Success);
            REQUIRE(b.openFile(second) == ErrFile::Success);
            REQUIRE(alias.openFile(first) == ErrFile::Success);
            REQUIRE(a.readBytes(0, 3).isEmpty());
            {
                auto usage = a.use();
                REQUIRE(usage);
                REQUIRE(a.readBytes(0, 3) == QByteArrayLiteral("abc"));
                REQUIRE(a.findNext(0, QByteArrayLiteral("bc")) == 1);
                REQUIRE(!b.use());
            }
            REQUIRE(a.readBytes(0, 3).isEmpty());
            REQUIRE(b.activate());
            REQUIRE(plugin.switchDocument(
                host.m_plgviewMap[&plugin].contexts.at(1)->fid));
            REQUIRE(plugin.readBytes(0, 3) == QByteArrayLiteral("abc"));
            REQUIRE(a.closeHandle() == ErrFile::Success);
            REQUIRE(!alias.isValid());
            REQUIRE(alias.openFile(first) == ErrFile::Success);
            REQUIRE(alias.close() == ErrFile::Success);
        }
        auto remaining = findView(WingHex::fileUrl(second));
        REQUIRE(remaining);
        REQUIRE(window.closeEditor(remaining, false) == ErrFile::Success);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        REQUIRE(EditorView::instances().size() == initialViews);
        {
            WingHexDocument device(&plugin);
            const auto devicePath = QStringLiteral("/中文 # %?x/a");
            REQUIRE(device.openExt(driverInfo.id, devicePath) ==
                    ErrFile::Success);
            REQUIRE(driver.lastPath == devicePath);
            REQUIRE(device.documentInfo().documentType ==
                    DocumentType::Extension);
            driver.allowClose = false;
            REQUIRE(device.close() == ErrFile::Permission);
            driver.allowClose = true;
            REQUIRE(device.close() == ErrFile::Success);
        }
        const auto workspace =
            directory.filePath(QStringLiteral("example.wingpro"));
        WorkSpaceData data;
        data.file = WingHex::fileUrl(first);
        data.bookmarks.insert(2, QStringLiteral("bookmark"));
        REQUIRE(WorkSpaceManager::saveWorkSpace(workspace, data));
        {
            WingHexDocument document(&plugin), alias(&plugin);
            REQUIRE(document.openWorkSpace(workspace) == ErrFile::Success);
            REQUIRE(document.documentInfo().workSpaceName == workspace);
            {
                auto env = document.use();
                REQUIRE(env);
                REQUIRE(document.bookMarkComment(2) ==
                        QStringLiteral("bookmark"));
            }
            REQUIRE(alias.openWorkSpace(workspace) == ErrFile::Success);
            REQUIRE(document.close() == ErrFile::Success);
        }
        const auto deviceWorkspace =
            directory.filePath(QStringLiteral("device.wingpro"));
        data = {};
        data.file = WingHex::deviceUrl(driverInfo.id,
                                       QStringLiteral("device#workspace"));
        REQUIRE(WorkSpaceManager::saveWorkSpace(deviceWorkspace, data));
        {
            WingHexDocument document(&plugin);
            REQUIRE(document.openWorkSpace(deviceWorkspace) ==
                    ErrFile::Success);
            REQUIRE(driver.lastPath == QStringLiteral("device#workspace"));
            REQUIRE(document.close() == ErrFile::Success);
        }
        {
            WingHexDocument raw(&plugin), workspaceObject(&plugin);
            REQUIRE(raw.openFile(first) == ErrFile::Success);
            REQUIRE(workspaceObject.openWorkSpace(workspace) ==
                    ErrFile::AlreadyOpened);
            REQUIRE(raw.documentInfo().workSpaceName.isEmpty());
            REQUIRE(raw.close() == ErrFile::Success);
        }
        {
            WingHexDocument parent(&plugin);
            REQUIRE(parent.openFile(first) == ErrFile::Success);
            auto view = findView(WingHex::fileUrl(first));
            REQUIRE(view);
            auto clone = view->clone();
            REQUIRE(clone);
            const auto cloneHandle =
                host.assginHandleForOpenPluginView(&plugin, clone, false);
            REQUIRE(cloneHandle >= 0);
            REQUIRE(host.handle2EditorView(&plugin, cloneHandle) == view);
            REQUIRE(host.documentInfo(&plugin, cloneHandle).documentType ==
                    DocumentType::File);
            REQUIRE(parent.close() == ErrFile::Success);
        }
        {
            WingHexDocument invalid(&plugin);
            REQUIRE(invalid.openFile(directory.filePath(
                        QStringLiteral("missing"))) == ErrFile::NotExist);
            QFile bad(directory.filePath(QStringLiteral("invalid.wingpro")));
            REQUIRE(bad.open(QIODevice::WriteOnly));
            REQUIRE(bad.write("not json") == 8);
            bad.close();
            REQUIRE(invalid.openWorkSpace(bad.fileName()) ==
                    ErrFile::InvalidFormat);
            REQUIRE(invalid.openExt(QStringLiteral("missing"),
                                    QStringLiteral("device")) ==
                    ErrFile::NotExist);
            REQUIRE(plugin.openFile(QUrl(QStringLiteral(
                        "https://example.invalid"))) == ErrFile::InvalidFormat);
            REQUIRE(plugin.openFile(QUrl(QStringLiteral(
                        "wdrv://probedriver"))) == ErrFile::InvalidFormat);
        }
        {
            const auto released =
                host.openFile(&plugin, WingHex::fileUrl(first));
            const auto foreign =
                host.openFile(&other, WingHex::fileUrl(second));
            REQUIRE(released >= 0 && foreign >= 0 && released != foreign);
            REQUIRE(host.documentInfo(&other, released).fileNameUrl.isEmpty());
            REQUIRE(host.closeFileHandle(&plugin, released) ==
                    ErrFile::Success);
            REQUIRE(host.documentInfo(&plugin, released).fileNameUrl.isEmpty());
            const auto replacement =
                host.openFile(&plugin, WingHex::fileUrl(first));
            REQUIRE(replacement >= 0);
            REQUIRE(host.closeFile(&plugin, foreign, false) ==
                    ErrFile::NotExist);
            REQUIRE(host.documentInfo(&other, foreign).fileNameUrl ==
                    WingHex::fileUrl(second));
            REQUIRE(host.closeFile(&other, foreign, false) == ErrFile::Success);
            REQUIRE(host.closeFile(&plugin, replacement, false) ==
                    ErrFile::Success);
        }
        {
            std::vector<WingHexDocument> documents;
            for (int i = 0; i < 256; ++i) {
                documents.emplace_back(&plugin);
                const auto filePath = directory.filePath(
                    QStringLiteral("capacity-%1.bin").arg(i));
                QFile file(filePath);
                REQUIRE(file.open(QIODevice::WriteOnly));
                REQUIRE(file.write("abcdefgh") == 8);
                file.close();
                REQUIRE(documents.back().openFile(filePath) ==
                        ErrFile::Success);
            }
            REQUIRE(plugin.openFile(WingHex::fileUrl(directory.filePath(
                        QStringLiteral("capacity-0.bin")))) ==
                    host.m_plgviewMap[&plugin].contexts.front()->fid);
            WingHexDocument overflow(&plugin);
            REQUIRE(overflow.openFile(first) == ErrFile::TooManyOpenedFile);
            REQUIRE(documents.front().close() == ErrFile::Success);
            WingHexDocument next(&plugin);
            REQUIRE(next.openFile(second) == ErrFile::Success);
            REQUIRE(next.close() == ErrFile::Success);
            for (auto &document : documents) {
                if (document.isValid()) {
                    REQUIRE(document.close() == ErrFile::Success);
                }
            }
        }
        {
            auto ephemeral = std::make_unique<ProbePlugin>();
            host.applyFunctionTables(ephemeral.get());
            WingHexDocument document(ephemeral.get());
            REQUIRE(document.openFile(second) == ErrFile::Success);
            auto owner = ephemeral.get();
            ephemeral.reset();
            REQUIRE(!host.m_plgviewMap.contains(owner));
            auto view = findView(WingHex::fileUrl(second));
            REQUIRE(view);
            REQUIRE(window.closeEditor(view, false) == ErrFile::Success);
        }
        {
            const auto handle = plugin.openFile(WingHex::fileUrl(second));
            const auto alias = plugin.openFile(WingHex::fileUrl(second));
            REQUIRE(handle >= 0 && alias == handle);
            auto view = findView(WingHex::fileUrl(second));
            REQUIRE(view);
            REQUIRE(host.boundPluginsForView(view).count(&plugin) == 1);
            REQUIRE(plugin.switchDocument(handle));
            REQUIRE(plugin.beginMarco(QStringLiteral("shared handle")));
            REQUIRE(plugin.closeFileHandle(handle) == ErrFile::Success);
            REQUIRE(!plugin.isFileOpened(alias));
            REQUIRE(host.boundPluginsForView(view).isEmpty());
            const auto reopened = plugin.openFile(WingHex::fileUrl(second));
            REQUIRE(reopened >= 0);
            REQUIRE(plugin.switchDocument(reopened));
            REQUIRE(plugin.beginMarco(QStringLiteral("unfinished outer")));
            REQUIRE(plugin.beginMarco(QStringLiteral("unfinished inner")));
            REQUIRE(window.closeEditor(view, true) == ErrFile::Success);
            REQUIRE(host.boundPluginsForView(view).isEmpty());
            REQUIRE(host.m_plgviewMap[&plugin].contexts.isEmpty());
        }
        REQUIRE(plugin.acquired == plugin.released);
        REQUIRE(host.m_plgviewMap[&plugin].contexts.isEmpty());
        host._loadeddevs.removeAll(&driver);
        host._pinfos.remove(&driver);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        REQUIRE(EditorView::instances().size() == initialViews);
        std::cout << "Host document integration passed\n";
        return true;
    }
};

int main(int argc, char **argv) {
    qputenv("WINGHEX_DISABLE_PLUGIN_SYSTEM", "1");
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("WingHexDocumentProbe"));
    application.setOrganizationName(QStringLiteral("WingHexTests"));
    return PluginDocumentHostProbe::run() ? 0 : 1;
}

#include "host_probe.moc"
