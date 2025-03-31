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

#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QReadWriteLock>
#include <QStackedWidget>

#include "QHexView/qhexview.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "WingPlugin/iwingdevice.h"
#include "WingPlugin/wingeditorviewwidget.h"
#include "dialog/finddialog.h"
#include "gotowidget.h"
#include "model/bookmarksmodel.h"
#include "model/findresultmodel.h"
#include "model/metadatamodel.h"
#include "utilities.h"

using namespace WingHex;

class EditorView : public ads::CDockWidget {
    Q_OBJECT

public:
    enum class DocumentType { InValid, File, Extension, Cloned };

    enum class FindError { Success, Busy, MayOutOfRange };

    enum class SaveWorkSpaceAttr {
        ForceNoWorkSpace,
        AutoWorkSpace,
        ForceWorkSpace
    };

public:
    static QString getDeviceFileName(const QString &ext, const QString &file);

public:
    // helper functions for plugin system
    template <typename T>
    static T readBasicTypeContent(EditorView *view, qsizetype offset,
                                  QReadWriteLock &lock) {
        Q_ASSERT(view);
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);

        lock.lockForRead();
        auto buffer = view->hexEditor()->document()->read(offset, sizeof(T));
        if (buffer.size() == sizeof(T)) {
            auto pb = reinterpret_cast<const T *>(buffer.constData());
            lock.unlock();
            return *pb;
        }

        if constexpr (std::is_floating_point_v<T>) {
            return qQNaN();
        } else {
            return T(0);
        }
    }

    template <typename T>
    static bool insertBasicTypeContent(EditorView *view, qsizetype offset,
                                       const T &value, QUndoCommand *uc,
                                       QReadWriteLock &lock) {
        Q_ASSERT(view);
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);

        auto editor = view->hexEditor();
        auto doc = editor->document();
        auto buffer = reinterpret_cast<const char *>(&value);
        auto cmd = doc->MakeInsert(uc, editor->cursor(), offset,
                                   QByteArray(buffer, sizeof(T)));
        if (uc == nullptr && cmd) {
            lock.lockForWrite();
            doc->pushMakeUndo(cmd);
            lock.unlock();
            return true;
        }

        return false;
    }

    template <typename T>
    static bool writeBasicTypeContent(EditorView *view, qsizetype offset,
                                      const T &value, QUndoCommand *uc,
                                      QReadWriteLock &lock) {
        Q_ASSERT(view);
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);

        auto editor = view->hexEditor();
        auto doc = editor->document();
        auto buffer = reinterpret_cast<const char *>(&value);
        auto cmd = doc->MakeReplace(uc, editor->cursor(), offset,
                                    QByteArray(buffer, sizeof(T)));
        if (uc == nullptr && cmd) {
            lock.lockForWrite();
            doc->pushMakeUndo(cmd);
            lock.unlock();
            return true;
        }

        return false;
    }

    template <typename T>
    static bool appendBasicTypeContent(EditorView *view, const T &value,
                                       QUndoCommand *uc, QReadWriteLock &lock) {
        Q_ASSERT(view);
        Q_STATIC_ASSERT(std::is_integral_v<T> || std::is_floating_point_v<T>);

        auto editor = view->hexEditor();
        auto doc = editor->document();
        auto buffer = reinterpret_cast<const char *>(&value);
        auto cmd = doc->MakeAppend(uc, editor->cursor(),
                                   QByteArray(buffer, sizeof(T)));
        if (uc == nullptr && cmd) {
            lock.lockForWrite();
            doc->pushMakeUndo(cmd);
            lock.unlock();
            return true;
        }
        return false;
    }

public:
    explicit EditorView(QWidget *parent = nullptr);
    virtual ~EditorView() override;

    QString fileName() const;

    bool isOriginWorkSpace() const;
    bool isNewFile() const;
    bool isBigFile() const;
    bool isCloneFile() const;
    bool isExtensionFile() const;
    bool isCommonFile() const;

    FindResultModel *findResultModel() const;

    BookMarksModel *bookmarksModel() const;

    MetaDataModel *metadataModel() const;

    bool hasCloneChildren() const;

    void closeAllClonedChildren();

    void setFontSize(qreal size);

    int findResultCount() const;

    EditorView *cloneParent() const;

    bool isCloned() const;

    bool change2WorkSpace() const;

    QHexView *hexEditor() const;

    DocumentType documentType() const;

    qsizetype copyLimit() const;

    bool isWingEditorViewEnabled(const QString &id) const;

    bool processWingEditorViewClosing();

    void notifyOnWorkSpace(bool b);

public slots:
    void raiseAndSwitchView(const QString &id);

public:
    EditorView *clone();

    void registerView(const QString &id, WingHex::WingEditorViewWidget *view);

    void switchView(const QString &id);
    void registerQMenu(QMenu *menu);

    FindError find(const FindDialog::Result &result);

    void clearFindResult();

    void triggerGoto();

    ErrFile newFile(size_t index);
    ErrFile openFile(const QString &filename);
    ErrFile openExtFile(const QString &ext, const QString &file);
    ErrFile openWorkSpace(const QString &filename);
    ErrFile
    save(const QString &workSpaceName, const QString &path = QString(),
         bool isExport = false,
         SaveWorkSpaceAttr workSpaceAttr = SaveWorkSpaceAttr::AutoWorkSpace);
    ErrFile reload();
    ErrFile closeFile();

    void setCopyLimit(qsizetype sizeMB);

    void applySettings();

private:
    inline qsizetype findAvailCloneIndex();

    bool hasMeta() const;

    QHash<QString, QByteArray> savePluginData();
    bool checkHasUnsavedState() const;

    FindResultModel::FindInfo readContextFinding(qsizetype offset,
                                                 qsizetype findSize,
                                                 int contextSize,
                                                 int maxDisplayBytes);

    void applyFunctionTables(WingHex::WingEditorViewWidget *view,
                             const CallTable &fns);

private:
    bool checkErrAndReport(QObject *sender, const char *func);
    bool checkThreadAff();

private:
    WING_SERVICE bool existsServiceHost(QObject *caller, const QString &puid);

    WING_SERVICE bool
    invokeService(QObject *caller, const QString &puid, const char *method,
                  Qt::ConnectionType type, QGenericReturnArgument ret,
                  QGenericArgument val0, QGenericArgument val1,
                  QGenericArgument val2, QGenericArgument val3,
                  QGenericArgument val4);

private:
    WING_SERVICE QString currentDocFilename(QObject *caller);

    // document
    WING_SERVICE bool isReadOnly(QObject *caller);
    WING_SERVICE bool isInsertionMode(QObject *caller);
    WING_SERVICE bool isKeepSize(QObject *caller);
    WING_SERVICE bool isLocked(QObject *caller);
    WING_SERVICE qsizetype documentLines(QObject *caller);
    WING_SERVICE qsizetype documentBytes(QObject *caller);
    WING_SERVICE WingHex::HexPosition currentPos(QObject *caller);
    WING_SERVICE qsizetype currentRow(QObject *caller);
    WING_SERVICE qsizetype currentColumn(QObject *caller);
    WING_SERVICE qsizetype currentOffset(QObject *caller);
    WING_SERVICE qsizetype selectedLength(QObject *caller);

    WING_SERVICE QByteArray selectedBytes(QObject *caller, qsizetype index);
    WING_SERVICE QByteArrayList selectionBytes(QObject *caller);

    WING_SERVICE WingHex::HexPosition selectionStart(QObject *caller,
                                                     qsizetype index);
    WING_SERVICE WingHex::HexPosition selectionEnd(QObject *caller,
                                                   qsizetype index);
    WING_SERVICE qsizetype selectionLength(QObject *caller, qsizetype index);
    WING_SERVICE qsizetype selectionCount(QObject *caller);

    WING_SERVICE bool stringVisible(QObject *caller);
    WING_SERVICE bool addressVisible(QObject *caller);
    WING_SERVICE bool headerVisible(QObject *caller);
    WING_SERVICE quintptr addressBase(QObject *caller);
    WING_SERVICE bool isModified(QObject *caller);

    WING_SERVICE qint8 readInt8(QObject *caller, qsizetype offset);
    WING_SERVICE qint16 readInt16(QObject *caller, qsizetype offset);
    WING_SERVICE qint32 readInt32(QObject *caller, qsizetype offset);
    WING_SERVICE qint64 readInt64(QObject *caller, qsizetype offset);
    WING_SERVICE quint8 readUInt8(QObject *caller, qsizetype offset);
    WING_SERVICE quint16 readUInt16(QObject *caller, qsizetype offset);
    WING_SERVICE quint32 readUInt32(QObject *caller, qsizetype offset);
    WING_SERVICE quint64 readUInt64(QObject *caller, qsizetype offset);
    WING_SERVICE float readFloat(QObject *caller, qsizetype offset);
    WING_SERVICE double readDouble(QObject *caller, qsizetype offset);
    WING_SERVICE QString readString(QObject *caller, qsizetype offset,
                                    const QString &encoding);
    WING_SERVICE QByteArray readBytes(QObject *caller, qsizetype offset,
                                      qsizetype count);

    WING_SERVICE qsizetype findNext(QObject *caller, qsizetype begin,
                                    const QByteArray &ba);
    WING_SERVICE qsizetype findPrevious(QObject *caller, qsizetype begin,
                                        const QByteArray &ba);

    WING_SERVICE QString bookMarkComment(QObject *caller, qsizetype pos);
    WING_SERVICE bool existBookMark(QObject *caller, qsizetype pos);

    WING_SERVICE bool setLockedFile(QObject *caller, bool b);
    WING_SERVICE bool setKeepSize(QObject *caller, bool b);
    WING_SERVICE bool setStringVisible(QObject *caller, bool b);
    WING_SERVICE bool setAddressVisible(QObject *caller, bool b);
    WING_SERVICE bool setHeaderVisible(QObject *caller, bool b);
    WING_SERVICE bool setAddressBase(QObject *caller, quintptr base);

    WING_SERVICE bool beginMarco(QObject *caller, const QString &txt);
    bool endMarco(QObject *caller);

    WING_SERVICE bool writeInt8(QObject *caller, qsizetype offset, qint8 value);
    WING_SERVICE bool writeInt16(QObject *caller, qsizetype offset,
                                 qint16 value);
    WING_SERVICE bool writeInt32(QObject *caller, qsizetype offset,
                                 qint32 value);
    WING_SERVICE bool writeInt64(QObject *caller, qsizetype offset,
                                 qint64 value);
    WING_SERVICE bool writeUInt8(QObject *caller, qsizetype offset,
                                 quint8 value);
    WING_SERVICE bool writeUInt16(QObject *caller, qsizetype offset,
                                  quint16 value);
    WING_SERVICE bool writeUInt32(QObject *caller, qsizetype offset,
                                  quint32 value);
    WING_SERVICE bool writeUInt64(QObject *caller, qsizetype offset,
                                  quint64 value);
    WING_SERVICE bool writeFloat(QObject *caller, qsizetype offset,
                                 float value);
    WING_SERVICE bool writeDouble(QObject *caller, qsizetype offset,
                                  double value);
    WING_SERVICE
    bool writeString(QObject *caller, qsizetype offset, const QString &value,
                     const QString &encoding);
    WING_SERVICE bool writeBytes(QObject *caller, qsizetype offset,
                                 const QByteArray &data);

    WING_SERVICE bool insertInt8(QObject *caller, qsizetype offset,
                                 qint8 value);
    WING_SERVICE bool insertInt16(QObject *caller, qsizetype offset,
                                  qint16 value);
    WING_SERVICE bool insertInt32(QObject *caller, qsizetype offset,
                                  qint32 value);
    WING_SERVICE bool insertInt64(QObject *caller, qsizetype offset,
                                  qint64 value);
    WING_SERVICE bool insertUInt8(QObject *caller, qsizetype offset,
                                  quint8 value);
    WING_SERVICE
    bool insertUInt16(QObject *caller, qsizetype offset, quint16 value);
    WING_SERVICE
    bool insertUInt32(QObject *caller, qsizetype offset, quint32 value);
    WING_SERVICE bool insertUInt64(QObject *caller, qsizetype offset,
                                   quint64 value);
    WING_SERVICE bool insertFloat(QObject *caller, qsizetype offset,
                                  float value);
    WING_SERVICE bool insertDouble(QObject *caller, qsizetype offset,
                                   double value);
    WING_SERVICE bool insertString(QObject *caller, qsizetype offset,
                                   const QString &value,
                                   const QString &encoding);
    WING_SERVICE bool insertBytes(QObject *caller, qsizetype offset,
                                  const QByteArray &data);

    WING_SERVICE bool appendInt8(QObject *caller, qint8 value);
    WING_SERVICE bool appendInt16(QObject *caller, qint16 value);
    WING_SERVICE bool appendInt32(QObject *caller, qint32 value);
    WING_SERVICE bool appendInt64(QObject *caller, qint64 value);
    WING_SERVICE bool appendUInt8(QObject *caller, quint8 value);
    WING_SERVICE bool appendUInt16(QObject *caller, quint16 value);
    WING_SERVICE bool appendUInt32(QObject *caller, quint32 value);
    WING_SERVICE bool appendUInt64(QObject *caller, quint64 value);
    WING_SERVICE bool appendFloat(QObject *caller, float value);
    WING_SERVICE bool appendDouble(QObject *caller, double value);
    WING_SERVICE bool appendString(QObject *caller, const QString &value,
                                   const QString &encoding);
    WING_SERVICE bool appendBytes(QObject *caller, const QByteArray &data);

    WING_SERVICE bool removeBytes(QObject *caller, qsizetype offset,
                                  qsizetype len);

    // cursor
    WING_SERVICE bool moveTo(QObject *caller, qsizetype line, qsizetype column,
                             int nibbleindex, bool clearSelection);
    WING_SERVICE bool moveTo(QObject *caller, qsizetype offset,
                             bool clearSelection);
    WING_SERVICE bool select(QObject *caller, qsizetype offset,
                             qsizetype length, SelectionMode mode);
    WING_SERVICE bool setInsertionMode(QObject *caller, bool isinsert);

    // metadata
    WING_SERVICE bool metadata(QObject *caller, qsizetype begin,
                               qsizetype length, const QColor &fgcolor,
                               const QColor &bgcolor, const QString &comment);

    WING_SERVICE bool removeMetadata(QObject *caller, qsizetype offset);
    WING_SERVICE bool clearMetadata(QObject *caller);
    WING_SERVICE bool setMetaVisible(QObject *caller, bool b);
    WING_SERVICE bool setMetafgVisible(QObject *caller, bool b);
    WING_SERVICE bool setMetabgVisible(QObject *caller, bool b);
    WING_SERVICE bool setMetaCommentVisible(QObject *caller, bool b);

    // bookmark
    WING_SERVICE bool addBookMark(QObject *caller, qsizetype pos,
                                  const QString &comment);
    WING_SERVICE bool modBookMark(QObject *caller, qsizetype pos,
                                  const QString &comment);
    WING_SERVICE bool removeBookMark(QObject *caller, qsizetype pos);
    WING_SERVICE bool clearBookMark(QObject *caller);

private:
    WING_SERVICE void toast(QObject *caller, const QPixmap &icon,
                            const QString &message);
    WING_SERVICE void logTrace(QObject *caller, const QString &message);
    WING_SERVICE void logDebug(QObject *caller, const QString &message);
    WING_SERVICE void logWarn(QObject *caller, const QString &message);
    WING_SERVICE void logError(QObject *caller, const QString &message);
    WING_SERVICE void logInfo(QObject *caller, const QString &message);

    WING_SERVICE bool raiseDockWidget(QObject *caller, QWidget *w);

    // theme
    WING_SERVICE WingHex::AppTheme currentAppTheme(QObject *caller);

    // not available for AngelScript
    // only for plugin UI extenstion
    WING_SERVICE QDialog *createDialog(QObject *caller, QWidget *content);

private:
    WING_SERVICE void msgAboutQt(QObject *caller, QWidget *parent = nullptr,
                                 const QString &title = QString());

    WING_SERVICE QMessageBox::StandardButton msgInformation(
        QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    WING_SERVICE QMessageBox::StandardButton msgQuestion(
        QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons =
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    WING_SERVICE QMessageBox::StandardButton msgWarning(
        QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    WING_SERVICE QMessageBox::StandardButton msgCritical(
        QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    WING_SERVICE void msgAbout(QObject *caller, QWidget *parent,
                               const QString &title, const QString &text);

    WING_SERVICE QMessageBox::StandardButton
    msgbox(QObject *caller, QWidget *parent, QMessageBox::Icon icon,
           const QString &title, const QString &text,
           QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

private:
    WING_SERVICE QString dlgGetText(QObject *caller, QWidget *parent,
                                    const QString &title, const QString &label,
                                    QLineEdit::EchoMode echo,
                                    const QString &text, bool *ok,
                                    Qt::InputMethodHints inputMethodHints);
    WING_SERVICE QString
    dlgGetMultiLineText(QObject *caller, QWidget *parent, const QString &title,
                        const QString &label, const QString &text, bool *ok,
                        Qt::InputMethodHints inputMethodHints);

    WING_SERVICE QString dlgGetItem(QObject *caller, QWidget *parent,
                                    const QString &title, const QString &label,
                                    const QStringList &items, int current,
                                    bool editable, bool *ok,
                                    Qt::InputMethodHints inputMethodHints);

    WING_SERVICE int dlgGetInt(QObject *caller, QWidget *parent,
                               const QString &title, const QString &label,
                               int value, int minValue, int maxValue, int step,
                               bool *ok);

    WING_SERVICE double dlgGetDouble(QObject *caller, QWidget *parent,
                                     const QString &title, const QString &label,
                                     double value, double minValue,
                                     double maxValue, int decimals, bool *ok,
                                     double step);

private:
    WING_SERVICE QString dlgGetExistingDirectory(QObject *caller,
                                                 QWidget *parent,
                                                 const QString &caption,
                                                 const QString &dir,
                                                 QFileDialog::Options options);

    WING_SERVICE QString dlgGetOpenFileName(QObject *caller, QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            const QString &filter,
                                            QString *selectedFilter,
                                            QFileDialog::Options options);

    WING_SERVICE QStringList dlgGetOpenFileNames(
        QObject *caller, QWidget *parent, const QString &caption,
        const QString &dir, const QString &filter, QString *selectedFilter,
        QFileDialog::Options options);

    WING_SERVICE QString dlgGetSaveFileName(QObject *caller, QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            const QString &filter,
                                            QString *selectedFilter,
                                            QFileDialog::Options options);

private:
    WING_SERVICE QColor dlgGetColor(QObject *caller, const QString &caption,
                                    QWidget *parent);

private:
    template <typename Func>
    inline void newAction(QWidget *parent, const QString &icon,
                          const QString &title, Func &&slot,
                          const QKeySequence &shortcut = QKeySequence()) {
        auto a = new QAction(parent);
        if (shortcut.isEmpty()) {
            a->setText(title);
        } else {
            a->setText(title + QStringLiteral("\t") + shortcut.toString());
        }
        a->setIcon(ICONRES(icon));
        connect(a, &QAction::triggered, this, slot);
        parent->addAction(a);
    }

    void connectDocSavedFlag(EditorView *editor);

signals:
    void sigOnCutFile();
    void sigOnCutHex();
    void sigOnCopyFile();
    void sigOnCopyHex();
    void sigOnPasteFile();
    void sigOnPasteHex();
    void sigOnDelete();
    void sigOnFindFile();
    void sigOnGoToLine();
    void sigOnFill();
    void sigOnMetadata();
    void sigOnBookMark();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QStackedWidget *m_stack = nullptr;
    GotoWidget *m_goto = nullptr;
    QWidget *m_hexContainer = nullptr;
    bool _hasRegistered = false;

    QHexView *m_hex = nullptr;
    QMenu *m_menu = nullptr;

    QHash<QString, WingEditorViewWidget *> m_others;
    QString m_fileName;
    bool m_isNewFile = true;
    QByteArray m_md5;

    QVector<EditorView *> m_cloneChildren;
    EditorView *m_cloneParent = nullptr;

    QMutex m_findMutex;
    FindResultModel *m_findResults = nullptr;
    BookMarksModel *m_bookmarks = nullptr;
    MetaDataModel *m_metadata = nullptr;

    DocumentType m_docType = DocumentType::InValid;
    bool m_isWorkSpace = false;
    QHash<QString, QByteArray> _pluginData;

    // only for extension use
    QString _ext;
    WingHex::WingIODevice *_dev = nullptr;
    QString _file;

    QReadWriteLock _rwlock;
    CallTable _viewFns;
};

#endif // EDITORVIEW_H
