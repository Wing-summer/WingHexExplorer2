#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QObject>
#include <QStackedWidget>

#include "QHexView/qhexview.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "define.h"
#include "dialog/finddialog.h"
#include "gotowidget.h"
#include "model/bookmarksmodel.h"
#include "plugin/iwingplugin.h"
#include "utilities.h"

#include "model/findresultmodel.h"

using namespace WingHex;

class EditorView : public ads::CDockWidget {
    Q_OBJECT

public:
    enum class DocumentType { InValid, File, RegionFile, Driver, Cloned };

    enum class FindError { Success, Busy, MayOutOfRange };

    enum class SaveWorkSpaceAttr {
        ForceNoWorkSpace,
        AutoWorkSpace,
        ForceWorkSpace
    };

public:
    explicit EditorView(QWidget *parent = nullptr);
    virtual ~EditorView() override;

    QString fileName() const;

    bool isOriginWorkSpace() const;
    bool isNewFile() const;
    bool isBigFile() const;
    bool isCloneFile() const;
    bool isDriver() const;

    FindResultModel *findResultModel() const;

    BookMarksModel *bookmarksModel() const;

    void setFontSize(qreal size);

    int findResultCount() const;

    EditorView *cloneParent() const;

    bool isCloned() const;

    bool change2WorkSpace() const;

    QHexView *hexEditor() const;

    DocumentType documentType() const;

    WingEditorViewWidget *otherEditor(qindextype index) const;

    qsizetype copyLimit() const;

public slots:
    EditorView *clone();

    void registerView(WingEditorViewWidget *view);
    void switchView(qindextype index);
    void registerQMenu(QMenu *menu);

    FindError find(const QByteArray &data, const FindDialog::Result &result);

    void clearFindResult();

    void triggerGoto();

    ErrFile newFile(size_t index);
    ErrFile openFile(const QString &filename,
                     const QString &encoding = QString());
    ErrFile openWorkSpace(const QString &filename,
                          const QString &encoding = QString());
    ErrFile openRegionFile(QString filename, qsizetype start, qsizetype length,
                           const QString &encoding = QString());
    ErrFile openDriver(const QString &driver,
                       const QString &encoding = QString());
    ErrFile
    save(const QString &workSpaceName, const QString &path = QString(),
         bool isExport = false,
         SaveWorkSpaceAttr workSpaceAttr = SaveWorkSpaceAttr::AutoWorkSpace,
         bool ignoreMd5 = false);
    ErrFile reload();

    void setCopyLimit(qsizetype sizeMB);

    void applySettings();

private:
    inline qindextype findAvailCloneIndex();

    bool hasMeta() const;

    void applyPluginData(const QHash<QString, QByteArray> &data);
    QHash<QString, QByteArray> savePluginData();

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
    void viewChanged(int index);

    void sigFileSaved(QString filename, QString oldFileName);

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
    void sigOnEncoding();

private:
    QStackedWidget *m_stack = nullptr;
    GotoWidget *m_goto = nullptr;
    QWidget *m_hexContainer = nullptr;

    QHexView *m_hex = nullptr;
    QMenu *m_menu = nullptr;

    QList<WingEditorViewWidget *> m_others;
    QString m_fileName;
    QString m_rawName;
    QByteArray m_md5;

    QVector<EditorView *> m_cloneChildren;
    EditorView *m_cloneParent = nullptr;

    QMutex m_findMutex;
    FindResultModel *m_findResults = nullptr;
    BookMarksModel *m_bookmarks = nullptr;

    DocumentType m_docType = DocumentType::InValid;
    bool m_isWorkSpace = false;
};

#endif // EDITORVIEW_H
