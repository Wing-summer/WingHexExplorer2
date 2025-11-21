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
#include "Qt-Advanced-Docking-System/src/DockWidgetTab.h"

#include "class/compositeiconengine.h"
#include "class/logger.h"
#include "class/pluginsystem.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/workspacemanager.h"
#include "utilities.h"

#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QVBoxLayout>

constexpr auto CLONE_LIMIT = 3;

constexpr auto VIEW_PROPERTY = "__VIEW__";
constexpr auto VIEW_ID_PROPERTY = "__ID__";

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
    m_hex = new QHexView(this);
    _context = new EditorViewContext(m_hex);
    auto &plgsys = PluginSystem::instance();
    auto hexExt = plgsys.hexEditorExtension();
    if (hexExt) {
        connect(m_hex, &QHexView::onPaintCustomEventBegin, this, [this]() {
            auto m =
                PluginSystem::instance().hexEditorExtension()->contentMargins(
                    _context);
            m.setLeft(qMax(0, m.left()));
            m.setTop(qMax(0, m.top()));
            m.setRight(qMax(0, m.right()));
            m.setBottom(qMax(0, m.bottom()));
            m_hex->viewport()->setContentsMargins(m);
        });
        m_hex->setDisableInternalPaint(hexExt->disableOriginPainting());
    }
    connect(m_hex, &QHexView::onPaintCustomEvent, this,
            [this](int XOffset, qsizetype firstVisible, qsizetype begin,
                   qsizetype end) {
                auto vp = m_hex->viewport();
                QPainter painter(vp);
                auto &plgsys = PluginSystem::instance();
                auto hext = plgsys.hexEditorExtension();

                _context->setCurrentHorizontalOffset(XOffset);
                _context->setFirstVisibleLine(firstVisible);
                _context->setBeginLine(begin);
                _context->setEndLine(end);

                if (hext) {
                    painter.save();
                    hext->onPaintEvent(&painter, vp, _context);
                    painter.restore();
                }
                plgsys.dispatchEvent(
                    IWingPlugin::RegisteredEvent::HexEditorViewPaint,
                    {quintptr(&painter), quintptr(vp), quintptr(_context)});
            });

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

    newAction(m_menu, QStringLiteral("cut"), tr("Cut"),
              &EditorView::sigOnCutFile, QKeySequence::Cut);
    newAction(m_menu, QStringLiteral("cuthex"), tr("CutHex"),
              &EditorView::sigOnCutHex,
              shortcut.keySequence(QKeySequences::Key::CUT_HEX));
    newAction(m_menu, QStringLiteral("copy"), tr("Copy"),
              &EditorView::sigOnCopyFile, QKeySequence::Copy);
    newAction(m_menu, QStringLiteral("copyhex"), tr("CopyHex"),
              &EditorView::sigOnCopyHex,
              shortcut.keySequence(QKeySequences::Key::COPY_HEX));
    newAction(m_menu, QStringLiteral("paste"), tr("Paste"),
              &EditorView::sigOnPasteFile, QKeySequence::Paste);
    newAction(m_menu, QStringLiteral("pastehex"), tr("PasteHex"),
              &EditorView::sigOnPasteHex,
              shortcut.keySequence(QKeySequences::Key::PASTE_HEX));
    newAction(m_menu, QStringLiteral("del"), tr("Delete"),
              &EditorView::sigOnDelete, QKeySequence::Delete);
    m_menu->addSeparator();
    newAction(m_menu, QStringLiteral("find"), tr("Find"),
              &EditorView::sigOnFindFile, QKeySequence::Find);
    newAction(m_menu, QStringLiteral("jmp"), tr("Goto"),
              &EditorView::sigOnGoToLine,
              shortcut.keySequence(QKeySequences::Key::GOTO));
    newAction(m_menu, QStringLiteral("fill"), tr("Fill"),
              &EditorView::sigOnFill,
              shortcut.keySequence(QKeySequences::Key::HEX_FILL));
    newAction(m_menu, QStringLiteral("metadata"), tr("MetaData"),
              &EditorView::sigOnMetadata,
              shortcut.keySequence(QKeySequences::Key::METADATA));
    newAction(m_menu, QStringLiteral("bookmark"), tr("BookMark"),
              &EditorView::sigOnBookMark,
              shortcut.keySequence(QKeySequences::Key::BOOKMARK));
    newAction(m_menu, QStringLiteral("sum"), tr("CheckSum"),
              &EditorView::sigOnCheckSum);
    m_hex->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_hex, &QHexView::customContextMenuRequested, this,
            [=](const QPoint &pos) { m_menu->popup(mapToGlobal(pos)); });

    m_stack->setCurrentWidget(m_hexContainer);

    m_cloneChildren.fill(nullptr, CLONE_LIMIT);

    applySettings();

    // build up call tables
    auto mobj = EditorView::metaObject();
    auto total = mobj->methodCount();
    for (int i = mobj->methodOffset(); i < total; ++i) {
        auto m = mobj->method(i);
        // all private slots
        if (m.methodType() == QMetaMethod::Slot &&
            m.access() == QMetaMethod::Private) {
            auto total = m.parameterCount();
            if (total < 1 || m.parameterMetaType(0) !=
                                 QMetaType::fromType<const QObject *>()) {
                continue;
            }

            WingHex::FunctionSig msig;
            msig.fnName = m.name();
            msig.types.reserve(total - 1);

            for (int i = 1; i < total; ++i) {
                auto mt = m.parameterType(i);
                msig.types.append(mt);
            }

            _viewFns.insert(msig, m);
        }
    }

    // checksum table data
    for (auto &cs : Utilities::supportedHashAlgorithms()) {
        _checkSumData.insert(cs, QString());
    }

    this->tabWidget()->installEventFilter(this);

    connect(m_stack, &QStackedWidget::currentChanged, this,
            &EditorView::updateIcon);
}

EditorView::~EditorView() {}

void EditorView::registerView(const QString &id, WingEditorViewWidget *view,
                              const QIcon &viewIcon) {
    Q_ASSERT(view);
    m_others.insert(id, view);
    m_stack->addWidget(view);
    view->setProperty(VIEW_PROPERTY, quintptr(this));
    view->setProperty(VIEW_ID_PROPERTY, id);
    view->installEventFilter(this);
    applyFunctionTables(view, _viewFns);

    if (!isCloneFile()) {
        connect(view, &WingEditorViewWidget::savedStateChanged, this,
                [this]() { updateDocSavedFlag(m_hex->isSaved()); });

        if (_pluginData.contains(id)) {
            view->loadState(_pluginData.value(id));
        }
    }
    m_iconOrigin.insert(view, viewIcon);
    m_iconCaches.insert(view,
                        QIcon(new CompositeIconEngine(
                            viewIcon, m_iconCaches.value(m_hexContainer))));
    view->onWorkSpaceNotify(isWorkSpace());
}

void EditorView::switchView(const QString &id) {
    auto curWidget = m_stack->currentWidget();
    if (id.isEmpty()) {
        if (curWidget != m_hexContainer) {
            m_stack->setCurrentWidget(m_hexContainer);
            auto o = qobject_cast<WingEditorViewWidget *>(curWidget);
            if (o) {
                o->toggled(false);
            }
        }
    } else {
        if (m_others.contains(id)) {
            auto sw = m_others.value(id);
            if (curWidget != sw) {
                m_stack->setCurrentWidget(sw);
                auto o = qobject_cast<WingEditorViewWidget *>(curWidget);
                if (o) {
                    o->toggled(false);
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

void EditorView::getCheckSum(const QVector<int> &algorithmID) {
    auto hashes = Utilities::supportedHashAlgorithms();
    for (auto &cs : hashes) {
        _checkSumData.insert(cs, QString());
    }
    if (m_hex->hasSelection()) {
        auto data = m_hex->selectedBytes();
        for (auto &c : algorithmID) {
            auto h = hashes.at(c);
            QCryptographicHash hash(h);
            hash.addData(data.join());
            _checkSumData.insert(
                h, QString::fromLatin1((hash.result().toHex().toUpper())));
        }
    } else {
        auto io = m_hex->document()->buffer()->ioDevice();
        for (auto &c : algorithmID) {
            auto h = hashes.at(c);
            QCryptographicHash hash(h);
            hash.addData(io);
            _checkSumData.insert(
                h, QString::fromLatin1(hash.result().toHex().toUpper()));
        }
    }
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
            begin = 0;
            end = -1;
        } break;
        }

        QString data;
        data = result.value;

        qsizetype contextLen = 0;
        m_findData.clear();

        if (result.isStringFind) {
            auto raw = Utilities::encodingString(data, result.encoding);
            contextLen = raw.length();
            m_findData.encoding = result.encoding;
            d->findAllBytes(begin, end, raw, results);
            m_findData.lastFindData = qMakePair(data, contextLen);
        } else {
            // assuming the find pattern is 'xxxxxxxx'
            contextLen = data.length() / 2;
            d->findAllBytesExt(begin, end, result.value, results);
            m_findData.lastFindData = qMakePair(data, contextLen);
        }

        auto lineWidth = m_hex->renderer()->hexLineWidth();
        for (auto &ritem : results) {
            FindResultModel::FindResult r;
            r.offset = ritem;
            r.line = r.offset / lineWidth;
            r.col = r.offset % lineWidth;
            m_findData.results.append(r);
            m_findData.findData.append(readContextFinding(ritem, contextLen));
        }

        if (m_findData.results.size() >= QHEXVIEW_FIND_LIMIT) {
            return FindError::MayOutOfRange;
        } else {
            return FindError::Success;
        }
    } else {
        return FindError::Busy;
    }
}

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
    auto fname = tr("Untitled") + istr;
    m_docType = DocumentType::File;
    m_isNewFile = true;
    m_workSpaceName.clear();
    auto p = QHexDocument::fromInternalBuffer<QFileBuffer>(false);
    p->setDocSaved();
    connect(p, &QHexDocument::documentSaved, this,
            &EditorView::updateDocSavedFlag);
    m_hex->setDocument(QSharedPointer<QHexDocument>(p));
    m_hex->cursor()->setInsertionMode(QHexCursor::InsertMode);
    setFileNameUrl(QStringLiteral("file://") + newFileAuthority() +
                   QStringLiteral("/") + fname);
    generateIconBaseCache({});
    updateIcon();
    return ErrFile::Success;
}

ErrFile EditorView::openFile(const QString &filename, bool generateCache) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }

    QFileInfo info(filename);
    if (!info.exists()) {
        return ErrFile::NotExist;
    }

    if (Q_UNLIKELY(!info.isReadable())) {
        return ErrFile::Permission;
    }

    if (!info.isFile()) {
        return ErrFile::InvalidFormat;
    }

    bool retry;
    auto readonly = !info.isWritable();
    QHexDocument *p;
    do {
        retry = false;
        p = QHexDocument::fromFile<QFileBuffer>(filename, readonly);
        if (Q_UNLIKELY(p == nullptr)) {
            if (!readonly) {
                // retry to open with readonly
                readonly = true;
                retry = true;
                continue;
            }
            return ErrFile::Permission;
        }
    } while (retry);

    m_hex->setDocument(QSharedPointer<QHexDocument>(p));
    m_hex->setLockedFile(readonly);
    m_hex->setKeepSize(true);

    m_docType = DocumentType::File;
    auto fName = info.absoluteFilePath();
    m_isNewFile = false;
    m_workSpaceName.clear();
    p->setDocSaved();
    connect(p, &QHexDocument::documentSaved, this,
            &EditorView::updateDocSavedFlag);

    auto tab = this->tabWidget();
    tab->setToolTip(fName);
    setFileNameUrl(QUrl::fromLocalFile(fName));
    if (generateCache) {
        generateIconBaseCache(Utilities::getIconFromFile(style(), fName));
        updateIcon();
    }
    return ErrFile::Success;
}

ErrFile EditorView::openExtFile(const QUrl &fileName) {
    auto ext = fileName.authority();
    auto file = fileName.path();
    if (file.front() == '/') {
        file.removeFirst();
    }
    return openExtFile(ext, file);
}

ErrFile EditorView::openExtFile(const QString &ext, const QString &file,
                                bool generateCache) {
    auto dev = PluginSystem::instance().ext2Device(ext);
    if (dev == nullptr) {
        return ErrFile::NotExist;
    }

    auto d = dev->onOpenFile(file);
    if (d == nullptr) {
        return ErrFile::NotExist;
    }

    if (d->isSequential()) {
        d->deleteLater();
        return ErrFile::InvalidFormat;
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
            d->deleteLater();
            return ErrFile::Permission;
        }
    }

    auto *p = QHexDocument::fromDevice<QFileBuffer>(d, readonly);

    if (Q_UNLIKELY(p == nullptr)) {
        d->deleteLater();
        return ErrFile::Error;
    }

    m_hex->setDocument(QSharedPointer<QHexDocument>(p));
    m_hex->setLockedFile(readonly);
    m_hex->setKeepSize(true);
    if (d->keepSize()) {
        m_hex->setLockKeepSize(true);
    }

    auto fileName = Utilities::getDeviceFileName(ext, file);

    // store the additional info to reload
    _dev = d;

    m_docType = DocumentType::Extension;
    m_isNewFile = false;
    m_workSpaceName.clear();
    p->setDocSaved();
    connect(p, &QHexDocument::documentSaved, this,
            &EditorView::updateDocSavedFlag);

    auto tab = this->tabWidget();
    tab->setToolTip(fileName);
    setFileNameUrl(fileName);
    if (generateCache) {
        generateIconBaseCache(dev->supportedFileIcon());
        updateIcon();
    }
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

    QUrl file;
    QMap<qsizetype, QString> bookmarks;
    QVector<QHexMetadataItem> metas;
    WorkSpaceInfo infos;

    if (WorkSpaceManager::loadWorkSpace(filename, file, bookmarks, metas,
                                        infos)) {
        // it's a workspace project
        // we should check the type of "file"
        ErrFile ret;

        if (file.isLocalFile()) {
            // regular file
            ret = openFile(file.toLocalFile(), false);
            m_docType = DocumentType::File;
        } else {
            // extension file
            auto scheme = file.scheme();
            if (scheme.compare(QStringLiteral("wdrv"), Qt::CaseInsensitive)) {
                return WingHex::InvalidFormat;
            }

            auto ext = file.authority();
            auto f = file.path();
            if (f.front() == '/') {
                f.removeFirst();
            }
            ret = openExtFile(ext, f, false);
            m_docType = DocumentType::Extension;
        }

        if (ret != ErrFile::Success) {
            m_docType = DocumentType::InValid;
            return ret;
        }

        // apply the content
        auto doc = m_hex->document();
        doc->applyBookMarks(bookmarks);
        doc->setBaseAddress(infos.base);
        doc->metadata()->applyMetas(metas);
        _pluginData = infos.pluginData;
        doc->setDocSaved();

        // Don't use filename because it's pointer of origin workSpace
        // It will be empty after openFile or openExtFile
        m_workSpaceName = finfo.absoluteFilePath();

        applyWorkSpaceStyle(this);
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

    bool saveSelf = path.isEmpty();
    auto saveName = QUrl::fromLocalFile(path);

    if (saveSelf) {
        if (isExport) {
            return ErrFile::NotExist;
        }
        if (isNewFile()) {
            return ErrFile::IsNewFile;
        }
    } else {
        if (isExport) {
            if (this->fileNameUrl() == saveName) {
                return ErrFile::Permission;
            }
        }
    }

    auto fileName = saveSelf ? this->fileNameUrl() : saveName;
    auto doc = m_hex->document();
    auto orginWorkSpace = this->isWorkSpace();

    bool needSaveWS = false;
    if (workSpaceAttr == SaveWorkSpaceAttr::AutoWorkSpace &&
        (doc->isMetaDataUnsaved() || checkHasUnsavedState())) {
        needSaveWS = true;
    } else {
        if (workSpaceAttr == SaveWorkSpaceAttr::ForceWorkSpace) {
            needSaveWS = true;
        }
    }

    if (needSaveWS) {
        WorkSpaceInfo infos;
        infos.base = doc->baseAddress();
        savePluginData();
        infos.pluginData = _pluginData;

        auto b = WorkSpaceManager::saveWorkSpace(
            workSpaceName, fileName.url(), doc->bookMarks(),
            doc->metadata()->getAllMetadata(), infos);
        if (!b) {
            return ErrFile::WorkSpaceUnSaved;
        }
        this->m_workSpaceName = workSpaceName;

        for (auto &item : m_others) {
            if (item->hasUnsavedState()) {
                item->setSaved();
            }
        }
    }

    bool needSave = false;
    if (!doc->isDocSaved() || isNewFile()) {
        needSave = true;
    } else {
        if (!saveSelf) {
            needSave = true;
        }
    }

    if (needSave) {
        if (saveSelf) {
            // when path is empty, only save self
            if (m_docType == DocumentType::Extension) {
                if (doc->saveTo(nullptr, true)) {
                    doc->setDocSaved();
                    if (!orginWorkSpace && !workSpaceName.isEmpty()) {
                        applyWorkSpaceStyle(this);
                        notifyOnWorkSpace(true);
                    }
                    return ErrFile::Success;
                }
            } else {
                if (doc->saveTo(nullptr, true)) {
                    m_isNewFile = false;
                    m_docType = DocumentType::File;
                    doc->setDocSaved();
                    if (!orginWorkSpace && !workSpaceName.isEmpty()) {
                        applyWorkSpaceStyle(this);
                        notifyOnWorkSpace(true);
                    }
                    return ErrFile::Success;
                }
            }
        } else {
            auto file = new QFile(path);
            bool flag = !isExport;
            if (m_docType == DocumentType::Extension) {
                if (doc->saveTo(file, flag)) {
                    if (flag) {
                        m_isNewFile = false;
                        m_docType = DocumentType::File;

                        auto buffer = new QFileBuffer;
                        if (!buffer->open(file, false)) {
                            delete file;
                            delete buffer;
                            return ErrFile::Permission;
                        }
                        doc->setBuffer(buffer);
                        doc->setDocSaved();

                        auto fName = QFileInfo(path).absoluteFilePath();
                        if (workSpaceName.isEmpty()) {
                            clearWorkSpaceStyle(this);
                            generateIconBaseCache(
                                Utilities::getIconFromFile(style(), fName));
                            updateIcon();
                        } else {
                            applyWorkSpaceStyle(this);
                        }
                        setFileNameUrl(QUrl::fromLocalFile(fName));
                    } else {
                        delete file;
                    }

                    if (!orginWorkSpace && !workSpaceName.isEmpty()) {
                        notifyOnWorkSpace(true);
                    }
                    return ErrFile::Success;
                }
            } else {
                if (doc->saveTo(file, flag)) {
                    if (flag) {
                        auto buffer = new QFileBuffer;
                        if (!buffer->open(file, false)) {
                            delete file;
                            delete buffer;
                            return ErrFile::Permission;
                        }
                        doc->setBuffer(buffer);

                        m_isNewFile = false;
                        m_docType = DocumentType::File;
                        doc->setDocSaved();
                        auto fName = QFileInfo(path).absoluteFilePath();
                        if (workSpaceName.isEmpty()) {
                            clearWorkSpaceStyle(this);
                            generateIconBaseCache(
                                Utilities::getIconFromFile(style(), fName));
                            updateIcon();
                        } else {
                            applyWorkSpaceStyle(this);
                        }
                        setFileNameUrl(QUrl::fromLocalFile(fName));
                    } else {
                        delete file;
                    }

                    if (!orginWorkSpace && !workSpaceName.isEmpty()) {
                        notifyOnWorkSpace(true);
                    }
                    return ErrFile::Success;
                }
            }
        }
        return ErrFile::Permission;
    }

    if (!orginWorkSpace && !workSpaceName.isEmpty()) {
        applyWorkSpaceStyle(this);
        notifyOnWorkSpace(true);
    }
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
        if (m_workSpaceName.isEmpty()) {
            auto fileName = fileNameUrl();
            return openFile(fileName.toLocalFile());
        } else {
            auto fileName = m_workSpaceName;
            return openWorkSpace(fileName);
        }
    case DocumentType::Extension:
        return openExtFile(fileNameUrl());
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

    if (!m_workSpaceName.isEmpty()) {
        // check whether having plugin metadata
        if (checkHasUnsavedState()) {
            return ErrFile::WorkSpaceUnSaved;
        }
    }

    if (m_docType == DocumentType::Extension) {
        auto dev = PluginSystem::instance().ext2Device(m_fileName.authority());
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

    m_hex->resetDocument();
    return ErrFile::Success;
}

bool EditorView::change2WorkSpace() const {
    return m_workSpaceName.isEmpty() && m_hex->document()->isMetaDataUnsaved();
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

void EditorView::raiseAndSwitchView(const QString &id) {
    raise();
    switchView(id);
}

void EditorView::updateDocSavedFlag(bool b) {
    QString fileName;
    auto fName = this->fileNameUrl();
    if (isNewFile()) {
        fileName = fName.fileName();
    } else if (isExtensionFile()) {
        fileName = fName.authority() + fName.path();
    } else {
        fileName = fName.fileName();
    }
    QString content;

    if (b && !checkHasUnsavedState()) {
        content = fileName;
    } else {
        content = QStringLiteral("* ") + fileName;
    }

    setWindowTitle(content);
    for (int i = 0; i < m_cloneChildren.size(); ++i) {
        auto c = m_cloneChildren.at(i);
        if (c) {
            c->setWindowTitle(content + QStringLiteral(" : ") +
                              QString::number(i + 1));
        }
    }
}

void EditorView::generateIconBaseCache(const QIcon &baseIcon) {
    m_iconCaches.insert(m_hexContainer, baseIcon);
    if (baseIcon.isNull()) {
        return;
    }

    for (auto &w : m_others) {
        if (w) {
            m_iconCaches.insert(w, QIcon(new CompositeIconEngine(
                                       m_iconOrigin.value(w), baseIcon)));
        } else {
            m_iconCaches.insert(w, {});
        }
    }
}

void EditorView::updateIcon() {
    auto cur = m_stack->currentWidget();
    this->setIcon(m_iconCaches.value(cur, m_iconCaches.value(m_hexContainer)));
}

void EditorView::saveState(QXmlStreamWriter &Stream) const {
    Q_UNUSED(Stream);
    // do nothing
}

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

void EditorView::setFileNameUrl(const QUrl &fileName) {
    if (isCloneFile()) {
        this->cloneParent()->setFileNameUrl(fileName);
    }
    if (fileName.isValid()) {
        m_fileName = fileName;
        updateDocSavedFlag(true);
    }
}

void EditorView::savePluginData() {
    for (auto p = m_others.constKeyValueBegin();
         p != m_others.constKeyValueEnd(); ++p) {
        if (p->second->hasUnsavedState()) {
            auto data = p->second->saveState();
            _pluginData.insert(p->first, data);
        }
    }
}

bool EditorView::checkHasUnsavedState() const {
    for (auto &item : m_others) {
        if (item->hasUnsavedState()) {
            return true;
        }
    }
    return false;
}

void EditorView::switchViewStackLoop(bool next) {
    auto curIdx = m_stack->currentIndex();
    auto delta = next ? 1 : -1;
    auto total = m_stack->count();
    curIdx += delta;
    if (curIdx < 0) {
        curIdx = total - 1;
    }
    if (curIdx >= total) {
        curIdx = 0;
    }
    m_stack->setCurrentIndex(curIdx);
}

FindResultModel::FindInfo EditorView::readContextFinding(qsizetype offset,
                                                         qsizetype findSize) {
    constexpr long DISPLAY_SIZE = 16;
    constexpr long FIND_CONTENXT_LEN = 10;
    constexpr long HT_SIZE = (DISPLAY_SIZE - FIND_CONTENXT_LEN) / 2;

    auto doc = m_hex->document();
    if (findSize <= FIND_CONTENXT_LEN) {
        long leftsize = FIND_CONTENXT_LEN - findSize;
        auto rs = std::div(leftsize, 2l);

        auto headerlen = HT_SIZE + rs.quot + rs.rem;
        auto taillen = HT_SIZE + rs.quot;

        auto begin = qMax(offset - headerlen, 0);
        auto end = qMin(offset + findSize + taillen, doc->length());
        auto boff = offset - begin;

        auto buffer = doc->read(begin, end - begin + 1);

        FindResultModel::FindInfo info;
        info.cheader = buffer.left(boff);
        info.hbuffer = buffer.sliced(boff, findSize);
        info.tbuffer = {};
        info.ctailer = buffer.sliced(boff + findSize);
        return info;
    } else {
        constexpr long FIND_CONTENXT_HALF = FIND_CONTENXT_LEN / 2;

        auto hbegin = qMax(offset - HT_SIZE, 0);
        auto hlen = offset - hbegin + FIND_CONTENXT_HALF;
        auto header = doc->read(hbegin, hlen);
        auto toff = offset - hbegin;

        auto tbegin = offset + findSize - FIND_CONTENXT_HALF;
        auto tlen = HT_SIZE + FIND_CONTENXT_HALF;
        auto tailer = doc->read(tbegin, tlen);

        FindResultModel::FindInfo info;
        info.cheader = header.left(toff);
        info.hbuffer = header.sliced(toff);
        info.tbuffer = tailer.left(FIND_CONTENXT_HALF);
        info.ctailer = tailer.sliced(FIND_CONTENXT_HALF);
        return info;
    }
}

void EditorView::applyFunctionTables(WingEditorViewWidget *view,
                                     const CallTable &fns) {
    view->setProperty("__CALL_TABLE__", QVariant::fromValue(fns));
    view->setProperty("__CALL_POINTER__", quintptr(this));
}

bool EditorView::checkErrAndReport(const QObject *sender, const char *func) {
    for (auto p = m_others.constBegin(); p != m_others.constEnd(); ++p) {
        if (*p == sender) {
            return false;
        }
    }
    qCritical("[EvilCall] Invalid sender called '%s'", func);
    return true;
}

bool EditorView::checkThreadAff() {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(tr("Not allowed operation in non-UI thread"));
        return false;
    }
    return true;
}

void EditorView::applyWorkSpaceStyle(EditorView *view) {
    view->generateIconBaseCache(ICONRES(QStringLiteral("pro")));
    view->updateIcon();
    auto tab = view->tabWidget();
    tab->setStyleSheet(QStringLiteral("QLabel {text-decoration: underline;}"));

    if (!view->isCloneFile()) {
        for (auto &c : view->m_cloneChildren) {
            if (c) {
                applyWorkSpaceStyle(c);
            }
        }
    }
}

void EditorView::clearWorkSpaceStyle(EditorView *view) {
    auto tab = view->tabWidget();
    tab->setStyleSheet({});

    if (!view->isCloneFile()) {
        for (auto &c : view->m_cloneChildren) {
            if (c) {
                clearWorkSpaceStyle(c);
            }
        }
    }
}

bool EditorView::existsServiceHost(const QObject *caller, const QString &puid) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }

    return PluginSystem::instance().existsServiceHost(caller, puid);
}

bool EditorView::invokeServiceImpl(const QObject *sender, const QString &puid,
                                   const MetaCallInfo &infos) {
    if (checkErrAndReport(sender, __func__)) {
        return false;
    }

    return PluginSystem::instance().invokeServiceImpl(sender, puid, infos);
}

QUrl EditorView::currentDocFile(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    return fileNameUrl();
}

QString EditorView::currentDocFileName(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    auto file = fileNameUrl();
    return file.url();
}

QUrl EditorView::currentDocWorkSpace(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    if (isWorkSpace()) {
        return QUrl::fromLocalFile(m_workSpaceName);
    } else {
        return {};
    }
}

QString EditorView::currentDocWorkSpaceName(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    if (isWorkSpace()) {
        return m_workSpaceName;
    } else {
        return {};
    }
}

bool EditorView::isReadOnly(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return true;
        }
    }

    return m_hex->isReadOnly();
}

bool EditorView::isInsertionMode(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return m_hex->cursor()->insertionMode() ==
           QHexCursor::InsertionMode::InsertMode;
}

bool EditorView::isKeepSize(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    return m_hex->isKeepSize();
}

bool EditorView::isLocked(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return true;
        }
    }

    return m_hex->isLocked();
}

qsizetype EditorView::documentLines(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->documentLines();
}

qsizetype EditorView::documentBytes(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->documentBytes();
}

qsizetype EditorView::currentRow(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->currentRow();
}

qsizetype EditorView::currentColumn(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->currentColumn();
}

qsizetype EditorView::currentOffset(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->currentOffset();
}

qsizetype EditorView::selectedLength(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->cursor()->currentSelectionLength();
}

QByteArray EditorView::selectedBytes(const QObject *caller, qsizetype index) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    return m_hex->selectedBytes(index);
}

QByteArrayList EditorView::selectionBytes(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    return m_hex->selectedBytes();
}

qsizetype EditorView::selectionLength(const QObject *caller, qsizetype index) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    auto cursor = m_hex->cursor();
    if (index >= 0 && index < cursor->selectionCount()) {
        return cursor->selectionLength(index);
    }
    return -1;
}

qsizetype EditorView::selectionCount(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->selectionCount();
}

bool EditorView::stringVisible(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return m_hex->asciiVisible();
}

bool EditorView::addressVisible(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return m_hex->addressVisible();
}

bool EditorView::headerVisible(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return m_hex->headerVisible();
}

quintptr EditorView::addressBase(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return m_hex->addressBase();
}

bool EditorView::isModified(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return !m_hex->isSaved();
}

qint8 EditorView::readInt8(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<qint8>(this, offset, _rwlock);
}

qint16 EditorView::readInt16(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<qint16>(this, offset, _rwlock);
}

qint32 EditorView::readInt32(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<qint32>(this, offset, _rwlock);
}

qint64 EditorView::readInt64(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<qint64>(this, offset, _rwlock);
}

quint8 EditorView::readUInt8(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<quint8>(this, offset, _rwlock);
}

quint16 EditorView::readUInt16(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<quint16>(this, offset, _rwlock);
}

quint32 EditorView::readUInt32(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<quint32>(this, offset, _rwlock);
}

quint64 EditorView::readUInt64(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return 0;
        }
    }

    return readBasicTypeContent<quint64>(this, offset, _rwlock);
}

float EditorView::readFloat(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return qQNaN();
        }
    }

    return readBasicTypeContent<float>(this, offset, _rwlock);
}

double EditorView::readDouble(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return qQNaN();
        }
    }

    return readBasicTypeContent<double>(this, offset, _rwlock);
}

QString EditorView::readString(const QObject *caller, qsizetype offset,
                               const QString &encoding) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    QReadLocker locker(&_rwlock);
    auto doc = m_hex->document();
    auto pos = doc->findNext(offset, QByteArray(1, 0));
    if (pos < 0) {
        pos = doc->findNext(offset, QByteArray(1, '\n'));
        if (pos < 0) {
            return {};
        }
    }
    auto buffer = doc->read(offset, int(pos - offset));
    return Utilities::decodingString(buffer, encoding);
}

QByteArray EditorView::readBytes(const QObject *caller, qsizetype offset,
                                 qsizetype count) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    QReadLocker locker(&_rwlock);
    return m_hex->document()->read(offset, count);
}

qsizetype EditorView::findNext(const QObject *caller, qsizetype begin,
                               const QByteArray &ba) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->document()->findNext(begin, ba);
}

qsizetype EditorView::findPrevious(const QObject *caller, qsizetype begin,
                                   const QByteArray &ba) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return -1;
        }
    }

    return m_hex->document()->findPrevious(begin, ba);
}

QString EditorView::bookMarkComment(const QObject *caller, qsizetype pos) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    return m_hex->document()->bookMark(pos);
}

bool EditorView::existBookMark(const QObject *caller, qsizetype pos) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return m_hex->document()->existBookMark(pos);
}

bool EditorView::setLockedFile(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return m_hex->setLockedFile(b);
}

bool EditorView::setKeepSize(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return m_hex->setKeepSize(b);
}

bool EditorView::setStringVisible(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->setAsciiVisible(b);
    return true;
}

bool EditorView::setAddressVisible(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->setAddressVisible(b);
    return true;
}

bool EditorView::setHeaderVisible(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->setHeaderVisible(b);
    return true;
}

bool EditorView::setAddressBase(const QObject *caller, quintptr base) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->setAddressBase(base);
    return true;
}

bool EditorView::beginMarco(const QObject *caller, const QString &txt) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->document()->beginMarco(txt);
    return true;
}

bool EditorView::endMarco(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->document()->endMarco();
    return true;
}

bool EditorView::writeInt8(const QObject *caller, qsizetype offset,
                           qint8 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeInt16(const QObject *caller, qsizetype offset,
                            qint16 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeInt32(const QObject *caller, qsizetype offset,
                            qint32 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeInt64(const QObject *caller, qsizetype offset,
                            qint64 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt8(const QObject *caller, qsizetype offset,
                            quint8 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt16(const QObject *caller, qsizetype offset,
                             quint16 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt32(const QObject *caller, qsizetype offset,
                             quint32 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt64(const QObject *caller, qsizetype offset,
                             quint64 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeFloat(const QObject *caller, qsizetype offset,
                            float value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeDouble(const QObject *caller, qsizetype offset,
                             double value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeString(const QObject *caller, qsizetype offset,
                             const QString &value, const QString &encoding) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto unicode = Utilities::encodingString(value, encoding);
    auto cmd = doc->MakeReplace(nullptr, m_hex->cursor(), offset, unicode);
    if (cmd) {
        QWriteLocker l(&_rwlock);
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::writeBytes(const QObject *caller, qsizetype offset,
                            const QByteArray &data) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeReplace(nullptr, m_hex->cursor(), offset, data);
    if (cmd) {
        QWriteLocker l(&_rwlock);
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::insertInt8(const QObject *caller, qsizetype offset,
                            qint8 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertInt16(const QObject *caller, qsizetype offset,
                             qint16 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertInt32(const QObject *caller, qsizetype offset,
                             qint32 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertInt64(const QObject *caller, qsizetype offset,
                             qint64 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt8(const QObject *caller, qsizetype offset,
                             quint8 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt16(const QObject *caller, qsizetype offset,
                              quint16 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt32(const QObject *caller, qsizetype offset,
                              quint32 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt64(const QObject *caller, qsizetype offset,
                              quint64 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertFloat(const QObject *caller, qsizetype offset,
                             float value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertDouble(const QObject *caller, qsizetype offset,
                              double value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertString(const QObject *caller, qsizetype offset,
                              const QString &value, const QString &encoding) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto unicode = Utilities::encodingString(value, encoding);
    auto cmd = doc->MakeInsert(nullptr, m_hex->cursor(), offset, unicode);
    if (cmd) {
        QWriteLocker l(&_rwlock);
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::insertBytes(const QObject *caller, qsizetype offset,
                             const QByteArray &data) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeInsert(nullptr, m_hex->cursor(), offset, data);
    if (cmd) {
        QWriteLocker l(&_rwlock);
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::appendInt8(const QObject *caller, qint8 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendInt16(const QObject *caller, qint16 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendInt32(const QObject *caller, qint32 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendInt64(const QObject *caller, qint64 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt8(const QObject *caller, quint8 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt16(const QObject *caller, quint16 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt32(const QObject *caller, quint32 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt64(const QObject *caller, quint64 value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendFloat(const QObject *caller, float value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendDouble(const QObject *caller, double value) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendString(const QObject *caller, const QString &value,
                              const QString &encoding) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto unicode = Utilities::encodingString(value, encoding);
    auto cmd = doc->MakeAppend(nullptr, m_hex->cursor(), unicode);
    if (cmd) {
        QWriteLocker l(&_rwlock);
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::appendBytes(const QObject *caller, const QByteArray &data) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeAppend(nullptr, m_hex->cursor(), data);
    if (cmd) {
        QWriteLocker l(&_rwlock);
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::removeBytes(const QObject *caller, qsizetype offset,
                             qsizetype len) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeRemove(nullptr, m_hex->cursor(), offset, len);
    if (cmd) {
        QWriteLocker l(&_rwlock);
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::moveTo(const QObject *caller, qsizetype line, qsizetype column,
                        int nibbleindex, bool clearSelection) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->cursor()->moveTo(line, column, nibbleindex, clearSelection);
    return true;
}

bool EditorView::moveTo(const QObject *caller, qsizetype offset,
                        bool clearSelection) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->cursor()->moveTo(offset, clearSelection);
    return true;
}

bool EditorView::select(const QObject *caller, qsizetype offset,
                        qsizetype length, SelectionMode mode) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto cursor = m_hex->cursor();
    cursor->moveTo(offset);
    QHexCursor::SelectionMode smode;
    switch (mode) {
    case WingHex::SelectionMode::Add:
        smode = QHexCursor::SelectionAdd;
        break;
    case WingHex::SelectionMode::Remove:
        smode = QHexCursor::SelectionRemove;
        break;
    case WingHex::SelectionMode::Single:
        smode = QHexCursor::SelectionNormal;
        break;
    }
    cursor->select(length, smode);
    return true;
}

bool EditorView::setInsertionMode(const QObject *caller, bool isinsert) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    m_hex->cursor()->setInsertionMode(isinsert ? QHexCursor::InsertMode
                                               : QHexCursor::OverwriteMode);
    return true;
}

bool EditorView::metadata(const QObject *caller, qsizetype begin,
                          qsizetype length, const QColor &fgcolor,
                          const QColor &bgcolor, const QString &comment) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->metadata()->MakeMetadata(nullptr, begin, begin + length - 1,
                                             fgcolor, bgcolor, comment);
    if (cmd) {
        doc->pushMakeUndo(cmd);
        return true;
    }

    return false;
}

bool EditorView::removeMetadata(const QObject *caller, qsizetype offset) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->metadata()->MakeRemoveMetadata(nullptr, offset);
    if (cmd) {
        doc->pushMakeUndo(cmd);
        return true;
    }

    return false;
}

bool EditorView::clearMetadata(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->metadata()->MakeClear(nullptr);
    if (cmd) {
        doc->pushMakeUndo(cmd);
        return true;
    }

    return false;
}

bool EditorView::setMetaVisible(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    doc->setMetafgVisible(b);
    doc->setMetabgVisible(b);
    doc->setMetaCommentVisible(b);
    return true;
}

bool EditorView::setMetafgVisible(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    doc->setMetafgVisible(b);
    return true;
}

bool EditorView::setMetabgVisible(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    doc->setMetabgVisible(b);
    return true;
}

bool EditorView::setMetaCommentVisible(const QObject *caller, bool b) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    doc->setMetaCommentVisible(b);
    return true;
}

bool EditorView::addBookMark(const QObject *caller, qsizetype pos,
                             const QString &comment) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeAddBookMark(nullptr, pos, comment);
    if (cmd) {
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::modBookMark(const QObject *caller, qsizetype pos,
                             const QString &comment) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeModBookMark(nullptr, pos, comment);
    if (cmd) {
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::removeBookMark(const QObject *caller, qsizetype pos) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeRemoveBookMark(nullptr, pos);
    if (cmd) {
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

bool EditorView::clearBookMark(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (!checkThreadAff()) {
            return false;
        }
        if (checkErrAndReport(caller, __func__)) {
            return false;
        }
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeClearBookMark(nullptr);
    if (cmd) {
        doc->pushMakeUndo(cmd);
        return true;
    }
    return false;
}

void EditorView::toast(const QObject *caller, const QPixmap &icon,
                       const QString &message) {
    PluginSystem::instance().toast(caller, icon, message);
}

void EditorView::logTrace(const QObject *caller, const QString &message) {
    PluginSystem::instance().logTrace(caller, message);
}

void EditorView::logDebug(const QObject *caller, const QString &message) {
    PluginSystem::instance().logDebug(caller, message);
}

HexPosition EditorView::selectionEnd(const QObject *caller, qsizetype index) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    HexPosition pos;
    auto cursor = m_hex->cursor();
    if (index >= 0 && index < cursor->selectionCount()) {
        auto qpos = cursor->selectionEnd(index);
        pos.line = qpos.line;
        pos.column = qpos.column;
        pos.lineWidth = qpos.lineWidth;
        pos.nibbleindex = qpos.nibbleindex;
    }
    return pos;
}

HexPosition EditorView::selectionStart(const QObject *caller, qsizetype index) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    HexPosition pos;
    auto cursor = m_hex->cursor();
    if (index >= 0 && index < cursor->selectionCount()) {
        auto qpos = cursor->selectionStart(index);
        pos.line = qpos.line;
        pos.column = qpos.column;
        pos.lineWidth = qpos.lineWidth;
        pos.nibbleindex = qpos.nibbleindex;
    }
    return pos;
}

HexPosition EditorView::currentPos(const QObject *caller) {
    auto &plgsys = PluginSystem::instance();
    if (caller != &plgsys) {
        if (checkErrAndReport(caller, __func__)) {
            return {};
        }
    }

    HexPosition pos;
    auto cursor = m_hex->cursor();
    pos.line = cursor->currentLine();
    pos.column = cursor->currentColumn();
    pos.lineWidth = m_hex->document()->hexLineWidth();
    pos.nibbleindex = cursor->currentNibble();
    return pos;
}

void EditorView::logWarn(const QObject *caller, const QString &message) {
    PluginSystem::instance().logWarn(caller, message);
}

void EditorView::logError(const QObject *caller, const QString &message) {
    PluginSystem::instance().logError(caller, message);
}

void EditorView::logInfo(const QObject *caller, const QString &message) {
    PluginSystem::instance().logInfo(caller, message);
}

bool EditorView::raiseDockWidget(const QObject *caller, QWidget *w) {
    return PluginSystem::instance().raiseDockWidget(caller, w);
}

QDialog *EditorView::createDialog(const QObject *caller, QWidget *content) {
    return PluginSystem::instance().createDialog(caller, content);
}

void EditorView::msgAboutQt(const QObject *caller, QWidget *parent,
                            const QString &title) {
    return PluginSystem::instance().msgAboutQt(caller, parent, title);
}

QMessageBox::StandardButton
EditorView::msgInformation(const QObject *caller, QWidget *parent,
                           const QString &title, const QString &text,
                           QMessageBox::StandardButtons buttons,
                           QMessageBox::StandardButton defaultButton) {
    return PluginSystem::instance().msgInformation(caller, parent, title, text,
                                                   buttons, defaultButton);
}

QMessageBox::StandardButton
EditorView::msgQuestion(const QObject *caller, QWidget *parent,
                        const QString &title, const QString &text,
                        QMessageBox::StandardButtons buttons,
                        QMessageBox::StandardButton defaultButton) {
    return PluginSystem::instance().msgQuestion(caller, parent, title, text,
                                                buttons, defaultButton);
}

QMessageBox::StandardButton
EditorView::msgWarning(const QObject *caller, QWidget *parent,
                       const QString &title, const QString &text,
                       QMessageBox::StandardButtons buttons,
                       QMessageBox::StandardButton defaultButton) {
    return PluginSystem::instance().msgWarning(caller, parent, title, text,
                                               buttons, defaultButton);
}

QMessageBox::StandardButton
EditorView::msgCritical(const QObject *caller, QWidget *parent,
                        const QString &title, const QString &text,
                        QMessageBox::StandardButtons buttons,
                        QMessageBox::StandardButton defaultButton) {
    return PluginSystem::instance().msgCritical(caller, parent, title, text,
                                                buttons, defaultButton);
}

void EditorView::msgAbout(const QObject *caller, QWidget *parent,
                          const QString &title, const QString &text) {
    return PluginSystem::instance().msgAbout(caller, parent, title, text);
}

QMessageBox::StandardButton
EditorView::msgbox(const QObject *caller, QWidget *parent,
                   QMessageBox::Icon icon, const QString &title,
                   const QString &text, QMessageBox::StandardButtons buttons,
                   QMessageBox::StandardButton defaultButton) {
    return PluginSystem::instance().msgbox(caller, parent, icon, title, text,
                                           buttons, defaultButton);
}

QString EditorView::dlgGetText(const QObject *caller, QWidget *parent,
                               const QString &title, const QString &label,
                               QLineEdit::EchoMode echo, const QString &text,
                               bool *ok,
                               Qt::InputMethodHints inputMethodHints) {
    return PluginSystem::instance().dlgGetText(
        caller, parent, title, label, echo, text, ok, inputMethodHints);
}

QString EditorView::dlgGetMultiLineText(const QObject *caller, QWidget *parent,
                                        const QString &title,
                                        const QString &label,
                                        const QString &text, bool *ok,
                                        Qt::InputMethodHints inputMethodHints) {
    return PluginSystem::instance().dlgGetMultiLineText(
        caller, parent, title, label, text, ok, inputMethodHints);
}

QString EditorView::dlgGetItem(const QObject *caller, QWidget *parent,
                               const QString &title, const QString &label,
                               const QStringList &items, int current,
                               bool editable, bool *ok,
                               Qt::InputMethodHints inputMethodHints) {
    return PluginSystem::instance().dlgGetItem(caller, parent, title, label,
                                               items, current, editable, ok,
                                               inputMethodHints);
}

int EditorView::dlgGetInt(const QObject *caller, QWidget *parent,
                          const QString &title, const QString &label, int value,
                          int minValue, int maxValue, int step, bool *ok) {
    return PluginSystem::instance().dlgGetInt(
        caller, parent, title, label, value, minValue, maxValue, step, ok);
}

double EditorView::dlgGetDouble(const QObject *caller, QWidget *parent,
                                const QString &title, const QString &label,
                                double value, double minValue, double maxValue,
                                int decimals, bool *ok, double step) {
    return PluginSystem::instance().dlgGetDouble(caller, parent, title, label,
                                                 value, minValue, maxValue,
                                                 decimals, ok, step);
}

QString EditorView::dlgGetExistingDirectory(const QObject *caller,
                                            QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            QFileDialog::Options options) {
    return PluginSystem::instance().dlgGetExistingDirectory(
        caller, parent, caption, dir, options);
}

QString EditorView::dlgGetOpenFileName(const QObject *caller, QWidget *parent,
                                       const QString &caption,
                                       const QString &dir,
                                       const QString &filter,
                                       QString *selectedFilter,
                                       QFileDialog::Options options) {
    return PluginSystem::instance().dlgGetOpenFileName(
        caller, parent, caption, dir, filter, selectedFilter, options);
}

QStringList
EditorView::dlgGetOpenFileNames(const QObject *caller, QWidget *parent,
                                const QString &caption, const QString &dir,
                                const QString &filter, QString *selectedFilter,
                                QFileDialog::Options options) {
    return PluginSystem::instance().dlgGetOpenFileNames(
        caller, parent, caption, dir, filter, selectedFilter, options);
}

QString EditorView::dlgGetSaveFileName(const QObject *caller, QWidget *parent,
                                       const QString &caption,
                                       const QString &dir,
                                       const QString &filter,
                                       QString *selectedFilter,
                                       QFileDialog::Options options) {
    return PluginSystem::instance().dlgGetSaveFileName(
        caller, parent, caption, dir, filter, selectedFilter, options);
}

QColor EditorView::dlgGetColor(const QObject *caller, const QString &caption,
                               QWidget *parent) {
    return PluginSystem::instance().dlgGetColor(caller, caption, parent);
}

AppTheme EditorView::currentAppTheme(const QObject *caller) {
    return PluginSystem::instance().currentAppTheme(caller);
}

EditorView *EditorView::cloneParent() const { return m_cloneParent; }

EditorView *EditorView::clone() {
    if (isCloneFile()) {
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

    connect(ev, &EditorView::sigOnCutFile, this, &EditorView::sigOnCutFile);
    connect(ev, &EditorView::sigOnCutHex, this, &EditorView::sigOnCutHex);
    connect(ev, &EditorView::sigOnCopyFile, this, &EditorView::sigOnCopyFile);
    connect(ev, &EditorView::sigOnCopyHex, this, &EditorView::sigOnCopyHex);
    connect(ev, &EditorView::sigOnPasteFile, this, &EditorView::sigOnPasteFile);
    connect(ev, &EditorView::sigOnPasteHex, this, &EditorView::sigOnPasteHex);
    connect(ev, &EditorView::sigOnDelete, this, &EditorView::sigOnDelete);
    connect(ev, &EditorView::sigOnFindFile, this, &EditorView::sigOnFindFile);
    connect(ev, &EditorView::sigOnGoToLine, this, &EditorView::sigOnGoToLine);
    connect(ev, &EditorView::sigOnFill, this, &EditorView::sigOnFill);
    connect(ev, &EditorView::sigOnMetadata, this, &EditorView::sigOnMetadata);
    connect(ev, &EditorView::sigOnBookMark, this, &EditorView::sigOnBookMark);
    connect(ev, &EditorView::sigOnCheckSum, this, &EditorView::sigOnCheckSum);

    auto doc = m_hex->document();

    ev->m_cloneParent = this;
    ev->m_hex->setDocument(doc, ev->m_hex->cursor());

    ev->m_hex->setWindowFilePath(m_hex->windowFilePath());
    ev->setWindowTitle(this->windowTitle() + QStringLiteral(" : ") +
                       QString::number(cloneIndex + 1));

    auto tab = ev->tabWidget();
    if (!m_workSpaceName.isEmpty()) {
        tab->setStyleSheet(
            QStringLiteral("QLabel {text-decoration: underline;}"));
    }

    ev->m_docType = DocumentType::Cloned;
    ev->m_iconCaches.insert(ev->m_hexContainer,
                            this->m_iconCaches.value(this->m_hexContainer));
    for (auto p = m_others.constKeyValueBegin();
         p != m_others.constKeyValueEnd(); ++p) {
        auto cw = p->second->clone();
        if (cw) {
            ev->registerView(p->first, cw, this->m_iconOrigin.value(p->second));
        }
    }
    ev->updateIcon();
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

int EditorView::cloneIndex() const {
    if (isCloneFile()) {
        return m_cloneParent->m_cloneChildren.indexOf(this);
    }
    return -1;
}

bool EditorView::isSaved() const {
    if (isCloneFile()) {
        return this->cloneParent()->isSaved();
    }
    if (isWorkSpace()) {
        return m_hex->isSaved() && !m_hex->document()->isMetaDataUnsaved() &&
               !checkHasUnsavedState();
    } else {
        return m_hex->isSaved();
    }
}

FindResultModel::FindData &EditorView::findResult() { return m_findData; }

QMap<QCryptographicHash::Algorithm, QString> &EditorView::checkSumResult() {
    return _checkSumData;
}

void EditorView::setFontSize(qreal size) { m_hex->setFontSize(size); }

bool EditorView::isWorkSpace() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->isWorkSpace();
    }
    return !m_workSpaceName.isEmpty();
}

QUrl EditorView::fileNameUrl() const {
    Q_ASSERT(m_docType != DocumentType::InValid);
    if (isCloneFile()) {
        return this->cloneParent()->fileNameUrl();
    }
    return m_fileName;
}

bool EditorView::eventFilter(QObject *watched, QEvent *event) {
    auto type = event->type();
    if (type == QEvent::DynamicPropertyChange) {
        auto e = static_cast<QDynamicPropertyChangeEvent *>(event);
        if (e->propertyName() == VIEW_PROPERTY ||
            e->propertyName() == VIEW_ID_PROPERTY) {
            std::abort();
        }
    } else if (type == QEvent::Wheel) {
        if (watched == this->tabWidget()) {
            auto e = static_cast<QWheelEvent *>(event);
            if (e->modifiers() == Qt::ControlModifier) {
                auto delta = e->angleDelta();
                if (delta.x() == 0) {
                    auto y = delta.y();
                    if (y != 0) {
                        switchViewStackLoop(y < 0);
                        return true;
                    }
                }
            }
        }
    }
    return ads::CDockWidget::eventFilter(watched, event);
}

QString EditorView::workSpaceName() const { return m_workSpaceName; }

EditorViewContext *EditorView::editorContext() const { return _context; }

bool EditorView::isNewFileUrl(const QUrl &url) {
    if (!url.isLocalFile()) {
        return false;
    }
    return newFileAuthority().compare(url.authority(), Qt::CaseInsensitive) ==
           0;
}

QString EditorView::newFileAuthority() { return QStringLiteral("WingNew"); }
