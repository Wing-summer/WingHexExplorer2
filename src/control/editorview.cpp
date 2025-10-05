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

#include "class/logger.h"
#include "class/pluginsystem.h"
#include "class/qkeysequences.h"
#include "class/settingmanager.h"
#include "class/workspacemanager.h"
#include "utilities.h"

#include <QFile>
#include <QFileInfo>
#include <QVBoxLayout>

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

constexpr auto CLONE_LIMIT = 3;

constexpr auto VIEW_PROPERTY = "__VIEW__";
constexpr auto VIEW_ID_PROPERTY = "__ID__";

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

    connect(m_hex, &QHexView::documentChanged, this,
            &EditorView::documentChanged);

    connect(&_watcher, &QFileSystemWatcher::fileChanged, this,
            &EditorView::need2Reload);

    applySettings();

    // build up call tables
    auto mobj = EditorView::metaObject();
    auto total = mobj->methodCount();
    for (int i = 0; i < total; ++i) {
        auto m = mobj->method(i);
        // all private slots
        if (m.methodType() == QMetaMethod::Slot &&
            m.access() == QMetaMethod::Private) {
            WingHex::FunctionSig msig;
            msig.fnName = m.name();

            auto total = m.parameterCount();
            msig.types.reserve(total);

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
}

EditorView::~EditorView() {}

void EditorView::registerView(const QString &id, WingEditorViewWidget *view) {
    Q_ASSERT(view);
    m_others.insert(id, view);
    m_stack->addWidget(view);
    view->setProperty(VIEW_PROPERTY, quintptr(this));
    view->setProperty(VIEW_ID_PROPERTY, id);
    view->installEventFilter(this);
    applyFunctionTables(view, _viewFns);
    if (!isCloneFile()) {
        if (_pluginData.contains(id)) {
            view->loadState(_pluginData.value(id));
        }
        view->onWorkSpaceNotify(m_isWorkSpace);
    }
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

void EditorView::getCheckSum(const QVector<int> &algorithmID) {
    auto hashes = Utilities::supportedHashAlgorithms();

    if (m_hex->hasSelection()) {
        auto data = m_hex->selectedBytes();
        for (auto &c : algorithmID) {
            auto h = hashes.at(c);
            QCryptographicHash hash(h);
            hash.addData(data.join());
            _checkSumData.insert(h, hash.result().toHex().toUpper());
        }
    } else {
        if (isNewFile()) {
            auto bytes = m_hex->document()->read(0);
            for (auto &c : algorithmID) {
                auto h = hashes.at(c);
                QCryptographicHash hash(h);
                hash.addData(bytes);
                _checkSumData.insert(h, hash.result().toHex().toUpper());
            }
        } else {
            QFile f(fileName());
            for (auto &c : algorithmID) {
                auto h = hashes.at(c);
                QCryptographicHash hash(h);
                hash.addData(&f);
                _checkSumData.insert(h, hash.result().toHex().toUpper());
            }
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

void EditorView::clearFindResult() { m_findData.clear(); }

void EditorView::triggerGoto() {
    m_goto->activeInput(int(m_hex->currentRow()), int(m_hex->currentColumn()),
                        m_hex->currentOffset(), m_hex->documentBytes(),
                        int(m_hex->documentLines()));
}

ErrFile EditorView::newFile(size_t index) {
    if (isCloneFile()) {
        return ErrFile::ClonedFile;
    }

    removeMonitorPaths();
    auto istr = QString::number(index);
    auto fname = tr("Untitled") + istr;
    m_hex->setWindowFilePath(fname);
    this->setWindowTitle(fname);
    m_docType = DocumentType::File;
    m_isWorkSpace = false;
    m_isNewFile = true;
    auto p = QHexDocument::fromMemory<QFileBuffer>(QByteArray(), false);
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

        auto *p = QHexDocument::fromFile<QFileBuffer>(filename, readonly);
        if (Q_UNLIKELY(p == nullptr)) {
            return ErrFile::Permission;
        }

        removeMonitorPaths();

        m_hex->setDocument(QSharedPointer<QHexDocument>(p));
        m_hex->setLockedFile(readonly);
        m_hex->setKeepSize(true);

        m_docType = DocumentType::File;
        auto fName = info.absoluteFilePath();
        m_hex->setWindowFilePath(fName);
        m_isNewFile = false;
        p->setDocSaved();

        this->setWindowTitle(info.fileName());
        connectDocSavedFlag(this);

        auto tab = this->tabWidget();
        tab->setIcon(Utilities::getIconFromFile(style(), fName));
        tab->setToolTip(fName);

        addMonitorPath();
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

    auto *p = QHexDocument::fromDevice<QFileBuffer>(d, readonly);

    if (Q_UNLIKELY(p == nullptr)) {
        return ErrFile::Error;
    }

    removeMonitorPaths();

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
    m_hex->setWindowFilePath(fileName);
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
    auto fileName = path.isEmpty() ? m_hex->windowFilePath() : path;
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
                if (c) {
                    convertTabW(c);
                }
            }
        }
    }

    if (!doc->isDocSaved() || m_hex->windowFilePath() != fileName ||
        isNewFile()) {
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

            removeMonitorPaths();

            if (doc->saveTo(&file, !isExport)) {
                file.close();
                if (!isExport) {
                    m_hex->setWindowFilePath(
                        QFileInfo(fileName).absoluteFilePath());

                    if (isNewFile()) {
                        auto buffer = new QFileBuffer;
                        buffer->read(new QFile(fileName));
                        doc->setBuffer(buffer);
                    }

                    m_isNewFile = false;
                    m_docType = DocumentType::File;
                    doc->setDocSaved();
                }

                addMonitorPath();

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
        return openFile(m_hex->windowFilePath());
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

void EditorView::raiseAndSwitchView(const QString &id) {
    raise();
    switchView(id);
}

void EditorView::connectDocSavedFlag(EditorView *editor) {
    connect(editor->m_hex->document().get(), &QHexDocument::documentSaved, this,
            [=](bool b) {
                QString fileName;
                auto fName = m_hex->windowFilePath();
                if (editor->isNewFile()) {
                    fileName = fName;
                } else if (editor->isExtensionFile()) {
                    auto idx = fName.indexOf('}');
                    fileName = fName.mid(idx);
                } else {
                    fileName = QFileInfo(fName).fileName();
                }
                QString content;

                if (b && !checkHasUnsavedState()) {
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
                            Utilities::getIconFromFile(style(), fName));
                    }
                }
            });
}

void EditorView::removeMonitorPaths() {
    auto files = _watcher.files();
    if (files.isEmpty()) {
        return;
    }
    _watcher.removePaths(files);
}

void EditorView::addMonitorPath() { _watcher.addPath(m_hex->windowFilePath()); }

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
            return true;
        }
    }
    qCritical("[EvilCall] Invalid sender called '%s'", func);
    return false;
}

bool EditorView::checkThreadAff() {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(tr("Not allowed operation in non-UI thread"));
        return false;
    }
    return true;
}

bool EditorView::existsServiceHost(const QObject *caller, const QString &puid) {
    return PluginSystem::instance().existsServiceHost(caller, puid);
}

bool EditorView::invokeServiceImpl(const QObject *sender, const QString &puid,
                                   const MetaCallInfo &infos) {
    if (checkErrAndReport(sender, __func__)) {
        return false;
    }

    return PluginSystem::instance().invokeServiceImpl(this, puid, infos);
}

QString EditorView::currentDocFilename(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return {};
    }
    return fileName();
}

bool EditorView::isReadOnly(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->isReadOnly();
}

bool EditorView::isInsertionMode(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->cursor()->insertionMode() ==
           QHexCursor::InsertionMode::InsertMode;
}

bool EditorView::isKeepSize(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->isKeepSize();
}

bool EditorView::isLocked(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->isLocked();
}

qsizetype EditorView::documentLines(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->documentLines();
}

qsizetype EditorView::documentBytes(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->documentBytes();
}

qsizetype EditorView::currentRow(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->currentRow();
}

qsizetype EditorView::currentColumn(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->currentColumn();
}

qsizetype EditorView::currentOffset(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->currentOffset();
}

qsizetype EditorView::selectedLength(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->cursor()->currentSelectionLength();
}

QByteArray EditorView::selectedBytes(const QObject *caller, qsizetype index) {
    if (checkErrAndReport(caller, __func__)) {
        return {};
    }
    return m_hex->selectedBytes(index);
}

QByteArrayList EditorView::selectionBytes(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return {};
    }
    return m_hex->selectedBytes();
}

qsizetype EditorView::selectionLength(const QObject *caller, qsizetype index) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }

    auto cursor = m_hex->cursor();
    if (index >= 0 && index < cursor->selectionCount()) {
        return cursor->selectionLength(index);
    }

    return 0;
}

qsizetype EditorView::selectionCount(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->selectionCount();
}

bool EditorView::stringVisible(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->asciiVisible();
}

bool EditorView::addressVisible(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->addressVisible();
}

bool EditorView::headerVisible(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->headerVisible();
}

quintptr EditorView::addressBase(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return m_hex->addressBase();
}

bool EditorView::isModified(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return !m_hex->isSaved();
}

qint8 EditorView::readInt8(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<qint8>(this, offset, _rwlock);
}

qint16 EditorView::readInt16(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<qint16>(this, offset, _rwlock);
}

qint32 EditorView::readInt32(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<qint32>(this, offset, _rwlock);
}

qint64 EditorView::readInt64(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<qint64>(this, offset, _rwlock);
}

quint8 EditorView::readUInt8(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<quint8>(this, offset, _rwlock);
}

quint16 EditorView::readUInt16(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<quint16>(this, offset, _rwlock);
}

quint32 EditorView::readUInt32(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<quint32>(this, offset, _rwlock);
}

quint64 EditorView::readUInt64(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<quint64>(this, offset, _rwlock);
}

float EditorView::readFloat(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<float>(this, offset, _rwlock);
}

double EditorView::readDouble(const QObject *caller, qsizetype offset) {
    if (checkErrAndReport(caller, __func__)) {
        return 0;
    }
    return readBasicTypeContent<double>(this, offset, _rwlock);
}

QString EditorView::readString(const QObject *caller, qsizetype offset,
                               const QString &encoding) {
    if (checkErrAndReport(caller, __func__)) {
        return {};
    }
    _rwlock.lockForRead();
    auto doc = m_hex->document();
    auto pos = doc->findNext(offset, QByteArray(1, 0));
    if (pos < 0) {
        pos = doc->findNext(offset, QByteArray(1, '\n'));
        if (pos < 0) {
            return QString();
        }
    }
    auto buffer = doc->read(offset, int(pos - offset));
    _rwlock.unlock();
    return Utilities::decodingString(buffer, encoding);
}

QByteArray EditorView::readBytes(const QObject *caller, qsizetype offset,
                                 qsizetype count) {
    if (checkErrAndReport(caller, __func__)) {
        return {};
    }
    _rwlock.lockForRead();
    auto ret = m_hex->document()->read(offset, count);
    _rwlock.unlock();
    return ret;
}

qsizetype EditorView::findNext(const QObject *caller, qsizetype begin,
                               const QByteArray &ba) {
    if (checkErrAndReport(caller, __func__)) {
        return -1;
    }
    return m_hex->document()->findNext(begin, ba);
}

qsizetype EditorView::findPrevious(const QObject *caller, qsizetype begin,
                                   const QByteArray &ba) {
    if (checkErrAndReport(caller, __func__)) {
        return -1;
    }
    return m_hex->document()->findPrevious(begin, ba);
}

QString EditorView::bookMarkComment(const QObject *caller, qsizetype pos) {
    if (checkErrAndReport(caller, __func__)) {
        return {};
    }
    return m_hex->document()->bookMark(pos);
}

bool EditorView::existBookMark(const QObject *caller, qsizetype pos) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return m_hex->document()->existBookMark(pos);
}

bool EditorView::setLockedFile(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }

    return m_hex->setLockedFile(b);
}

bool EditorView::setKeepSize(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    return m_hex->setKeepSize(b);
}

bool EditorView::setStringVisible(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    m_hex->setAsciiVisible(b);
    return true;
}

bool EditorView::setAddressVisible(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    m_hex->setAddressVisible(b);
    return true;
}

bool EditorView::setHeaderVisible(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    m_hex->setHeaderVisible(b);
    return true;
}

bool EditorView::setAddressBase(const QObject *caller, quintptr base) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    m_hex->setAddressBase(base);
    return true;
}

bool EditorView::beginMarco(const QObject *caller, const QString &txt) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    m_hex->document()->beginMarco(txt);
    return true;
}

bool EditorView::endMarco(const QObject *caller) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    m_hex->document()->endMarco();
    return true;
}

bool EditorView::writeInt8(const QObject *caller, qsizetype offset,
                           qint8 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeInt16(const QObject *caller, qsizetype offset,
                            qint16 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeInt32(const QObject *caller, qsizetype offset,
                            qint32 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeInt64(const QObject *caller, qsizetype offset,
                            qint64 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt8(const QObject *caller, qsizetype offset,
                            quint8 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt16(const QObject *caller, qsizetype offset,
                             quint16 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt32(const QObject *caller, qsizetype offset,
                             quint32 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeUInt64(const QObject *caller, qsizetype offset,
                             quint64 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeFloat(const QObject *caller, qsizetype offset,
                            float value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeDouble(const QObject *caller, qsizetype offset,
                             double value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return writeBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::writeString(const QObject *caller, qsizetype offset,
                             const QString &value, const QString &encoding) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    auto doc = m_hex->document();

    auto unicode = Utilities::encodingString(value, encoding);
    auto cmd = doc->MakeReplace(nullptr, m_hex->cursor(), offset, unicode);
    if (cmd) {
        _rwlock.lockForWrite();
        doc->pushMakeUndo(cmd);
        _rwlock.unlock();
        return true;
    }
    return false;
}

bool EditorView::writeBytes(const QObject *caller, qsizetype offset,
                            const QByteArray &data) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    auto doc = m_hex->document();
    auto cmd = doc->MakeReplace(nullptr, m_hex->cursor(), offset, data);
    if (cmd) {
        _rwlock.lockForWrite();
        doc->pushMakeUndo(cmd);
        _rwlock.unlock();
        return true;
    }
    return false;
}

bool EditorView::insertInt8(const QObject *caller, qsizetype offset,
                            qint8 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertInt16(const QObject *caller, qsizetype offset,
                             qint16 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertInt32(const QObject *caller, qsizetype offset,
                             qint32 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertInt64(const QObject *caller, qsizetype offset,
                             qint64 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt8(const QObject *caller, qsizetype offset,
                             quint8 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt16(const QObject *caller, qsizetype offset,
                              quint16 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt32(const QObject *caller, qsizetype offset,
                              quint32 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertUInt64(const QObject *caller, qsizetype offset,
                              quint64 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertFloat(const QObject *caller, qsizetype offset,
                             float value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertDouble(const QObject *caller, qsizetype offset,
                              double value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return insertBasicTypeContent(this, offset, value, nullptr, _rwlock);
}

bool EditorView::insertString(const QObject *caller, qsizetype offset,
                              const QString &value, const QString &encoding) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }

    auto doc = m_hex->document();

    auto unicode = Utilities::encodingString(value, encoding);
    auto cmd = doc->MakeInsert(nullptr, m_hex->cursor(), offset, unicode);
    if (cmd) {
        _rwlock.lockForWrite();
        doc->pushMakeUndo(cmd);
        _rwlock.unlock();
        return true;
    }
    return false;
}

bool EditorView::insertBytes(const QObject *caller, qsizetype offset,
                             const QByteArray &data) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeInsert(nullptr, m_hex->cursor(), offset, data);
    if (cmd) {
        _rwlock.lockForWrite();
        doc->pushMakeUndo(cmd);
        _rwlock.unlock();
        return true;
    }
    return false;
}

bool EditorView::appendInt8(const QObject *caller, qint8 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendInt16(const QObject *caller, qint16 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendInt32(const QObject *caller, qint32 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendInt64(const QObject *caller, qint64 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt8(const QObject *caller, quint8 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt16(const QObject *caller, quint16 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt32(const QObject *caller, quint32 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendUInt64(const QObject *caller, quint64 value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendFloat(const QObject *caller, float value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendDouble(const QObject *caller, double value) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    return appendBasicTypeContent(this, value, nullptr, _rwlock);
}

bool EditorView::appendString(const QObject *caller, const QString &value,
                              const QString &encoding) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }

    auto doc = m_hex->document();

    auto unicode = Utilities::encodingString(value, encoding);
    auto cmd = doc->MakeAppend(nullptr, m_hex->cursor(), unicode);
    if (cmd) {
        _rwlock.lockForWrite();
        doc->pushMakeUndo(cmd);
        _rwlock.unlock();
        return true;
    }
    return false;
}

bool EditorView::appendBytes(const QObject *caller, const QByteArray &data) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeAppend(nullptr, m_hex->cursor(), data);
    if (cmd) {
        _rwlock.lockForWrite();
        doc->pushMakeUndo(cmd);
        _rwlock.unlock();
        return true;
    }
    return false;
}

bool EditorView::removeBytes(const QObject *caller, qsizetype offset,
                             qsizetype len) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }

    auto doc = m_hex->document();
    auto cmd = doc->MakeRemove(nullptr, m_hex->cursor(), offset, len);
    if (cmd) {
        _rwlock.lockForWrite();
        doc->pushMakeUndo(cmd);
        _rwlock.unlock();
        return true;
    }
    return false;
}

bool EditorView::moveTo(const QObject *caller, qsizetype line, qsizetype column,
                        int nibbleindex, bool clearSelection) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    m_hex->cursor()->moveTo(line, column, nibbleindex, clearSelection);
    return true;
}

bool EditorView::moveTo(const QObject *caller, qsizetype offset,
                        bool clearSelection) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    m_hex->cursor()->moveTo(offset, clearSelection);
    return true;
}

bool EditorView::select(const QObject *caller, qsizetype offset,
                        qsizetype length, SelectionMode mode) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    m_hex->cursor()->setInsertionMode(isinsert ? QHexCursor::InsertMode
                                               : QHexCursor::OverwriteMode);
    return true;
}

bool EditorView::metadata(const QObject *caller, qsizetype begin,
                          qsizetype length, const QColor &fgcolor,
                          const QColor &bgcolor, const QString &comment) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    auto doc = m_hex->document();
    doc->setMetafgVisible(b);
    doc->setMetabgVisible(b);
    doc->setMetaCommentVisible(b);
    return true;
}

bool EditorView::setMetafgVisible(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    auto doc = m_hex->document();
    doc->setMetafgVisible(b);
    return true;
}

bool EditorView::setMetabgVisible(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    auto doc = m_hex->document();
    doc->setMetabgVisible(b);
    return true;
}

bool EditorView::setMetaCommentVisible(const QObject *caller, bool b) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
    }
    auto doc = m_hex->document();
    doc->setMetaCommentVisible(b);
    return true;
}

bool EditorView::addBookMark(const QObject *caller, qsizetype pos,
                             const QString &comment) {
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return false;
    }
    if (!checkThreadAff()) {
        return false;
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
    if (checkErrAndReport(caller, __func__)) {
        return {};
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
    if (checkErrAndReport(caller, __func__)) {
        return {};
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
    if (checkErrAndReport(caller, __func__)) {
        return {};
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

FindResultModel::FindData &EditorView::findResult() { return m_findData; }

QMap<QCryptographicHash::Algorithm, QString> &EditorView::checkSumResult() {
    return _checkSumData;
}

void EditorView::setFontSize(qreal size) { m_hex->setFontSize(size); }

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
    return m_hex->windowFilePath();
}

bool EditorView::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto e = static_cast<QDynamicPropertyChangeEvent *>(event);
        if (e->propertyName() == VIEW_PROPERTY ||
            e->propertyName() == VIEW_ID_PROPERTY) {
            std::abort();
        }
    }
    return ads::CDockWidget::eventFilter(watched, event);
}

EditorViewContext *EditorView::editorContext() const { return _context; }
