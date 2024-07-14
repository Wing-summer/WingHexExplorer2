#include "editorview.h"

#include "../../QHexView/document/buffer/qfileregionbuffer.h"
#include "../../QHexView/document/buffer/qmemorybuffer.h"
#include "../../Qt-Advanced-Docking-System/src/DockWidgetTab.h"
#include "../class/eventfilter.h"
#include "../class/qkeysequences.h"
#include "../class/workspacemanager.h"
#include "../dialog/fileinfodialog.h"
#include "../plugin/pluginsystem.h"
#include "../utilities.h"

#include <QFile>
#include <QFileInfo>

constexpr qsizetype FILEMAXBUFFER = 0x6400000; // 100MB

EditorView::EditorView(bool enableplugin, QWidget *parent)
    : ads::CDockWidget(QString(), parent), _enableplugin(enableplugin) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setObjectName(QStringLiteral("EditorView"));

    m_stack = new QStackedWidget(this);
    this->setWidget(m_stack);

    m_hex = new QHexView;
    m_stack->addWidget(m_hex);

    auto efilter = new EventFilter(QEvent::MouseButtonPress, this->tabWidget());
    connect(efilter, &EventFilter::eventTriggered, this,
            [this](QObject *obj, QEvent *event) {
                Q_UNUSED(obj);
                auto e = reinterpret_cast<QMouseEvent *>(event);
                if (e->modifiers() == Qt::NoModifier &&
                    e->button() == Qt::MiddleButton) {
                    FileInfoDialog fd(m_fileName, this->documentType() ==
                                                      DocumentType::RegionFile);
                    fd.exec();
                }
            });

    auto tabWidget = this->tabWidget();
    tabWidget->installEventFilter(efilter);

    m_hexMenu = new QMenu(m_hex);
    auto &shortcut = QKeySequences::instance();

    newAction(m_hexMenu, "cut", tr("Cut"), &EditorView::sigOnCutFile,
              QKeySequence::Cut);
    newAction(m_hexMenu, "cuthex", tr("CutHex"), &EditorView::sigOnCutHex,
              shortcut.keySequence(QKeySequences::Key::CUT_HEX));
    newAction(m_hexMenu, "copy", tr("Copy"), &EditorView::sigOnCopyFile,
              QKeySequence::Copy);
    newAction(m_hexMenu, "copyhex", tr("CopyHex"), &EditorView::sigOnCopyHex,
              shortcut.keySequence(QKeySequences::Key::COPY_HEX));
    newAction(m_hexMenu, "paste", tr("Paste"), &EditorView::sigOnPasteFile,
              QKeySequence::Paste);
    newAction(m_hexMenu, "pastehex", tr("PasteHex"), &EditorView::sigOnPasteHex,
              shortcut.keySequence(QKeySequences::Key::PASTE_HEX));
    newAction(m_hexMenu, "del", tr("Delete"), &EditorView::sigOnDelete,
              QKeySequence::Delete);
    m_hexMenu->addSeparator();
    newAction(m_hexMenu, "find", tr("Find"), &EditorView::sigOnFindFile,
              QKeySequence::Find);
    newAction(m_hexMenu, "jmp", tr("Goto"), &EditorView::sigOnGoToLine,
              shortcut.keySequence(QKeySequences::Key::GOTO));
    newAction(m_hexMenu, "fill", tr("Fill"), &EditorView::sigOnFill,
              shortcut.keySequence(QKeySequences::Key::HEX_FILL));
    newAction(m_hexMenu, "metadata", tr("MetaData"), &EditorView::sigOnMetadata,
              shortcut.keySequence(QKeySequences::Key::METADATA));
    newAction(m_hexMenu, "bookmark", tr("BookMark"), &EditorView::sigOnBookMark,
              shortcut.keySequence(QKeySequences::Key::BOOKMARK));
    newAction(m_hexMenu, "encoding", tr("Encoding"), &EditorView::sigOnEncoding,
              shortcut.keySequence(QKeySequences::Key::ENCODING));

    m_hex->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_hex, &QHexView::customContextMenuRequested, this,
            &EditorView::on_hexeditor_customContextMenuRequested);

    m_stack->setCurrentWidget(m_hex);
}

void EditorView::registerView(QWidget *view) {
    Q_ASSERT(view);
    m_others << view;
}

void EditorView::switchView(qindextype index) {
    if (index < 0) {
        m_stack->setCurrentWidget(qobject_cast<QWidget *>(m_hex));
    } else {
        m_stack->setCurrentWidget(m_others.at(index));
    }
    emit viewChanged(index);
}

void EditorView::newFile(size_t index) {
    this->setWindowTitle(tr("Untitled") + QString::number(index));
    m_docType = DocumentType::File;
    m_isNewFile = true;
    m_fileName = QString();
}

ErrFile EditorView::openFile(const QString &filename, const QString &encoding) {
    QList<QVariant> params;
    auto &plgsys = PluginSystem::instance();

    if (_enableplugin) {
        params << filename;
        plgsys.raiseDispatch(HookIndex::OpenFileBegin, params);
    }

    QFileInfo info(filename);
    if (info.exists()) {
        if (Q_UNLIKELY(!info.permission(QFile::ReadUser))) {
            if (_enableplugin) {
                params << ErrFile::Permission;
                plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
            }
            return ErrFile::Permission;
        }

        auto readonly = !Utilities::fileCanWrite(filename);

        auto *p = info.size() > FILEMAXBUFFER
                      ? QHexDocument::fromLargeFile(filename, readonly, this)
                      : QHexDocument::fromFile<QMemoryBuffer>(filename,
                                                              readonly, this);

        if (Q_UNLIKELY(p == nullptr)) {
            if (_enableplugin) {
                params << ErrFile::Permission;
                plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
            }
            return ErrFile::Permission;
        }

        m_hex->setDocument(QSharedPointer<QHexDocument>(p));
        m_hex->setLockedFile(readonly);
        m_hex->setKeepSize(true);

        if (!encoding.isEmpty()) {
            m_hex->renderer()->setEncoding(encoding);
        }

        m_docType = DocumentType::File;
        m_fileName = filename;
        p->setDocSaved();

        this->setWindowTitle(info.fileName());

        auto tab = this->tabWidget();
        tab->setIcon(Utilities::getIconFromFile(style(), filename));
        tab->setToolTip(filename);

        if (_enableplugin) {
            params << ErrFile::Success;
            plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
        }
    }

    if (_enableplugin) {
        params << ErrFile::NotExist;
        plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
    }

    return ErrFile::Success;
}

ErrFile EditorView::openWorkSpace(const QString &filename,
                                  const QString &encoding) {

    Q_ASSERT(!filename.isEmpty());
    QFileInfo finfo(filename);
    if (!finfo.exists()) {
        return ErrFile::NotExist;
    }

    QString file;
    QList<BookMarkStruct> bookmarks;
    QList<QHexMetadataAbsoluteItem> metas;
    WorkSpaceInfo infos;

    if (WorkSpaceManager::loadWorkSpace(filename, file, bookmarks, metas,
                                        infos)) {
        // it's a workspace project
        auto ret = openFile(file, encoding);

        // apply the content
        auto doc = m_hex->document();
        auto render = m_hex->renderer();
        doc->applyBookMarks(bookmarks);
        doc->setBaseAddress(infos.base);
        m_hex->setLockedFile(infos.locked);
        m_hex->setKeepSize(infos.keepsize);
        doc->setMetabgVisible(infos.showmetabg);
        doc->setMetafgVisible(infos.showmetafg);
        doc->setMetaCommentVisible(infos.showmetacomment);
        render->setStringVisible(infos.showstr);
        render->setHeaderVisible(infos.showheader);
        render->setAddressVisible(infos.showaddr);
        render->setEncoding(infos.encoding);
        doc->metadata()->applyMetas(metas);
        doc->setDocSaved();

        m_isWorkSpace = true;
        this->tabWidget()->setIcon(ICONRES(QStringLiteral("pro")));

        return ret;
    }
    return ErrFile::Error;
}

ErrFile EditorView::openRegionFile(QString filename, qsizetype start,
                                   qsizetype length, const QString &encoding) {
    QList<QVariant> params;
    auto &plgsys = PluginSystem::instance();

    if (_enableplugin) {
        params << filename;
        plgsys.raiseDispatch(HookIndex::OpenFileBegin, params);
    }

    QFileInfo info(filename);
    if (info.exists()) {
        if (Q_UNLIKELY(!info.permission(QFile::ReadUser))) {
            if (_enableplugin) {
                params << ErrFile::Permission;
                plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
            }
            return ErrFile::Permission;
        }

        auto readonly = !Utilities::fileCanWrite(filename);

        auto *p = QHexDocument::fromRegionFile(filename, start, length,
                                               readonly, this);
        if (Q_UNLIKELY(p == nullptr)) {
            if (_enableplugin) {
                params << ErrFile::Permission;
                plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
            }
            return ErrFile::Permission;
        }

        m_docType = DocumentType::RegionFile;

        m_hex->setDocument(QSharedPointer<QHexDocument>(p));
        m_hex->setLockedFile(readonly);
        m_hex->setKeepSize(true);

        if (!encoding.isEmpty()) {
            m_hex->renderer()->setEncoding(encoding);
        }

        p->setDocSaved();
        m_fileName = filename;

        this->setWindowTitle(info.fileName());

        auto tab = this->tabWidget();
        tab->setIcon(Utilities::getIconFromFile(style(), filename));
        tab->setToolTip(filename);

        if (_enableplugin) {
            params << ErrFile::Success;
            plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
        }
    }

    if (_enableplugin) {
        params << ErrFile::NotExist;
        plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
    }

    return ErrFile::Success;
}

ErrFile EditorView::openDriver(const QString &driver, const QString &encoding) {
    QList<QVariant> params;
    auto &plgsys = PluginSystem::instance();

    if (_enableplugin) {
        params << driver;
        plgsys.raiseDispatch(HookIndex::OpenFileBegin, params);
    }

    QFileInfo info(driver);
    if (info.exists()) {
        if (Q_UNLIKELY(!info.permission(QFile::ReadUser))) {
            if (_enableplugin) {
                params << ErrFile::Permission;
                plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
            }
            return ErrFile::Permission;
        }

        auto readonly = !Utilities::fileCanWrite(driver);

        auto *p = QHexDocument::fromLargeFile(driver, readonly, this);

        if (Q_UNLIKELY(p == nullptr)) {
            if (_enableplugin) {
                params << ErrFile::Permission;
                plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
            }
            return ErrFile::Permission;
        }

        m_docType = DocumentType::Driver;

        m_hex->setDocument(QSharedPointer<QHexDocument>(p));
        m_hex->setLockedFile(readonly);
        m_hex->setKeepSize(true);

        if (!encoding.isEmpty()) {
            m_hex->renderer()->setEncoding(encoding);
        }

        p->setDocSaved();
        m_fileName = driver;

        this->setWindowTitle(info.fileName());

        auto tab = this->tabWidget();
        tab->setIcon(ICONRES(QStringLiteral("opendriver")));
        tab->setToolTip(driver);

        if (_enableplugin) {
            params << ErrFile::Success;
            plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
        }
    }

    if (_enableplugin) {
        params << ErrFile::NotExist;
        plgsys.raiseDispatch(HookIndex::OpenFileEnd, params);
    }

    return ErrFile::Success;
}

ErrFile EditorView::save(const QString &workSpaceName, const QString &path,
                         bool isExport, bool forceWorkSpace, bool ignoreMd5) {
    auto fileName = path.isEmpty() ? m_fileName : path;
    auto doc = m_hex->document();

    if (m_isNewFile) {
        if (fileName.isEmpty()) {
            return ErrFile::IsNewFile;
        }
    }

    QFile file(fileName);

    switch (m_docType) {
    case DocumentType::RegionFile: {
        if (!ignoreMd5 && Utilities::getMd5(m_fileName) != m_md5) {
            return ErrFile::SourceFileChanged;
        }
        if (!file.open(QFile::ReadWrite)) {
            return ErrFile::Permission;
        }
    } break;
    default: {
        if (!file.open(QFile::WriteOnly)) {
            return ErrFile::Permission;
        }
    } break;
    }

    if (doc->saveTo(&file, true)) {
        file.close();
        if (forceWorkSpace || doc->metadata()->hasMetadata()) {
            auto render = m_hex->renderer();

            WorkSpaceInfo infos;
            infos.base = doc->baseAddress();
            infos.locked = doc->isLocked();
            infos.keepsize = doc->isKeepSize();
            infos.showstr = render->stringVisible();
            infos.showaddr = render->addressVisible();
            infos.showheader = render->headerVisible();
            infos.encoding = render->encoding();
            infos.showmetabg = doc->metabgVisible();
            infos.showmetafg = doc->metafgVisible();
            infos.showmetacomment = doc->metaCommentVisible();

            auto b = WorkSpaceManager::saveWorkSpace(
                workSpaceName, m_fileName, doc->getAllBookMarks(),
                doc->metadata()->getallMetas(), infos);
            if (!b)
                return ErrFile::WorkSpaceUnSaved;
            if (!isExport) {
                m_isWorkSpace = true;
            }
        } else {
            if (!isExport) {
                m_isNewFile = false;
            }
        }

        if (!isExport) {
            m_fileName = fileName;
            doc->setDocSaved();
        }

        return ErrFile::Success;
    }

    return ErrFile::Permission;
}

ErrFile EditorView::reload() {
    Q_ASSERT(m_docType != DocumentType::InValid);
    Q_ASSERT(!m_isNewFile);

    switch (documentType()) {
    case DocumentType::File:
        return openFile(m_fileName, m_hex->renderer()->encoding());
    case DocumentType::RegionFile: {
        auto doc = qobject_cast<QFileRegionBuffer *>(m_hex->document());
        Q_ASSERT(doc);
        return openRegionFile(m_fileName, doc->readOffset(),
                              doc->readMaxBytes(),
                              m_hex->renderer()->encoding());
    }
    case DocumentType::Driver:
        return openDriver(m_fileName, m_hex->renderer()->encoding());
    default:
        break;
    }
    return ErrFile::Error;
}

bool EditorView::change2WorkSpace() const {
    return !m_isWorkSpace && m_hex->document()->metadata()->hasMetadata();
}

QHexView *EditorView::hexEditor() const { return m_hex; }

EditorView::DocumentType EditorView::documentType() const { return m_docType; }

QWidget *EditorView::otherEditor(qindextype index) const {
    if (index < 0 || index >= m_others.size()) {
        return nullptr;
    }
    return m_others.at(index);
}

void EditorView::setCopyLimit(qsizetype sizeMB) {
    m_hex->document()->setCopyLimit(sizeMB);
}

qsizetype EditorView::copyLimit() const {
    return m_hex->document()->copyLimit();
}

void EditorView::on_hexeditor_customContextMenuRequested(const QPoint &pos) {
    Q_UNUSED(pos)
    m_hexMenu->popup(QCursor::pos());
}

bool EditorView::isNewFile() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    return m_isNewFile;
}

bool EditorView::isWorkSpace() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    return m_isWorkSpace;
}

QString EditorView::fileName() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    return m_fileName;
}
