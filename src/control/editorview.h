#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QObject>
#include <QStackedWidget>

#include "QHexView/qhexview.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "gotowidget.h"
#include "src/define.h"
#include "src/dialog/finddialog.h"
#include "src/plugin/iwingplugin.h"
#include "src/utilities.h"

#include <QTableWidgetItem>

using namespace WingHex;

class EditorView : public ads::CDockWidget {
    Q_OBJECT

public:
    enum class DocumentType { InValid, File, RegionFile, Driver, Cloned };

    enum class FindError { Success, Busy, MayOutOfRange };

public:
    explicit EditorView(bool enableplugin, QWidget *parent = nullptr);
    virtual ~EditorView() override;

    QString fileName() const;

    bool isWorkSpace() const;
    bool isNewFile() const;
    bool isBigFile() const;
    bool isCloneFile() const;

    const QList<qsizetype> &findResult() const;

    void setFontSize(qreal size);

    int findResultCount() const;

    QByteArray lastFindData() const;

    void setIsWorkSpace(bool newIsWorkSpace);

    EditorView *cloneParent() const;

    bool isCloned() const;

    bool enablePlugin() const;
    void setEnablePlugin(bool newEnableplugin);

public slots:
    EditorView *clone();

    void registerView(WingEditorViewWidget *view);
    void switchView(qindextype index);

    FindError find(const QByteArray &data, const FindDialog::Result &result,
                   qsizetype findMaxCount);

    void clearFindResult();

    void triggerGoto();

    void newFile(size_t index);
    ErrFile openFile(const QString &filename,
                     const QString &encoding = QString());
    ErrFile openWorkSpace(const QString &filename,
                          const QString &encoding = QString());
    ErrFile openRegionFile(QString filename, qsizetype start, qsizetype length,
                           const QString &encoding = QString());
    ErrFile openDriver(const QString &driver,
                       const QString &encoding = QString());
    ErrFile save(const QString &workSpaceName, const QString &path = QString(),
                 bool isExport = false, bool forceWorkSpace = false,
                 bool ignoreMd5 = false);
    ErrFile reload();

    bool change2WorkSpace() const;

    QHexView *hexEditor() const;

    DocumentType documentType() const;

    WingEditorViewWidget *otherEditor(qindextype index) const;

    void setCopyLimit(qsizetype sizeMB);

    qsizetype copyLimit() const;

    void applySettings();

private:
    template <typename Func>
    inline void newAction(QMenu *parent, const QString &icon,
                          const QString &title, Func &&slot,
                          const QKeySequence &shortcut = QKeySequence()) {
        auto a = new QAction(parent);
        a->setText(title);
        a->setIcon(ICONRES(icon));
        a->setShortcut(shortcut);
        a->setShortcutVisibleInContextMenu(true);
        connect(a, &QAction::triggered, this, slot);
        parent->addAction(a);
    }

    void connectDocSavedFlag();

private slots:
    void on_hexeditor_customContextMenuRequested(const QPoint &pos);

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

    QList<WingEditorViewWidget *> m_others;
    QString m_fileName;
    QString m_rawName;
    QByteArray m_md5;

    QList<EditorView *> m_cloneChildren;
    EditorView *m_cloneParent = nullptr;

    QMutex m_findMutex;
    QList<qsizetype> m_findResults;
    QByteArray m_lastFindData;

    QMenu *m_hexMenu = nullptr;

    DocumentType m_docType = DocumentType::InValid;
    bool m_isWorkSpace = false;

    bool _enableplugin = true;
};

#endif // EDITORVIEW_H
