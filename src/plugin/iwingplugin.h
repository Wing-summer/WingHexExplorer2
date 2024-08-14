#ifndef IWINGPLUGIN_H
#define IWINGPLUGIN_H

#include "settingpage.h"

#include <QCryptographicHash>
#include <QDockWidget>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <QtCore>

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

/**
 * Don't try to modify this file, unless you are the dev
 * 不要尝试来修改该文件，除非你是开发者
 */

namespace WingHex {

Q_DECL_UNUSED constexpr auto SDKVERSION = 12;

Q_DECL_UNUSED static QString GETPLUGINQM(const QString &name) {
    return QCoreApplication::applicationDirPath() +
           QStringLiteral("/plglang/") + name;
};

Q_DECL_UNUSED static QString PLUGINDIR() {
    return QCoreApplication::applicationDirPath() + QStringLiteral("/plugin");
}

Q_DECL_UNUSED static QString HOSTRESPIMG(const QString &name) {
    return QStringLiteral(":/com.wingsummer.winghex/images/") + name +
           QStringLiteral(".png");
}

Q_DECL_UNUSED constexpr auto HOSTAUTHORPIMG =
    ":/com.wingsummer.winghex/images/author.jpg";
Q_DECL_UNUSED constexpr auto SPONSORQRCODE =
    ":/com.wingsummer.winghex/images/sponsorqr.png";

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
    ClonedFile = -10
};
Q_ENUM_NS(ErrFile)

struct FindResult {
    qsizetype offset = -1;
    qsizetype line = -1;
    qsizetype col = -1;
};

struct BookMark {
    qsizetype pos = -1;
    QString comment;
};

class IWingPlugin;

struct HexPosition {
    qsizetype line;
    int column;
    quint8 lineWidth;
    int nibbleindex;

    inline qsizetype offset() const {
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

struct HexMetadataAbsoluteItem {
    qsizetype begin;
    qsizetype end;
    QColor foreground, background;
    QString comment;

    // added by wingsummer
    bool operator==(const HexMetadataAbsoluteItem &item) {
        return begin == item.begin && end == item.end &&
               foreground == item.foreground && background == item.background &&
               comment == item.comment;
    }

    HexMetadataAbsoluteItem() = default;

    HexMetadataAbsoluteItem(qsizetype begin, qsizetype end, QColor foreground,
                            QColor background, QString comment) {
        this->begin = begin;
        this->end = end;
        this->foreground = foreground;
        this->background = background;
        this->comment = comment;
    }
};

struct HexMetadataItem {
    qsizetype line;
    int start, length;
    QColor foreground, background;
    QString comment;

    HexMetadataItem() = default;

    // added by wingsummer
    bool operator==(const HexMetadataItem &item) {
        return line == item.line && start == item.start &&
               foreground == item.foreground && background == item.background &&
               comment == item.comment;
    }

    HexMetadataItem(qsizetype line, int start, int length, QColor foreground,
                    QColor background, QString comment) {
        this->line = line;
        this->start = start;
        this->length = length;
        this->foreground = foreground;
        this->background = background;
        this->comment = comment;
    }
};

typedef QList<HexMetadataItem> HexLineMetadata;

enum class WingPluginMessage {
    PluginLoading,
    PluginLoaded,
    PluginUnLoading,
    PluginUnLoaded,
    ErrorMessage,
    MessageResponse,
    HookMessage
};

enum class ResponseMsg { UnImplement, Success, ErrorParams, Permission };

enum HookIndex {
    None = 0,
    OpenFileBegin = 1,
    OpenFileEnd = 2,
    CloseFileBegin = 4,
    CloseFileEnd = 8,
    NewFileBegin = 16,
    NewFileEnd = 32,
    DocumentSwitched = 64
};
Q_DECLARE_FLAGS(HookIndices, HookIndex)

namespace WingPlugin {

class Reader : public QObject {
    Q_OBJECT
signals:
    bool isCurrentDocEditing();
    QString currentDocFilename();

    // document
    bool isReadOnly();
    bool isKeepSize();
    bool isLocked();
    qsizetype documentLines();
    qsizetype documentBytes();
    HexPosition currentPos();
    HexPosition selectionPos();
    qsizetype currentRow();
    qsizetype currentColumn();
    qsizetype currentOffset();
    qsizetype selectedLength();
    QByteArray selectedBytes();

    bool stringVisible();
    bool addressVisible();
    bool headerVisible();
    quintptr addressBase();
    bool isModified();

    bool isEmpty();
    bool atEnd();
    bool canUndo();
    bool canRedo();

    bool copy(bool hex = false);

    // extension
    qint8 readInt8(qsizetype offset);
    qint16 readInt16(qsizetype offset);
    qint32 readInt32(qsizetype offset);
    qint64 readInt64(qsizetype offset);
    QString readString(qsizetype offset, const QString &encoding = QString());
    QByteArray readBytes(qsizetype offset, qsizetype count);

    // an extension for AngelScript
    // void read(? &in);    // this function can read bytes to input container

    qsizetype searchForward(qsizetype begin, const QByteArray &ba);
    qsizetype searchBackward(qsizetype begin, const QByteArray &ba);
    QList<qsizetype> findAllBytes(qsizetype begin, qsizetype end,
                                  const QByteArray &b);

    // render
    qsizetype documentLastLine();
    qsizetype documentLastColumn();

    // metadata
    bool lineHasMetadata(qsizetype line);
    QList<HexMetadataAbsoluteItem> getMetadatas(qsizetype offset);
    HexLineMetadata getMetaLine(qsizetype line);

    // bookmark
    bool lineHasBookMark(qsizetype line);
    QList<qsizetype> getsBookmarkPos(qsizetype line);
    BookMark bookMark(qsizetype pos);
    QString bookMarkComment(qsizetype pos);
    QList<BookMark> getBookMarks();
    bool existBookMark(qsizetype pos);

    // extension
    QStringList getOpenFiles();
    QStringList getSupportedEncodings();
    QString currentEncoding();

    // not available for AngelScript
    // only for plugin UI extenstion
    QDialog *createDialog(QWidget *content);
};

class Controller : public QObject {
    Q_OBJECT
signals:
    // document
    bool switchDocument(int handle);
    bool raiseDocument(int handle);
    bool setLockedFile(bool b);
    bool setKeepSize(bool b);
    bool setStringVisible(bool b);
    bool setAddressVisible(bool b);
    bool setHeaderVisible(bool b);
    bool setAddressBase(quintptr base);

    bool undo();
    bool redo();
    bool cut(bool hex = false);
    bool paste(bool hex = false);

    bool write(qsizetype offset, const uchar b);
    bool write(qsizetype offset, const QByteArray &data);

    // extesion
    bool writeInt8(qsizetype offset, qint8 value);
    bool writeInt16(qsizetype offset, qint16 value);
    bool writeInt32(qsizetype offset, qint32 value);
    bool writeInt64(qsizetype offset, qint64 value);
    bool writeString(qsizetype offset, const QString &value,
                     const QString &encoding = QString());

    bool insert(qsizetype offset, const uchar b);
    bool insert(qsizetype offset, const QByteArray &data);

    // extesion
    bool insertInt8(qsizetype offset, qint8 value);
    bool insertInt16(qsizetype offset, qint16 value);
    bool insertInt32(qsizetype offset, qint32 value);
    bool insertInt64(qsizetype offset, qint64 value);
    bool insertString(qsizetype offset, const QString &value,
                      const QString &encoding = QString());

    bool append(uchar b);
    bool append(const QByteArray &data);

    // extension
    bool appendInt8(qint8 value);
    bool appendInt16(qint16 value);
    bool appendInt32(qint32 value);
    bool appendInt64(qint64 value);
    bool appendString(const QString &value,
                      const QString &encoding = QString());

    bool remove(qsizetype offset, qsizetype len);
    bool removeAll(); // extension

    // cursor
    bool moveTo(const HexPosition &pos);
    bool moveTo(qsizetype line, qsizetype column, int nibbleindex = 1);
    bool moveTo(qsizetype offset);
    bool select(qsizetype line, qsizetype column, int nibbleindex = 1);
    bool selectOffset(qsizetype offset, qsizetype length);
    bool setInsertionMode(bool isinsert);
    bool enabledCursor(bool b);

    // metadata
    bool metadata(qsizetype begin, qsizetype end, const QColor &fgcolor,
                  const QColor &bgcolor, const QString &comment);
    bool metadata(qsizetype line, qsizetype start, qsizetype length,
                  const QColor &fgcolor, const QColor &bgcolor,
                  const QString &comment);
    bool removeMetadata(qsizetype offset);
    bool clearMeta();
    bool color(qsizetype line, qsizetype start, qsizetype length,
               const QColor &fgcolor, const QColor &bgcolor);
    bool foreground(qsizetype line, qsizetype start, qsizetype length,
                    const QColor &fgcolor);
    bool background(qsizetype line, qsizetype start, qsizetype length,
                    const QColor &bgcolor);
    bool comment(qsizetype line, qsizetype start, qsizetype length,
                 const QString &comment);
    bool applyMetas(const QList<HexMetadataAbsoluteItem> &metas);
    bool setMetaVisible(bool b);
    bool setMetafgVisible(bool b);
    bool setMetabgVisible(bool b);
    bool setMetaCommentVisible(bool b);

    // mainwindow
    bool newFile();
    ErrFile openFile(const QString &filename);
    ErrFile openRegionFile(const QString &filename, qsizetype start = 0,
                           qsizetype length = 1024);
    ErrFile openDriver(const QString &driver);
    ErrFile closeFile(const QString &filename, bool force = false);
    ErrFile saveFile(const QString &filename, bool ignoreMd5 = false);
    ErrFile exportFile(const QString &filename, const QString &savename,
                       bool ignoreMd5 = false);
    bool exportFileGUI();
    ErrFile saveasFile(const QString &filename, const QString &savename,
                       bool ignoreMd5 = false);
    bool saveasFileGUI();
    ErrFile closeCurrentFile(bool force = false);
    ErrFile saveCurrentFile(bool ignoreMd5 = false);
    bool openFileGUI();
    bool openRegionFileGUI();
    bool openDriverGUI();
    bool findGUI();
    bool gotoGUI();
    bool fillGUI();
    bool fillZeroGUI();

    // bookmark
    bool addBookMark(qsizetype pos, const QString &comment);
    bool modBookMark(qsizetype pos, const QString &comment);
    bool applyBookMarks(const QList<BookMark> &books);
    bool removeBookMark(qsizetype pos);
    bool clearBookMark();

    // workspace
    bool openWorkSpace(const QString &filename);
    bool setCurrentEncoding(const QString &encoding);
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
    QString getText(QWidget *parent, const QString &title, const QString &label,
                    QLineEdit::EchoMode echo = QLineEdit::Normal,
                    const QString &text = QString(), bool *ok = nullptr,
                    Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
    QString
    getMultiLineText(QWidget *parent, const QString &title,
                     const QString &label, const QString &text = QString(),
                     bool *ok = nullptr,
                     Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    QString getItem(QWidget *parent, const QString &title, const QString &label,
                    const QStringList &items, int current = 0,
                    bool editable = true, bool *ok = nullptr,
                    Qt::InputMethodHints inputMethodHints = Qt::ImhNone);

    int getInt(QWidget *parent, const QString &title, const QString &label,
               int value = 0, int minValue = -2147483647,
               int maxValue = 2147483647, int step = 1, bool *ok = nullptr);

    double getDouble(QWidget *parent, const QString &title,
                     const QString &label, double value = 0,
                     double minValue = -2147483647,
                     double maxValue = 2147483647, int decimals = 1,
                     bool *ok = nullptr, double step = 1);
};

class FileDialog : public QObject {
    Q_OBJECT
signals:
    QString getExistingDirectory(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(),
        QFileDialog::Options options = QFileDialog::ShowDirsOnly);

    QString getOpenFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    QStringList getOpenFileNames(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    QString getSaveFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());
};

class ColorDialog : public QObject {
    Q_OBJECT
signals:
    QColor getColor(const QString &caption, QWidget *parent = nullptr);
};

} // namespace WingPlugin

struct WingPluginInfo {
    QString pluginName;
    QString pluginAuthor;
    uint pluginVersion;
    QString puid;
    QString pluginComment;
};

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
    virtual QIcon icon() const = 0;

    virtual QString name() const = 0;

public slots:
    virtual void toggled(bool isVisible) = 0;

    virtual void loadState(QByteArray state) {}

    virtual QByteArray saveState() { return {}; }

    virtual WingEditorViewWidget *clone() = 0;

signals:
    void raise();
};

class IWingPlugin : public QObject {
    Q_OBJECT
public:
    virtual int sdkVersion() const = 0;
    virtual const QString signature() const = 0;
    QString const puid() const { return GetPUID(this); }
    virtual ~IWingPlugin() = default;

    virtual bool init(const QList<WingPluginInfo> &loadedplugin) = 0;
    virtual void unload() = 0;
    virtual const QString pluginName() const = 0;
    virtual const QString pluginAuthor() const = 0;
    virtual uint pluginVersion() const = 0;
    virtual const QString pluginComment() const = 0;
    virtual HookIndices getHookSubscribe() const { return HookIndex::None; }

    virtual QList<WingDockWidgetInfo> registeredDockWidgets() const {
        return {};
    }
    virtual QMenu *registeredHexContextMenu() const { return nullptr; }
    virtual QList<WingRibbonToolBoxInfo> registeredRibbonTools() const {
        return {};
    }
    virtual QList<SettingPage *> registeredSettingPages() const { return {}; }
    virtual QList<WingEditorViewWidget *> registeredEditorViewWidgets() const {
        return {};
    }

    static QString GetPUID(const IWingPlugin *plugin) {
        auto str = WINGSUMMER + plugin->pluginName() + plugin->pluginAuthor() +
                   QString::number(plugin->pluginVersion());
        return QCryptographicHash::hash(str.toLatin1(), QCryptographicHash::Md5)
            .toHex();
    }

public slots:
    virtual void plugin2MessagePipe(WingPluginMessage type,
                                    const QVariantList &msg) = 0;

signals:
    // extension and exposed to WingHexAngelScript
    void toast(const QPixmap &icon, const QString &message);
    void debug(const QString &message);
    void warn(const QString &message);
    void error(const QString &message);
    void info(const QString &message);

public:
    WingPlugin::Reader reader;
    WingPlugin::Controller controller;
    WingPlugin::MessageBox msgbox;
    WingPlugin::InputBox inputbox;
    WingPlugin::FileDialog filedlg;
    WingPlugin::ColorDialog colordlg;
};

} // namespace WingHex

constexpr auto IWINGPLUGIN_INTERFACE_IID = "com.wingsummer.iwingplugin";

Q_DECLARE_METATYPE(WingHex::WingPluginMessage)
Q_DECLARE_METATYPE(WingHex::ResponseMsg)
Q_DECLARE_METATYPE(WingHex::HookIndex)
Q_DECLARE_INTERFACE(WingHex::IWingPlugin, IWINGPLUGIN_INTERFACE_IID)

#endif // IWINGPLUGIN_H
