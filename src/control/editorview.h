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
#include "class/editorviewcontext.h"
#include "dialog/finddialog.h"
#include "gotowidget.h"
#include "model/findresultmodel.h"
#include "utilities.h"

using namespace WingHex;

// only plugin api can be declared with `private slot`
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

    QUrl fileNameUrl() const;

    bool isOriginWorkSpace() const;
    bool isNewFile() const;
    bool isCloneFile() const;
    bool isExtensionFile() const;
    bool isCommonFile() const;

    FindResultModel::FindData &findResult();
    QMap<QCryptographicHash::Algorithm, QString> &checkSumResult();

    bool hasCloneChildren() const;

    void closeAllClonedChildren();

    void setFontSize(qreal size);

    EditorView *cloneParent() const;

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
    void getCheckSum(const QVector<int> &algorithmID);

    void clearFindResult();

    void triggerGoto();

    ErrFile newFile(size_t index);
    ErrFile openFile(const QString &filename);
    ErrFile openExtFile(const QString &ext, const QString &file);
    ErrFile openExtFile(const QUrl &fileName);
    ErrFile openWorkSpace(const QString &filename);
    ErrFile
    save(const QString &workSpaceName, const QString &path = QString(),
         bool isExport = false,
         SaveWorkSpaceAttr workSpaceAttr = SaveWorkSpaceAttr::AutoWorkSpace);
    ErrFile reload();
    ErrFile closeFile();

    void setCopyLimit(qsizetype sizeMB);

    void applySettings();

    EditorViewContext *editorContext() const;

    static bool isNewFileUrl(const QUrl &url);

    static QString newFileAuthority();

private:
    inline qsizetype findAvailCloneIndex();

    void setFileNameUrl(const QUrl &fileName);

    QHash<QString, QByteArray> savePluginData();
    bool checkHasUnsavedState() const;

    FindResultModel::FindInfo readContextFinding(qsizetype offset,
                                                 qsizetype findSize);

    void applyFunctionTables(WingHex::WingEditorViewWidget *view,
                             const CallTable &fns);

private:
    bool checkErrAndReport(const QObject *sender, const char *func);
    bool checkThreadAff();

    // all private slots
private slots:
    bool existsServiceHost(const QObject *caller, const QString &puid);

    bool invokeServiceImpl(const QObject *sender, const QString &puid,
                           const WingHex::MetaCallInfo &infos);

private slots:
    QString currentDocFile(const QObject *caller);

    // document
    bool isReadOnly(const QObject *caller);
    bool isInsertionMode(const QObject *caller);
    bool isKeepSize(const QObject *caller);
    bool isLocked(const QObject *caller);
    qsizetype documentLines(const QObject *caller);
    qsizetype documentBytes(const QObject *caller);
    WingHex::HexPosition currentPos(const QObject *caller);
    qsizetype currentRow(const QObject *caller);
    qsizetype currentColumn(const QObject *caller);
    qsizetype currentOffset(const QObject *caller);
    qsizetype selectedLength(const QObject *caller);

    QByteArray selectedBytes(const QObject *caller, qsizetype index);
    QByteArrayList selectionBytes(const QObject *caller);

    WingHex::HexPosition selectionStart(const QObject *caller, qsizetype index);
    WingHex::HexPosition selectionEnd(const QObject *caller, qsizetype index);
    qsizetype selectionLength(const QObject *caller, qsizetype index);
    qsizetype selectionCount(const QObject *caller);

    bool stringVisible(const QObject *caller);
    bool addressVisible(const QObject *caller);
    bool headerVisible(const QObject *caller);
    quintptr addressBase(const QObject *caller);
    bool isModified(const QObject *caller);

    qint8 readInt8(const QObject *caller, qsizetype offset);
    qint16 readInt16(const QObject *caller, qsizetype offset);
    qint32 readInt32(const QObject *caller, qsizetype offset);
    qint64 readInt64(const QObject *caller, qsizetype offset);
    quint8 readUInt8(const QObject *caller, qsizetype offset);
    quint16 readUInt16(const QObject *caller, qsizetype offset);
    quint32 readUInt32(const QObject *caller, qsizetype offset);
    quint64 readUInt64(const QObject *caller, qsizetype offset);
    float readFloat(const QObject *caller, qsizetype offset);
    double readDouble(const QObject *caller, qsizetype offset);
    QString readString(const QObject *caller, qsizetype offset,
                       const QString &encoding);
    QByteArray readBytes(const QObject *caller, qsizetype offset,
                         qsizetype count);

    qsizetype findNext(const QObject *caller, qsizetype begin,
                       const QByteArray &ba);
    qsizetype findPrevious(const QObject *caller, qsizetype begin,
                           const QByteArray &ba);

    QString bookMarkComment(const QObject *caller, qsizetype pos);
    bool existBookMark(const QObject *caller, qsizetype pos);

    bool setLockedFile(const QObject *caller, bool b);
    bool setKeepSize(const QObject *caller, bool b);
    bool setStringVisible(const QObject *caller, bool b);
    bool setAddressVisible(const QObject *caller, bool b);
    bool setHeaderVisible(const QObject *caller, bool b);
    bool setAddressBase(const QObject *caller, quintptr base);

    bool beginMarco(const QObject *caller, const QString &txt);
    bool endMarco(const QObject *caller);

    bool writeInt8(const QObject *caller, qsizetype offset, qint8 value);
    bool writeInt16(const QObject *caller, qsizetype offset, qint16 value);
    bool writeInt32(const QObject *caller, qsizetype offset, qint32 value);
    bool writeInt64(const QObject *caller, qsizetype offset, qint64 value);
    bool writeUInt8(const QObject *caller, qsizetype offset, quint8 value);
    bool writeUInt16(const QObject *caller, qsizetype offset, quint16 value);
    bool writeUInt32(const QObject *caller, qsizetype offset, quint32 value);
    bool writeUInt64(const QObject *caller, qsizetype offset, quint64 value);
    bool writeFloat(const QObject *caller, qsizetype offset, float value);
    bool writeDouble(const QObject *caller, qsizetype offset, double value);

    bool writeString(const QObject *caller, qsizetype offset,
                     const QString &value, const QString &encoding);
    bool writeBytes(const QObject *caller, qsizetype offset,
                    const QByteArray &data);

    bool insertInt8(const QObject *caller, qsizetype offset, qint8 value);
    bool insertInt16(const QObject *caller, qsizetype offset, qint16 value);
    bool insertInt32(const QObject *caller, qsizetype offset, qint32 value);
    bool insertInt64(const QObject *caller, qsizetype offset, qint64 value);
    bool insertUInt8(const QObject *caller, qsizetype offset, quint8 value);

    bool insertUInt16(const QObject *caller, qsizetype offset, quint16 value);

    bool insertUInt32(const QObject *caller, qsizetype offset, quint32 value);
    bool insertUInt64(const QObject *caller, qsizetype offset, quint64 value);
    bool insertFloat(const QObject *caller, qsizetype offset, float value);
    bool insertDouble(const QObject *caller, qsizetype offset, double value);
    bool insertString(const QObject *caller, qsizetype offset,
                      const QString &value, const QString &encoding);
    bool insertBytes(const QObject *caller, qsizetype offset,
                     const QByteArray &data);

    bool appendInt8(const QObject *caller, qint8 value);
    bool appendInt16(const QObject *caller, qint16 value);
    bool appendInt32(const QObject *caller, qint32 value);
    bool appendInt64(const QObject *caller, qint64 value);
    bool appendUInt8(const QObject *caller, quint8 value);
    bool appendUInt16(const QObject *caller, quint16 value);
    bool appendUInt32(const QObject *caller, quint32 value);
    bool appendUInt64(const QObject *caller, quint64 value);
    bool appendFloat(const QObject *caller, float value);
    bool appendDouble(const QObject *caller, double value);
    bool appendString(const QObject *caller, const QString &value,
                      const QString &encoding);
    bool appendBytes(const QObject *caller, const QByteArray &data);

    bool removeBytes(const QObject *caller, qsizetype offset, qsizetype len);

    // cursor
    bool moveTo(const QObject *caller, qsizetype line, qsizetype column,
                int nibbleindex, bool clearSelection);
    bool moveTo(const QObject *caller, qsizetype offset, bool clearSelection);
    bool select(const QObject *caller, qsizetype offset, qsizetype length,
                WingHex::SelectionMode mode);
    bool setInsertionMode(const QObject *caller, bool isinsert);

    // metadata
    bool metadata(const QObject *caller, qsizetype begin, qsizetype length,
                  const QColor &fgcolor, const QColor &bgcolor,
                  const QString &comment);

    bool removeMetadata(const QObject *caller, qsizetype offset);
    bool clearMetadata(const QObject *caller);
    bool setMetaVisible(const QObject *caller, bool b);
    bool setMetafgVisible(const QObject *caller, bool b);
    bool setMetabgVisible(const QObject *caller, bool b);
    bool setMetaCommentVisible(const QObject *caller, bool b);

    // bookmark
    bool addBookMark(const QObject *caller, qsizetype pos,
                     const QString &comment);
    bool modBookMark(const QObject *caller, qsizetype pos,
                     const QString &comment);
    bool removeBookMark(const QObject *caller, qsizetype pos);
    bool clearBookMark(const QObject *caller);

private slots:
    void toast(const QObject *caller, const QPixmap &icon,
               const QString &message);
    void logTrace(const QObject *caller, const QString &message);
    void logDebug(const QObject *caller, const QString &message);
    void logWarn(const QObject *caller, const QString &message);
    void logError(const QObject *caller, const QString &message);
    void logInfo(const QObject *caller, const QString &message);

    bool raiseDockWidget(const QObject *caller, QWidget *w);

    // theme
    WingHex::AppTheme currentAppTheme(const QObject *caller);

    // not available for AngelScript
    // only for plugin UI extenstion
    QDialog *createDialog(const QObject *caller, QWidget *content);

private slots:
    void msgAboutQt(const QObject *caller, QWidget *parent = nullptr,
                    const QString &title = QString());

    QMessageBox::StandardButton msgInformation(
        const QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton msgQuestion(
        const QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons =
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton msgWarning(
        const QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    QMessageBox::StandardButton msgCritical(
        const QObject *caller, QWidget *parent, const QString &title,
        const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    void msgAbout(const QObject *caller, QWidget *parent, const QString &title,
                  const QString &text);

    QMessageBox::StandardButton
    msgbox(const QObject *caller, QWidget *parent, QMessageBox::Icon icon,
           const QString &title, const QString &text,
           QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

private slots:
    QString dlgGetText(const QObject *caller, QWidget *parent,
                       const QString &title, const QString &label,
                       QLineEdit::EchoMode echo, const QString &text, bool *ok,
                       Qt::InputMethodHints inputMethodHints);
    QString dlgGetMultiLineText(const QObject *caller, QWidget *parent,
                                const QString &title, const QString &label,
                                const QString &text, bool *ok,
                                Qt::InputMethodHints inputMethodHints);

    QString dlgGetItem(const QObject *caller, QWidget *parent,
                       const QString &title, const QString &label,
                       const QStringList &items, int current, bool editable,
                       bool *ok, Qt::InputMethodHints inputMethodHints);

    int dlgGetInt(const QObject *caller, QWidget *parent, const QString &title,
                  const QString &label, int value, int minValue, int maxValue,
                  int step, bool *ok);

    double dlgGetDouble(const QObject *caller, QWidget *parent,
                        const QString &title, const QString &label,
                        double value, double minValue, double maxValue,
                        int decimals, bool *ok, double step);

private slots:
    QString dlgGetExistingDirectory(const QObject *caller, QWidget *parent,
                                    const QString &caption, const QString &dir,
                                    QFileDialog::Options options);

    QString dlgGetOpenFileName(const QObject *caller, QWidget *parent,
                               const QString &caption, const QString &dir,
                               const QString &filter, QString *selectedFilter,
                               QFileDialog::Options options);

    QStringList dlgGetOpenFileNames(const QObject *caller, QWidget *parent,
                                    const QString &caption, const QString &dir,
                                    const QString &filter,
                                    QString *selectedFilter,
                                    QFileDialog::Options options);

    QString dlgGetSaveFileName(const QObject *caller, QWidget *parent,
                               const QString &caption, const QString &dir,
                               const QString &filter, QString *selectedFilter,
                               QFileDialog::Options options);

private slots:
    QColor dlgGetColor(const QObject *caller, const QString &caption,
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

private slots:
    void updateDocSavedFlag(bool b);

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
    void sigOnCheckSum();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QStackedWidget *m_stack = nullptr;
    GotoWidget *m_goto = nullptr;
    QWidget *m_hexContainer = nullptr;
    EditorViewContext *_context = nullptr;
    bool _hasRegistered = false;
    quintptr _oldbase = 0;

    QHexView *m_hex = nullptr;
    QMenu *m_menu = nullptr;

    QHash<QString, WingEditorViewWidget *> m_others;
    bool m_isNewFile = true;
    QByteArray m_md5;

    QVector<EditorView *> m_cloneChildren;
    EditorView *m_cloneParent = nullptr;

    QMutex m_findMutex;
    FindResultModel::FindData m_findData;
    QMap<QCryptographicHash::Algorithm, QString> _checkSumData;

    DocumentType m_docType = DocumentType::InValid;
    QString workSpaceName;
    QHash<QString, QByteArray> _pluginData;

    WingHex::WingIODevice *_dev = nullptr;
    QUrl m_fileName;

    QReadWriteLock _rwlock;
    CallTable _viewFns;
};

#endif // EDITORVIEW_H
