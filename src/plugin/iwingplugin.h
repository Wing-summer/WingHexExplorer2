/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** You can redistribute this file and/or modify it under the terms of the
** BSD 3-Clause.
**
** THIS FILE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** =============================================================================
*/

#ifndef IWINGPLUGIN_H
#define IWINGPLUGIN_H

#include "settingpage.h"

#include <functional>
#include <memory>

#include <QCryptographicHash>
#include <QDockWidget>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QObject>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <QtCore>

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMetaObject>

/**
 * Don't try to modify this file, unless you are the dev
 * 不要尝试来修改该文件，除非你是开发者
 */

namespace WingHex {

using qusizetype = QIntegerForSizeof<std::size_t>::Unsigned;

Q_DECL_UNUSED constexpr auto SDKVERSION = 15;

Q_DECL_UNUSED static QString PLUGINDIR() {
    return QCoreApplication::applicationDirPath() + QStringLiteral("/plugin");
}

Q_DECL_UNUSED static QString HOSTRESPIMG(const QString &name) {
    return QStringLiteral(":/com.wingsummer.winghex/images/") + name +
           QStringLiteral(".png");
}

Q_NAMESPACE
enum ErrFile : int {
    Success = 0,
    Error = -1,
    UnSaved = -2,
    Permission = -3,
    NotExist = -4,
    AlreadyOpened = -5,
    IsNewFile = -6,
    IsDirver = -7,
    WorkSpaceUnSaved = -8,
    SourceFileChanged = -9,
    ClonedFile = -10,
    InvalidFormat = -11,
    TooManyOpenedFile = -12,
    NotAllowedInNoneGUIThread = -13
};
Q_ENUM_NS(ErrFile)

Q_NAMESPACE
enum class SelectionMode : int { Add, Remove, Single };
Q_ENUM_NS(SelectionMode)

struct FindResult {
    qsizetype offset = -1;
    qsizetype line = -1;
    qsizetype col = -1;
};

class IWingPlugin;

struct HexPosition {
    qsizetype line;
    int column;
    quint8 lineWidth;
    int nibbleindex;

    Q_REQUIRED_RESULT inline qsizetype offset() const {
        return static_cast<qsizetype>(line * lineWidth) + column;
    }
    inline qsizetype operator-(const HexPosition &rhs) const {
        return qsizetype(this->offset() - rhs.offset());
    }
    inline bool operator==(const HexPosition &rhs) const {
        return (line == rhs.line) && (column == rhs.column) &&
               (nibbleindex == rhs.nibbleindex);
    }
    inline bool operator!=(const HexPosition &rhs) const {
        return (line != rhs.line) || (column != rhs.column) ||
               (nibbleindex != rhs.nibbleindex);
    }
};

namespace WingPlugin {

class Reader : public QObject {
    Q_OBJECT
signals:
    Q_REQUIRED_RESULT bool isCurrentDocEditing();
    Q_REQUIRED_RESULT QString currentDocFilename();

    // document
    Q_REQUIRED_RESULT bool isReadOnly();
    Q_REQUIRED_RESULT bool isKeepSize();
    Q_REQUIRED_RESULT bool isLocked();
    Q_REQUIRED_RESULT qsizetype documentLines();
    Q_REQUIRED_RESULT qsizetype documentBytes();
    Q_REQUIRED_RESULT WingHex::HexPosition currentPos();
    Q_REQUIRED_RESULT qsizetype currentRow();
    Q_REQUIRED_RESULT qsizetype currentColumn();
    Q_REQUIRED_RESULT qsizetype currentOffset();
    Q_REQUIRED_RESULT qsizetype selectedLength();

    Q_REQUIRED_RESULT QByteArray selectedBytes(qsizetype index);
    Q_REQUIRED_RESULT QByteArrayList selectionBytes();

    Q_REQUIRED_RESULT WingHex::HexPosition selectionStart(qsizetype index);
    Q_REQUIRED_RESULT WingHex::HexPosition selectionEnd(qsizetype index);
    Q_REQUIRED_RESULT qsizetype selectionLength(qsizetype index);
    Q_REQUIRED_RESULT qsizetype selectionCount();

    Q_REQUIRED_RESULT bool stringVisible();
    Q_REQUIRED_RESULT bool addressVisible();
    Q_REQUIRED_RESULT bool headerVisible();
    Q_REQUIRED_RESULT quintptr addressBase();
    Q_REQUIRED_RESULT bool isModified();

    Q_REQUIRED_RESULT qint8 readInt8(qsizetype offset);
    Q_REQUIRED_RESULT qint16 readInt16(qsizetype offset);
    Q_REQUIRED_RESULT qint32 readInt32(qsizetype offset);
    Q_REQUIRED_RESULT qint64 readInt64(qsizetype offset);
    Q_REQUIRED_RESULT float readFloat(qsizetype offset);
    Q_REQUIRED_RESULT double readDouble(qsizetype offset);
    Q_REQUIRED_RESULT QString readString(qsizetype offset,
                                         const QString &encoding = QString());
    Q_REQUIRED_RESULT QByteArray readBytes(qsizetype offset, qsizetype count);

    Q_REQUIRED_RESULT qsizetype searchForward(qsizetype begin,
                                              const QByteArray &ba);
    Q_REQUIRED_RESULT qsizetype searchBackward(qsizetype begin,
                                               const QByteArray &ba);
    Q_REQUIRED_RESULT QList<qsizetype>
    findAllBytes(qsizetype begin, qsizetype end, const QByteArray &b);

    // metadata
    Q_REQUIRED_RESULT bool lineHasMetadata(qsizetype line);

    // bookmark
    Q_REQUIRED_RESULT bool lineHasBookMark(qsizetype line);
    Q_REQUIRED_RESULT QList<qsizetype> getsBookmarkPos(qsizetype line);
    Q_REQUIRED_RESULT QString bookMarkComment(qsizetype pos);
    Q_REQUIRED_RESULT bool existBookMark(qsizetype pos);

    // extension
    Q_REQUIRED_RESULT QStringList getSupportedEncodings();
    Q_REQUIRED_RESULT QStringList getStorageDrivers();
    Q_REQUIRED_RESULT QString currentEncoding();
};

class Controller : public QObject {
    Q_OBJECT
signals:
    // document
    Q_REQUIRED_RESULT bool switchDocument(int handle);
    Q_REQUIRED_RESULT bool raiseDocument(int handle);
    Q_REQUIRED_RESULT bool setLockedFile(bool b);
    Q_REQUIRED_RESULT bool setKeepSize(bool b);
    Q_REQUIRED_RESULT bool setStringVisible(bool b);
    Q_REQUIRED_RESULT bool setAddressVisible(bool b);
    Q_REQUIRED_RESULT bool setHeaderVisible(bool b);
    Q_REQUIRED_RESULT bool setAddressBase(quintptr base);

    Q_REQUIRED_RESULT bool writeInt8(qsizetype offset, qint8 value);
    Q_REQUIRED_RESULT bool writeInt16(qsizetype offset, qint16 value);
    Q_REQUIRED_RESULT bool writeInt32(qsizetype offset, qint32 value);
    Q_REQUIRED_RESULT bool writeInt64(qsizetype offset, qint64 value);
    Q_REQUIRED_RESULT bool writeFloat(qsizetype offset, float value);
    Q_REQUIRED_RESULT bool writeDouble(qsizetype offset, double value);
    Q_REQUIRED_RESULT bool writeString(qsizetype offset, const QString &value,
                                       const QString &encoding = QString());
    Q_REQUIRED_RESULT bool writeBytes(qsizetype offset, const QByteArray &data);

    Q_REQUIRED_RESULT bool insertInt8(qsizetype offset, qint8 value);
    Q_REQUIRED_RESULT bool insertInt16(qsizetype offset, qint16 value);
    Q_REQUIRED_RESULT bool insertInt32(qsizetype offset, qint32 value);
    Q_REQUIRED_RESULT bool insertInt64(qsizetype offset, qint64 value);
    Q_REQUIRED_RESULT bool insertFloat(qsizetype offset, float value);
    Q_REQUIRED_RESULT bool insertDouble(qsizetype offset, double value);
    Q_REQUIRED_RESULT bool insertString(qsizetype offset, const QString &value,
                                        const QString &encoding = QString());
    Q_REQUIRED_RESULT bool insertBytes(qsizetype offset,
                                       const QByteArray &data);

    Q_REQUIRED_RESULT bool appendInt8(qint8 value);
    Q_REQUIRED_RESULT bool appendInt16(qint16 value);
    Q_REQUIRED_RESULT bool appendInt32(qint32 value);
    Q_REQUIRED_RESULT bool appendInt64(qint64 value);
    Q_REQUIRED_RESULT bool appendFloat(float value);
    Q_REQUIRED_RESULT bool appendDouble(double value);
    Q_REQUIRED_RESULT bool appendString(const QString &value,
                                        const QString &encoding = QString());
    Q_REQUIRED_RESULT bool appendBytes(const QByteArray &data);

    Q_REQUIRED_RESULT bool remove(qsizetype offset, qsizetype len);
    Q_REQUIRED_RESULT bool removeAllBytes(); // extension

    // cursor
    Q_REQUIRED_RESULT bool moveTo(qsizetype line, qsizetype column,
                                  int nibbleindex = 1,
                                  bool clearSelection = true);
    Q_REQUIRED_RESULT bool moveTo(qsizetype offset, bool clearSelection = true);
    Q_REQUIRED_RESULT bool select(qsizetype offset, qsizetype length,
                                  SelectionMode mode = SelectionMode::Add);
    Q_REQUIRED_RESULT bool setInsertionMode(bool isinsert);

    // metadata
    Q_REQUIRED_RESULT bool foreground(qsizetype begin, qusizetype length,
                                      const QColor &fgcolor);
    Q_REQUIRED_RESULT bool background(qsizetype begin, qusizetype length,
                                      const QColor &bgcolor);
    Q_REQUIRED_RESULT bool comment(qsizetype begin, qusizetype length,
                                   const QString &comment);

    Q_REQUIRED_RESULT bool metadata(qsizetype begin, qusizetype length,
                                    const QColor &fgcolor,
                                    const QColor &bgcolor,
                                    const QString &comment);

    Q_REQUIRED_RESULT bool removeMetadata(qsizetype offset);
    Q_REQUIRED_RESULT bool clearMetadata();
    Q_REQUIRED_RESULT bool setMetaVisible(bool b);
    Q_REQUIRED_RESULT bool setMetafgVisible(bool b);
    Q_REQUIRED_RESULT bool setMetabgVisible(bool b);
    Q_REQUIRED_RESULT bool setMetaCommentVisible(bool b);

    // mainwindow
    Q_REQUIRED_RESULT WingHex::ErrFile newFile();
    Q_REQUIRED_RESULT WingHex::ErrFile openFile(const QString &filename);
    Q_REQUIRED_RESULT WingHex::ErrFile openRegionFile(const QString &filename,
                                                      qsizetype start = 0,
                                                      qsizetype length = 1024);
    Q_REQUIRED_RESULT WingHex::ErrFile openDriver(const QString &driver);
    Q_REQUIRED_RESULT WingHex::ErrFile closeFile(int handle,
                                                 bool force = false);
    Q_REQUIRED_RESULT WingHex::ErrFile saveFile(int handle,
                                                bool ignoreMd5 = false);
    Q_REQUIRED_RESULT WingHex::ErrFile
    exportFile(int handle, const QString &savename, bool ignoreMd5 = false);
    Q_REQUIRED_RESULT WingHex::ErrFile
    saveAsFile(int handle, const QString &savename, bool ignoreMd5 = false);
    Q_REQUIRED_RESULT WingHex::ErrFile closeCurrentFile(bool force = false);
    Q_REQUIRED_RESULT WingHex::ErrFile saveCurrentFile(bool ignoreMd5 = false);

    // bookmark
    Q_REQUIRED_RESULT bool addBookMark(qsizetype pos, const QString &comment);
    Q_REQUIRED_RESULT bool modBookMark(qsizetype pos, const QString &comment);
    Q_REQUIRED_RESULT bool removeBookMark(qsizetype pos);
    Q_REQUIRED_RESULT bool clearBookMark();

    // workspace
    Q_REQUIRED_RESULT WingHex::ErrFile openWorkSpace(const QString &filename);
    Q_REQUIRED_RESULT bool setCurrentEncoding(const QString &encoding);

    // extension
    bool closeAllPluginFiles();
};

class MessageBox : public QObject {
    Q_OBJECT
signals:
    void aboutQt(QWidget *parent = nullptr, const QString &title = QString());

    QMessageBox::StandardButton information(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton question(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons =
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton
    warning(QWidget *parent, const QString &title, const QString &text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton
    critical(QWidget *parent, const QString &title, const QString &text,
             QMessageBox::StandardButtons buttons = QMessageBox::Ok,
             QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    void about(QWidget *parent, const QString &title, const QString &text);

    QMessageBox::StandardButton
    msgbox(QWidget *parent, QMessageBox::Icon icon, const QString &title,
           const QString &text,
           QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
};

class InputBox : public QObject {
    Q_OBJECT
signals:
    Q_REQUIRED_RESULT QString
    getText(QWidget *parent, const QString &title, const QString &label,
            QLineEdit::EchoMode echo = QLineEdit::Normal,
            const QString &text = QString(), bool *ok = nullptr,
            Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    Q_REQUIRED_RESULT QString getMultiLineText(
        QWidget *parent, const QString &title, const QString &label,
        const QString &text = QString(), bool *ok = nullptr,
        Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    Q_REQUIRED_RESULT QString
    getItem(QWidget *parent, const QString &title, const QString &label,
            const QStringList &items, int current = 0, bool editable = true,
            bool *ok = nullptr,
            Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    Q_REQUIRED_RESULT int getInt(QWidget *parent, const QString &title,
                                 const QString &label, int value = 0,
                                 int minValue = -2147483647,
                                 int maxValue = 2147483647, int step = 1,
                                 bool *ok = nullptr);

    Q_REQUIRED_RESULT double getDouble(QWidget *parent, const QString &title,
                                       const QString &label, double value = 0,
                                       double minValue = -2147483647,
                                       double maxValue = 2147483647,
                                       int decimals = 1, bool *ok = nullptr,
                                       double step = 1);
};

class FileDialog : public QObject {
    Q_OBJECT
signals:
    Q_REQUIRED_RESULT QString getExistingDirectory(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(),
        QFileDialog::Options options = QFileDialog::ShowDirsOnly);

    Q_REQUIRED_RESULT QString getOpenFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    Q_REQUIRED_RESULT QStringList getOpenFileNames(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    Q_REQUIRED_RESULT QString getSaveFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());
};

class ColorDialog : public QObject {
    Q_OBJECT
signals:
    Q_REQUIRED_RESULT QColor getColor(const QString &caption,
                                      QWidget *parent = nullptr);
};

class DataVisual : public QObject {
    Q_OBJECT
public:
    typedef std::function<void(const QModelIndex &)> ClickedCallBack;
    typedef ClickedCallBack DoubleClickedCallBack;

signals:
    bool updateText(const QString &data, const QString &title = {});
    bool updateTextList(const QStringList &data, const QString &title = {},
                        ClickedCallBack clicked = {},
                        DoubleClickedCallBack dblClicked = {});

    Q_REQUIRED_RESULT bool
    updateTextTree(const QString &json, const QString &title = {},
                   ClickedCallBack clicked = {},
                   DoubleClickedCallBack dblClicked = {});
    Q_REQUIRED_RESULT bool
    updateTextTable(const QString &json, const QStringList &headers,
                    const QStringList &headerNames = {},
                    const QString &title = {}, ClickedCallBack clicked = {},
                    DoubleClickedCallBack dblClicked = {});

    // API for Qt Plugin Only
    Q_REQUIRED_RESULT bool
    updateTextListByModel(QAbstractItemModel *model, const QString &title = {},
                          ClickedCallBack clicked = {},
                          DoubleClickedCallBack dblClicked = {});
    Q_REQUIRED_RESULT bool
    updateTextTableByModel(QAbstractItemModel *model, const QString &title = {},
                           ClickedCallBack clicked = {},
                           DoubleClickedCallBack dblClicked = {});
    Q_REQUIRED_RESULT bool
    updateTextTreeByModel(QAbstractItemModel *model, const QString &title = {},
                          ClickedCallBack clicked = {},
                          DoubleClickedCallBack dblClicked = {});
};

} // namespace WingPlugin

const auto WINGSUMMER = QStringLiteral("wingsummer");

struct WingDockWidgetInfo {
    QString widgetName;
    QString displayName;
    QWidget *widget = nullptr;
    Qt::DockWidgetArea area = Qt::DockWidgetArea::NoDockWidgetArea;
};

struct WingRibbonToolBoxInfo {
    struct RibbonCatagories {
        const QString FILE = QStringLiteral("File");
        const QString EDIT = QStringLiteral("Edit");
        const QString VIEW = QStringLiteral("View");
        const QString SCRIPT = QStringLiteral("Script");
        const QString PLUGIN = QStringLiteral("Plugin");
        const QString SETTING = QStringLiteral("Setting");
        const QString ABOUT = QStringLiteral("About");
    };

    QString catagory;
    QString displayName;

    struct Toolbox {
        QString name;
        QList<QToolButton *> tools;
    };
    QList<Toolbox> toolboxs;
};

class WingEditorViewWidget : public QWidget {
    Q_OBJECT

public:
    explicit WingEditorViewWidget(QWidget *parent = nullptr)
        : QWidget(parent) {}

public:
    virtual QIcon icon() const = 0;

    virtual QString name() const = 0;

    virtual QString id() const = 0;

signals:
    void docSaved(bool saved);

public slots:
    virtual void toggled(bool isVisible) = 0;

    virtual void loadState(const QByteArray &state) { Q_UNUSED(state); }

    virtual QByteArray saveState() { return {}; }

    virtual WingEditorViewWidget *clone() = 0;

signals:
    void raiseView();
};

struct WingDependency {
    QString puid;
    uint version;
    QByteArray md5; // optional, but recommend
};

struct SenderInfo {
    QString plgcls;
    QString puid;
    QVariant meta;
};

#ifdef WING_SERVICE
#undef WING_SERVICE
#endif

#define WING_SERVICE Q_INVOKABLE

// for bad broken Qt API
#define WINGAPI_ARG(type, data) QArgument<type>(#type, data)
#define WINGAPI_RETURN_ARG(type, data) QReturnArgument<type>(#type, data)

enum class AppTheme { Dark, Light };

class IWingPlugin : public QObject {
    Q_OBJECT
public:
    typedef std::function<QVariant(const QVariantList &)> ScriptFn;

    enum MetaType : uint {
        Void,

        Bool,
        Int,
        UInt,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int64,
        UInt64,

        Float,
        Double,

        String,
        Char,
        Color,

        MetaMax, // reserved
        MetaTypeMask = 0xFFFFF,
        Ref = 0x10000000,
        Array = 0x100000,
        Map = 0x200000
    };

    static_assert(MetaType::MetaMax < MetaType::Array);

    struct ScriptFnInfo {
        MetaType ret;
        QVector<QPair<MetaType, QString>> params;
        ScriptFn fn;
    };

    enum class RegisteredEvent : uint {
        None,
        AppReady = 1u,
        SelectionChanged = 1u << 1,
        CursorPositionChanged = 1u << 2,
        FileOpened = 1u << 3,
        FileSaved = 1u << 4,
        FileSwitched = 1u << 5,
        FileClosed = 1u << 6
    };
    Q_DECLARE_FLAGS(RegisteredEvents, RegisteredEvent)

    enum class PluginFileEvent { Opened, Saved, Switched, Closed };

public:
    virtual int sdkVersion() const = 0;
    virtual const QString signature() const = 0;
    virtual ~IWingPlugin() = default;

    virtual bool init(const std::unique_ptr<QSettings> &set) = 0;
    virtual void unload(std::unique_ptr<QSettings> &set) = 0;
    virtual const QString pluginName() const = 0;
    virtual QIcon pluginIcon() const { return {}; }
    virtual const QString pluginAuthor() const = 0;
    virtual uint pluginVersion() const = 0;
    virtual const QString pluginComment() const = 0;

    virtual QList<WingDependency> dependencies() const { return {}; }

    virtual RegisteredEvents registeredEvents() const {
        return RegisteredEvent::None;
    }

public:
    virtual QList<WingDockWidgetInfo> registeredDockWidgets() const {
        return {};
    }
    virtual QMenu *registeredHexContextMenu() const { return nullptr; }
    virtual QList<WingRibbonToolBoxInfo> registeredRibbonTools() const {
        return {};
    }
    // QMap<page, whether is visible in setting tab>
    virtual QHash<SettingPage *, bool> registeredSettingPages() const {
        return {};
    }
    virtual QList<PluginPage *> registeredPages() const { return {}; }
    virtual QList<WingEditorViewWidget *> registeredEditorViewWidgets() const {
        return {};
    }

public:
    // QHash< function-name, fn >
    virtual QHash<QString, ScriptFnInfo> registeredScriptFns() const {
        return {};
    }

    // QHash< enum , members >
    virtual QHash<QString, QList<QPair<QString, int>>>
    registeredScriptEnums() const {
        return {};
    }

public:
    virtual void eventSelectionChanged(const QByteArrayList &selections,
                                       bool isPreview) {
        Q_UNUSED(selections);
        Q_UNUSED(isPreview);
    }

    virtual void eventCursorPositionChanged(const WingHex::HexPosition &pos) {
        Q_UNUSED(pos);
    }

    virtual void eventPluginFile(PluginFileEvent e, const QString &newfileName,
                                 const QString &oldfileName) {
        Q_UNUSED(e);
        Q_UNUSED(newfileName);
        Q_UNUSED(oldfileName);
    }

    virtual void eventReady() {}

signals:
    // extension and exposed to WingHexAngelScript
    void toast(const QPixmap &icon, const QString &message);
    void trace(const QString &message);
    void debug(const QString &message);
    void warn(const QString &message);
    void error(const QString &message);
    void info(const QString &message);

    // theme
    WingHex::AppTheme currentAppTheme();

    // not available for AngelScript
    // only for plugin UI extenstion

    QDialog *createDialog(QWidget *content);

    bool invokeService(const QString &puid, const char *method,
                       Qt::ConnectionType type, QGenericReturnArgument ret,
                       QGenericArgument val0 = QGenericArgument(nullptr),
                       QGenericArgument val1 = QGenericArgument(),
                       QGenericArgument val2 = QGenericArgument(),
                       QGenericArgument val3 = QGenericArgument(),
                       QGenericArgument val4 = QGenericArgument(),
                       QGenericArgument val5 = QGenericArgument(),
                       QGenericArgument val6 = QGenericArgument(),
                       QGenericArgument val7 = QGenericArgument(),
                       QGenericArgument val8 = QGenericArgument());

public:
    inline bool invokeService(const QString &puid, const char *member,
                              QGenericReturnArgument ret,
                              QGenericArgument val0 = QGenericArgument(nullptr),
                              QGenericArgument val1 = QGenericArgument(),
                              QGenericArgument val2 = QGenericArgument(),
                              QGenericArgument val3 = QGenericArgument(),
                              QGenericArgument val4 = QGenericArgument(),
                              QGenericArgument val5 = QGenericArgument(),
                              QGenericArgument val6 = QGenericArgument(),
                              QGenericArgument val7 = QGenericArgument(),
                              QGenericArgument val8 = QGenericArgument()) {
        return emit invokeService(puid, member, Qt::AutoConnection, ret, val0,
                                  val1, val2, val3, val4, val5, val6, val7,
                                  val8);
    }

    inline bool invokeService(const QString &puid, const char *member,
                              Qt::ConnectionType type, QGenericArgument val0,
                              QGenericArgument val1 = QGenericArgument(),
                              QGenericArgument val2 = QGenericArgument(),
                              QGenericArgument val3 = QGenericArgument(),
                              QGenericArgument val4 = QGenericArgument(),
                              QGenericArgument val5 = QGenericArgument(),
                              QGenericArgument val6 = QGenericArgument(),
                              QGenericArgument val7 = QGenericArgument(),
                              QGenericArgument val8 = QGenericArgument()) {
        return emit invokeService(puid, member, type, QGenericReturnArgument(),
                                  val0, val1, val2, val3, val4, val5, val6,
                                  val7, val8);
    }

    inline bool invokeService(const QString &puid, const char *member,
                              QGenericArgument val0,
                              QGenericArgument val1 = QGenericArgument(),
                              QGenericArgument val2 = QGenericArgument(),
                              QGenericArgument val3 = QGenericArgument(),
                              QGenericArgument val4 = QGenericArgument(),
                              QGenericArgument val5 = QGenericArgument(),
                              QGenericArgument val6 = QGenericArgument(),
                              QGenericArgument val7 = QGenericArgument(),
                              QGenericArgument val8 = QGenericArgument()) {
        return emit invokeService(puid, member, Qt::AutoConnection,
                                  QGenericReturnArgument(), val0, val1, val2,
                                  val3, val4, val5, val6, val7, val8);
    }

public:
    WingPlugin::Reader reader;
    WingPlugin::Controller controller;
    WingPlugin::MessageBox msgbox;
    WingPlugin::InputBox inputbox;
    WingPlugin::FileDialog filedlg;
    WingPlugin::ColorDialog colordlg;
    WingPlugin::DataVisual visual;
};

} // namespace WingHex

Q_DECLARE_METATYPE(WingHex::SenderInfo)
Q_DECLARE_INTERFACE(WingHex::IWingPlugin, "com.wingsummer.iwingplugin")

#endif // IWINGPLUGIN_H
