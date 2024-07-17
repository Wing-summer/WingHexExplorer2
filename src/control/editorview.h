#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QObject>
#include <QStackedWidget>

#include "../../QHexView/qhexview.h"
#include "../../Qt-Advanced-Docking-System/src/DockWidget.h"
#include "../define.h"
#include "../plugin/iwingplugin.h"
#include "../utilities.h"

using namespace WingHex;

class EditorView : public ads::CDockWidget {
    Q_OBJECT

public:
    enum class DocumentType { InValid, File, RegionFile, Driver };

public:
    explicit EditorView(bool enableplugin, QWidget *parent = nullptr);

    QString fileName() const;

    bool isWorkSpace() const;

    bool isNewFile() const;

    bool isBigFile() const;

public slots:
    void registerView(QWidget *view);

    void switchView(qindextype index);

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

    QWidget *otherEditor(qindextype index) const;

    void setCopyLimit(qsizetype sizeMB);

    qsizetype copyLimit() const;

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
    QHexView *m_hex = nullptr;
    QList<QWidget *> m_others;
    QString m_fileName;
    QByteArray m_md5;

    QMenu *m_hexMenu = nullptr;

    DocumentType m_docType = DocumentType::InValid;
    bool m_isNewFile = false;
    bool m_isWorkSpace = false;

    bool _enableplugin = true;
};

#endif // EDITORVIEW_H
