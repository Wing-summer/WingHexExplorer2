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

#include "editorview.h"

#include "QHexView/document/buffer/qfilebuffer.h"
#include "QHexView/document/buffer/qmemorybuffer.h"
#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"
#include "class/qkeysequences.h"

#include "class/settingmanager.h"
#include "class/workspacemanager.h"
#include "plugin/pluginsystem.h"
#include "utilities.h"

#include <QFile>
#include <QFileInfo>
#include <QVBoxLayout>

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

constexpr qsizetype FILE_MAX_BUFFER = 0x6400000; // 100MB
constexpr auto CLONE_LIMIT = 3;

QString EditorView::getDeviceFileName(const QString &ext, const QString &file) {
    return QStringLiteral("wdrv:///") + ext + QStringLiteral("/") + file;
}

EditorView::EditorView(QWidget *parent)
    : ads::CDockWidget(nullptr, QString(), parent) {
    this->setFeatures(
        CDockWidget::DockWidgetFocusable | CDockWidget::DockWidgetMovable |
        CDockWidget::DockWidgetClosable | CDockWidget::DockWidgetPinnable |
        CDockWidget::CustomCloseHandling |
        CDockWidget::DockWidgetDeleteOnClose);
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

void EditorView::registerView(const QString &id, WingEditorViewWidget *view) {
    Q_ASSERT(view);
    m_others.insert(id, view);
    m_stack->addWidget(view);
    if (!isCloneFile()) {
        if (_pluginData.contains(id)) {
            view->loadState(_pluginData.value(id));
        }
        view->onWorkSpaceNotify(m_isWorkSpace);
    }
}

void EditorView::switchView(const QString &id) {
    // Dont use qobject_cast because
    // each plugin source code has its own WingEditorViewWidget class
    // implement though they should be the same.
    auto curWidget = m_stack->currentWidget();
    if (id.isEmpty()) {
        if (curWidget != m_hexContainer) {
            m_stack->setCurrentWidget(m_hexContainer);
            if (curWidget->inherits("WingHex::WingEditorViewWidget")) {
                auto o = static_cast<WingEditorViewWidget *>(curWidget);
                if (o) {
                    o->toggled(false);
                }
            }
        }
    } else {
        if (m_others.contains(id)) {
            auto sw = m_others.value(id);
            if (curWidget != sw) {
                m_stack->setCurrentWidget(sw);
                if (curWidget->inherits("WingHex::WingEditorViewWidget")) {
                    auto o = static_cast<WingEditorViewWidget *>(curWidget);
                    if (o) {
                        o->toggled(false);
                    }
                }

                sw->toggled(true);
            }
        }
    }
}

void EditorView::registerQMenu(QMenu *menu) {
    if (menu == nullptr) {
        return;
    }
    if (!_hasRegistered) {
        m_menu->addSeparator();
        _hasRegistered = true;
    }
    m_menu->addMenu(menu);
}

EditorView::FindError EditorView::find(const FindDialog::Result &result) {
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
            begin = cur->selectionStart(0).offset();
            end = cur->selectionEnd(0).offset();
        } break;
        default: {
            begin = -1;
            end = -1;
        } break;
        }

        QString data;
        data = result.str;

        qsizetype contextLen = 0;

        if (result.isStringFind) {
            auto raw = Utilities::encodingString(data, result.encoding);
            contextLen = raw.length();
            m_findResults->setEncoding(result.encoding);
            d->findAllBytes(begin, end, raw, results);
        } else {
            contextLen = d->findAllBytesExt(begin, end, result.str, results);
        }

        m_findResults->beginUpdate();
        m_findResults->clear();

        auto lineWidth = m_hex->renderer()->hexLineWidth();
        for (auto &ritem : results) {
            FindResult r;
            r.offset = ritem;
            r.line = r.offset / lineWidth;
            r.col = r.offset % lineWidth;
            m_findResults->results().append(r);
            m_findResults->findData().append(
                readContextFinding(ritem, contextLen, FIND_CONTEXT_SIZE,
                                   FIND_MAX_DISPLAY_FIND_CHARS));
        }

        m_findResults->lastFindData() = data;

        m_findResults->endUpdate();

        if (m_findResults->size() >= QHEXVIEW_FIND_LIMIT) {
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
    m_fileName = tr("Untitled") + istr;
    this->setWindowTitle(m_fileName);
    m_docType = DocumentType::File;
    m_isWorkSpace = false;
    m_isNewFile = true;
    auto p = QHexDocument::fromMemory<QMemoryBuffer>(QByteArray(), false);
    p->setDocSaved();
    m_hex->setDocument(QSharedPointer<QHexDocument>(p));
    m_hex->cursor()->setInsertionMode(QHexCursor::InsertMode);
    connectDocSavedFlag(this);
    return ErrFile::Success;
}

ErrFile EditorView::openFile(const QString &filename) {
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
            info.size() > FILE_MAX_BUFFER
                ? QHexDocument::fromLargeFile(filename, readonly)
                : QHexDocument::fromFile<QMemoryBuffer>(filename, readonly);

        if (Q_UNLIKELY(p == nullptr)) {
            return ErrFile::Permission;
        }

        m_hex->setDocument(QSharedPointer<QHexDocument>(p));
        m_hex->setLockedFile(readonly);
        m_hex->setKeepSize(true);

        m_docType = DocumentType::File;
        m_fileName = info.absoluteFilePath();
        m_isNewFile = false;
        p->setDocSaved();

        this->setWindowTitle(info.fileName());
        connectDocSavedFlag(this);

        auto tab = this->tabWidget();
        tab->setIcon(Utilities::getIconFromFile(style(), m_fileName));
        tab->setToolTip(m_fileName);
    }

    return ErrFile::Success;
}

ErrFile EditorView::openExtFile(const QString &ext, const QString &file) {
    auto dev = PluginSystem::instance().ext2Device(ext);
    if (dev == nullptr) {
        return ErrFile::NotExist;
    }

    auto d = dev->onOpenFile(file);
    if (d == nullptr) {
        return ErrFile::NotExist;
    }

    bool readonly = true;
    if (d->open(QIODevice::ReadWrite)) {
        // ok
        readonly = false;
        d->close();
    } else {
        if (d->open(QIODevice::ReadOnly)) {
            d->close();
        } else {
            return ErrFile::Permission;
        }
    }

    auto *p = (d->size() > FILE_MAX_BUFFER || d->size() < 0)
                  ? QHexDocument::fromDevice<QFileBuffer>(d, readonly)
                  : QHexDocument::fromDevice<QMemoryBuffer>(d, readonly);

    if (Q_UNLIKELY(p == nullptr)) {
        return ErrFile::Error;
    }

    m_hex->setDocument(QSharedPointer<QHexDocument>(p));
    m_hex->setLockedFile(readonly);
    m_hex->setKeepSize(true);
    if (d->keepSize()) {
        m_hex->setLockKeepSize(true);
    }

    auto fileName = getDeviceFileName(ext, file);

    // store the additional info to reload
    _ext = ext;
    _dev = d;
    _file = file;

    m_docType = DocumentType::Extension;
    m_fileName = fileName;
    m_isNewFile = false;
    p->setDocSaved();

    setWindowTitle(fileName);
    connectDocSavedFlag(this);

    auto tab = this->tabWidget();
    tab->setIcon(dev->supportedFileIcon());
    tab->setToolTip(fileName);
    return ErrFile::Success;
}

ErrFile EditorView::openWorkSpace(const QString &filename) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }

    Q_ASSERT(!filename.isEmpty());
    QFileInfo finfo(filename);
    if (!finfo.exists()) {
        return ErrFile::NotExist;
    }

    QString file;
    QMap<qsizetype, QString> bookmarks;
    QVector<QHexMetadataItem> metas;
    WorkSpaceInfo infos;

    if (WorkSpaceManager::loadWorkSpace(filename, file, bookmarks, metas,
                                        infos)) {
        // it's a workspace project
        // we should check the type of "file"
        ErrFile ret;
        auto extPrefix = QStringLiteral("wdrv:///");

        if (file.startsWith(extPrefix)) {
            // extension file
            auto c = file.mid(extPrefix.length());
            auto ci = c.indexOf('/');
            auto ext = c.left(ci);
            ret = openExtFile(ext, file);
        } else {
            // regard as regular files
            ret = openFile(file);
        }

        if (ret != ErrFile::Success) {
            return ret;
        }

        // apply the content
        auto doc = m_hex->document();
        doc->applyBookMarks(bookmarks);
        doc->setBaseAddress(infos.base);
        doc->metadata()->applyMetas(metas);
        _pluginData = infos.pluginData;
        doc->setDocSaved();

        m_docType = DocumentType::File;
        m_isWorkSpace = true;

        auto tab = this->tabWidget();
        tab->setIcon(ICONRES(QStringLiteral("pro")));
        tab->setStyleSheet(
            QStringLiteral("QLabel {text-decoration: underline;}"));

        return ret;
    }
    return ErrFile::Error;
}

ErrFile EditorView::save(const QString &workSpaceName, const QString &path,
                         bool isExport, SaveWorkSpaceAttr workSpaceAttr) {
    if (isCloneFile()) {
        return this->cloneParent()->save(workSpaceName, path, isExport,
                                         workSpaceAttr);
    }
    auto fileName = path.isEmpty() ? m_fileName : path;
    auto doc = m_hex->document();

#ifdef Q_OS_LINUX
    bool needAdjustFile = !QFile::exists(fileName);
    bool needAdjustWs = false;

    auto adjustPermission = [&]() {
        if (Utilities::isRoot()) {
            // a trick off when root under linux OS
            // When new file created, change file's permission to 666.

            // Because you cannot open it when you use it in common user
            // after saving under root user.

            // It's a workaround and not eligent for permission system

            if (needAdjustFile) {
                if (Utilities::isFileOwnerRoot(fileName)) {
                    Utilities::fixUpFilePermissions(fileName);
                }
            }

            if (needAdjustWs) {
                if (Utilities::isFileOwnerRoot(workSpaceName)) {
                    Utilities::fixUpFilePermissions(workSpaceName);
                }
            }
        }
    };
#endif

    if (isNewFile()) {
        if (fileName.isEmpty()) {
            return ErrFile::IsNewFile;
        }
    }

    if (workSpaceAttr == SaveWorkSpaceAttr::ForceWorkSpace ||
        (workSpaceAttr == SaveWorkSpaceAttr::AutoWorkSpace &&
         (m_isWorkSpace || hasMeta()))) {

#ifdef Q_OS_LINUX
        Q_ASSERT(!workSpaceName.isEmpty());
        needAdjustWs = !QFile::exists(workSpaceName);
#endif

        WorkSpaceInfo infos;
        infos.base = doc->baseAddress();
        infos.pluginData = savePluginData();

        auto b = WorkSpaceManager::saveWorkSpace(
            workSpaceName, fileName, doc->bookMarks(),
            doc->metadata()->getAllMetadata(), infos);
        if (!b)
            return ErrFile::WorkSpaceUnSaved;
        if (!isExport) {
            m_isWorkSpace = true;
            notifyOnWorkSpace(true);

            auto convertTabW = [](EditorView *view) {
                auto tab = view->tabWidget();
                tab->setIcon(ICONRES(QStringLiteral("pro")));
                tab->setStyleSheet(
                    QStringLiteral("QLabel {text-decoration: underline;}"));
            };

            convertTabW(this);
            for (auto &c : m_cloneChildren) {
                convertTabW(c);
            }
        }
    }

    if (doc->isUndoByteModified() || m_fileName != fileName || isNewFile()) {
        if (m_docType == DocumentType::Extension) {
            if (_dev->isOpen()) {
                _dev->close();
            }
            _dev->open(QIODevice::WriteOnly);
            if (doc->saveTo(_dev, true)) {
                doc->setDocSaved();
                return ErrFile::Success;
            }
            _dev->close();
        } else {
            QFile file(fileName);
            if (!file.open(QFile::WriteOnly)) {
                return ErrFile::Permission;
            }

            if (doc->saveTo(&file, !isExport)) {
                file.close();

                if (!isExport) {
                    m_fileName = QFileInfo(fileName).absoluteFilePath();
                    m_isNewFile = false;
                    m_docType = DocumentType::File;
                    doc->setDocSaved();
                }
#ifdef Q_OS_LINUX
                adjustPermission();
#endif
                return ErrFile::Success;
            }
        }
        return ErrFile::Permission;
    } else {
        if (!isExport) {
            doc->setDocSaved();
        }
    }
#ifdef Q_OS_LINUX
    adjustPermission();
#endif
    return ErrFile::Success;
}

ErrFile EditorView::reload() {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->reload();
    }
    if (isNewFile()) {
        return ErrFile::IsNewFile;
    }

    auto cret = closeFile();
    if (cret != ErrFile::Success) {
        return cret;
    }

    switch (documentType()) {
    case DocumentType::File:
        return openFile(m_fileName);
    case DocumentType::Extension:
        return openExtFile(_ext, _file);
    default:
        break;
    }
    return ErrFile::Error;
}

ErrFile EditorView::closeFile() {
    if (isCloneFile()) {
        // it will be removed after destoried
        return ErrFile::Success;
    }

    if (m_isWorkSpace) {
        // check whether having plugin metadata
        if (checkHasUnsavedState()) {
            return ErrFile::WorkSpaceUnSaved;
        }
    }

    if (m_docType == DocumentType::Extension) {
        auto dev = PluginSystem::instance().ext2Device(_ext);
        if (dev == nullptr) {
            return ErrFile::Error;
        }
        if (!dev->onCloseFile(_dev)) {
            return ErrFile::Permission;
        }
    }

    for (auto &c : m_cloneChildren) {
        if (c) {
            c->closeDockWidget();
        }
    }

    return ErrFile::Success;
}

bool EditorView::change2WorkSpace() const {
    return !m_isWorkSpace && hasMeta();
}

QHexView *EditorView::hexEditor() const { return m_hex; }

EditorView::DocumentType EditorView::documentType() const { return m_docType; }

void EditorView::setCopyLimit(qsizetype sizeMB) { m_hex->setCopyLimit(sizeMB); }

qsizetype EditorView::copyLimit() const { return m_hex->copyLimit(); }

bool EditorView::isWingEditorViewEnabled(const QString &id) const {
    return m_others.value(id, nullptr) != nullptr;
}

bool EditorView::processWingEditorViewClosing() {
    for (auto &o : m_others) {
        if (!o->onClosing()) {
            return false;
        }
    }
    return true;
}

void EditorView::notifyOnWorkSpace(bool b) {
    for (auto &o : m_others) {
        o->onWorkSpaceNotify(b);
    }
}

void EditorView::connectDocSavedFlag(EditorView *editor) {
    connect(editor->m_hex->document().get(), &QHexDocument::documentSaved, this,
            [=](bool b) {
                QString fileName;
                if (editor->isNewFile()) {
                    fileName = m_fileName;
                } else if (editor->isExtensionFile()) {
                    auto idx = m_fileName.indexOf('}');
                    fileName = m_fileName.mid(idx);
                } else {
                    fileName = QFileInfo(m_fileName).fileName();
                }
                QString content;

                if (b) {
                    content = fileName;
                } else {
                    content = QStringLiteral("* ") + fileName;
                }

                editor->setWindowTitle(content);
                for (int i = 0; i < m_cloneChildren.size(); ++i) {
                    auto c = m_cloneChildren.at(i);
                    if (c) {
                        c->setWindowTitle(content + QStringLiteral(" : ") +
                                          QString::number(i + 1));
                    }
                }

                if (!isNewFile()) {
                    auto tab = this->tabWidget();
                    if (tab->icon().isNull()) {
                        tab->setIcon(
                            Utilities::getIconFromFile(style(), m_fileName));
                    }
                }
            });
}

BookMarksModel *EditorView::bookmarksModel() const { return m_bookmarks; }

MetaDataModel *EditorView::metadataModel() const { return m_metadata; }

bool EditorView::hasCloneChildren() const {
    for (auto &c : m_cloneChildren) {
        if (c) {
            return true;
        }
    }
    return false;
}

void EditorView::closeAllClonedChildren() {
    for (auto &c : m_cloneChildren) {
        if (c) {
            c->deleteDockWidget();
        }
    }
}

void EditorView::applySettings() {
    auto &set = SettingManager::instance();
    m_hex->setHeaderVisible(set.editorShowHeader());
    m_hex->setAddressVisible(set.editorShowcol());
    m_hex->setAsciiVisible(set.editorShowtext());
    m_hex->setFontSize(set.editorfontSize());
}

qsizetype EditorView::findAvailCloneIndex() {
    return m_cloneChildren.indexOf(nullptr);
}

bool EditorView::hasMeta() const {
    auto doc = m_hex->document();
    return doc->metadata()->hasMetadata() || doc->bookMarksCount() > 0;
}

QHash<QString, QByteArray> EditorView::savePluginData() {
    QHash<QString, QByteArray> ret;
    for (auto p = m_others.constKeyValueBegin();
         p != m_others.constKeyValueEnd(); ++p) {
        if (p->second->hasUnsavedState()) {
            auto data = p->second->saveState();
            if (data.isEmpty()) {
                continue;
            }
            ret.insert(p->first, data);
        }
    }
    return ret;
}

bool EditorView::checkHasUnsavedState() const {
    for (auto item : m_others) {
        if (item->hasUnsavedState()) {
            auto data = item->saveState();
            if (data.isEmpty()) {
                continue;
            }
            return true;
        }
    }
    return false;
}

FindResultModel::FindInfo EditorView::readContextFinding(qsizetype offset,
                                                         qsizetype findSize,
                                                         int contextSize,
                                                         int maxDisplayBytes) {
    auto doc = m_hex->document();

    qsizetype halfSize = maxDisplayBytes / 2;
    auto header = doc->read(offset, qMin(findSize, halfSize));
    QByteArray tailer;
    if (header.size() < findSize) {
        auto len = qMin(findSize, qsizetype(maxDisplayBytes) - halfSize);
        tailer = doc->read(offset + findSize - len, len);
    }

    auto left = qsizetype(maxDisplayBytes) - header.size() - tailer.size();

    // append to contextSize
    contextSize += (left / 2);

    auto cheader = doc->read(offset - contextSize, contextSize);
    auto ctailer = doc->read(offset + findSize, contextSize);

    FindResultModel::FindInfo info;
    info.cheader = cheader;
    info.hbuffer = header;
    info.tbuffer = tailer;
    info.ctailer = ctailer;

    return info;
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

    ev->m_fileName = this->m_fileName;
    ev->setWindowTitle(this->windowTitle() + QStringLiteral(" : ") +
                       QString::number(cloneIndex + 1));

    auto tab = ev->tabWidget();
    tab->setIcon(this->icon());
    if (m_isWorkSpace) {
        tab->setStyleSheet(
            QStringLiteral("QLabel {text-decoration: underline;}"));
    }

    ev->m_docType = DocumentType::Cloned;

    for (auto p = m_others.constKeyValueBegin();
         p != m_others.constKeyValueEnd(); ++p) {
        ev->m_others.insert(p->first, p->second->clone());
    }

    this->m_cloneChildren[cloneIndex] = ev;

    return ev;
}

bool EditorView::isNewFile() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->isNewFile();
    }
    return m_isNewFile;
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

bool EditorView::isExtensionFile() const {
    if (isCloneFile()) {
        return this->cloneParent()->isExtensionFile();
    }
    return m_docType == EditorView::DocumentType::Extension;
}

bool EditorView::isCommonFile() const {
    if (isCloneFile()) {
        return this->cloneParent()->isCommonFile();
    }
    return m_docType == EditorView::DocumentType::File;
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
