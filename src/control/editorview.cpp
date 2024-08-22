#include "editorview.h"

#include "QHexView/document/buffer/qfilebuffer.h"
#include "QHexView/document/buffer/qfileregionbuffer.h"
#include "QHexView/document/buffer/qmemorybuffer.h"
#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"
#include "class/eventfilter.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/workspacemanager.h"
#include "dialog/fileinfodialog.h"
#include "utilities.h"

#include <QFile>
#include <QFileInfo>
#include <QVBoxLayout>

constexpr qsizetype FILEMAXBUFFER = 0x6400000; // 100MB
constexpr auto CLONE_LIMIT = 5;

EditorView::EditorView(QWidget *parent) : ads::CDockWidget(QString(), parent) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setObjectName(QStringLiteral("EditorView"));

    m_stack = new QStackedWidget(this);
    this->setWidget(m_stack);

    m_hexContainer = new QWidget(m_stack);
    auto hexLayout = new QVBoxLayout(m_hexContainer);
    hexLayout->setSpacing(0);
    hexLayout->setContentsMargins(0, 0, 0, 0);
    m_hex = new QHexView;
    hexLayout->addWidget(m_hex, 1);
    m_goto = new GotoWidget(this);
    connect(m_goto, &GotoWidget::jumpToLine, this,
            [=](qsizetype pos, bool isline) {
                auto cur = m_hex->cursor();
                isline ? cur->moveTo(pos, 0) : cur->moveTo(pos);
            });
    hexLayout->addWidget(m_goto);
    m_goto->hide();

    m_stack->addWidget(m_hexContainer);

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

    m_menu = new QMenu(m_hex);
    auto &shortcut = QKeySequences::instance();

    newAction(m_menu, "cut", tr("Cut"), &EditorView::sigOnCutFile,
              QKeySequence::Cut);
    newAction(m_menu, "cuthex", tr("CutHex"), &EditorView::sigOnCutHex,
              shortcut.keySequence(QKeySequences::Key::CUT_HEX));
    newAction(m_menu, "copy", tr("Copy"), &EditorView::sigOnCopyFile,
              QKeySequence::Copy);
    newAction(m_menu, "copyhex", tr("CopyHex"), &EditorView::sigOnCopyHex,
              shortcut.keySequence(QKeySequences::Key::COPY_HEX));
    newAction(m_menu, "paste", tr("Paste"), &EditorView::sigOnPasteFile,
              QKeySequence::Paste);
    newAction(m_menu, "pastehex", tr("PasteHex"), &EditorView::sigOnPasteHex,
              shortcut.keySequence(QKeySequences::Key::PASTE_HEX));
    newAction(m_menu, "del", tr("Delete"), &EditorView::sigOnDelete,
              QKeySequence::Delete);
    m_menu->addSeparator();
    newAction(m_menu, "find", tr("Find"), &EditorView::sigOnFindFile,
              QKeySequence::Find);
    newAction(m_menu, "jmp", tr("Goto"), &EditorView::sigOnGoToLine,
              shortcut.keySequence(QKeySequences::Key::GOTO));
    newAction(m_menu, "fill", tr("Fill"), &EditorView::sigOnFill,
              shortcut.keySequence(QKeySequences::Key::HEX_FILL));
    newAction(m_menu, "metadata", tr("MetaData"), &EditorView::sigOnMetadata,
              shortcut.keySequence(QKeySequences::Key::METADATA));
    newAction(m_menu, "bookmark", tr("BookMark"), &EditorView::sigOnBookMark,
              shortcut.keySequence(QKeySequences::Key::BOOKMARK));
    newAction(m_menu, "encoding", tr("Encoding"), &EditorView::sigOnEncoding,
              shortcut.keySequence(QKeySequences::Key::ENCODING));
    m_hex->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_hex, &QHexView::customContextMenuRequested, this,
            [=](const QPoint &pos) { m_menu->popup(mapToGlobal(pos)); });

    m_stack->setCurrentWidget(m_hexContainer);

    m_cloneChildren.fill(nullptr, CLONE_LIMIT);

    m_findResults = new FindResultModel(this);
    auto doc = m_hex->document().get();
    m_bookmarks = new BookMarksModel(doc, this);
    m_metadata = new MetaDataModel(doc, this);
    connect(m_hex, &QHexView::documentChanged, this, [=](QHexDocument *doc) {
        m_bookmarks->setDocument(doc);
        m_metadata->setDocument(doc);
    });

    applySettings();
}

EditorView::~EditorView() {}

void EditorView::registerView(WingEditorViewWidget *view) {
    Q_ASSERT(view);
    m_others << view;
}

void EditorView::switchView(qindextype index) {
    if (index < 0) {
        m_stack->setCurrentWidget(m_hex);
    } else {
        m_stack->setCurrentWidget(m_others.at(index));
    }
    emit viewChanged(index);
}

void EditorView::registerQMenu(QMenu *menu) {
    if (menu == nullptr) {
        return;
    }
    static bool hasRegistered = false;
    if (!hasRegistered) {
        m_menu->addSeparator();
        hasRegistered = true;
    }
    m_menu->addMenu(menu);
}

EditorView::FindError EditorView::find(const QByteArray &data,
                                       const FindDialog::Result &result) {
    if (m_findMutex.tryLock(3000)) {
        std::unique_lock<QMutex> locker(m_findMutex, std::adopt_lock_t());
        auto d = m_hex->document();
        QList<qsizetype> results;
        qsizetype begin, end;
        switch (result.dir) {
        case SearchDirection::Foreword: {
            begin = 0;
            end = m_hex->currentOffset();
        } break;
        case SearchDirection::Backword: {
            begin = m_hex->currentOffset();
            end = -1;
        } break;
        case SearchDirection::Selection: {
            auto cur = m_hex->cursor();
            begin = cur->selectionStart().offset();
            end = cur->selectionEnd().offset();
        } break;
        default: {
            begin = -1;
            end = -1;
        } break;
        }
        d->findAllBytes(begin, end, data, results);

        m_findResults->beginUpdate();
        m_findResults->clear();
        for (auto &result : results) {
            FindResult r;
            r.offset = result;
            r.line = r.offset / m_hex->renderer()->hexLineWidth();
            r.col = r.offset % m_hex->renderer()->hexLineWidth();
            m_findResults->results().append(r);
        }

        m_findResults->lastFindData() = data.toHex(' ').toUpper();
        m_findResults->endUpdate();

        if (m_findResults->size() ==
            std::numeric_limits<QList<qsizetype>::size_type>::max()) {
            return FindError::MayOutOfRange;
        } else {
            return FindError::Success;
        }
    } else {
        return FindError::Busy;
    }
}

void EditorView::clearFindResult() { m_findResults->clear(); }

void EditorView::triggerGoto() {
    m_goto->activeInput(int(m_hex->currentRow()), int(m_hex->currentColumn()),
                        m_hex->currentOffset(), m_hex->documentBytes(),
                        int(m_hex->documentLines()));
}

ErrFile EditorView::newFile(size_t index) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }
    auto istr = QString::number(index);
    m_rawName = tr("Untitled") + istr;
    this->setWindowTitle(m_rawName);
    m_docType = DocumentType::File;
    m_isWorkSpace = false;
    m_fileName = QStringLiteral(":") + istr;
    auto p = QHexDocument::fromMemory<QMemoryBuffer>(QByteArray(), false);
    p->setDocSaved();
    m_hex->setDocument(QSharedPointer<QHexDocument>(p));
    connectDocSavedFlag(this);
    return ErrFile::Success;
}

ErrFile EditorView::openFile(const QString &filename, const QString &encoding) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }

    QFileInfo info(filename);
    if (info.exists()) {
        if (Q_UNLIKELY(!info.permission(QFile::ReadUser))) {
            return ErrFile::Permission;
        }

        auto readonly = !Utilities::fileCanWrite(filename);

        auto *p =
            info.size() > FILEMAXBUFFER
                ? QHexDocument::fromLargeFile(filename, readonly)
                : QHexDocument::fromFile<QMemoryBuffer>(filename, readonly);

        if (Q_UNLIKELY(p == nullptr)) {
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
        m_rawName = info.fileName();
        p->setDocSaved();

        this->setWindowTitle(m_rawName);
        connectDocSavedFlag(this);

        auto tab = this->tabWidget();
        tab->setIcon(Utilities::getIconFromFile(style(), filename));
        tab->setToolTip(filename);
    }

    return ErrFile::Success;
}

ErrFile EditorView::openWorkSpace(const QString &filename,
                                  const QString &encoding) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }

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
        render->setEncoding(infos.encoding);
        doc->metadata()->applyMetas(metas);
        applyPluginData(infos.pluginData);
        doc->setDocSaved();

        m_docType = DocumentType::File;
        m_isWorkSpace = true;
        this->tabWidget()->setIcon(ICONRES(QStringLiteral("pro")));

        return ret;
    }
    return ErrFile::Error;
}

ErrFile EditorView::openRegionFile(QString filename, qsizetype start,
                                   qsizetype length, const QString &encoding) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }

    QFileInfo info(filename);
    if (info.exists()) {
        if (Q_UNLIKELY(!info.permission(QFile::ReadUser))) {
            return ErrFile::Permission;
        }

        auto readonly = !Utilities::fileCanWrite(filename);

        auto *p =
            QHexDocument::fromRegionFile(filename, start, length, readonly);
        if (Q_UNLIKELY(p == nullptr)) {
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
        m_rawName = info.fileName();

        this->setWindowTitle(m_rawName);
        connectDocSavedFlag(this);

        auto tab = this->tabWidget();
        tab->setIcon(Utilities::getIconFromFile(style(), filename));
        tab->setToolTip(filename);
    }

    return ErrFile::Success;
}

ErrFile EditorView::openDriver(const QString &driver, const QString &encoding) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }

    QFileInfo info(driver);
    if (info.exists()) {
        if (Q_UNLIKELY(!info.permission(QFile::ReadUser))) {
            return ErrFile::Permission;
        }

        auto readonly = !Utilities::fileCanWrite(driver);

        auto *p = QHexDocument::fromLargeFile(driver, readonly);

        if (Q_UNLIKELY(p == nullptr)) {
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
        m_rawName = info.fileName();

        this->setWindowTitle(m_rawName);
        connectDocSavedFlag(this);

        auto tab = this->tabWidget();
        tab->setIcon(ICONRES(QStringLiteral("opendriver")));
        tab->setToolTip(driver);
    }

    return ErrFile::Success;
}

ErrFile EditorView::save(const QString &workSpaceName, const QString &path,
                         bool isExport, SaveWorkSpaceAttr workSpaceAttr,
                         bool ignoreMd5) {
    if (isCloneFile()) {
        return this->cloneParent()->save(workSpaceName, path, isExport,
                                         workSpaceAttr, ignoreMd5);
    }
    auto fileName = path.isEmpty() ? m_fileName : path;
    auto doc = m_hex->document();

    if (isNewFile()) {
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

    if (workSpaceAttr == SaveWorkSpaceAttr::ForceWorkSpace ||
        (workSpaceAttr == SaveWorkSpaceAttr::AutoWorkSpace &&
         (m_isWorkSpace || hasMeta()))) {
        WorkSpaceInfo infos;
        infos.base = doc->baseAddress();

        auto b = WorkSpaceManager::saveWorkSpace(
            workSpaceName, m_fileName, doc->getAllBookMarks(),
            doc->metadata()->getallMetas(), infos);
        if (!b)
            return ErrFile::WorkSpaceUnSaved;
        if (!isExport) {
            m_isWorkSpace = true;
        }
    }

    if (doc->saveTo(&file, true)) {
        file.close();

        if (!isExport) {
            m_fileName = fileName;
            m_rawName = QFileInfo(fileName).fileName();
            doc->setDocSaved();
        }

        return ErrFile::Success;
    }

    return ErrFile::Permission;
}

ErrFile EditorView::reload() {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->reload();
    }
    if (isNewFile()) {
        return ErrFile::IsNewFile;
    }

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
    return !m_isWorkSpace && hasMeta();
}

QHexView *EditorView::hexEditor() const { return m_hex; }

EditorView::DocumentType EditorView::documentType() const { return m_docType; }

WingEditorViewWidget *EditorView::otherEditor(qindextype index) const {
    if (index < 0 || index >= m_others.size()) {
        return nullptr;
    }
    return m_others.at(index);
}

void EditorView::setCopyLimit(qsizetype sizeMB) { m_hex->setCopyLimit(sizeMB); }

qsizetype EditorView::copyLimit() const { return m_hex->copyLimit(); }

void EditorView::connectDocSavedFlag(EditorView *editor) {
    connect(editor->m_hex->document().get(), &QHexDocument::documentSaved, this,
            [=](bool b) {
                if (b) {
                    editor->setWindowTitle(m_rawName);
                } else {
                    editor->setWindowTitle(QStringLiteral("* ") + m_rawName);
                }
            });
}

BookMarksModel *EditorView::bookmarksModel() const { return m_bookmarks; }

MetaDataModel *EditorView::metadataModel() const { return m_metadata; }

void EditorView::applySettings() {
    auto &set = SettingManager::instance();
    m_hex->setHeaderVisible(set.editorShowHeader());
    m_hex->setAddressVisible(set.editorShowcol());
    m_hex->setAsciiVisible(set.editorShowtext());
    m_hex->setFontSize(set.editorfontSize());
    m_hex->renderer()->SetEncoding(set.editorEncoding());
}

qindextype EditorView::findAvailCloneIndex() {
    return m_cloneChildren.indexOf(nullptr);
}

bool EditorView::hasMeta() const {
    auto doc = m_hex->document();
    return doc->metadata()->hasMetadata() || doc->bookMarksPtr()->size() > 0;
}

void EditorView::applyPluginData(const QHash<QString, QByteArray> &data) {
    for (auto p = data.begin(); p != data.end(); p++) {
        auto plgw = std::find_if(m_others.begin(), m_others.end(),
                                 [=](const WingEditorViewWidget *editor) {
                                     return editor->id() == p.key();
                                 });
        if (plgw != m_others.end()) {
            (*plgw)->loadState(p.value());
        }
    }
}

QHash<QString, QByteArray> EditorView::savePluginData() {
    QHash<QString, QByteArray> ret;
    for (auto &item : m_others) {
        ret.insert(item->id(), item->saveState());
    }
    return ret;
}

EditorView *EditorView::cloneParent() const { return m_cloneParent; }

bool EditorView::isCloned() const { return m_cloneParent != nullptr; }

EditorView *EditorView::clone() {
    if (isCloneFile()) {
        Q_ASSERT(this->cloneParent());
        return this->cloneParent()->clone();
    }

    auto cloneIndex = findAvailCloneIndex();
    if (cloneIndex < 0) {
        return nullptr;
    }

    auto ev = new EditorView(this->parentWidget());
    connect(ev, &EditorView::destroyed, this, [=] {
        this->m_cloneChildren[this->m_cloneChildren.indexOf(ev)] = nullptr;
    });

    auto doc = this->m_hex->document();

    ev->m_cloneParent = this;
    ev->m_hex->setDocument(doc, ev->m_hex->cursor());

    ev->m_rawName = this->m_rawName + QStringLiteral(" : ") +
                    QString::number(cloneIndex + 1);

    if (doc->isDocSaved()) {
        ev->setWindowTitle(ev->m_rawName);
    } else {
        ev->setWindowTitle(QStringLiteral("* ") + m_rawName);
    }

    ev->setIcon(this->icon());

    ev->m_docType = DocumentType::Cloned;

    for (auto &evw : m_others) {
        ev->m_others << evw->clone();
    }
    this->m_cloneChildren[cloneIndex] = ev;

    connectDocSavedFlag(ev);

    return ev;
}

bool EditorView::isNewFile() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->isNewFile();
    }
    return m_fileName.startsWith(':');
}

bool EditorView::isBigFile() const {
    if (isCloneFile()) {
        return this->cloneParent()->isBigFile();
    }
    return qobject_cast<QFileBuffer *>(m_hex->document()) != nullptr;
}

bool EditorView::isCloneFile() const {
    return m_docType == EditorView::DocumentType::Cloned;
}

bool EditorView::isDriver() const {
    if (isCloneFile()) {
        return this->cloneParent()->isDriver();
    }
    return m_docType == EditorView::DocumentType::Driver;
}

FindResultModel *EditorView::findResultModel() const {
    if (isCloneFile()) {
        return this->cloneParent()->findResultModel();
    }
    return m_findResults;
}

void EditorView::setFontSize(qreal size) { m_hex->setFontSize(size); }

int EditorView::findResultCount() const {
    if (isCloneFile()) {
        return this->cloneParent()->findResultCount();
    }
    return m_findResults->size();
}

bool EditorView::isOriginWorkSpace() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->isOriginWorkSpace();
    }
    return m_isWorkSpace;
}

QString EditorView::fileName() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->fileName();
    }
    return m_fileName;
}
