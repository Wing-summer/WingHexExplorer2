#include "pluginsystem.h"
#include "../../Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "../class/logger.h"
#include "../control/toast.h"
#include "../dialog/mainwindow.h"

#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QPluginLoader>
#include <QtCore>

PluginSystem::PluginSystem(QObject *parent) : QObject(parent) {}

PluginSystem::~PluginSystem() {
    for (auto &item : loadedplgs) {
        item->plugin2MessagePipe(WingPluginMessage::PluginUnLoading,
                                 emptyparam);
        item->controller.disconnect();
        item->reader.disconnect();
        item->unload();
        item->plugin2MessagePipe(WingPluginMessage::PluginUnLoaded, emptyparam);
        item->deleteLater();
    }
}

QList<IWingPlugin *> PluginSystem::plugins() { return loadedplgs; }

void PluginSystem::raiseDispatch(HookIndex hookindex, QList<QVariant> params) {
    auto dispatch = dispatcher[hookindex];
    params.push_front(hookindex);
    for (auto item : dispatch) {
        item->plugin2MessagePipe(WingPluginMessage::HookMessage, params);
    }
}

void PluginSystem::loadPlugin(QFileInfo fileinfo) {
    Q_ASSERT(_win);

    if (fileinfo.exists()) {
        QPluginLoader loader(fileinfo.absoluteFilePath());
        Logger::info(tr("LoadingPlugin") + fileinfo.fileName());
        QList<WingPluginInfo> loadedplginfos;
        try {
            auto p = qobject_cast<IWingPlugin *>(loader.instance());
            if (p) {
                if (p->signature() != WINGSUMMER) {
                    throw tr("ErrLoadPluginSign");
                }

                if (p->sdkVersion() != SDKVERSION) {
                    throw tr("ErrLoadPluginSDKVersion");
                }

                if (!p->pluginName().trimmed().length()) {
                    throw tr("ErrLoadPluginNoName");
                }

                auto puid = IWingPlugin::GetPUID(p);
                if (puid != p->puid()) {
                    throw tr("ErrLoadPluginPUID");
                }

                if (loadedpuid.contains(puid)) {
                    throw tr("ErrLoadLoadedPlugin");
                }

                p->plugin2MessagePipe(WingPluginMessage::PluginLoading,
                                      emptyparam);

                if (!p->init(loadedplginfos)) {
                    throw tr("ErrLoadInitPlugin");
                }

                WingPluginInfo info;
                info.puid = p->puid();
                info.pluginName = p->pluginName();
                info.pluginAuthor = p->pluginAuthor();
                info.pluginComment = p->pluginComment();
                info.pluginVersion = p->pluginVersion();

                loadedplginfos.push_back(info);
                loadedplgs.push_back(p);
                loadedpuid << puid;

                Logger::warning(tr("PluginName :") + info.pluginName);
                Logger::warning(tr("PluginAuthor :") + info.pluginAuthor);
                Logger::warning(tr("PluginWidgetRegister"));

                auto ribbonToolboxInfos = p->registeredRibbonTools();
                if (!ribbonToolboxInfos.isEmpty()) {
                    for (auto &item : ribbonToolboxInfos) {
                        if (_win->m_ribbonMaps.contains(item.catagory)) {
                            if (!item.toolboxs.isEmpty()) {
                                auto &cat = _win->m_ribbonMaps[item.catagory];
                                for (auto &group : item.toolboxs) {
                                    if (group.tools.isEmpty()) {
                                        continue;
                                    }
                                    auto g = cat->addGroup(group.name);
                                    for (auto &tool : group.tools) {
                                        g->addButton(tool);
                                    }
                                }
                            }
                        } else {
                            if (!item.toolboxs.isEmpty()) {
                                auto cat =
                                    _win->m_ribbon->addTab(item.displayName);
                                bool hasAdded = false;
                                for (auto &group : item.toolboxs) {
                                    if (group.tools.isEmpty()) {
                                        continue;
                                    }
                                    auto g = cat->addGroup(group.name);
                                    for (auto &tool : group.tools) {
                                        g->addButton(tool);
                                    }
                                    hasAdded = true;
                                }
                                if (hasAdded) {
                                    _win->m_ribbonMaps[item.catagory] = cat;
                                } else {
                                    _win->m_ribbon->removeTab(cat);
                                }
                            }
                        }
                    }
                }

                auto dockWidgets = p->registeredDockWidgets();
                if (!dockWidgets.isEmpty()) {
                    for (auto &info : dockWidgets) {
                        auto dw = _win->buildDockWidget(
                            _win->m_dock, info.widgetName, info.displayName,
                            info.widget, MainWindow::PLUGIN_VIEWS);
                        switch (info.area) {
                        case Qt::LeftDockWidgetArea: {
                            if (_win->m_leftViewArea == nullptr) {
                                _win->m_leftViewArea =
                                    _win->m_dock->addDockWidget(
                                        ads::LeftDockWidgetArea, dw);
                            } else {
                                _win->m_leftViewArea =
                                    _win->m_dock->addDockWidget(
                                        ads::CenterDockWidgetArea, dw,
                                        _win->m_leftViewArea);
                            }
                        } break;
                        case Qt::RightDockWidgetArea:
                            _win->m_dock->addDockWidget(
                                ads::CenterDockWidgetArea, dw,
                                _win->m_rightViewArea);
                            break;
                        case Qt::TopDockWidgetArea: {
                            if (_win->m_topViewArea == nullptr) {
                                _win->m_topViewArea =
                                    _win->m_dock->addDockWidget(
                                        ads::LeftDockWidgetArea, dw);
                            } else {
                                _win->m_topViewArea =
                                    _win->m_dock->addDockWidget(
                                        ads::CenterDockWidgetArea, dw,
                                        _win->m_topViewArea);
                            }
                        } break;
                        case Qt::BottomDockWidgetArea:
                            _win->m_dock->addDockWidget(
                                ads::CenterDockWidgetArea, dw,
                                _win->m_bottomViewArea);
                            break;
                        case Qt::DockWidgetArea_Mask:
                        case Qt::NoDockWidgetArea:
                            _win->m_dock->addDockWidget(
                                ads::CenterDockWidgetArea, dw,
                                _win->m_rightViewArea);
                            dw->hide();
                            break;
                        }
                    }
                }

                _win->m_hexContextMenu.append(p->registeredHexContextMenu());
                _win->m_editorViewWidgets.append(
                    p->registeredEditorViewWidgets());
                _win->m_settingPages.append(p->registeredSettingPages());

                subscribeDispatcher(p, HookIndex::OpenFileBegin);
                subscribeDispatcher(p, HookIndex::OpenFileEnd);
                subscribeDispatcher(p, HookIndex::CloseFileBegin);
                subscribeDispatcher(p, HookIndex::CloseFileEnd);
                subscribeDispatcher(p, HookIndex::NewFileBegin);
                subscribeDispatcher(p, HookIndex::NewFileEnd);
                subscribeDispatcher(p, HookIndex::DocumentSwitched);

                m_plgviewMap.insert(p, nullptr);

                p->plugin2MessagePipe(WingPluginMessage::PluginLoaded,
                                      emptyparam);

            } else {
                throw loader.errorString();
            }
        } catch (const QString &error) {
            Logger::critical(error);
            loader.unload();
        }

        Logger::_log("");
    }
}

void PluginSystem::subscribeDispatcher(IWingPlugin *plg, HookIndex hookIndex) {
    Q_ASSERT(plg);
    if (plg->getHookSubscribe().testFlag(hookIndex)) {
        dispatcher[hookIndex].push_back(plg);
    }
}

void PluginSystem::connectInterface(IWingPlugin *plg) {
    connectBaseInterface(plg);
    connectReaderInterface(plg);
    connectControllerInterface(plg);
    connectUIInterface(plg);
}

void PluginSystem::connectBaseInterface(IWingPlugin *plg) {
    connect(plg, &IWingPlugin::toast, this,
            [=](const QPixmap &icon, const QString &message) {
                Toast::toast(_win, icon, message);
            });
    connect(plg, &IWingPlugin::debug, this,
            [=](const QString &message) { Logger::debug(message); });
    connect(plg, &IWingPlugin::info, this,
            [=](const QString &message) { Logger::info(message); });
    connect(plg, &IWingPlugin::warn, this,
            [=](const QString &message) { Logger::warning(message); });
    connect(plg, &IWingPlugin::error, this,
            [=](const QString &message) { Logger::critical(message); });
}

void PluginSystem::connectReaderInterface(IWingPlugin *plg) {
    auto preader = &plg->reader;

    connect(preader, &WingPlugin::Reader::isCurrentDocEditing, _win,
            [=]() -> bool { return pluginCurrentEditor(sender()); });
    connect(preader, &WingPlugin::Reader::currentDocFilename, _win,
            [=]() -> QString {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->fileName();
                }
                return QString();
            });
    connect(preader, &WingPlugin::Reader::isLocked, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->isLocked();
        }
        return true;
    });
    connect(preader, &WingPlugin::Reader::isEmpty, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->document()->isEmpty();
        }
        return true;
    });
    connect(preader, &WingPlugin::Reader::isKeepSize, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->isKeepSize();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::isModified, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->document()->isDocSaved();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::isReadOnly, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->isReadOnly();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::documentLines, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->renderer()->documentLines();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::documentBytes, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->documentBytes();
                }
                return 0;
            });
    connect(
        preader, &WingPlugin::Reader::currentPos, _win, [=]() -> HexPosition {
            HexPosition pos;
            memset(&pos, 0, sizeof(HexPosition));

            auto e = pluginCurrentEditor(sender());
            if (e) {
                auto qpos = e->hexEditor()->document()->cursor()->position();
                pos.line = qpos.line;
                pos.column = qpos.column;
                pos.lineWidth = qpos.lineWidth;
                pos.nibbleindex = qpos.nibbleindex;
            }
            return pos;
        });
    connect(preader, &WingPlugin::Reader::selectionPos, _win,
            [=]() -> HexPosition {
                HexPosition pos;
                memset(&pos, 0, sizeof(HexPosition));

                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto cur = e->hexEditor()->document()->cursor();
                    pos.line = cur->selectionLine();
                    pos.column = cur->selectionColumn();
                    pos.nibbleindex = cur->selectionNibble();
                    pos.lineWidth = cur->position().lineWidth;
                }
                return pos;
            });
    connect(preader, &WingPlugin::Reader::currentRow, _win, [=]() -> qsizetype {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->currentRow();
        }
        return 0;
    });
    connect(preader, &WingPlugin::Reader::currentColumn, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->currentColumn();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::currentOffset, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()
                        ->document()
                        ->cursor()
                        ->position()
                        .offset();
                }
                return 0;
            });
    connect(
        preader, &WingPlugin::Reader::selectLength, _win, [=]() -> qsizetype {
            auto e = pluginCurrentEditor(sender());
            if (e) {
                return e->hexEditor()->document()->cursor()->selectionLength();
            }
            return 0;
        });
    connect(preader, &WingPlugin::Reader::selectedBytes, _win,
            [=]() -> QByteArray {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->selectedBytes();
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::stringVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->asciiVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::headerVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->headerVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::addressVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->addressVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::addressBase, _win, [=]() -> quintptr {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->addressBase();
        }
        return 0;
    });
    connect(preader, &WingPlugin::Reader::atEnd, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->document()->atEnd();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::canUndo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->document()->canUndo();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::canRedo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->document()->canRedo();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::documentLastLine, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->renderer()->documentLastLine();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::documentLastColumn, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->renderer()->documentLastColumn();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::copy, _win, [=](bool hex) -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            return e->hexEditor()->document()->copy(hex);
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::read, _win,
            [=](qsizetype offset, qsizetype len) -> QByteArray {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->read(offset, len);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::readInt8, _win,
            [=](qsizetype offset) -> qint8 {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto buffer =
                        e->hexEditor()->document()->read(offset, sizeof(qint8));
                    auto pb =
                        reinterpret_cast<const qint8 *>(buffer.constData());
                    return *pb;
                }
                return qint8(-1);
            });
    connect(preader, &WingPlugin::Reader::readInt16, _win,
            [=](qsizetype offset) -> qint16 {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto buffer = e->hexEditor()->document()->read(
                        offset, sizeof(qint16));
                    auto pb =
                        reinterpret_cast<const qint16 *>(buffer.constData());
                    return *pb;
                }
                return qint16(-1);
            });
    connect(preader, &WingPlugin::Reader::readInt32, _win,
            [=](qsizetype offset) -> qint32 {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto buffer = e->hexEditor()->document()->read(
                        offset, sizeof(qint32));
                    auto pb =
                        reinterpret_cast<const qint32 *>(buffer.constData());
                    return *pb;
                }
                return qint32(-1);
            });
    connect(preader, &WingPlugin::Reader::readInt64, _win,
            [=](qsizetype offset) -> qint64 {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto buffer = e->hexEditor()->document()->read(
                        offset, sizeof(qint64));
                    auto pb =
                        reinterpret_cast<const qint64 *>(buffer.constData());
                    return *pb;
                }
                return qint64(-1);
            });
    connect(preader, &WingPlugin::Reader::readString, _win,
            [=](qsizetype offset, const QString &encoding) -> QString {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto hexeditor = e->hexEditor();
                    auto doc = hexeditor->document();
                    auto render = hexeditor->renderer();
                    auto pos = doc->searchForward(offset, QByteArray(1, 0));
                    if (pos < 0)
                        return QString();
                    auto buffer = doc->read(offset, int(pos - offset));
                    QString enco = encoding;
                    if (!enco.length()) {
                        enco = render->encoding();
                    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    auto en = QStringConverter::encodingForName(enco.toUtf8());
                    QString unicode;
                    if (en.has_value()) {
                        QStringDecoder e(en.value());
                        unicode = e.decode(buffer);
                    }
#else
                    auto enc = QTextCodec::codecForName(enco.toUtf8());
                    auto d = enc->makeDecoder();
                    auto unicode = d->toUnicode(buffer);
#endif
                    return unicode;
                }
                return QString();
            });

    connect(preader, &WingPlugin::Reader::findAllBytes, _win,
            [=](qsizetype begin, qsizetype end, QByteArray b,
                qsizetype maxCount) -> QList<qsizetype> {
                QList<qsizetype> results;
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    e->hexEditor()->document()->findAllBytes(begin, end, b,
                                                             results, maxCount);
                }
                return results;
            });
    connect(preader, &WingPlugin::Reader::searchForward, _win,
            [=](qsizetype begin, const QByteArray ba) -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->searchForward(begin, ba);
                }
                return qsizetype(-1);
            });
    connect(preader, &WingPlugin::Reader::searchBackward, _win,
            [=](qsizetype begin, const QByteArray ba) -> qsizetype {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->searchBackward(begin,
                                                                      ba);
                }
                return qsizetype(-1);
            });
    connect(preader, &WingPlugin::Reader::getMetaLine, _win,
            [=](qsizetype line) -> HexLineMetadata {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto ometas =
                        e->hexEditor()->document()->metadata()->get(line);
                    HexLineMetadata metas;
                    for (auto &item : ometas) {
                        metas.push_back(HexMetadataItem(
                            item.line, item.start, item.length, item.foreground,
                            item.background, item.comment));
                    }
                    return metas;
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::getMetadatas, _win,
            [=](qsizetype offset) -> QList<HexMetadataAbsoluteItem> {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto ometaline =
                        e->hexEditor()->document()->metadata()->gets(offset);
                    QList<HexMetadataAbsoluteItem> metaline;
                    for (auto &item : ometaline) {
                        metaline.push_back(HexMetadataAbsoluteItem(
                            item.begin, item.end, item.foreground,
                            item.background, item.comment));
                    }
                    return metaline;
                }
                return {};
            });
    connect(
        preader, &WingPlugin::Reader::lineHasMetadata, _win,
        [=](qsizetype line) -> bool {
            auto e = pluginCurrentEditor(sender());
            if (e) {
                return e->hexEditor()->document()->metadata()->lineHasMetadata(
                    line);
            }
            return false;
        });
    connect(preader, &WingPlugin::Reader::lineHasBookMark, _win,
            [=](qsizetype line) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->lineHasBookMark(line);
                }
                return false;
            });
    connect(preader, &WingPlugin::Reader::getsBookmarkPos, _win,
            [=](qsizetype line) -> QList<qsizetype> {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->getsBookmarkPos(line);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::bookMark, _win,
            [=](qsizetype pos) -> BookMark {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto b = e->hexEditor()->document()->bookMark(pos);
                    BookMark book;
                    book.pos = b.pos;
                    book.comment = b.comment;
                    return book;
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::bookMarkComment, _win,
            [=](qsizetype pos) -> QString {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->bookMarkComment(pos);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::getBookMarks, _win,
            [=]() -> QList<BookMark> {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto bs = e->hexEditor()->document()->getAllBookMarks();
                    QList<BookMark> bookmarks;
                    for (auto &item : bs) {
                        BookMark i;
                        i.pos = item.pos;
                        i.comment = item.comment;
                        bookmarks.push_back(i);
                    }
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::existBookMark, _win,
            [=](qsizetype pos) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->existBookMark(pos);
                }
                return false;
            });
    connect(preader, &WingPlugin::Reader::getOpenFiles, _win,
            [=]() -> QStringList { return _win->m_openedFileNames; });
    connect(preader, &WingPlugin::Reader::getSupportedEncodings, _win,
            [=]() -> QStringList { return Utilities::getEncodings(); });
    connect(preader, &WingPlugin::Reader::currentEncoding, _win,
            [=]() -> QString {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->renderer()->encoding();
                }
                return {};
            });
}

void PluginSystem::connectControllerInterface(IWingPlugin *plg) {
    Q_ASSERT(plg);

    auto pctl = &plg->controller;
    connect(pctl, &WingPlugin::Controller::switchDocument, _win,
            [=](int index, bool gui) -> bool {
                // TODO
                if (gui) {

                } else {
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setLockedFile, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->setLockedFile(b);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setKeepSize, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->setKeepSize(b);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setStringVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    e->hexEditor()->setAsciiVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setHeaderVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    e->hexEditor()->setHeaderVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setAddressVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    e->hexEditor()->setAddressVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setAddressBase, _win,
            [=](quintptr base) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    e->hexEditor()->setAddressBase(base);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::undo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            e->hexEditor()->document()->undo();
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::redo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            e->hexEditor()->document()->redo();
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::cut, _win, [=](bool hex) -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            e->hexEditor()->document()->cut(hex);
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::paste, _win, [=](bool hex) -> bool {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            e->hexEditor()->document()->paste(hex);
            return true;
        }
        return false;
    });
    connect(
        pctl,
        QOverload<qsizetype, const uchar>::of(&WingPlugin::Controller::insert),
        _win, [=](qsizetype offset, uchar b) -> bool {
            auto e = pluginCurrentEditor(sender());
            if (e) {
                return e->hexEditor()->document()->insert(offset, b);
            }
            return false;
        });
    connect(pctl,
            QOverload<qsizetype, const QByteArray>::of(
                &WingPlugin::Controller::insert),
            _win, [=](qsizetype offset, const QByteArray data) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->insert(offset, data);
                }
                return false;
            });
    connect(
        pctl,
        QOverload<qsizetype, const uchar>::of(&WingPlugin::Controller::write),
        _win, [=](qsizetype offset, uchar b) -> bool {
            auto e = pluginCurrentEditor(sender());
            if (e) {
                return e->hexEditor()->document()->replace(offset, b);
            }
            return false;
        });
    connect(pctl,
            QOverload<qsizetype, const QByteArray>::of(
                &WingPlugin::Controller::write),
            _win, [=](qsizetype offset, const QByteArray data) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    return e->hexEditor()->document()->replace(offset, data);
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::insertInt8, _win,
            [=](qsizetype offset, qint8 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->insert(offset,
                                       QByteArray(buffer, sizeof(qint8)));
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::insertInt16, _win,
            [=](qsizetype offset, qint16 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->insert(offset,
                                       QByteArray(buffer, sizeof(qint16)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::insertInt32, _win,
            [=](qsizetype offset, qint32 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->insert(offset,
                                       QByteArray(buffer, sizeof(qint32)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::insertInt64, _win,
            [=](qsizetype offset, qint64 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->insert(offset,
                                       QByteArray(buffer, sizeof(qint64)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::insertString, _win,
            [=](qsizetype offset, const QString value,
                const QString encoding) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto render = editor->renderer();

                    QString enco = encoding;
                    if (!enco.length()) {
                        enco = render->encoding();
                    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    auto en = QStringConverter::encodingForName(enco.toUtf8());
                    QByteArray unicode;
                    if (en.has_value()) {
                        QStringEncoder e(en.value());
                        unicode = e.encode(value);
                    }
#else
                    auto enc = QTextCodec::codecForName(enco.toUtf8());
                    auto e = enc->makeEncoder();
                    auto unicode = e->fromUnicode(value);
#endif
                    return doc->insert(offset, unicode);
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::writeInt8, _win,
            [=](qsizetype offset, qint8 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->replace(offset,
                                        QByteArray(buffer, sizeof(qint8)));
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::writeInt16, _win,
            [=](qsizetype offset, qint16 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->replace(offset,
                                        QByteArray(buffer, sizeof(qint16)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::writeInt32, _win,
            [=](qsizetype offset, qint32 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->replace(offset,
                                        QByteArray(buffer, sizeof(qint32)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::writeInt64, _win,
            [=](qsizetype offset, qint64 value) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->replace(offset,
                                        QByteArray(buffer, sizeof(qint64)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::writeString, _win,
            [=](qsizetype offset, const QString value,
                const QString encoding) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto editor = e->hexEditor();
                    auto doc = editor->document();
                    auto render = editor->renderer();

                    QString enco = encoding;
                    if (!enco.length()) {
                        enco = render->encoding();
                    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    auto en = QStringConverter::encodingForName(enco.toUtf8());
                    QByteArray unicode;
                    if (en.has_value()) {
                        QStringEncoder e(en.value());
                        unicode = e.encode(value);
                    }
#else
                    auto enc = QTextCodec::codecForName(enco.toUtf8());
                    auto e = enc->makeEncoder();
                    auto unicode = e->fromUnicode(value);
#endif
                    return doc->replace(offset, unicode);
                }
                return false;
            });

    connect(pctl, QOverload<uchar>::of(&WingPlugin::Controller::append), this,
            [=](uchar b) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    return doc->insert(offset, b);
                }
                return false;
            });
    connect(pctl,
            QOverload<const QByteArray &>::of(&WingPlugin::Controller::append),
            this, [=](const QByteArray &data) {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendInt8, this, [=](qint8 value) {
        auto e = pluginCurrentEditor(sender());
        if (e) {
            auto doc = e->hexEditor()->document();
            auto offset = doc->length();
            auto buffer = reinterpret_cast<char *>(&value);
            QByteArray data(buffer, sizeof(qint8));
            return doc->insert(offset, data);
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::appendInt16, this,
            [=](qint16 value) {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    auto buffer = reinterpret_cast<char *>(&value);
                    QByteArray data(buffer, sizeof(qint16));
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendInt32, this,
            [=](qint32 value) {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    auto buffer = reinterpret_cast<char *>(&value);
                    QByteArray data(buffer, sizeof(qint32));
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendInt64, this,
            [=](qint64 value) {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    auto buffer = reinterpret_cast<char *>(&value);
                    QByteArray data(buffer, sizeof(qint64));
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendString, this,
            [=](const QString value, const QString encoding) -> bool {
                auto e = pluginCurrentEditor(sender());
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    auto en =
                        QStringConverter::encodingForName(encoding.toUtf8());
                    QByteArray unicode;
                    if (en.has_value()) {
                        QStringEncoder e(en.value());
                        unicode = e.encode(value);
                    }
#else
                    auto enc = QTextCodec::codecForName(encoding.toUtf8());
                    auto e = enc->makeEncoder();
                    auto unicode = e->fromUnicode(value);
#endif
                    return doc->insert(offset, unicode);
                }
                return false;
            });

    /*
  ConnectControlLamba2(
      WingPlugin::Controller::remove, [=](qint64 offset, int len) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECKRETURN(hexfiles[_pcurfile].doc->remove(offset, len),
                     hexeditor->document()->remove(offset, len), false);
      });
  ConnectControlLamba2(WingPlugin::Controller::removeAll, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    PCHECK(
        {
          auto doc = hexfiles[_pcurfile].doc;
          auto len = doc->length();
          return doc->remove(0, int(len));
        },
        {
          auto doc = hexeditor->document();
          auto len = doc->length();
          return doc->remove(0, int(len));
        },
        return false;);
  });
  connect(pctl,
          QOverload<const HexPosition &>::of(&WingPlugin::Controller::moveTo),
          this, [=](const HexPosition &pos) {
            plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
            PCHECK(
                {
                  QHexPosition p;
                  p.line = pos.line;
                  p.column = pos.column;
                  p.lineWidth = pos.lineWidth;
                  p.nibbleindex = pos.nibbleindex;
                  hexfiles[_pcurfile].doc->cursor()->moveTo(p);
                },
                {
                  QHexPosition p;
                  p.line = pos.line;
                  p.column = pos.column;
                  p.lineWidth = pos.lineWidth;
                  p.nibbleindex = pos.nibbleindex;
                  hexeditor->document()->cursor()->moveTo(p);
                }, );
          });
  connect(pctl,
          QOverload<quint64, int, int>::of(&WingPlugin::Controller::moveTo),
          this, [=](quint64 line, int column, int nibbleindex) {
            plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
            PCHECK(hexfiles[_pcurfile].doc->cursor()->moveTo(line, column,
                                                             nibbleindex),
                   hexeditor->document()->cursor()->moveTo(line, column,
                                                           nibbleindex), );
          });
  connect(pctl, QOverload<qint64>::of(&WingPlugin::Controller::moveTo), this,
          [=](qint64 offset) {
            plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
            PCHECK(hexfiles[_pcurfile].doc->cursor()->moveTo(offset),
                   hexeditor->document()->cursor()->moveTo(offset), );
          });
  connect(pctl, QOverload<qint64, int>::of(&WingPlugin::Controller::select),
          this, [=](qint64 offset, int length) {
            PCHECK(
                hexfiles[_pcurfile].doc->cursor()->setSelection(offset, length),
                {
                  hexeditor->document()->cursor()->setSelection(offset, length);
                  hexeditor->viewport()->update();
                }, )
          });
  connect(pctl,
          QOverload<quint64, int, int>::of(&WingPlugin::Controller::select),
          this, [=](quint64 line, int column, int nibbleindex) {
            plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
            PCHECK(hexfiles[_pcurfile].doc->cursor()->select(line, column,
                                                             nibbleindex),
                   hexeditor->document()->cursor()->select(line, column,
                                                           nibbleindex), );
          });
  ConnectControlLamba2(WingPlugin::Controller::enabledCursor, [=](bool b) {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    PCHECK(hexfiles[_pcurfile].render->enableCursor(b),
           hexeditor->renderer()->enableCursor(b), );
  });

  ConnectControlLamba2(
      WingPlugin::Controller::selectOffset, [=](qint64 offset, int length) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(hexfiles[_pcurfile].doc->cursor()->selectOffset(offset, length),
               hexeditor->document()->cursor()->selectOffset(offset, length), );
      });
  ConnectControlLamba2(
      WingPlugin::Controller::setInsertionMode, [=](bool isinsert) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            hexfiles[_pcurfile].doc->cursor()->setInsertionMode(
                isinsert ? QHexCursor::InsertMode : QHexCursor::OverwriteMode),
            hexeditor->document()->cursor()->setInsertionMode(
                isinsert ? QHexCursor::InsertMode
                         : QHexCursor::OverwriteMode), );
      });
  connect(
      pctl,
      QOverload<qint64, qint64, const QColor &, const QColor &,
                const QString &>::of(&WingPlugin::Controller::metadata),
      this,
      [=](qint64 begin, qint64 end, const QColor &fgcolor,
          const QColor &bgcolor, const QString &comment) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            {
              auto doc = hexfiles[_pcurfile].doc;
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->metadata(begin, end, fgcolor, bgcolor, comment);
              doc->setDocSaved(false);
              return true;
            },
            {
              auto doc = hexeditor->document();
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->metadata(begin, end, fgcolor, bgcolor, comment);
              doc->setDocSaved(false);
              return true;
            },
            return false);
      });
  connect(pctl,
          QOverload<quint64, int, int, const QColor &, const QColor &,
                    const QString &>::of(&WingPlugin::Controller::metadata),
          this,
          [=](quint64 line, int start, int length, const QColor &fgcolor,
              const QColor &bgcolor, const QString &comment) {
            plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
            PCHECK(
                {
                  auto doc = hexfiles[_pcurfile].doc;
                  if (!doc->isKeepSize())
                    return false;
                  doc->metadata()->metadata(line, start, length, fgcolor,
                                            bgcolor, comment);

                  return true;
                },
                {
                  auto doc = hexeditor->document();
                  if (!doc->isKeepSize())
                    return false;
                  doc->metadata()->metadata(line, start, length, fgcolor,
                                            bgcolor, comment);

                  return true;
                },
                return false);
          });

  ConnectControlLamba2(
      WingPlugin::Controller::removeMetadata, [=](qint64 offset) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            {
              auto doc = hexfiles[_pcurfile].doc;
              doc->metadata()->removeMetadata(offset);
              doc->setDocSaved(false);
            },
            {
              auto doc = hexeditor->document();
              doc->metadata()->removeMetadata(offset);
              doc->setDocSaved(false);
            }, );
      });

  connect(pctl, QOverload<>::of(&WingPlugin::Controller::clearMeta), this,
          [=]() {
            plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
            PCHECK(
                {
                  auto doc = hexfiles[_pcurfile].doc;
                  if (!doc->isKeepSize())
                    return false;
                  doc->metadata()->clear();
                  return true;
                },
                {
                  auto doc = hexeditor->document();
                  if (!doc->isKeepSize())
                    return false;
                  doc->metadata()->clear();
                  return true;
                },
                return false);
          });

  ConnectControlLamba2(
      WingPlugin::Controller::color,
      [=](quint64 line, int start, int length, const QColor &fgcolor,
          const QColor &bgcolor) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            {
              auto doc = hexfiles[_pcurfile].doc;
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->color(line, start, length, fgcolor, bgcolor);
              return true;
            },
            {
              auto doc = hexeditor->document();
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->color(line, start, length, fgcolor, bgcolor);
              return true;
            },
            return false);
      });

  ConnectControlLamba2(
      WingPlugin::Controller::comment,
      [=](quint64 line, int start, int length, const QString &comment) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            {
              auto doc = hexfiles[_pcurfile].doc;
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->comment(line, start, length, comment);
              return true;
            },
            {
              auto doc = hexeditor->document();
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->comment(line, start, length, comment);
              return true;
            },
            return false);
      });

  ConnectControlLamba2(
      WingPlugin::Controller::foreground,
      [=](quint64 line, int start, int length, const QColor &fgcolor) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            {
              auto doc = hexfiles[_pcurfile].doc;
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->foreground(line, start, length, fgcolor);
              return true;
            },
            {
              auto doc = hexeditor->document();
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->foreground(line, start, length, fgcolor);
              return true;
            },
            return false);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::background,
      [=](quint64 line, int start, int length, const QColor &bgcolor) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            {
              auto doc = hexfiles[_pcurfile].doc;
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->background(line, start, length, bgcolor);
              return true;
            },
            {
              auto doc = hexeditor->document();
              if (!doc->isKeepSize())
                return false;
              doc->metadata()->background(line, start, length, bgcolor);
              return true;
            },
            return false);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::applyMetas,
      [=](const QList<HexMetadataAbsoluteItem> &metas) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(
            {
              QList<QHexMetadataAbsoluteItem> ms;
              for (auto &item : metas) {
                QHexMetadataAbsoluteItem i;
                i.begin = item.begin;
                i.end = item.end;
                i.comment = item.comment;
                i.background = item.background;
                i.foreground = item.foreground;
                ms.append(i);
              }
              hexfiles[_pcurfile].doc->metadata()->applyMetas(ms);
            },
            {
              QList<QHexMetadataAbsoluteItem> ms;
              for (auto &item : metas) {
                QHexMetadataAbsoluteItem i;
                i.begin = item.begin;
                i.end = item.end;
                i.comment = item.comment;
                i.background = item.background;
                i.foreground = item.foreground;
                ms.append(i);
              }
              hexeditor->document()->metadata()->applyMetas(ms);
            }, );
      });
  ConnectControlLamba2(WingPlugin::Controller::setMetafgVisible, [=](bool b) {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    PCHECK(hexfiles[_pcurfile].doc->setMetafgVisible(b),
           hexeditor->document()->SetMetafgVisible(b), );
  });
  ConnectControlLamba2(WingPlugin::Controller::setMetabgVisible, [=](bool b) {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    PCHECK(hexfiles[_pcurfile].doc->setMetabgVisible(b),
           hexeditor->document()->SetMetabgVisible(b), );
  });
  ConnectControlLamba2(
      WingPlugin::Controller::setMetaCommentVisible, [=](bool b) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECK(hexfiles[_pcurfile].doc->setMetaCommentVisible(b),
               hexeditor->document()->SetMetaCommentVisible(b), );
      });

  ConnectControlLamba2(
      WingPlugin::Controller::setCurrentEncoding, [=](const QString &encoding) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECKRETURN(hexfiles[_pcurfile].render->setEncoding(encoding),
                     hexeditor->renderer()->setEncoding(encoding), false);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::addBookMark,
      [=](qint64 pos, const QString &comment) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECKRETURN(hexfiles[_pcurfile].doc->addBookMark(pos, comment),
                     hexeditor->document()->addBookMark(pos, comment), false);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::modBookMark,
      [=](qint64 pos, const QString &comment) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        PCHECKRETURN(hexfiles[_pcurfile].doc->modBookMark(pos, comment),
                     hexeditor->document()->modBookMark(pos, comment), false);
      });
  ConnectControlLamba2(WingPlugin::Controller::removeBookMark, [=](qint64 pos) {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    PCHECKRETURN(hexfiles[_pcurfile].doc->removeBookMark(pos),
                 hexeditor->document()->removeBookMark(pos), false);
  });
  ConnectControlLamba2(WingPlugin::Controller::clearBookMark, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    PCHECKRETURN(hexfiles[_pcurfile].doc->clearBookMark(),
                 hexeditor->document()->clearBookMark(), false);
  });
  ConnectControlLamba2(WingPlugin::Controller::applyBookMarks,
                       [=](const QList<BookMark> &books) {
                         plgsys->resetTimeout(
                             qobject_cast<IWingPlugin *>(sender()));
                         PCHECK(
                             {
                               QList<BookMarkStruct> bs;
                               for (auto &item : books) {
                                 BookMarkStruct b;
                                 b.pos = item.pos;
                                 b.comment = item.comment;
                                 bs.append(b);
                               }
                               hexfiles[_pcurfile].doc->applyBookMarks(bs);
                             },
                             {
                               QList<BookMarkStruct> bs;
                               for (auto &item : books) {
                                 BookMarkStruct b;
                                 b.pos = item.pos;
                                 b.comment = item.comment;
                                 bs.append(b);
                               }
                               hexeditor->document()->applyBookMarks(bs);
                             }, );
                       });

  ConnectControlLamba2(
      WingPlugin::Controller::openWorkSpace, [=](const QString &filename) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        return openWorkSpace(filename);
      });
  ConnectControlLamba2(WingPlugin::Controller::newFile, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    newFile();
  });
  ConnectControlLamba2(WingPlugin::Controller::openFile,
                       [=](const QString &filename, int *openedIndex) {
                         plgsys->resetTimeout(
                             qobject_cast<IWingPlugin *>(sender()));
                         return openFile(filename, openedIndex);
                       });
  ConnectControlLamba2(
      WingPlugin::Controller::openRegionFile,
      [=](const QString &filename, int *openedIndex, qint64 start,
          qint64 length) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        return openRegionFile(filename, openedIndex, start, length);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::openDriver, [=](const QString &driver) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        return openDriver(driver);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::closeFile, [=](int index, bool force) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        return closeFile(index, force);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::saveFile, [=](int index, bool ignoreMd5) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        return save(index, ignoreMd5);
      });
  ConnectControlLamba2(WingPlugin::Controller::exportFile,
                       [=](const QString &filename, int index, bool ignoreMd5) {
                         plgsys->resetTimeout(
                             qobject_cast<IWingPlugin *>(sender()));
                         return exportFile(filename, index, ignoreMd5);
                       });
  ConnectControlLamba2(WingPlugin::Controller::exportFileGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_exportfile();
  });
  ConnectControlLamba2(WingPlugin::Controller::saveasFile,
                       [=](const QString &filename, int index, bool ignoreMd5) {
                         plgsys->resetTimeout(
                             qobject_cast<IWingPlugin *>(sender()));
                         return saveAs(filename, index, ignoreMd5);
                       });
  ConnectControlLamba2(WingPlugin::Controller::saveasFileGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_saveas();
  });
  ConnectControlLamba2(
      WingPlugin::Controller::closeCurrentFile, [=](bool force) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        return closeFile(_pcurfile, force);
      });
  ConnectControlLamba2(
      WingPlugin::Controller::saveCurrentFile, [=](bool ignoreMd5) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        return save(_pcurfile, ignoreMd5);
      });
  ConnectControlLamba2(WingPlugin::Controller::openFileGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_openfile();
  });
  ConnectControlLamba2(
      WingPlugin::Controller::openRegionFileGUI,
      [=](const QString &filename, qint64 start, qint64 length) {
        plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
        OpenRegionDialog d(lastusedpath, filename, int(start), int(length));
        if (d.exec()) {
          auto res = d.getResult();
          int index;
          auto ret =
              openRegionFile(res.filename, &index, res.start, res.length);
          if (ret == ErrFile::NotExist) {
            QMessageBox::critical(this, tr("Error"), tr("FileNotExist"));
            return;
          }
          if (ret == ErrFile::Permission) {
            QMessageBox::critical(this, tr("Error"), tr("FilePermission"));
            return;
          }
          if (ret == ErrFile::AlreadyOpened) {
            setFilePage(index);
            return;
          }
        }
      });
  ConnectControlLamba2(WingPlugin::Controller::openDriverGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_opendriver();
  });
  ConnectControlLamba2(WingPlugin::Controller::gotoGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_gotoline();
  });
  ConnectControlLamba2(WingPlugin::Controller::findGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_findfile();
  });
  ConnectControlLamba2(WingPlugin::Controller::fillGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_fill();
  });
  ConnectControlLamba2(WingPlugin::Controller::fillzeroGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_fillzero();
  });
  ConnectControlLamba2(WingPlugin::Controller::fillnopGUI, [=] {
    plgsys->resetTimeout(qobject_cast<IWingPlugin *>(sender()));
    on_fillnop();
  });
*/
}

void PluginSystem::connectUIInterface(IWingPlugin *plg) {}

PluginSystem &PluginSystem::instance() {
    static PluginSystem ins;
    return ins;
}

void PluginSystem::setMainWindow(MainWindow *win) { _win = win; }

void PluginSystem::LoadPlugin() {
    Q_ASSERT(_win);

#ifdef QT_DEBUG
    QDir plugindir(QCoreApplication::applicationDirPath() +
                   QStringLiteral("/plugin"));
    //这是我的插件调试目录，如果调试插件，请更换路径
#ifdef Q_OS_WIN
    plugindir.setNameFilters({"*.dll", "*.wingplg"});
#else
    plugindir.setNameFilters({"*.so", "*.wingplg"});
#endif
#else
    QDir plugindir(QCoreApplication::applicationDirPath() +
                   QStringLiteral("/plugin"));
    plugindir.setNameFilters({"*.wingplg"});
#endif
    auto plgs = plugindir.entryInfoList();
    Logger::info(tr("FoundPluginCount") + QString::number(plgs.count()));

    for (auto &item : plgs) {
        loadPlugin(item);
    }

    Logger::info(tr("PluginLoadingFinished"));
}

EditorView *PluginSystem::pluginCurrentEditor(QObject *sender) const {
    if (sender) {
        auto editor = m_plgviewMap.value(sender);
        if (editor) {
            return editor;
        } else {
            return _win->m_curEditor.loadAcquire();
        }
    }
    return nullptr;
}
