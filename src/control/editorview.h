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

#include <QObject>
#include <QStackedWidget>

#include "QHexView/qhexview.h"
#include "Qt-Advanced-Docking-System/src/DockWidget.h"
#include "dialog/finddialog.h"
#include "gotowidget.h"
#include "model/bookmarksmodel.h"
#include "model/metadatamodel.h"
#include "plugin/iwingplugin.h"
#include "utilities.h"

#include "model/findresultmodel.h"

using namespace WingHex;

class EditorView : public ads::CDockWidget {
    Q_OBJECT

public:
    enum class DocumentType {
        InValid,
        File,
        RegionFile,
        Driver,
        Extension,
        Cloned
    };

    enum class FindError { Success, Busy, MayOutOfRange };

    enum class SaveWorkSpaceAttr {
        ForceNoWorkSpace,
        AutoWorkSpace,
        ForceWorkSpace
    };

public:
    static QString getDeviceFileName(const QString &ext, const QString &file);

public:
    explicit EditorView(QWidget *parent = nullptr);
    virtual ~EditorView() override;

    QString fileName() const;

    bool isOriginWorkSpace() const;
    bool isNewFile() const;
    bool isBigFile() const;
    bool isCloneFile() const;
    bool isDriver() const;
    bool isExtensionFile() const;

    FindResultModel *findResultModel() const;

    BookMarksModel *bookmarksModel() const;

    MetaDataModel *metadataModel() const;

    void setFontSize(qreal size);

    int findResultCount() const;

    EditorView *cloneParent() const;

    bool isCloned() const;

    bool change2WorkSpace() const;

    QHexView *hexEditor() const;

    DocumentType documentType() const;

    qsizetype copyLimit() const;

public slots:
    EditorView *clone();

    void registerView(const QString &id, WingHex::WingEditorViewWidget *view);
    void switchView(const QString &id);
    void registerQMenu(QMenu *menu);

    FindError find(const FindDialog::Result &result);

    void clearFindResult();

    void triggerGoto();

    ErrFile newFile(size_t index);
    ErrFile openFile(const QString &filename);
    ErrFile openExtFile(const QString &ext, const QString &file,
                        const QVariantList &params);
    ErrFile openWorkSpace(const QString &filename);
    ErrFile openRegionFile(QString filename, qsizetype start, qsizetype length);
    ErrFile openDriver(const QString &driver);
    ErrFile
    save(const QString &workSpaceName, const QString &path = QString(),
         bool ignoreMd5 = false, bool isExport = false,
         SaveWorkSpaceAttr workSpaceAttr = SaveWorkSpaceAttr::AutoWorkSpace);
    ErrFile reload();
    ErrFile closeFile();

    void setCopyLimit(qsizetype sizeMB);

    void applySettings();

private:
    inline qsizetype findAvailCloneIndex();

    bool hasMeta() const;

    void applyPluginData(const QHash<QString, QByteArray> &data);
    QHash<QString, QByteArray> savePluginData();

    FindResultModel::FindInfo readContextFinding(qsizetype offset,
                                                 qsizetype findSize,
                                                 int contextSize,
                                                 int maxDisplayBytes);

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

    // only for extension use
    QString _ext;
    QIODevice *_dev = nullptr;
    QString _file;
    QVariantList _params;
};

#endif // EDITORVIEW_H
