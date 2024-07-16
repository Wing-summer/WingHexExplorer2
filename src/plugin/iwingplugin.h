#ifndef IWINGPLUGIN_H
#define IWINGPLUGIN_H

#include <QCryptographicHash>
#include <QDockWidget>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <QtCore>

#include <QMessageBox>

#define SDKVERSION 12
#define GETPLUGINQM(name)                                                      \
    (QCoreApplication::applicationDirPath() + "/plglang/" + name)
#define PLUGINDIR (QCoreApplication::applicationDirPath() + "/plugin")

#define HOSTRESPIMG(name) ":/com.wingsummer.winghex/images/" name ".png"
#define HOSTAUTHORPIMG ":/com.wingsummer.winghex/images/author.jpg"
#define SPONSORQRCODE ":/com.wingsummer.winghex/images/sponsorqr.png"

enum ErrFile : uint {
    Success,
    Error,
    UnSaved,
    Permission,
    NotExist,
    AlreadyOpened,
    IsNewFile,
    IsDirver,
    WorkSpaceUnSaved,
    SourceFileChanged
};

struct FindResult {
    int fid;
    QList<int> indices;

    FindResult() {}
};

struct BookMark {
    qlonglong pos;
    QString comment;
};

class IWingPlugin;

struct HexPosition {
    quint64 line;
    int column;
    quint8 lineWidth;
    int nibbleindex;

    HexPosition() = default;
    inline qint64 offset() const {
        return static_cast<qint64>(line * lineWidth) + column;
    }
    inline int operator-(const HexPosition &rhs) const {
        return int(this->offset() - rhs.offset());
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
    qint64 begin;
    qint64 end;
    QColor foreground, background;
    QString comment;

    // added by wingsummer
    bool operator==(const HexMetadataAbsoluteItem &item) {
        return begin == item.begin && end == item.end &&
               foreground == item.foreground && background == item.background &&
               comment == item.comment;
    }

    HexMetadataAbsoluteItem() = default;

    HexMetadataAbsoluteItem(qint64 begin, qint64 end, QColor foreground,
                            QColor background, QString comment) {
        this->begin = begin;
        this->end = end;
        this->foreground = foreground;
        this->background = background;
        this->comment = comment;
    }
};

struct HexMetadataItem {
    quint64 line;
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

    HexMetadataItem(quint64 line, int start, int length, QColor foreground,
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
    ConnectTimeout,
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

Q_DECLARE_METATYPE(WingPluginMessage)
Q_DECLARE_METATYPE(ResponseMsg)
Q_DECLARE_METATYPE(HookIndex)
Q_DECLARE_METATYPE(ErrFile)

namespace WingPlugin {
class Reader : public QObject {
    Q_OBJECT
signals:
    qsizetype currentDoc();
    qsizetype currentHostDoc();
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
    qsizetype selectLength();
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

    QByteArray read(qsizetype offset, qsizetype len);

    // extension
    qint8 readInt8(qsizetype offset);
    qint16 readInt16(qsizetype offset);
    qint32 readInt32(qsizetype offset);
    qint64 readInt64(qsizetype offset);
    QString readString(qsizetype offset, const QString encoding = QString());
    QByteArray readBytes(qsizetype offset, qsizetype count);

    qsizetype searchForward(qsizetype begin, const QByteArray &ba);
    qsizetype searchBackward(qsizetype begin, const QByteArray &ba);
    void findAllBytes(qsizetype begin, qsizetype end, QByteArray b,
                      QList<quint64> &results, qsizetype maxCount = -1);

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
    void getBookMarks(QList<BookMark> &bookmarks);
    bool existBookMark(qsizetype pos);

    // extension
    QList<QString> getOpenFiles();
    QStringList getSupportedEncodings();
    QString currentEncoding();
};

class Controller : public QObject {
    Q_OBJECT
signals:
    // document
    bool switchDocument(qsizetype index, bool gui = false);
    bool setLockedFile(bool b);
    bool setKeepSize(bool b);
    void setStringVisible(bool b);
    void setAddressVisible(bool b);
    void setHeaderVisible(bool b);
    void setAddressBase(quintptr base);

    void undo();
    void redo();
    bool cut(bool hex = false);
    void paste(bool hex = false);

    bool write(qsizetype offset, uchar b);
    bool write(qsizetype offset, const QByteArray &data);

    // extesion
    bool writeInt8(qsizetype offset, qint8 value);
    bool writeInt16(qsizetype offset, qint16 value);
    bool writeInt32(qsizetype offset, qint32 value);
    bool writeInt64(qsizetype offset, qint64 value);
    bool writeString(qsizetype offset, const QString &value,
                     const QString &encoding = QString());
    bool writeBytes(qsizetype offset, QByteArray bytes);

    bool insert(qsizetype offset, const uchar b);
    bool insert(qsizetype offset, const QByteArray &data);

    // extesion
    bool insertInt8(qsizetype offset, qint8 value);
    bool insertInt16(qsizetype offset, qint16 value);
    bool insertInt32(qsizetype offset, qint32 value);
    bool insertInt64(qsizetype offset, qint64 value);
    bool insertString(qsizetype offset, const QString &value,
                      const QString &encoding = QString());
    bool insertBytes(qsizetype offset, QByteArray bytes);

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
    bool removeAll(qsizetype offset); // extension

    // cursor
    void moveTo(const HexPosition &pos);
    void moveTo(qsizetype line, qsizetype column, int nibbleindex = 1);
    void moveTo(qsizetype offset);
    void select(qsizetype line, qsizetype column, int nibbleindex = 1);
    void selectOffset(qsizetype offset, qsizetype length);
    void setInsertionMode(bool isinsert);
    void enabledCursor(bool b);
    void selectLength(qsizetype offset, qsizetype length);

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
    void applyMetas(const QList<HexMetadataAbsoluteItem> &metas);
    bool setMetaVisible(bool b);
    void setMetafgVisible(bool b);
    void setMetabgVisible(bool b);
    void setMetaCommentVisible(bool b);

    // mainwindow
    void newFile();
    ErrFile openFile(const QString &filename, qsizetype *openedIndex = nullptr);
    ErrFile openRegionFile(const QString &filename,
                           qsizetype *openedIndex = nullptr,
                           qsizetype start = 0, qsizetype length = 1024);
    ErrFile openDriver(const QString &driver);
    ErrFile closeFile(qsizetype index, bool force = false);
    ErrFile saveFile(qsizetype index, bool ignoreMd5 = false);
    ErrFile exportFile(const QString &filename, qsizetype index,
                       bool ignoreMd5 = false);
    void exportFileGUI();
    ErrFile saveasFile(const QString &filename, qsizetype index,
                       bool ignoreMd5 = false);
    void saveasFileGUI();
    ErrFile closeCurrentFile(bool force = false);
    ErrFile saveCurrentFile(bool ignoreMd5 = false);
    void openFileGUI();
    void openRegionFileGUI();
    void openDriverGUI();
    void findGUI();
    void gotoGUI();
    void fillGUI();
    void fillzeroGUI();
    void fillnopGUI();

    // bookmark
    bool addBookMark(qsizetype pos, const QString &comment);
    bool modBookMark(qsizetype pos, const QString &comment);
    void applyBookMarks(const QList<BookMark> &books);
    bool removeBookMark(qsizetype pos);
    bool clearBookMark();

    // workspace
    bool openWorkSpace(const QString &filename);
    bool setCurrentEncoding(const QString &encoding);
};

/**
 * @brief The UI class
 */
class UI : public QObject {
    Q_OBJECT
signals:
    // for WingMsgBox
    void msgAboutQt(QWidget *parent = nullptr,
                    const QString &title = QString());

    QMessageBox::StandardButton msgInformation(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton msgQuestion(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons =
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton msgWarning(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton msgCritical(
        QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    void msgAbout(QWidget *parent, const QString &title, const QString &text);

    QMessageBox::StandardButton
    msgbox(QWidget *parent, QMessageBox::Icon icon, const QString &title,
           const QString &text,
           QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    // InputBox

    // Create Window
};

} // namespace WingPlugin

struct WingPluginInfo {
    QString pluginName;
    QString pluginAuthor;
    uint pluginVersion;
    QString puid;
    QString pluginComment;
};

#define WINGSUMMER "wingsummer"

class IWingPlugin : public QObject {
    Q_OBJECT
public:
    virtual int sdkVersion() const = 0;
    virtual const QString signature() const = 0;
    QString const puid() { return GetPUID(this); }
    virtual ~IWingPlugin() = default;

    virtual QHash<QWidget *, Qt::DockWidgetArea> registeredDockWidgets() const {
        return {};
    }
    virtual QHash<QString, QMenu *> registeredContextMenu() const { return {}; }
    virtual QHash<QString, QPair<QString, QList<QToolButton *>>>
    registerToolBars() {
        return {};
    }

    virtual bool init(const QList<WingPluginInfo> &loadedplugin) = 0;
    virtual void unload() = 0;
    virtual const QString pluginName() const = 0;
    virtual const QString pluginAuthor() const = 0;
    virtual uint pluginVersion() const = 0;
    virtual const QString pluginComment() const = 0;
    virtual HookIndex getHookSubscribe() const { return HookIndex::None; }

    static QString GetPUID(IWingPlugin *plugin) {
        auto str =
            QString("%1%2%3%4")
                .arg(WINGSUMMER, plugin->pluginName(), plugin->pluginAuthor(),
                     QString::number(plugin->pluginVersion()));
        return QCryptographicHash::hash(str.toLatin1(), QCryptographicHash::Md5)
            .toHex();
    }

public slots:
    virtual void plugin2MessagePipe(WingPluginMessage type,
                                    const QVariantList &msg) = 0;

signals:
    // extension and exposed to WingHexAngelScript
    void toast(const QIcon &icon, const QString &message);
    void debug(const QString &message);
    void warn(const QString &message);
    void error(const QString &message);
    void info(const QString &message);

public:
    WingPlugin::Reader reader;
    WingPlugin::Controller controller;
    WingPlugin::UI ui;
};

#define IWINGPLUGIN_INTERFACE_IID "com.wingsummer.iwingplugin"

Q_DECLARE_INTERFACE(IWingPlugin, IWINGPLUGIN_INTERFACE_IID)

#endif // IWINGPLUGIN_H
