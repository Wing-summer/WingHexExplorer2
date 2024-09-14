#include "pluginsystem.h"
#include "QJsonModel/include/QJsonModel.hpp"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "class/logger.h"
#include "class/settingmanager.h"
#include "class/wingfiledialog.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "dialog/colorpickerdialog.h"
#include "dialog/mainwindow.h"
#include "model/qjsontablemodel.h"

#include <QDir>
#include <QFileInfoList>
#include <QMessageBox>
#include <QPluginLoader>
#include <QtCore>

PluginSystem::PluginSystem(QObject *parent) : QObject(parent) {}

PluginSystem::~PluginSystem() {
    for (auto &item : loadedplgs) {
        item->unload();
        delete item;
    }
}

const QList<IWingPlugin *> &PluginSystem::plugins() const { return loadedplgs; }

const IWingPlugin *PluginSystem::plugin(qsizetype index) const {
    return loadedplgs.at(index);
}

void PluginSystem::loadPlugin(QFileInfo fileinfo) {
    Q_ASSERT(_win);

    if (fileinfo.exists()) {
        QPluginLoader loader(fileinfo.absoluteFilePath(), this);
        Logger::info(tr("LoadingPlugin") + fileinfo.fileName());
        auto p = qobject_cast<IWingPlugin *>(loader.instance());
        if (Q_UNLIKELY(p == nullptr)) {
            Logger::critical(loader.errorString());
        } else {
            if (!loadPlugin(p)) {
                loader.unload();
            }
        }
        Logger::_log("");
    }
}

WingAngelAPI *PluginSystem::angelApi() const { return _angelplg; }

void PluginSystem::cleanUpEditorViewHandle(EditorView *view) {
    if (m_viewBindings.contains(view)) {
        auto v = m_viewBindings.value(view);

        // clean up
        for (auto &plg : v) {
            auto handles = m_plgHandles.value(plg);
            auto r = std::remove_if(handles.begin(), handles.end(),
                                    [view](const QPair<int, EditorView *> &v) {
                                        return v.second == view;
                                    });
            Q_UNUSED(r);
        }

        m_viewBindings.remove(view);
    }
}

bool PluginSystem::loadPlugin(IWingPlugin *p) {
    QList<WingPluginInfo> loadedplginfos;
    try {

        if (p->signature() != WINGSUMMER) {
            throw tr("ErrLoadPluginSign");
        }

        if (p->sdkVersion() != SDKVERSION) {
            throw tr("ErrLoadPluginSDKVersion");
        }

        if (!p->pluginName().trimmed().length()) {
            throw tr("ErrLoadPluginNoName");
        }

        auto prop = p->property("puid").toString().trimmed();
        auto puid =
            prop.isEmpty() ? QString(p->metaObject()->className()) : prop;
        if (loadedpuid.contains(puid)) {
            throw tr("ErrLoadLoadedPlugin");
        }

        if (!p->init(loadedplginfos)) {
            throw tr("ErrLoadInitPlugin");
        }

        WingPluginInfo info;
        info.puid = puid;
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
                        auto cat = _win->m_ribbon->addTab(item.displayName);
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
                auto dw = _win->buildDockWidget(_win->m_dock, info.widgetName,
                                                info.displayName, info.widget,
                                                MainWindow::PLUGIN_VIEWS);
                switch (info.area) {
                case Qt::LeftDockWidgetArea: {
                    if (_win->m_leftViewArea == nullptr) {
                        _win->m_leftViewArea = _win->m_dock->addDockWidget(
                            ads::LeftDockWidgetArea, dw);
                    } else {
                        _win->m_leftViewArea = _win->m_dock->addDockWidget(
                            ads::CenterDockWidgetArea, dw,
                            _win->m_leftViewArea);
                    }
                } break;
                case Qt::RightDockWidgetArea:
                    _win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                                _win->m_rightViewArea);
                    break;
                case Qt::TopDockWidgetArea: {
                    if (_win->m_topViewArea == nullptr) {
                        _win->m_topViewArea = _win->m_dock->addDockWidget(
                            ads::LeftDockWidgetArea, dw);
                    } else {
                        _win->m_topViewArea = _win->m_dock->addDockWidget(
                            ads::CenterDockWidgetArea, dw, _win->m_topViewArea);
                    }
                } break;
                case Qt::BottomDockWidgetArea:
                    _win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                                _win->m_bottomViewArea);
                    break;
                case Qt::DockWidgetArea_Mask:
                case Qt::NoDockWidgetArea:
                    _win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                                _win->m_rightViewArea);
                    dw->hide();
                    break;
                }
            }
        }

        if (p->registeredHexContextMenu()) {
            _win->m_hexContextMenu.append(p->registeredHexContextMenu());
        }

        if (!p->registeredEditorViewWidgets().isEmpty()) {
            _win->m_editorViewWidgets.insert(p,
                                             p->registeredEditorViewWidgets());
        }

        if (!p->registeredSettingPages().isEmpty()) {
            _win->m_settingPages.append(p->registeredSettingPages());
        }

        connectInterface(p);
        m_plgviewMap.insert(p, nullptr);
    } catch (const QString &error) {
        Logger::critical(error);
        return false;
    }
    return true;
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
                if (message.isEmpty()) {
                    return;
                }
                Toast::toast(_win, icon, message);
            });
    connect(plg, &IWingPlugin::trace, this, [=](const QString &message) {
        Logger::trace(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::debug, this, [=](const QString &message) {
        Logger::debug(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::info, this, [=](const QString &message) {
        Logger::info(packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::warn, this, [=](const QString &message) {
        Logger::warning(
            packLogMessage(plg->metaObject()->className(), message));
    });
    connect(plg, &IWingPlugin::error, this, [=](const QString &message) {
        Logger::critical(
            packLogMessage(plg->metaObject()->className(), message));
    });
}

void PluginSystem::connectReaderInterface(IWingPlugin *plg) {
    auto preader = &plg->reader;

    connect(preader, &WingPlugin::Reader::isCurrentDocEditing, _win,
            [=]() -> bool { return pluginCurrentEditor(plg); });
    connect(preader, &WingPlugin::Reader::currentDocFilename, _win,
            [=]() -> QString {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->fileName();
                }
                return QString();
            });
    connect(preader, &WingPlugin::Reader::isLocked, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->isLocked();
        }
        return true;
    });
    connect(preader, &WingPlugin::Reader::isEmpty, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->document()->isEmpty();
        }
        return true;
    });
    connect(preader, &WingPlugin::Reader::isKeepSize, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->isKeepSize();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::isModified, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->document()->isDocSaved();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::isReadOnly, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->isReadOnly();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::documentLines, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->renderer()->documentLines();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::documentBytes, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->documentBytes();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::currentPos, _win,
            [=]() -> HexPosition {
                HexPosition pos;
                memset(&pos, 0, sizeof(HexPosition));

                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto qpos = e->hexEditor()->cursor()->position();
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

                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto cur = e->hexEditor()->cursor();
                    pos.line = cur->selectionLine();
                    pos.column = cur->selectionColumn();
                    pos.nibbleindex = cur->selectionNibble();
                    pos.lineWidth = cur->position().lineWidth;
                }
                return pos;
            });
    connect(preader, &WingPlugin::Reader::currentRow, _win, [=]() -> qsizetype {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->currentRow();
        }
        return 0;
    });
    connect(preader, &WingPlugin::Reader::currentColumn, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->currentColumn();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::currentOffset, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->cursor()->position().offset();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::selectedLength, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->cursor()->selectionLength();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::selectedBytes, _win,
            [=]() -> QByteArray {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->selectedBytes();
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::stringVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->asciiVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::headerVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->headerVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::addressVisible, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->addressVisible();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::addressBase, _win, [=]() -> quintptr {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->addressBase();
        }
        return 0;
    });
    connect(preader, &WingPlugin::Reader::atEnd, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->atEnd();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::canUndo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->document()->canUndo();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::canRedo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->document()->canRedo();
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::documentLastLine, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->renderer()->documentLastLine();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::documentLastColumn, _win,
            [=]() -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->renderer()->documentLastColumn();
                }
                return 0;
            });
    connect(preader, &WingPlugin::Reader::copy, _win, [=](bool hex) -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            return e->hexEditor()->copy(hex);
        }
        return false;
    });
    connect(preader, &WingPlugin::Reader::readBytes, _win,
            [=](qsizetype offset, qsizetype len) -> QByteArray {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->read(offset, len);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::readInt8, _win,
            [=](qsizetype offset) -> qint8 {
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
            [=](qsizetype begin, qsizetype end,
                const QByteArray &b) -> QList<qsizetype> {
                QList<qsizetype> results;
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->document()->findAllBytes(begin, end, b,
                                                             results);
                }
                return results;
            });
    connect(preader, &WingPlugin::Reader::searchForward, _win,
            [=](qsizetype begin, const QByteArray &ba) -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->searchForward(begin, ba);
                }
                return qsizetype(-1);
            });
    connect(preader, &WingPlugin::Reader::searchBackward, _win,
            [=](qsizetype begin, const QByteArray &ba) -> qsizetype {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->searchBackward(begin,
                                                                      ba);
                }
                return qsizetype(-1);
            });
    connect(preader, &WingPlugin::Reader::getMetaLine, _win,
            [=](qsizetype line) -> HexLineMetadata {
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
            auto e = pluginCurrentEditor(plg);
            if (e) {
                return e->hexEditor()->document()->metadata()->lineHasMetadata(
                    line);
            }
            return false;
        });
    connect(preader, &WingPlugin::Reader::lineHasBookMark, _win,
            [=](qsizetype line) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->lineHasBookMark(line);
                }
                return false;
            });
    connect(preader, &WingPlugin::Reader::getsBookmarkPos, _win,
            [=](qsizetype line) -> QList<qsizetype> {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->getsBookmarkPos(line);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::bookMark, _win,
            [=](qsizetype pos) -> BookMark {
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->bookMarkComment(pos);
                }
                return {};
            });
    connect(preader, &WingPlugin::Reader::getBookMarks, _win,
            [=]() -> QList<BookMark> {
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->existBookMark(pos);
                }
                return false;
            });
    connect(preader, &WingPlugin::Reader::getSupportedEncodings, _win,
            [=]() -> QStringList { return Utilities::getEncodings(); });
    connect(preader, &WingPlugin::Reader::currentEncoding, _win,
            [=]() -> QString {
                auto e = pluginCurrentEditor(plg);
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
            [=](int handle) -> bool {
                if (handle < 0) {
                    m_plgviewMap[plg] = nullptr;
                } else {
                    auto handles = m_plgHandles.value(plg);
                    auto ret = std::find_if(
                        handles.begin(), handles.end(),
                        [handle](const QPair<int, EditorView *> &v) {
                            return v.first == handle;
                        });
                    if (ret == handles.end()) {
                        return false;
                    }
                    m_plgviewMap[plg] = ret->second;
                }
                return true;
            });
    connect(pctl, &WingPlugin::Controller::raiseDocument, _win,
            [=](int handle) -> bool {
                if (handle < 0) {
                    auto cur = _win->m_curEditor;
                    if (cur) {
                        cur->raise();
                    } else {
                        return false;
                    }
                } else {
                    auto handles = m_plgHandles.value(plg);
                    auto ret = std::find_if(
                        handles.begin(), handles.end(),
                        [handle](const QPair<int, EditorView *> &v) {
                            return v.first == handle;
                        });
                    if (ret == handles.end()) {
                        return false;
                    }
                    ret->second->raise();
                }
                return true;
            });
    connect(pctl, &WingPlugin::Controller::setLockedFile, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->setLockedFile(b);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setKeepSize, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->setKeepSize(b);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setStringVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setAsciiVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setHeaderVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setHeaderVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setAddressVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setAddressVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setAddressBase, _win,
            [=](quintptr base) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->setAddressBase(base);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::undo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            e->hexEditor()->document()->undo();
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::redo, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            e->hexEditor()->document()->redo();
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::cut, _win, [=](bool hex) -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            e->hexEditor()->cut(hex);
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::paste, _win, [=](bool hex) -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            e->hexEditor()->paste(hex);
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::insertBytes, _win,
            [=](qsizetype offset, const QByteArray &data) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::writeBytes, _win,
            [=](qsizetype offset, const QByteArray &data) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->document()->replace(offset, data);
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::insertInt8, _win,
            [=](qsizetype offset, qint8 value) -> bool {
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->insert(offset,
                                       QByteArray(buffer, sizeof(qint64)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::insertString, _win,
            [=](qsizetype offset, const QString &value,
                const QString &encoding) -> bool {
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
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
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto buffer = reinterpret_cast<char *>(&value);
                    return doc->replace(offset,
                                        QByteArray(buffer, sizeof(qint64)));
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::writeString, _win,
            [=](qsizetype offset, const QString &value,
                const QString &encoding) -> bool {
                auto e = pluginCurrentEditor(plg);
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

    connect(pctl, &WingPlugin::Controller::appendInt8, _win,
            [=](qint8 value) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    auto buffer = reinterpret_cast<char *>(&value);
                    QByteArray data(buffer, sizeof(qint8));
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendInt16, _win,
            [=](qint16 value) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    auto buffer = reinterpret_cast<char *>(&value);
                    QByteArray data(buffer, sizeof(qint16));
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendInt32, _win,
            [=](qint32 value) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    auto buffer = reinterpret_cast<char *>(&value);
                    QByteArray data(buffer, sizeof(qint32));
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendInt64, _win,
            [=](qint64 value) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    auto offset = doc->length();
                    auto buffer = reinterpret_cast<char *>(&value);
                    QByteArray data(buffer, sizeof(qint64));
                    return doc->insert(offset, data);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::appendString, _win,
            [=](const QString &value, const QString &encoding) -> bool {
                auto e = pluginCurrentEditor(plg);
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

    connect(pctl, &WingPlugin::Controller::remove, _win,
            [=](qsizetype offset, qsizetype len) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    return doc->remove(offset, len);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::removeAll, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            auto len = doc->length();
            return doc->remove(0, len);
        }
        return false;
    });
    connect(pctl,
            QOverload<qsizetype, qsizetype, int>::of(
                &WingPlugin::Controller::moveTo),
            _win,
            [=](qsizetype line, qsizetype column, int nibbleindex) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->cursor()->moveTo(line, column, nibbleindex);
                    return true;
                }
                return false;
            });
    connect(pctl, QOverload<qsizetype>::of(&WingPlugin::Controller::moveTo),
            _win, [=](qsizetype offset) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->cursor()->moveTo(offset);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::select, _win,
            [=](qsizetype offset, qsizetype length) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->cursor()->setSelection(offset, length);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setInsertionMode, _win,
            [=](bool isinsert) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    e->hexEditor()->cursor()->setInsertionMode(
                        isinsert ? QHexCursor::InsertMode
                                 : QHexCursor::OverwriteMode);
                    return true;
                }
                return false;
            });
    connect(pctl,
            QOverload<qsizetype, qsizetype, const QColor &, const QColor &,
                      const QString &>::of(&WingPlugin::Controller::metadata),
            _win,
            [=](qsizetype begin, qsizetype end, const QColor &fgcolor,
                const QColor &bgcolor, const QString &comment) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    if (!doc->isKeepSize())
                        return false;
                    doc->metadata()->metadata(begin, end, fgcolor, bgcolor,
                                              comment);
                    doc->setDocSaved(false);
                    return true;
                }
                return false;
            });
    connect(pctl,
            QOverload<qsizetype, qsizetype, qsizetype, const QColor &,
                      const QColor &,
                      const QString &>::of(&WingPlugin::Controller::metadata),
            _win,
            [=](qsizetype line, qsizetype start, qsizetype length,
                const QColor &fgcolor, const QColor &bgcolor,
                const QString &comment) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    if (!doc->isKeepSize())
                        return false;
                    doc->metadata()->metadata(line, start, length, fgcolor,
                                              bgcolor, comment);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::removeMetadata, _win,
            [=](qsizetype offset) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->metadata()->removeMetadata(offset);
                    doc->setDocSaved(false);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::clearMetadata, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            if (!doc->isKeepSize())
                return false;
            doc->metadata()->clear();
            return true;
        }
        return false;
    });
    connect(pctl, &WingPlugin::Controller::comment, _win,
            [=](qsizetype line, qsizetype start, qsizetype length,
                const QString &comment) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    if (!doc->isKeepSize())
                        return false;
                    doc->metadata()->comment(line, start, length, comment);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::foreground, _win,
            [=](qsizetype line, qsizetype start, qsizetype length,
                const QColor &fgcolor) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    if (!doc->isKeepSize())
                        return false;
                    doc->metadata()->foreground(line, start, length, fgcolor);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::background, _win,
            [=](qsizetype line, qsizetype start, qsizetype length,
                const QColor &bgcolor) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    if (!doc->isKeepSize())
                        return false;
                    doc->metadata()->background(line, start, length, bgcolor);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setMetaVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetafgVisible(b);
                    doc->setMetabgVisible(b);
                    doc->setMetaCommentVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setMetafgVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetafgVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setMetabgVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetabgVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setMetaCommentVisible, _win,
            [=](bool b) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    doc->setMetaCommentVisible(b);
                    return true;
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::setCurrentEncoding, _win,
            [=](const QString &encoding) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    return e->hexEditor()->renderer()->setEncoding(encoding);
                }
                return false;
            });

    connect(pctl, &WingPlugin::Controller::addBookMark, _win,
            [=](qsizetype pos, const QString &comment) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    return doc->addBookMark(pos, comment);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::modBookMark, _win,
            [=](qsizetype pos, const QString &comment) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    return doc->modBookMark(pos, comment);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::removeBookMark, _win,
            [=](qsizetype pos) -> bool {
                auto e = pluginCurrentEditor(plg);
                if (e) {
                    auto doc = e->hexEditor()->document();
                    return doc->removeBookMark(pos);
                }
                return false;
            });
    connect(pctl, &WingPlugin::Controller::clearBookMark, _win, [=]() -> bool {
        auto e = pluginCurrentEditor(plg);
        if (e) {
            auto doc = e->hexEditor()->document();
            return doc->clearBookMark();
        }
        return false;
    });

    // mainwindow
    connect(pctl, &WingPlugin::Controller::newFile, _win, [=]() -> bool {
        m_plgviewMap[plg] = _win->newfileGUI();
        return true;
    });
    connect(pctl, &WingPlugin::Controller::openWorkSpace, _win,
            [=](const QString &filename) -> bool {
                EditorView *view = nullptr;
                auto ret = _win->openWorkSpace(filename, &view);
                m_plgviewMap[plg] = view;
                return ret == ErrFile::Success;
            });
    connect(pctl, &WingPlugin::Controller::openFile, _win,
            [=](const QString &filename) -> ErrFile {
                EditorView *view = nullptr;
                auto ret = _win->openFile(filename, &view);
                m_plgviewMap[plg] = view;
                return ret;
            });
    connect(pctl, &WingPlugin::Controller::openRegionFile, _win,
            [=](const QString &filename, qsizetype start,
                qsizetype length) -> ErrFile {
                EditorView *view = nullptr;
                auto ret = _win->openRegionFile(filename, &view, start, length);
                m_plgviewMap[plg] = view;
                return ret;
            });
    connect(pctl, &WingPlugin::Controller::openDriver, _win,
            [=](const QString &driver) -> ErrFile {
                EditorView *view = nullptr;
                auto ret = _win->openDriver(driver, &view);
                m_plgviewMap[plg] = view;
                return ret;
            });
    connect(pctl, &WingPlugin::Controller::closeFile, _win,
            [=](const QString &filename, bool force) -> ErrFile {
                // return closeFile(index, force);
                return ErrFile::Success;
            });
    connect(pctl, &WingPlugin::Controller::saveFile, _win,
            [=](const QString &filename, bool ignoreMd5) -> ErrFile {
                // return save(index, ignoreMd5);
                return ErrFile::Success;
            });
    connect(pctl, &WingPlugin::Controller::exportFile, _win,
            [=](const QString &filename, const QString &savename,
                bool ignoreMd5) -> ErrFile {
                // return exportFile(filename, index, ignoreMd5);
                return ErrFile::Success;
            });

    connect(pctl, &WingPlugin::Controller::exportFileGUI, _win,
            &MainWindow::on_exportfile);

    connect(pctl, &WingPlugin::Controller::saveAsFile, _win,
            [=](const QString &filename, const QString &savename,
                bool ignoreMd5) -> ErrFile {
                // return saveAs(filename, index, ignoreMd5);
                return ErrFile::Success;
            });

    connect(pctl, &WingPlugin::Controller::saveAsFileGUI, _win,
            &MainWindow::on_saveas);

    connect(pctl, &WingPlugin::Controller::closeCurrentFile, _win,
            [=](bool force) -> ErrFile {
                // return closeFile(_pcurfile, force);
                return ErrFile::Success;
            });
    connect(pctl, &WingPlugin::Controller::saveCurrentFile, _win,
            [=](bool ignoreMd5) -> ErrFile {
                // return save(_pcurfile, force);
                return ErrFile::Success;
            });

    connect(pctl, &WingPlugin::Controller::openFileGUI, _win,
            &MainWindow::on_openfile);
    connect(pctl, &WingPlugin::Controller::openRegionFileGUI, _win,
            &MainWindow::on_openregion);
    connect(pctl, &WingPlugin::Controller::openDriverGUI, _win,
            &MainWindow::on_opendriver);
    connect(pctl, &WingPlugin::Controller::gotoGUI, _win,
            &MainWindow::on_gotoline);
    connect(pctl, &WingPlugin::Controller::findGUI, _win,
            &MainWindow::on_findfile);
    connect(pctl, &WingPlugin::Controller::fillGUI, _win, &MainWindow::on_fill);
    connect(pctl, &WingPlugin::Controller::fillZeroGUI, _win,
            &MainWindow::on_fillzero);
}

void PluginSystem::connectUIInterface(IWingPlugin *plg) {
    auto msgbox = &plg->msgbox;
    connect(msgbox, &WingPlugin::MessageBox::aboutQt, _win,
            [this](QWidget *parent, const QString &title) -> void {
                if (checkThreadAff()) {
                    WingMessageBox::aboutQt(parent, title);
                }
            });
    connect(msgbox, &WingPlugin::MessageBox::information, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::information(parent, title, text,
                                                       buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::question, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::question(parent, title, text,
                                                    buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::warning, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::warning(parent, title, text, buttons,
                                                   defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::critical, _win,
            [=](QWidget *parent, const QString &title, const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::critical(parent, title, text,
                                                    buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });
    connect(msgbox, &WingPlugin::MessageBox::about, _win,
            [=](QWidget *parent, const QString &title,
                const QString &text) -> void {
                if (checkThreadAff()) {
                    WingMessageBox::about(parent, title, text);
                }
            });
    connect(msgbox, &WingPlugin::MessageBox::msgbox, _win,
            [=](QWidget *parent, QMessageBox::Icon icon, const QString &title,
                const QString &text, QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton defaultButton)
                -> QMessageBox::StandardButton {
                if (checkThreadAff()) {
                    return WingMessageBox::msgbox(parent, icon, title, text,
                                                  buttons, defaultButton);
                }
                return QMessageBox::StandardButton::NoButton;
            });

    auto inputbox = &plg->inputbox;
    connect(inputbox, &WingPlugin::InputBox::getText, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                QLineEdit::EchoMode echo, const QString &text, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getText(parent, title, label, echo,
                                                    text, ok, inputMethodHints);
                }
                return {};
            });
    connect(inputbox, &WingPlugin::InputBox::getMultiLineText, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                const QString &text, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getMultiLineText(
                        parent, title, label, text, ok, inputMethodHints);
                }
                return {};
            });
    connect(inputbox, &WingPlugin::InputBox::getItem, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                const QStringList &items, int current, bool editable, bool *ok,
                Qt::InputMethodHints inputMethodHints) -> QString {
                if (checkThreadAff()) {
                    return WingInputDialog::getItem(parent, title, label, items,
                                                    current, editable, ok,
                                                    inputMethodHints);
                }
                return {};
            });
    connect(
        inputbox, &WingPlugin::InputBox::getInt, _win,
        [=](QWidget *parent, const QString &title, const QString &label,
            int value, int minValue, int maxValue, int step, bool *ok) -> int {
            if (checkThreadAff()) {
                return WingInputDialog::getInt(parent, title, label, value,
                                               minValue, maxValue, step, ok);
            }
            return 0;
        });
    connect(inputbox, &WingPlugin::InputBox::getDouble, _win,
            [=](QWidget *parent, const QString &title, const QString &label,
                double value, double minValue, double maxValue, int decimals,
                bool *ok, double step) -> double {
                if (checkThreadAff()) {
                    return WingInputDialog::getDouble(parent, title, label,
                                                      value, minValue, maxValue,
                                                      decimals, ok, step);
                }
                return qQNaN();
            });

    auto filedlg = &plg->filedlg;
    connect(filedlg, &WingPlugin::FileDialog::getExistingDirectory, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                QFileDialog::Options options) -> QString {
                if (checkThreadAff()) {
                    return WingFileDialog::getExistingDirectory(parent, caption,
                                                                dir, options);
                }
                return {};
            });
    connect(filedlg, &WingPlugin::FileDialog::getOpenFileName, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                const QString &filter, QString *selectedFilter,
                QFileDialog::Options options) -> QString {
                if (checkThreadAff()) {
                    return WingFileDialog::getOpenFileName(
                        parent, caption, dir, filter, selectedFilter, options);
                }
                return {};
            });
    connect(filedlg, &WingPlugin::FileDialog::getOpenFileNames, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                const QString &filter, QString *selectedFilter,
                QFileDialog::Options options) -> QStringList {
                if (checkThreadAff()) {
                    return WingFileDialog::getOpenFileNames(
                        parent, caption, dir, filter, selectedFilter, options);
                }
                return {};
            });
    connect(filedlg, &WingPlugin::FileDialog::getSaveFileName, _win,
            [=](QWidget *parent, const QString &caption, const QString &dir,
                const QString &filter, QString *selectedFilter,
                QFileDialog::Options options) -> QString {
                if (checkThreadAff()) {
                    return WingFileDialog::getSaveFileName(
                        parent, caption, dir, filter, selectedFilter, options);
                }
                return {};
            });

    auto colordlg = &plg->colordlg;
    connect(colordlg, &WingPlugin::ColorDialog::getColor, _win,
            [=](const QString &caption, QWidget *parent) -> QColor {
                if (checkThreadAff()) {
                    ColorPickerDialog d;
                    if (d.exec()) {
                        return d.color();
                    }
                }
                return {};
            });

    auto visual = &plg->visual;
    connect(visual, &WingPlugin::DataVisual::updateText, _win->m_infotxt,
            &QTextBrowser::setText);
    connect(visual, &WingPlugin::DataVisual::updateTextList, _win,
            [=](const QStringList &data) {
                auto oldmodel = _win->m_infolist->model();
                if (oldmodel) {
                    oldmodel->deleteLater();
                }
                auto model = new QStringListModel(data);
                _win->m_infolist->setModel(model);
            });
    connect(visual, &WingPlugin::DataVisual::updateTextTree, _win,
            [=](const QString &json) {
                auto oldmodel = _win->m_infotree->model();
                if (oldmodel) {
                    oldmodel->deleteLater();
                }
                auto model = new QJsonModel(json.toUtf8());
                _win->m_infotree->setModel(model);
            });
    connect(visual, &WingPlugin::DataVisual::updateTextTable, _win,
            [=](const QString &json, const QStringList &headers,
                const QStringList &headerNames) {
                auto oldmodel = _win->m_infotable->model();
                if (oldmodel) {
                    oldmodel->deleteLater();
                }

                auto model = new QJsonTableModel({});
                model->setJson(QJsonDocument::fromJson(json.toUtf8()));
                _win->m_infotable->setModel(model);
            });
}

bool PluginSystem::checkThreadAff() {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(
            tr("Creating UI widget is not allowed in non-UI thread"));
        return false;
    }
    return true;
}

QString PluginSystem::packLogMessage(const char *header, const QString &msg) {
    return QStringLiteral("{") + header + QStringLiteral("} ") + msg;
}

PluginSystem &PluginSystem::instance() {
    static PluginSystem ins;
    return ins;
}

void PluginSystem::setMainWindow(MainWindow *win) { _win = win; }

QWidget *PluginSystem::mainWindow() const { return _win; }

void PluginSystem::LoadPlugin() {
    Q_ASSERT(_win);

    _angelplg = new WingAngelAPI;

    auto ret = loadPlugin(_angelplg);
    Q_ASSERT(ret);

    auto &set = SettingManager::instance();
    if (!set.enablePlugin()) {
        return;
    }
    if (Utilities::isRoot() && !set.enablePlgInRoot()) {
        return;
    }

#ifdef QT_DEBUG
    QDir plugindir(QCoreApplication::applicationDirPath() +
                   QStringLiteral("/plugin"));
    // 这是我的插件调试目录，如果调试插件，请更换路径
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

EditorView *PluginSystem::pluginCurrentEditor(IWingPlugin *sender) const {
    if (sender) {
        auto editor = m_plgviewMap.value(sender);
        if (editor) {
            return editor;
        } else {
            return _win->currentEditor();
        }
    }
    return nullptr;
}
