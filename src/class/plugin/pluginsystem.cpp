/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "pluginsystem.h"

#include <limits>
#include <utility>

#include <QDir>
#include <QFileInfoList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPluginLoader>
#include <QStringListModel>
#include <private/qmetaobject_p.h>

#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"

#include "class/application/applicationmode.h"
#include "class/application/logger.h"
#include "class/file/fileaccesscheck.h"
#include "class/file/workspacemanager.h"
#include "class/security/wingcert.h"
#include "class/settings/languagemanager.h"
#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "control/toast.h"
#include "define.h"
#include "dialog/colorpickerdialog.h"
#include "dialog/common/wingfiledialog.h"
#include "dialog/common/winginputdialog.h"
#include "dialog/common/wingmessagebox.h"
#include "dialog/framelessdialogbase.h"
#include "dialog/mainwindow.h"
#include "generated_bindings.hpp"
#include "luau/luauutil.h"
#include "luau/runtime/scriptdocumentregistry.h"
#include "luau/runtime/scriptmachine.h"
#include "luau/runtime/scriptserviceapi.h"

int PluginSystem::DocumentHandlePool::acquire() {
    if (!m_releasedIds.isEmpty()) {
        return m_releasedIds.dequeue();
    }
    if (m_nextId == std::numeric_limits<int>::max()) {
        return -1;
    }
    return m_nextId++;
}

void PluginSystem::DocumentHandlePool::release(int handle) {
    if (handle >= 0) {
        Q_ASSERT(handle < m_nextId);
        m_releasedIds.enqueue(handle);
    }
}

PluginSystem::PluginFileContext::PluginFileContext(
    QSharedPointer<DocumentHandlePool> pool)
    : fid(pool->acquire()), m_handlePool(std::move(pool)) {}

PluginSystem::PluginFileContext::~PluginFileContext() {
    m_handlePool->release(fid);
}

PluginSystem::PluginSystem() : QObject() {
    auto mobj = PluginSystem::metaObject();
    auto total = mobj->methodCount();

    // all public slots
    for (int i = mobj->methodOffset(); i < total; ++i) {
        auto m = mobj->method(i);
        if (m.methodType() == QMetaMethod::Slot &&
            m.access() == QMetaMethod::Public) {
            auto total = m.parameterCount();
            if (total < 1 || m.parameterMetaType(0) !=
                                 QMetaType::fromType<const QObject *>()) {
                continue;
            }

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
            WingHex::FunctionSig msig;
            msig.fnName = m.nameView();
            msig.types.fill(0);
            msig.typesCount = total - 1;
            Q_ASSERT(msig.types.size() >= msig.typesCount);

            for (int i = 1; i < total; ++i) {
                auto mt = m.parameterType(i);
                msig.types[i - 1] = mt;
            }
#else
            WingHex::FunctionSig msig;
            msig.fnName = m.name();
            msig.types.reserve(total - 1);

            for (int i = 1; i < total; ++i) {
                auto mt = m.parameterType(i);
                msig.types.append(mt);
            }
#endif
            m_api._fnTable.insert(msig, m);
        }
    }

    m_api._fnCaller = this;

#ifdef Q_OS_WIN
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
                             LOAD_LIBRARY_SEARCH_USER_DIRS);
#endif
}

PluginSystem::~PluginSystem() {}

// =========================================================================

void PluginSystem::toast(const QObject *sender, const QPixmap &icon,
                         const QString &message) {
    if (ApplicationMode::headless()) {
        return;
    }
    if (!checkThreadAff()) {
        return;
    }

    if (message.isEmpty()) {
        return;
    }

    QWidget *parent = m_win;
    if (!parent) {
        parent = QApplication::activeWindow();
    }
    Toast::toast(parent, icon, message);
}

void PluginSystem::logTrace(const QObject *sender, const QString &message) {
    Logger::trace(packLogMessage(sender->metaObject()->className(), message));
}

void PluginSystem::logDebug(const QObject *sender, const QString &message) {
    Logger::debug(packLogMessage(sender->metaObject()->className(), message));
}

void PluginSystem::logInfo(const QObject *sender, const QString &message) {
    Logger::info(packLogMessage(sender->metaObject()->className(), message));
}

void PluginSystem::logError(const QObject *sender, const QString &message) {
    Logger::critical(
        packLogMessage(sender->metaObject()->className(), message));
}

bool PluginSystem::raiseDockWidget(const QObject *sender, QWidget *w) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (!checkThreadAff()) {
        return false;
    }

    if (m_raisedw.contains(w)) {
        m_raisedw.value(w)->raise();
        return true;
    }
    return false;
}

QDialog *PluginSystem::createDialog(const QObject *sender, QWidget *content) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (!checkThreadAff()) {
        return nullptr;
    }

    if (content) {
        auto d = new FramelessDialogBase;
        d->buildUpContent(content);
        d->setWindowTitle(content->windowTitle());
        d->setWindowIcon(content->windowIcon());
        return d;
    } else {
        return nullptr;
    }
}

QMainWindow *PluginSystem::createMainWindow(const QObject *sender,
                                            QWidget *content) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (!checkThreadAff()) {
        return nullptr;
    }

    if (content) {
        auto d = new FramelessMainWindow;
        d->buildUpContent(content);
        d->setWindowTitle(content->windowTitle());
        d->setWindowIcon(content->windowIcon());
        return d;
    } else {
        return nullptr;
    }
}

void PluginSystem::msgAboutQt(const QObject *sender, QWidget *parent,
                              const QString &title) {
    if (ApplicationMode::headless()) {
        return;
    }

    if (checkThreadAff()) {
        WingMessageBox::aboutQt(parent, title);
    }
}

QMessageBox::StandardButton
PluginSystem::msgInformation(const QObject *sender, QWidget *parent,
                             const QString &title, const QString &text,
                             QMessageBox::StandardButtons buttons,
                             QMessageBox::StandardButton defaultButton) {
    if (ApplicationMode::headless()) {
        return QMessageBox::NoButton;
    }

    if (checkThreadAff()) {
        return WingMessageBox::information(parent, title, text, buttons,
                                           defaultButton);
    }
    return QMessageBox::StandardButton::NoButton;
}

QMessageBox::StandardButton
PluginSystem::msgQuestion(const QObject *sender, QWidget *parent,
                          const QString &title, const QString &text,
                          QMessageBox::StandardButtons buttons,
                          QMessageBox::StandardButton defaultButton) {
    if (ApplicationMode::headless()) {
        return QMessageBox::NoButton;
    }

    if (checkThreadAff()) {
        return WingMessageBox::question(parent, title, text, buttons,
                                        defaultButton);
    }
    return QMessageBox::StandardButton::NoButton;
}

QMessageBox::StandardButton
PluginSystem::msgWarning(const QObject *sender, QWidget *parent,
                         const QString &title, const QString &text,
                         QMessageBox::StandardButtons buttons,
                         QMessageBox::StandardButton defaultButton) {
    if (ApplicationMode::headless()) {
        return QMessageBox::NoButton;
    }

    if (checkThreadAff()) {
        return WingMessageBox::warning(parent, title, text, buttons,
                                       defaultButton);
    }
    return QMessageBox::StandardButton::NoButton;
}

QMessageBox::StandardButton
PluginSystem::msgCritical(const QObject *sender, QWidget *parent,
                          const QString &title, const QString &text,
                          QMessageBox::StandardButtons buttons,
                          QMessageBox::StandardButton defaultButton) {
    if (ApplicationMode::headless()) {
        return QMessageBox::NoButton;
    }

    if (checkThreadAff()) {
        return WingMessageBox::critical(parent, title, text, buttons,
                                        defaultButton);
    }
    return QMessageBox::StandardButton::NoButton;
}

void PluginSystem::msgAbout(const QObject *sender, QWidget *parent,
                            const QString &title, const QString &text) {
    if (ApplicationMode::headless()) {
        return;
    }

    if (checkThreadAff()) {
        WingMessageBox::about(parent, title, text);
    }
}

QMessageBox::StandardButton
PluginSystem::msgbox(const QObject *sender, QWidget *parent,
                     QMessageBox::Icon icon, const QString &title,
                     const QString &text, QMessageBox::StandardButtons buttons,
                     QMessageBox::StandardButton defaultButton) {
    if (ApplicationMode::headless()) {
        return QMessageBox::NoButton;
    }

    if (checkThreadAff()) {
        return WingMessageBox::msgbox(parent, icon, title, text, buttons,
                                      defaultButton);
    }
    return QMessageBox::StandardButton::NoButton;
}

QString PluginSystem::dlgGetText(const QObject *sender, QWidget *parent,
                                 const QString &title, const QString &label,
                                 QLineEdit::EchoMode echo, const QString &text,
                                 bool *ok,
                                 Qt::InputMethodHints inputMethodHints) {
    if (ApplicationMode::headless()) {
        if (ok) {
            *ok = false;
        }
        return {};
    }

    if (checkThreadAff()) {
        return WingInputDialog::getText(parent, title, label, echo, text, ok,
                                        inputMethodHints);
    }
    return {};
}

QString
PluginSystem::dlgGetMultiLineText(const QObject *sender, QWidget *parent,
                                  const QString &title, const QString &label,
                                  const QString &text, bool *ok,
                                  Qt::InputMethodHints inputMethodHints) {
    if (ApplicationMode::headless()) {
        if (ok) {
            *ok = false;
        }
        return {};
    }

    if (checkThreadAff()) {
        return WingInputDialog::getMultiLineText(parent, title, label, text, ok,
                                                 inputMethodHints);
    }
    return {};
}

QString PluginSystem::dlgGetItem(const QObject *sender, QWidget *parent,
                                 const QString &title, const QString &label,
                                 const QStringList &items, int current,
                                 bool editable, bool *ok,
                                 Qt::InputMethodHints inputMethodHints) {
    if (ApplicationMode::headless()) {
        if (ok) {
            *ok = false;
        }
        return {};
    }

    if (checkThreadAff()) {
        return WingInputDialog::getItem(parent, title, label, items, current,
                                        editable, ok, inputMethodHints);
    }
    return {};
}

int PluginSystem::dlgGetInt(const QObject *sender, QWidget *parent,
                            const QString &title, const QString &label,
                            int value, int minValue, int maxValue, int step,
                            bool *ok) {
    if (ApplicationMode::headless()) {
        if (ok) {
            *ok = false;
        }
        return {};
    }

    if (checkThreadAff()) {
        return WingInputDialog::getInt(parent, title, label, value, minValue,
                                       maxValue, step, ok);
    }
    return 0;
}

double PluginSystem::dlgGetDouble(const QObject *sender, QWidget *parent,
                                  const QString &title, const QString &label,
                                  double value, double minValue,
                                  double maxValue, int decimals, bool *ok,
                                  double step) {
    if (ApplicationMode::headless()) {
        if (ok) {
            *ok = false;
        }
        return {};
    }

    if (checkThreadAff()) {
        return WingInputDialog::getDouble(parent, title, label, value, minValue,
                                          maxValue, decimals, ok, step);
    }
    return qQNaN();
}

QString PluginSystem::dlgGetExistingDirectory(const QObject *sender,
                                              QWidget *parent,
                                              const QString &caption,
                                              const QString &dir,
                                              QFileDialog::Options options) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (checkThreadAff()) {
        return WingFileDialog::getExistingDirectory(parent, caption, dir,
                                                    options);
    }
    return {};
}

QString PluginSystem::dlgGetOpenFileName(const QObject *sender, QWidget *parent,
                                         const QString &caption,
                                         const QString &dir,
                                         const QString &filter,
                                         QString *selectedFilter,
                                         QFileDialog::Options options) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (checkThreadAff()) {
        return WingFileDialog::getOpenFileName(parent, caption, dir, filter,
                                               selectedFilter, options);
    }
    return {};
}

QStringList PluginSystem::dlgGetOpenFileNames(
    const QObject *sender, QWidget *parent, const QString &caption,
    const QString &dir, const QString &filter, QString *selectedFilter,
    QFileDialog::Options options) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (checkThreadAff()) {
        return WingFileDialog::getOpenFileNames(parent, caption, dir, filter,
                                                selectedFilter, options);
    }
    return {};
}

QString PluginSystem::dlgGetSaveFileName(const QObject *sender, QWidget *parent,
                                         const QString &caption,
                                         const QString &dir,
                                         const QString &filter,
                                         QString *selectedFilter,
                                         QFileDialog::Options options) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (checkThreadAff()) {
        return WingFileDialog::getSaveFileName(parent, caption, dir, filter,
                                               selectedFilter, options);
    }
    return {};
}

QColor PluginSystem::dlgGetColor(const QObject *sender, const QString &caption,
                                 QWidget *parent) {
    if (ApplicationMode::headless()) {
        return {};
    }

    if (checkThreadAff()) {
        ColorPickerDialog d(parent);
        d.setWindowTitle(caption);
        if (d.exec()) {
            return d.color();
        }
    }
    return {};
}

bool PluginSystem::existsServiceHost(const QObject *sender,
                                     const QString &puid) {
    return std::find_if(m_pinfos.keyBegin(), m_pinfos.keyEnd(),
                        [=](IWingPluginBase *plg) {
                            return puid.compare(getPUID(plg),
                                                Qt::CaseInsensitive) == 0;
                        }) != m_pinfos.keyEnd();
}

bool PluginSystem::invokeServiceImpl(const QObject *sender, const QString &puid,
                                     const WingHex::MetaCallInfo &infos) {

    QObject *obj = nullptr;
    QString rpuid;
    auto question = puid.indexOf('?');

    enum { None, MustPlugin, MustExt } state = None;

    if (question >= 0) {
        rpuid = puid.sliced(0, question);
        auto equest = puid.sliced(question + 1);
        if (equest.compare(QStringLiteral("[PLG]"))) {
            state = MustPlugin;
        } else if (equest.compare(QStringLiteral("[EXT]"))) {
            state = MustExt;
        } else {
            // warn and ignored
            qWarning().noquote()
                << QStringLiteral("[PluginSystem::invokeServiceImpl] Cannot "
                                  "parsing filter: '%1'")
                       .arg(equest);
        }
    } else {
        rpuid = puid;
        state = None;
    }

    static Cache<QString, IWingPluginBase *> cache(10);
    IWingPluginBase *rc = nullptr;
    if (!m_unloading && cache.contains(rpuid)) {
        rc = cache.object(rpuid);
    } else {
        auto r = std::find_if(
            m_pinfos.keyBegin(), m_pinfos.keyEnd(), [=](IWingPluginBase *plg) {
                return rpuid.compare(getPUID(plg), Qt::CaseInsensitive) == 0;
            });

        if (r == m_pinfos.keyEnd()) {
            return false;
        }
        rc = *r;
        cache.insert(rpuid, rc);
    }

    switch (state) {
    case None:
        obj = rc;
        break;
    case MustPlugin:
        if (qobject_cast<IWingPlugin *>(rc)) {
            obj = rc;
        }
        break;
    case MustExt:
        if (qobject_cast<IWingDevice *>(rc)) {
            obj = rc;
        }
        break;
    }

    if (obj == nullptr) {
        qCritical("[PluginSystem::invokeServiceImpl] Null caller");
        return false;
    }

    auto meta = obj->metaObject();

    Qt::ConnectionType c;
    qsizetype paramCount;
    const char *method;
    const void *const *parameters;
    const char *const *typeNames;
    const QtPrivate::QMetaTypeInterface *const *metaTypes;
    std::tie(method, c, paramCount, parameters, typeNames, metaTypes) = infos;

    if (parameters == nullptr || typeNames == nullptr || metaTypes == nullptr) {
        qCritical("[PluginSystem::invokeServiceImpl] Invalid calling info");
        return false;
    }

    QMetaMethod m;
    // retrive method
    auto len = meta->methodCount();
    for (int i = meta->methodOffset(); i < len; ++i) {
        auto met = meta->method(i);
        if (met.parameterCount() != paramCount) {
            continue;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        if (met.nameView() == method) {
#else
        if (met.name() == method) {
#endif
            bool err = false;
            for (int i = 1; i < paramCount; ++i) {
                if (metaTypes[i]->typeId == QMetaType::UnknownType) {
                    // user defined meta type
                    if (met.parameterTypeName(i).compare(metaTypes[i]->name)) {
                        err = true;
                        break;
                    }
                } else {
                    if (metaTypes[i]->typeId != met.parameterType(i)) {
                        err = true;
                        break;
                    }
                }
            }
            if (err) {
                continue;
            }
            m = met;
            break;
        }
    }

    if (!m.isValid()) {
        QByteArrayList args;
        args.reserve(len);
        for (int i = 1; i < paramCount; ++i) {
            args.append(metaTypes[i]->name);
        }
        QByteArray r = method + QByteArrayLiteral("(") + args.join(',') +
                       QByteArrayLiteral(")");
        qCritical("[PluginSystem::invokeServiceImpl] Invalid MetaMethod "
                  "(Not-Found) - %s",
                  r.constData());
        return false;
    }

    SenderInfo info;
    info.plgcls = QString::fromUtf8(meta->className());
    if (auto p =
            qobject_cast<IWingPluginBase *>(const_cast<QObject *>(sender))) {
        info.puid = getPUID(p);
    }

    constexpr auto metaName = "WINGHEX_META";
    // property first
    auto var = sender->property(metaName);
    if (var.isValid()) {
        info.meta = var;
    } else {
        auto iidx = meta->indexOfClassInfo(metaName);
        if (iidx >= 0) {
            info.meta = QString::fromUtf8(meta->classInfo(iidx).value());
        }
    }

    // prepend the 'SenderInfo' parameter
    // note: index 0 is return value, index 1 is the real first param
    auto data = QtPrivate::Invoke::dataHelper(info);
    auto tn = QtPrivate::Invoke::typenameHelper(info);
    auto inf = QtPrivate::Invoke::metaTypeHelper(info);

    auto nparamCount = paramCount + 1;
    QVarLengthArray<decltype(data), 16> nparameters;
    QVarLengthArray<decltype(tn), 16> ntypeNames;
    QVarLengthArray<decltype(inf), 16> nmetaTypes;
    nparameters.reserve(nparamCount);
    ntypeNames.reserve(nparamCount);
    nmetaTypes.reserve(nparamCount);

    nparameters.append(parameters, 1);
    nparameters.append(data);
    nparameters.append(parameters + 1, paramCount);

    ntypeNames.append(typeNames, 1);
    ntypeNames.append(tn);
    ntypeNames.append(typeNames + 1, paramCount);

    nmetaTypes.append(metaTypes, 1);
    nmetaTypes.append(inf);
    nmetaTypes.append(metaTypes + 1, paramCount);

    auto ret = QMetaMethodInvoker::invokeImpl(
        m, obj, c, nparamCount, nparameters.data(), ntypeNames.data(),
        nmetaTypes.data());

    static auto cstr =
        [](QMetaMethodInvoker::InvokeFailReason r) -> const char * {
        switch (r) {
        case QMetaMethodInvoker::InvokeFailReason::ReturnTypeMismatch:
            return "ReturnTypeMismatch";
        case QMetaMethodInvoker::InvokeFailReason::DeadLockDetected:
            return "DeadLockDetected";
        case QMetaMethodInvoker::InvokeFailReason::CallViaVirtualFailed:
            return "CallViaVirtualFailed";
        case QMetaMethodInvoker::InvokeFailReason::ConstructorCallOnObject:
            return "ConstructorCallOnObject";
        case QMetaMethodInvoker::InvokeFailReason::ConstructorCallWithoutResult:
            return "ConstructorCallWithoutResult";
        case QMetaMethodInvoker::InvokeFailReason::ConstructorCallFailed:
            return "ConstructorCallFailed";
        case QMetaMethodInvoker::InvokeFailReason::CouldNotQueueParameter:
            return "CouldNotQueueParameter";
        case QMetaMethodInvoker::InvokeFailReason::None:
            return "None";
        case QMetaMethodInvoker::InvokeFailReason::TooFewArguments:
            return "TooFewArguments";
        case QMetaMethodInvoker::InvokeFailReason::FormalParameterMismatch:
            return "FormalParameterMismatch";
            break;
        }
        return "";
    };

    if (ret != QMetaMethodInvoker::InvokeFailReason::None) {
        qCritical("[PluginSystem::invokeServiceImpl] MetaCall failed: %s (%d)",
                  cstr(ret), int(ret));
    }

    return ret == QMetaMethodInvoker::InvokeFailReason::None;
}

bool PluginSystem::isCurrentDocEditing(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    return pluginCurrentEditor(plg) != nullptr;
}

QUrl PluginSystem::currentDocFile(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentDocFile(this);
    }
    return {};
}

QString PluginSystem::currentDocFileName(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentDocFileName(this);
    }
    return {};
}

QUrl PluginSystem::currentDocWorkSpace(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentDocWorkSpace(this);
    }
    return {};
}

QString PluginSystem::currentDocWorkSpaceName(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentDocWorkSpaceName(this);
    }
    return {};
}

bool PluginSystem::isReadOnly(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return true;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->isReadOnly(this);
    }
    return true;
}

bool PluginSystem::isInsertionMode(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->isInsertionMode(this);
    }
    return false;
}

bool PluginSystem::isKeepSize(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return true;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->isKeepSize(this);
    }
    return true;
}

bool PluginSystem::isLocked(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return true;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->isLocked(this);
    }
    return true;
}

qsizetype PluginSystem::documentLines(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->documentLines(this);
    }
    return 0;
}

qsizetype PluginSystem::documentBytes(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->documentBytes(this);
    }
    return 0;
}

qsizetype PluginSystem::currentRow(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentRow(this);
    }
    return 0;
}

qsizetype PluginSystem::currentColumn(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentColumn(this);
    }
    return 0;
}

qsizetype PluginSystem::currentOffset(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentOffset(this);
    }
    return 0;
}

qsizetype PluginSystem::selectedLength(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->selectedLength(this);
    }
    return 0;
}

QByteArray PluginSystem::selectedBytes(const QObject *sender, qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->selectedBytes(this, index);
    }
    return {};
}

QByteArrayList PluginSystem::selectionBytes(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->selectionBytes(this);
    }
    return {};
}

qsizetype PluginSystem::selectionLength(const QObject *sender,
                                        qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->selectionLength(this, index);
    }
    return 0;
}

qsizetype PluginSystem::selectionCount(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->selectionCount(this);
    }
    return 0;
}

bool PluginSystem::stringVisible(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->stringVisible(this);
    }
    return false;
}

bool PluginSystem::addressVisible(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->addressVisible(this);
    }
    return false;
}

bool PluginSystem::headerVisible(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->headerVisible(this);
    }
    return false;
}

quintptr PluginSystem::addressBase(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->addressBase(this);
    }
    return 0;
}

bool PluginSystem::isModified(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return !e->isSaved();
    }
    return false;
}

qint8 PluginSystem::readInt8(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readInt8(this, offset);
    }
    return 0;
}

qint16 PluginSystem::readInt16(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readInt16(this, offset);
    }
    return 0;
}

qint32 PluginSystem::readInt32(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readInt32(this, offset);
    }
    return 0;
}

qint64 PluginSystem::readInt64(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readInt64(this, offset);
    }
    return 0;
}

quint8 PluginSystem::readUInt8(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readUInt8(this, offset);
    }
    return 0;
}

quint16 PluginSystem::readUInt16(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readUInt16(this, offset);
    }
    return 0;
}

quint32 PluginSystem::readUInt32(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readUInt32(this, offset);
    }
    return 0;
}

quint64 PluginSystem::readUInt64(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readUInt64(this, offset);
    }
    return 0;
}

float PluginSystem::readFloat(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return qQNaN();
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readFloat(this, offset);
    }
    return qQNaN();
}

double PluginSystem::readDouble(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return qQNaN();
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readDouble(this, offset);
    }
    return qQNaN();
}

QString PluginSystem::readString(const QObject *sender, qsizetype offset,
                                 const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readString(this, offset, encoding);
    }
    return {};
}

QByteArray PluginSystem::readBytes(const QObject *sender, qsizetype offset,
                                   qsizetype count) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->readBytes(this, offset, count);
    }
    return {};
}

qsizetype PluginSystem::findNext(const QObject *sender, qsizetype begin,
                                 const QByteArray &ba) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->findNext(this, begin, ba);
    }
    return -1;
}

qsizetype PluginSystem::findPrevious(const QObject *sender, qsizetype begin,
                                     const QByteArray &ba) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->findPrevious(this, begin, ba);
    }
    return -1;
}

qint64 PluginSystem::bookMarkCount(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->bookMarkCount(this);
    }
    return -1;
}

qint64 PluginSystem::bookMarkPos(const QObject *sender, qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->bookMarkPos(this, index);
    }
    return -1;
}

qint64 PluginSystem::metadataCount(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return -1;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->metadataCount(this);
    }
    return -1;
}

MetadataInfo PluginSystem::metadataInfoByIndex(const QObject *sender,
                                               qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {-1, -1, {}, {}, {}};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->metadataInfoByIndex(this, index);
    }
    return {-1, -1, {}, {}, {}};
}

MetadataInfo PluginSystem::metadataInfo(const QObject *sender,
                                        qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {-1, -1, {}, {}, {}};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->metadataInfo(this, offset);
    }
    return {-1, -1, {}, {}, {}};
}

QString PluginSystem::bookMarkComment(const QObject *sender, qsizetype pos) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->bookMarkComment(this, pos);
    }
    return {};
}

bool PluginSystem::existBookMark(const QObject *sender, qsizetype pos) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->existBookMark(this, pos);
    }
    return false;
}

bool PluginSystem::switchDocument(const QObject *sender, int handle) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (handle < 0) {
        m_plgviewMap[plg].currentFID = -1;
    } else {
        auto ctx = pluginContextById(plg, handle);
        if (ctx) {
            m_plgviewMap[plg].currentFID = handle;
        }
    }
    return true;
}

bool PluginSystem::raiseDocument(const QObject *sender, int handle) {
    if (!m_win) {
        return {};
    }

    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (handle < 0) {
        auto cur = m_win->m_curEditor;
        if (cur) {
            cur->raise();
            return true;
        }
    } else {
        auto ctx = pluginContextById(plg, handle);
        if (ctx) {
            ctx->view->raise();
            return true;
        }
    }
    return false;
}

bool PluginSystem::setLockedFile(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setLockedFile(this, b);
    }
    return false;
}

bool PluginSystem::setKeepSize(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setKeepSize(this, b);
    }
    return false;
}

bool PluginSystem::setStringVisible(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setStringVisible(this, b);
    }
    return false;
}

bool PluginSystem::setAddressVisible(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setAddressBase(this, b);
    }
    return false;
}

bool PluginSystem::setHeaderVisible(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setHeaderVisible(this, b);
    }
    return false;
}

bool PluginSystem::setAddressBase(const QObject *sender, quintptr base) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setAddressBase(this, base);
    }
    return false;
}

bool PluginSystem::beginMarco(const QObject *sender, const QString &txt) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto fid = m_plgviewMap[plg].currentFID;
    if (fid < 0) {
        return false;
    }

    auto r = pluginContextById(plg, fid);
    if (r) {
        auto &u = m_viewBindings[r->view];

        auto rtxt = txt.trimmed();
        if (rtxt.isEmpty()) {
            rtxt = m_pinfos.value(plg).id;
        }

        if (u.undoStackPlg.isEmpty()) {
            u.undoStackPlg.append(qMakePair(new QUndoCommand(rtxt), plg));
        } else {
            u.undoStackPlg.append(qMakePair(
                new QUndoCommand(rtxt, u.undoStackPlg.last().first), plg));
        }
    }
    return true;
}

bool PluginSystem::endMarco(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto fid = m_plgviewMap[plg].currentFID;
    if (fid < 0) {
        return false;
    }

    auto r = pluginContextById(plg, fid);
    if (r) {
        auto &u = m_viewBindings[r->view];
        auto &undo = u.undoStackPlg;

        if (undo.isEmpty()) {
            return false;
        } else {
            auto &l = undo.last();
            if (l.second != plg) {
                return false;
            }

            auto r = std::find_if(
                undo.rbegin(), undo.rend(),
                [plg](const QPair<QUndoCommand *, IWingPlugin *> &ctx) {
                    return ctx.second == plg;
                });

            if (r == undo.rend()) {
                return false;
            }

            if (r != undo.rbegin()) {
                qWarning("Detected an abnormally pushed undo command");
                undo.erase(r.base(), undo.end());
            }

            auto cmdl = undo.takeLast();
            if (undo.isEmpty()) {
                auto e = pluginCurrentEditor(plg);
                if (e == nullptr) {
                    return false;
                }
                auto doc = e->hexEditor()->document();
                doc->pushMakeUndo(cmdl.first);
                return true;
            }
            return true;
        }
    }
    return false;
}

bool PluginSystem::isMacroEmpty(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return true;
    }

    auto fid = m_plgviewMap[plg].currentFID;
    if (fid < 0) {
        return true;
    }

    auto r = pluginContextById(plg, fid);
    if (r) {
        auto &u = m_viewBindings[r->view];
        auto &undo = u.undoStackPlg;
        return undo.isEmpty();
    }

    return true;
}

bool PluginSystem::resetMarco(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto fid = m_plgviewMap[plg].currentFID;
    if (fid < 0) {
        return false;
    }

    auto r = pluginContextById(plg, fid);
    if (r) {
        auto &u = m_viewBindings[r->view];
        auto &undo = u.undoStackPlg;
        if (!undo.isEmpty()) {
            delete undo.first().first;
            undo.clear();
        }
        return true;
    }
    return false;
}

bool PluginSystem::writeInt8(const QObject *sender, qsizetype offset,
                             qint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeInt8(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeInt16(const QObject *sender, qsizetype offset,
                              qint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeInt16(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeInt32(const QObject *sender, qsizetype offset,
                              qint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeInt32(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeInt64(const QObject *sender, qsizetype offset,
                              qint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeInt64(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeUInt8(const QObject *sender, qsizetype offset,
                              quint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeUInt8(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeUInt16(const QObject *sender, qsizetype offset,
                               quint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeUInt16(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeUInt32(const QObject *sender, qsizetype offset,
                               quint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeUInt32(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeUInt64(const QObject *sender, qsizetype offset,
                               quint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeUInt64(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeFloat(const QObject *sender, qsizetype offset,
                              float value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeFloat(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeDouble(const QObject *sender, qsizetype offset,
                               double value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->writeDouble(this, offset, value);
    }
    return false;
}

bool PluginSystem::writeString(const QObject *sender, qsizetype offset,
                               const QString &value, const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        return e->writeString(this, offset, value, encoding);
    }
    return false;
}

bool PluginSystem::writeBytes(const QObject *sender, qsizetype offset,
                              const QByteArray &data) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        return e->writeBytes(this, offset, data);
    }
    return false;
}

bool PluginSystem::insertInt8(const QObject *sender, qsizetype offset,
                              qint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertInt8(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertInt16(const QObject *sender, qsizetype offset,
                               qint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertInt16(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertInt32(const QObject *sender, qsizetype offset,
                               qint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertInt32(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertInt64(const QObject *sender, qsizetype offset,
                               qint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertInt64(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertUInt8(const QObject *sender, qsizetype offset,
                               quint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertUInt8(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertUInt16(const QObject *sender, qsizetype offset,
                                quint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertUInt16(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertUInt32(const QObject *sender, qsizetype offset,
                                quint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertUInt32(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertUInt64(const QObject *sender, qsizetype offset,
                                quint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertUInt64(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertFloat(const QObject *sender, qsizetype offset,
                               float value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertFloat(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertDouble(const QObject *sender, qsizetype offset,
                                double value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->insertDouble(this, offset, value);
    }
    return false;
}

bool PluginSystem::insertString(const QObject *sender, qsizetype offset,
                                const QString &value, const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        return e->insertString(this, offset, value, encoding);
    }
    return false;
}

bool PluginSystem::insertBytes(const QObject *sender, qsizetype offset,
                               const QByteArray &data) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        return e->insertBytes(this, offset, data);
    }
    return false;
}

bool PluginSystem::appendInt8(const QObject *sender, qint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendInt8(this, value);
    }
    return false;
}

bool PluginSystem::appendInt16(const QObject *sender, qint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendInt16(this, value);
    }
    return false;
}

bool PluginSystem::appendInt32(const QObject *sender, qint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendInt32(this, value);
    }
    return false;
}

bool PluginSystem::appendInt64(const QObject *sender, qint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendInt64(this, value);
    }
    return false;
}

bool PluginSystem::appendUInt8(const QObject *sender, quint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendUInt8(this, value);
    }
    return false;
}

bool PluginSystem::appendUInt16(const QObject *sender, quint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendUInt16(this, value);
    }
    return false;
}

bool PluginSystem::appendUInt32(const QObject *sender, quint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendUInt32(this, value);
    }
    return false;
}

bool PluginSystem::appendUInt64(const QObject *sender, quint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }
    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendUInt64(this, value);
    }
    return false;
}

bool PluginSystem::appendFloat(const QObject *sender, float value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendFloat(this, value);
    }
    return false;
}

bool PluginSystem::appendDouble(const QObject *sender, double value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->appendDouble(this, value);
    }
    return false;
}

bool PluginSystem::appendString(const QObject *sender, const QString &value,
                                const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        return e->appendString(this, value, encoding);
    }
    return false;
}

bool PluginSystem::appendBytes(const QObject *sender, const QByteArray &data) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        return e->appendBytes(this, data);
    }
    return false;
}

bool PluginSystem::removeBytes(const QObject *sender, qsizetype offset,
                               qsizetype len) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        return e->removeBytes(this, offset, len);
    }
    return false;
}

bool PluginSystem::moveTo(const QObject *sender, qsizetype line,
                          qsizetype column, int nibbleindex,
                          bool clearSelection) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->moveTo(this, line, column, nibbleindex, clearSelection);
    }
    return false;
}

bool PluginSystem::moveTo(const QObject *sender, qsizetype offset,
                          bool clearSelection) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->moveTo(this, offset, clearSelection);
    }
    return false;
}

bool PluginSystem::select(const QObject *sender, qsizetype offset,
                          qsizetype length, WingHex::SelectionMode mode) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->select(this, offset, length, mode);
    }
    return false;
}

bool PluginSystem::setInsertionMode(const QObject *sender, bool isinsert) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setInsertionMode(this, isinsert);
    }
    return false;
}

bool PluginSystem::metadata(const QObject *sender, qsizetype begin,
                            qsizetype length, const QColor &fgcolor,
                            const QColor &bgcolor, const QString &comment) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->metadata(this, begin, length, fgcolor, bgcolor, comment);
    }
    return false;
}

bool PluginSystem::removeMetadata(const QObject *sender, qsizetype offset) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->removeMetadata(this, offset);
    }
    return false;
}

bool PluginSystem::clearMetadata(const QObject *sender) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->clearMetadata(this);
    }
    return false;
}

bool PluginSystem::setMetaVisible(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setMetaVisible(this, b);
    }
    return false;
}

bool PluginSystem::setMetafgVisible(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setMetafgVisible(this, b);
    }
    return false;
}

bool PluginSystem::setMetabgVisible(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setMetabgVisible(this, b);
    }
    return false;
}

bool PluginSystem::setMetaCommentVisible(const QObject *sender, bool b) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->setMetaCommentVisible(this, b);
    }
    return false;
}

bool PluginSystem::addBookMark(const QObject *sender, qsizetype pos,
                               const QString &comment) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->addBookMark(this, pos, comment);
    }
    return false;
}

bool PluginSystem::modBookMark(const QObject *sender, qsizetype pos,
                               const QString &comment) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->modBookMark(this, pos, comment);
    }
    return false;
}

bool PluginSystem::removeBookMark(const QObject *sender, qsizetype pos) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->removeBookMark(this, pos);
    }
    return false;
}

bool PluginSystem::clearBookMark(const QObject *sender) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->clearBookMark(this);
    }
    return false;
}

void PluginSystem::setSwitchingContext(bool newSwitchingContext) {
    m_switchingContext = newSwitchingContext;
}

bool PluginSystem::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto e = static_cast<QDynamicPropertyChangeEvent *>(event);
        // protect very important internal property
        // in any case, abort it to exit!!!
        if (e->propertyName() == QByteArrayLiteral("__PLG__")) {
            std::abort();
        }
        if (!m_switchingContext &&
            e->propertyName() == QByteArrayLiteral("__CONTEXT__")) {
            std::abort();
        }
    }
    return QObject::eventFilter(watched, event);
}

const QList<PluginSystem::BlockInfo> &PluginSystem::blockedDevPlugins() const {
    return m_blkdevs;
}

PluginSystem::DependencyMap PluginSystem::generatePluginsDepMap() const {
    const auto ltotal = m_loadedplgs.size();
    const auto btotal = m_blkplgs.size();
    const size_t total = ltotal + btotal;

    struct NodeInfo {
        QString id;
        QList<WingDependency> deps;
    };

    std::vector<NodeInfo> nodes;
    nodes.reserve(total);

    for (qsizetype i = 0; i < ltotal; ++i) {
        const auto &p = m_loadedplgs.at(i);
        const auto &info = m_pinfos.value(p);
        nodes.push_back({info.id, info.dependencies});
    }
    for (qsizetype i = 0; i < btotal; ++i) {
        const auto &info = m_blkplgs.at(i).info;
        nodes.push_back({info.id, info.dependencies});
    }

    QHash<QString, int> idToIndex;
    idToIndex.reserve(total);
    for (int i = 0; i < total; ++i) {
        idToIndex.insert(nodes[i].id, i);
    }

    DependencyMap ret;
    ret.host.resize(total);
    ret.dep.resize(total);

    for (int hi = 0; hi < total; ++hi) {
        const auto &deps = nodes[hi].deps;
        QList<int> &hostDeps = ret.host[hi];
        hostDeps.reserve(deps.size());

        for (const auto &dep : deps) {
            auto it = idToIndex.constFind(dep.puid);
            if (it != idToIndex.constEnd()) {
                const int pidx = it.value();
                hostDeps.append(pidx);
                ret.dep[pidx].append(hi);
            }
        }
    }

    return ret;
}

const QList<PluginSystem::BlockInfo> &PluginSystem::blockedPlugins() const {
    return m_blkplgs;
}

bool PluginSystem::doneRegisterScriptObj() {
    if (!checkThreadAff()) {
        return false;
    }

    auto &machine = ScriptMachine::instance();
    auto *engine = machine.scriptEngine();
    if (!machine.isInited() || !engine) {
        return false;
    }
    if (machine.scriptRegistrationFinished()) {
        return true;
    }

    QString currentPlugin;
    try {
        for (const auto &plugin : std::as_const(m_loadedplgs)) {
            currentPlugin = getPUID(plugin);
            plugin->onRegisterScriptObj(
                engine->global().beginNamespace(currentPlugin));
        }
        for (const auto &device : std::as_const(m_loadeddevs)) {
            currentPlugin = getPUID(device);
            device->onRegisterScriptObj(
                engine->global().beginNamespace(currentPlugin));
        }
        currentPlugin.clear();
        machine.finishScriptRegistration();
        return true;
    } catch (const WingScriptBind::ScriptException &error) {
        Logger::critical(tr("Script registration failed [%1]: %2")
                             .arg(currentPlugin, error.getErrorString()));
    } catch (const std::exception &error) {
        Logger::critical(
            tr("Script registration failed [%1]: %2")
                .arg(currentPlugin, QString::fromUtf8(error.what())));
    } catch (...) {
        Logger::critical(
            tr("Script registration failed [%1]").arg(currentPlugin));
    }
    machine.destoryMachine();
    return false;
}

IWingPlugin *PluginSystem::checkPluginAndReport(const QObject *sender,
                                                const char *func) {
    Q_ASSERT(func);
    auto p =
        qobject_cast<WingHex::IWingPlugin *>(const_cast<QObject *>(sender));
    if (p == nullptr) {
        qCritical("[EvilCall] Invalid sender called '%s'", func);
        return nullptr;
    }
    return p;
}

int PluginSystem::closeCurrent(const QObject *sender) {
    if (!m_win) {
        return ErrFile::Error;
    }

    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    auto handle = m_plgviewMap[plg].currentFID;
    auto view = closeHandle(plg, handle);
    if (view) {
        PluginSystem::instance().dispatchPluginFileClosedEvent(
            plg, m_win->getEditorViewFileType(view), view->fileNameUrl(),
            handle);
        return WingHex::ErrFile::Success;
    }
    return ErrFile::Error;
}

bool PluginSystem::isFileOpened(const QObject *sender, int handle) {
    auto *plugin = checkPluginAndReport(sender, __func__);
    if (!plugin || handle < 0) {
        return false;
    }
    const auto owner = m_plgviewMap.constFind(plugin);
    if (owner == m_plgviewMap.cend()) {
        return false;
    }
    return std::any_of(owner->contexts.cbegin(), owner->contexts.cend(),
                       [handle](const auto &context) {
                           return context->fid == handle && context->view &&
                                  !context->view->isClosed();
                       });
}

QList<QUrl> PluginSystem::openedFiles(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto r = m_plgviewMap.constFind(plg);
    if (r == m_plgviewMap.cend()) {
        return {};
    }

    QList<QUrl> files;
    for (const auto &context : r->contexts) {
        auto view = context->view.data();
        if (view->isClosed() || view->documentType() == DocumentType::InValid) {
            continue;
        }
        files.append(view->openedFileUrl());
    }
    return files;
}

QList<QUrl> PluginSystem::allOpenedFiles(const QObject *sender) {
    Q_UNUSED(sender);

    QList<QUrl> files;
    for (auto view : EditorView::instances()) {
        if (view->isClosed() || view->isCloneFile() ||
            view->documentType() == DocumentType::InValid) {
            continue;
        }
        files.append(view->openedFileUrl());
    }
    return files;
}

int PluginSystem::openCurrent(const QObject *sender) {
    if (!m_win) {
        return ErrFile::Error;
    }

    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    auto view = getCurrentPluginView(plg);
    if (view) {
        auto id = checkIDPluginAlreadyOpened(plg, view);
        if (id < 0) {
            id = assginHandleForOpenPluginView(plg, view);
            if (id < 0) {
                return id;
            }
            PluginSystem::instance().dispatchPluginFileOpenedEvent(
                plg, m_win->getEditorViewFileType(view), view->fileNameUrl(),
                id);
        }
        return id;
    }
    return ErrFile::Error;
}

int PluginSystem::closeDocument(EditorView *view, bool force) {
    if (!m_win) {
        return ErrFile::Error;
    }
    if (!view || view->isClosed()) {
        return ErrFile::NotExist;
    }
    if (!view->processWingEditorViewClosing()) {
        return ErrFile::Permission;
    }
    return m_win->closeEditor(view, force);
}

int PluginSystem::closeFile(const QObject *sender, int handle, bool force) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    auto context = pluginContextById(plg, handle);
    if (context == nullptr) {
        return ErrFile::NotExist;
    }
    return closeDocument(context->view, force);
}

PluginSystem::DocumentUrlType PluginSystem::documentUrlType(const QUrl &file) {
    if (!file.isValid()) {
        return DocumentUrlType::Invalid;
    }
    auto scheme = file.scheme();
    if (scheme == QLatin1String("wdrv")) {
        return DocumentUrlType::Device;
    }
    if (scheme == QLatin1String("file")) {
        return DocumentUrlType::File;
    }
    if (scheme == QLatin1String("wingpro")) {
        return DocumentUrlType::Workspace;
    }
    return DocumentUrlType::Invalid;
}

int PluginSystem::openDocument(const QUrl &file, EditorView **editor) {
    Q_ASSERT(editor);
    *editor = nullptr;
    if (!m_win) {
        return ErrFile::Error;
    }
    switch (documentUrlType(file)) {
    case DocumentUrlType::File:
        return m_win->openFile(file.toLocalFile(), editor);
    case DocumentUrlType::Device:
        return m_win->openExtFile(file.host(), file.path().sliced(1), editor);
    case DocumentUrlType::Workspace: {
        auto local = file;
        local.setScheme(QStringLiteral("file"));
        auto path = local.toLocalFile();
        return m_win->openWorkSpace(path, editor);
    }
    case DocumentUrlType::Invalid:
        return ErrFile::InvalidFormat;
    }
    Q_UNREACHABLE();
}

int PluginSystem::openFile(const QObject *sender, const QUrl &file) {
    if (!m_win) {
        return ErrFile::Error;
    }

    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }
    auto plg = checkPluginAndReport(sender, __func__);
    if (!plg) {
        return ErrFile::Error;
    }
    EditorView *view = nullptr;
    const auto result = openDocument(file, &view);
    if (result != ErrFile::Success && result != ErrFile::AlreadyOpened) {
        return result;
    }
    if (!view) {
        return result == ErrFile::AlreadyOpened ? result : ErrFile::Error;
    }
    auto id = checkIDPluginAlreadyOpened(plg, view);
    if (id >= 0) {
        return id;
    }
    const auto handle = assginHandleForOpenPluginView(plg, view, false);
    if (handle < 0) {
        if (result == ErrFile::Success) {
            m_win->closeEditor(view, true);
        }
        return handle;
    }
    dispatchPluginFileOpenedEvent(plg, m_win->getEditorViewFileType(view),
                                  view->fileNameUrl(), handle);
    return handle;
}

HexPosition PluginSystem::selectionEnd(const QObject *sender, qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->selectionEnd(this, index);
    }
    return {};
}

HexPosition PluginSystem::selectionStart(const QObject *sender,
                                         qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->selectionStart(this, index);
    }
    return {};
}

HexPosition PluginSystem::currentPos(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->currentPos(this);
    }
    return {};
}

AppTheme PluginSystem::currentAppTheme(const QObject *sender) {
    if (ApplicationMode::headless()) {
        return {};
    }

    auto theme = ThemeManager::instance().currentTheme();
    switch (theme) {
    case ThemeManager::Theme::Dark:
        return WingHex::AppTheme::Dark;
    case ThemeManager::Theme::System:
    case ThemeManager::Theme::Light:
        return WingHex::AppTheme::Light;
    }
    return WingHex::AppTheme::Invalid;
}

QIcon PluginSystem::standardIcon(const QObject *sender,
                                 WingHex::StandardIcon icon) {
    if (ApplicationMode::headless()) {
        return {};
    }

    Q_UNUSED(sender);
    return ThemeManager::instance().standardIcon(icon);
}

QPixmap PluginSystem::appAuthorPixmap(const QObject *sender) {
    if (ApplicationMode::headless()) {
        return {};
    }

    Q_UNUSED(sender);
    return QPixmap(
        QStringLiteral(":/com.wingsummer.winghex/images/author.jpg"));
}

QPixmap PluginSystem::standardPixmap(const QObject *sender,
                                     WingHex::StandardIcon icon) {
    if (ApplicationMode::headless()) {
        return {};
    }

    Q_UNUSED(sender);
    return ThemeManager::instance().standardPixmap(icon);
}

QByteArray PluginSystem::standardIconData(const QObject *sender,
                                          WingHex::StandardIcon icon) {
    if (ApplicationMode::headless()) {
        return {};
    }

    Q_UNUSED(sender);
    return ThemeManager::instance().standardIconData(icon);
}

void PluginSystem::logWarn(const QObject *sender, const QString &message) {
    Logger::warning(packLogMessage(sender->metaObject()->className(), message));
}

// =========================================================================

QString PluginSystem::currentLoadingPlugin() const { return m_curLoadingPlg; }

QSet<int> PluginSystem::scriptHandles(lua_State *L) const {
    if (L) {
        const auto registry =
            m_scriptDocumentRegistries.value(lua_mainthread(L)).lock();
        if (registry) {
            if (const auto context = registry->findCaller(L)) {
                return context->handles();
            }
        }
    }
    return {};
}

void PluginSystem::cleanScriptHandles(lua_State *L, const QSet<int> &handles) {
    if (L) {
        const auto registry =
            m_scriptDocumentRegistries.value(lua_mainthread(L)).lock();
        if (registry) {
            if (const auto context = registry->findCaller(L)) {
                context->cleanHandles(handles);
            }
        }
    }
}

qsizetype PluginSystem::pluginAPICount() const { return m_api._fnTable.size(); }

const QList<IWingPlugin *> &PluginSystem::plugins() const {
    return m_loadedplgs;
}

IWingPlugin *PluginSystem::plugin(qsizetype index) const {
    return m_loadedplgs.at(index);
}

const QList<IWingDevice *> &PluginSystem::devices() const {
    return m_loadeddevs;
}

IWingDevice *PluginSystem::device(qsizetype index) const {
    return m_loadeddevs.at(index);
}

EditorView *PluginSystem::getCurrentPluginView(IWingPlugin *plg) {
    if (plg == nullptr) {
        return nullptr;
    }

    auto fid = m_plgviewMap[plg].currentFID;
    auto ctx = pluginContextById(plg, fid);
    if (ctx) {
        return ctx->view.data();
    }
    return m_win ? m_win->m_curEditor : nullptr;
}

EditorView *PluginSystem::handle2EditorView(IWingPlugin *plg, int handle) {
    if (handle < 0) {
        return getCurrentPluginView(plg);
    }

    auto ctx = pluginContextById(plg, handle);
    if (ctx) {
        return ctx->view.data();
    }

    return nullptr;
}

void PluginSystem::retranslateMetadata(IWingPluginBase *plg, PluginInfo &meta) {
    meta.author = plg->retranslate(meta.author);
    meta.vendor = plg->retranslate(meta.vendor);
    meta.license = plg->retranslate(meta.license);
}

void PluginSystem::printErrorSummary(const QList<BlockInfo> &info) {
    if (!info.isEmpty()) {
        auto e = QMetaEnum::fromType<PluginStatus>();
        for (const auto &lplg : info) {
            auto [status, meta] = lplg;
            const char *s = e.valueToKey(int(status));
            QString r = meta.id + QStringLiteral(" - [") +
                        QString::fromLatin1(s) + QLatin1Char(']');
            Logger::critical(r);
        }
    }
}

int PluginSystem::assginHandleForOpenPluginView(IWingPlugin *plg,
                                                EditorView *view, bool select) {
    if (view && view->isCloneFile()) {
        view = view->cloneParent();
    }
    if (plg == nullptr || view == nullptr || view->isClosed()) {
        return ErrFile::NotExist;
    }

    auto id = checkIDPluginAlreadyOpened(plg, view);
    if (id >= 0) {
        if (select) {
            m_plgviewMap[plg].currentFID = id;
        }
        return id;
    }
    if (!checkPluginCanOpenedFile(plg)) {
        return ErrFile::TooManyOpenedFile;
    }
    auto context =
        QSharedPointer<PluginFileContext>::create(m_documentHandlePool);
    const int handle = context->fid;
    if (handle < 0) {
        return ErrFile::TooManyOpenedFile;
    }
    context->view = view;

    auto &file = m_plgviewMap[plg];
    file.contexts.append(context);
    if (select) {
        file.currentFID = handle;
    }
    bindPluginView(plg, view);

    return handle;
}

PluginInfo PluginSystem::parsePluginMetadata(const QJsonObject &meta) {
    PluginInfo info;

    info.id = meta["Id"].toString().trimmed();

    info.SDKVersion = meta["SDK"].toInt();
    info.version =
        QVersionNumber::fromString(meta["Version"].toString().trimmed());

    info.vendor = meta["Vendor"].toString().trimmed();
    info.author = meta["Author"].toString().trimmed();
    info.license = meta["License"].toString().trimmed();
    info.url = meta["Url"].toString().trimmed();
    info.certID = QByteArray::fromHex(meta["CertID"].toString().toUtf8());

    auto dependsObj = meta["Dependencies"];
    if (!dependsObj.isNull()) {
        if (dependsObj.isArray()) {
            auto arr = dependsObj.toArray();
            for (int i = 0; i < arr.size(); ++i) {
                auto d = arr.at(i);
                if (d.isObject()) {
                    auto id = d["Id"].toString().trimmed();
                    if (id.isEmpty()) {
                        continue;
                    }
                    auto version = QVersionNumber::fromString(
                        d["Version"].toString().trimmed());
                    WingDependency dp;
                    dp.puid = id;
                    dp.version = version;
                    info.dependencies.append(dp);
                } else if (d.isString()) {
                    auto vp = d.toString();
                    auto id = vp.section(':', 0, 0).trimmed();
                    auto version = QVersionNumber::fromString(
                        vp.section(':', 1).trimmed());
                    WingDependency dp;
                    dp.puid = id;
                    dp.version = version;
                    info.dependencies.append(dp);
                }
            }
        }
    }

    return info;
}

PluginSystem::PluginStatus
PluginSystem::checkPluginMetadata(const PluginInfo &meta) {
    constexpr auto puidLimit = 36; // same as uuid length, so enough

    if (meta.SDKVersion != SDKVERSION) {
        return PluginStatus::SDKVersion;
    }

    if (meta.id.length() > puidLimit) {
        return PluginStatus::InvalidID;
    }

    if (!Utilities::isValidIdentifier(meta.id)) {
        return PluginStatus::InvalidID;
    }

    if (meta.version.isNull()) {
        return PluginStatus::InvalidPlugin;
    }

    return PluginStatus::Valid;
}

bool PluginSystem::checkPluginCanOpenedFile(IWingPlugin *plg) {
    if (plg == nullptr) {
        return false;
    }
    return m_plgviewMap[plg].contexts.size() <= UCHAR_MAX;
}

int PluginSystem::checkIDPluginAlreadyOpened(IWingPlugin *plg,
                                             EditorView *view) {
    if (view && view->isCloneFile()) {
        view = view->cloneParent();
    }
    auto &maps = m_plgviewMap[plg].contexts;
    auto r =
        std::find_if(maps.begin(), maps.end(),
                     [view](const QSharedPointer<PluginFileContext> &content) {
                         return content->view == view;
                     });
    if (r != maps.end()) {
        return (*r)->fid;
    }
    return -1;
}

QList<IWingPlugin *> PluginSystem::boundPluginsForView(EditorView *view) const {
    auto r = m_viewBindings.constFind(view);
    if (r == m_viewBindings.cend()) {
        return {};
    }
    return r->linkedplg;
}

void PluginSystem::bindPluginView(IWingPlugin *plg, EditorView *view) {
    auto &plugins = m_viewBindings[view].linkedplg;
    if (!plugins.contains(plg)) {
        plugins.append(plg);
    }
}

void PluginSystem::releasePluginView(IWingPlugin *plg, EditorView *view) {
    if (view == nullptr) {
        return;
    }
    auto binding = m_viewBindings.find(view);
    if (binding == m_viewBindings.end()) {
        return;
    }

    binding->linkedplg.removeAll(plg);
    auto &undo = binding->undoStackPlg;
    auto r =
        std::find_if(undo.begin(), undo.end(),
                     [plg](const QPair<QUndoCommand *, IWingPlugin *> &cmd) {
                         return cmd.second == plg;
                     });
    if (r != undo.end()) {
        delete r->first;
        undo.erase(r, undo.end());
    }
    if (binding->linkedplg.isEmpty()) {
        m_viewBindings.erase(binding);
    }
}

void PluginSystem::cleanUpEditorViewHandle(EditorView *view) {
    const auto plugins = boundPluginsForView(view);
    for (auto plg : plugins) {
        auto &file = m_plgviewMap[plg];
        file.contexts.removeIf(
            [this, plg,
             view](const QSharedPointer<PluginFileContext> &context) {
                if (context->view != view) {
                    return false;
                }
                if (m_plgviewMap[plg].currentFID == context->fid) {
                    m_plgviewMap[plg].currentFID = -1;
                }
                dispatchPluginFileClosedEvent(
                    plg, m_win->getEditorViewFileType(view),
                    view->fileNameUrl(), context->fid);
                return true;
            });
    }
    m_viewBindings.remove(view);
}

EditorView *PluginSystem::closeHandle(IWingPlugin *plg, int handle) {
    if (handle < 0) {
        return nullptr;
    }
    auto &map = m_plgviewMap[plg];
    auto &contexts = map.contexts;
    auto it = std::find_if(
        contexts.begin(), contexts.end(),
        [handle](const QSharedPointer<PluginFileContext> &context) {
            return context->fid == handle;
        });
    if (it == contexts.end()) {
        return nullptr;
    }
    auto view = (*it)->view.data();
    contexts.erase(it);
    if (map.currentFID == handle) {
        map.currentFID = -1;
    }
    releasePluginView(plg, view);
    return view;
}

void PluginSystem::dispatchSelectionChangedEvent(
    const QByteArrayList &selections, bool isPreview) {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::SelectionChanged];
    for (const auto &plg : plgs) {
        plg->eventSelectionChanged(selections, isPreview);
    }
}

void PluginSystem::dispatchCursorPositionChangedEvent(const QHexPosition &pos) {
    HexPosition p;
    p.line = pos.line;
    p.column = pos.column;
    p.nibbleindex = pos.nibbleindex;
    p.lineWidth = pos.lineWidth;
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::CursorPositionChanged];
    for (const auto &plg : plgs) {
        plg->eventCursorPositionChanged(p);
    }
}

void PluginSystem::dispatchFileOpenedEvent(IWingPlugin::FileType type,
                                           const QUrl &newfileName) {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::FileOpened];
    for (const auto &plg : plgs) {
        plg->eventPluginFile(IWingPlugin::PluginFileEvent::Opened, type,
                             newfileName, -1, {});
    }
}

void PluginSystem::dispatchFileClosedEvent(IWingPlugin::FileType type,
                                           const QUrl &fileName) {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::FileClosed];
    for (const auto &plg : plgs) {
        plg->eventPluginFile(IWingPlugin::PluginFileEvent::Closed, type, {}, -1,
                             fileName);
    }
}

void PluginSystem::dispatchFileSavedEvent(IWingPlugin::FileType type,
                                          const QUrl &newfileName,
                                          const QUrl &oldfileName,
                                          bool isExported) {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::FileSaved];
    for (const auto &plg : plgs) {
        plg->eventPluginFile(isExported ? IWingPlugin::PluginFileEvent::Exported
                                        : IWingPlugin::PluginFileEvent::Saved,
                             type, oldfileName, -1, newfileName);
    }
}

void PluginSystem::dispatchFileSwitchedEvent(const QUrl &newfileName,
                                             const QUrl &oldfileName) {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::FileSwitched];
    for (const auto &plg : plgs) {
        plg->eventPluginFile(IWingPlugin::PluginFileEvent::Switched,
                             IWingPlugin::FileType::Invalid, oldfileName, -1,
                             newfileName);
    }
}

void PluginSystem::dispatchAppReadyEvent() {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::AppReady];
    for (const auto &plg : plgs) {
        plg->eventReady();
    }
}

void PluginSystem::dispatchPluginFileOpenedEvent(IWingPlugin *plg,
                                                 IWingPlugin::FileType type,
                                                 const QUrl &fileName, int id) {
    if (m_evplgs[WingHex::IWingPlugin::RegisteredEvent::PluginFileOpened]
            .contains(plg)) {
        plg->eventPluginFile(IWingPlugin::PluginFileEvent::PluginOpened, type,
                             fileName, id, {});
    }
}

void PluginSystem::dispatchPluginFileClosedEvent(IWingPlugin *plg,
                                                 IWingPlugin::FileType type,
                                                 const QUrl &fileName, int id) {
    if (m_evplgs[WingHex::IWingPlugin::RegisteredEvent::PluginFileClosed]
            .contains(plg)) {
        plg->eventPluginFile(IWingPlugin::PluginFileEvent::PluginClosed, type,
                             {}, id, fileName);
    }
}

bool PluginSystem::dispatchAppClosingEvent() {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::AppClosing];
    for (const auto &plg : plgs) {
        auto ret = plg->eventClosing();
        if (!ret) {
            Logger::warning(tr("Program shutdown canceled:") +
                            plg->pluginName() + QStringLiteral(" (") +
                            getPluginID(plg) + QStringLiteral(")"));
            if (!ApplicationMode::headless()) {
                return false;
            }
        }
    }
    return true;
}

void PluginSystem::dispatchHexEditorViewPaintEvent(QPainter *painter,
                                                   QWidget *w,
                                                   HexEditorContext *palette) {
    const auto &plgs =
        m_evplgs[WingHex::IWingPlugin::RegisteredEvent::HexEditorViewPaint];
    for (const auto &plg : plgs) {
        plg->onPaintHexEditorView(painter, w, palette);
    }
}

IWingDevice *PluginSystem::ext2Device(const QString &ext) {
    static QHash<QString, IWingDevice *> caches;
    if (caches.contains(ext)) {
        return caches.value(ext);
    }
    auto r = std::find_if(
        m_loadeddevs.begin(), m_loadeddevs.end(), [=](IWingDevice *dev) {
            return ext.compare(getPUID(dev), Qt::CaseInsensitive) == 0;
        });
    if (r == m_loadeddevs.end()) {
        return nullptr;
    }
    auto dev = *r;
    caches.insert(ext, dev);
    return dev;
}

void PluginSystem::installAPI(lua_State *L) {
    if (L == nullptr) {
        return;
    }

#include PLUGINSYSTEM_LUAU_BINDING

    m_scriptDocumentRegistries.removeIf(
        [](const auto &entry) { return entry.value().expired(); });
    m_scriptDocumentRegistries.insert(lua_mainthread(L), scriptContexts);
}

PluginInfo PluginSystem::getPluginInfo(IWingPluginBase *plg) const {
    Q_ASSERT(plg);
    return m_pinfos.value(plg);
}

QString PluginSystem::getPluginID(IWingPluginBase *plg) const {
    Q_ASSERT(plg);
    return m_pinfos.value(plg).id;
}

void PluginSystem::loadExtPlugin() {
    const auto folder = QStringLiteral("plugin");
    QDir udir(Utilities::getAppDataPath());
    if (!udir.exists(folder)) {
        udir.mkdir(folder);
    }

    const auto filter = QStringLiteral("*.wingplg");
    auto plgs =
        findPlugins(QCoreApplication::applicationDirPath() + QDir::separator() +
                        folder,
                    filter) +
        findPlugins(Utilities::getAppDataPath() + QDir::separator() + folder,
                    filter);

    const auto totalFound = plgs.size();
    Logger::info(tr("Total number of plugins found:") +
                 QString::number(totalFound));

    const auto plgset = QStringLiteral("plgset");
    if (!udir.exists(plgset)) {
        udir.mkdir(plgset);
    }
    bool udirOk = udir.cd(plgset);

    QHash<QString, QVersionNumber> loadedVersions;
    loadedVersions.reserve(m_pinfos.size());
    for (const auto &info : std::as_const(m_pinfos)) {
        loadedVersions.insert(info.id, info.version);
    }

    auto isDependencyMet = [&](const WingDependency &dep) -> bool {
        const auto it = loadedVersions.constFind(dep.puid);
        if (it == loadedVersions.constEnd()) {
            return false;
        }
        return dep.version.isNull() || it.value() >= dep.version;
    };

    struct PendingPlugin {
        QFileInfo file;
        std::unique_ptr<QPluginLoader> loader;
        PluginInfo meta;
    };

    std::vector<PendingPlugin> pending;
    pending.reserve(totalFound);

    for (const auto &item : std::as_const(plgs)) {
        if (item.isHidden()
#ifdef Q_OS_UNIX
            || !item.isExecutable()
#endif
        ) {
            m_blkplgs.append(
                {PluginStatus::InvalidPlugin, {item.baseName(), -1}});
            continue;
        }

        auto loader = std::make_unique<QPluginLoader>(item.absoluteFilePath());
        auto meta = readPluginMetaHeader(loader);
        if (!meta) {
            m_blkplgs.append(
                {PluginStatus::InvalidPlugin, {item.baseName(), -1}});
            continue;
        }

        PluginInfo m = meta.value();
        const auto cret = checkPluginMetadata(m);
        if (cret != PluginStatus::Valid) {
            m_blkplgs.append({cret, m});
            continue;
        }

        for (const auto &d : std::as_const(m.dependencies)) {
            if (!Utilities::isValidIdentifier(d.puid)) {
                m_blkplgs.append({PluginStatus::InvalidID, m});
                continue;
            }
        }

        if (!checkPluginCert(item, m.certID)) {
            m_blkplgs.append({PluginStatus::CertNotPassed, m});
            continue;
        }

        if (!m_enabledExtIDs.contains(m.id)) {
            m_blkplgs.append({PluginStatus::Blocked, m});
            continue;
        }

        pending.push_back({item, std::move(loader), std::move(m)});
    }

    QHash<QString, qsizetype> idCount;
    idCount.reserve(pending.size());
    for (const auto &p : pending) {
        ++idCount[p.meta.id];
    }

    QSet<QString> dupIds;
    for (auto it = idCount.cbegin(); it != idCount.cend(); ++it) {
        if (it.value() > 1) {
            dupIds.insert(it.key());
        }
    }

    std::vector<PendingPlugin> loadables;
    loadables.reserve(pending.size());
    for (auto &p : pending) {
        if (dupIds.contains(p.meta.id)) {
            m_blkplgs.append({PluginStatus::DupID, p.meta});
        } else {
            loadables.push_back(std::move(p));
        }
    }

    const auto n = qsizetype(loadables.size());
    QVector<QVector<qsizetype>> graph(n);
    QVector<qsizetype> indegree(n, 0);
    QVector<bool> blockedByMissing(n, false);

    QHash<QString, qsizetype> idToIndex;
    idToIndex.reserve(n);
    for (qsizetype i = 0; i < n; ++i) {
        idToIndex.insert(loadables[i].meta.id, i);
    }

    for (qsizetype i = 0; i < n; ++i) {
        const auto &plugin = loadables[i].meta;
        for (const auto &dep : plugin.dependencies) {
            if (isDependencyMet(dep)) {
                continue;
            }

            const auto it = idToIndex.constFind(dep.puid);
            if (it != idToIndex.cend()) {
                const auto provider = it.value();
                const auto lver = loadables[provider].meta.version;
                if (lver.isNull() || lver >= dep.version) {
                    graph[provider].push_back(i);
                    ++indegree[i];
                    continue;
                }
            }

            blockedByMissing[i] = true;
            ++indegree[i];
        }
    }

    QQueue<qsizetype> q;
    for (qsizetype i = 0; i < n; ++i) {
        if (indegree[i] == 0) {
            q.enqueue(i);
        }
    }

    QVector<qsizetype> topo;
    topo.reserve(n);
    while (!q.isEmpty()) {
        const qsizetype u = q.dequeue();
        topo.push_back(u);
        for (const qsizetype v : std::as_const(graph[u])) {
            if (--indegree[v] == 0) {
                q.enqueue(v);
            }
        }
    }

    QVector<bool> affectedByMissing(n, false);
    QQueue<qsizetype> mq;
    for (qsizetype i = 0; i < n; ++i) {
        if (blockedByMissing[i]) {
            affectedByMissing[i] = true;
            mq.enqueue(i);
        }
    }
    while (!mq.isEmpty()) {
        const qsizetype u = mq.dequeue();
        for (const qsizetype v : std::as_const(graph[u])) {
            if (!affectedByMissing[v]) {
                affectedByMissing[v] = true;
                mq.enqueue(v);
            }
        }
    }

    QVector<bool> processed(n, false);
    for (const qsizetype idx : topo) {
        processed[idx] = true;
    }

    for (qsizetype i = 0; i < n; ++i) {
        if (processed[i]) {
            continue;
        }

        const auto &l = loadables[i];
        if (affectedByMissing[i]) {
            m_blkplgs.append({PluginStatus::LackDependencies, l.meta});
        } else {
            m_blkplgs.append({PluginStatus::InvalidPlugin, loadables[i].meta});
        }
    }

    QSet<QString> failedIds;
    QStringList successfulLoadedIds;
    successfulLoadedIds.reserve(n);
    if (!m_preLoadedPlg.isEmpty()) {
        successfulLoadedIds.append(m_preLoadedPlg);
    }

    for (const qsizetype idx : topo) {
        const auto &candidate = loadables[idx];
        auto m = candidate.meta;

        bool depBroken = false;
        for (const auto &dep : std::as_const(m.dependencies)) {
            if (failedIds.contains(dep.puid) || !isDependencyMet(dep)) {
                depBroken = true;
                break;
            }
        }

        if (depBroken) {
            m_blkplgs.append({PluginStatus::LackDependencies, m});
            failedIds.insert(m.id);
            continue;
        }

        PluginLoadGuard guard(candidate.file);
        auto fName = candidate.file.fileName();
        Logger::info(tr("Loading plugins:") + fName);
        auto &loader = candidate.loader;
        auto p = qobject_cast<IWingPlugin *>(loader->instance());
        if (Q_UNLIKELY(p == nullptr)) {
            m_blkplgs.append({PluginStatus::InvalidPlugin, m});
            failedIds.insert(m.id);
            loader->unload();
            Logger::critical(packLoadPlgMessage(fName, loader->errorString()));
            continue;
        }

        retranslateMetadata(p, m);
        if (loadPlugin(p, m,
                       udirOk ? std::make_optional(udir) : std::nullopt)) {
            loadedVersions.insert(m.id, m.version);
            successfulLoadedIds.append(m.id);
        } else {
            m_blkplgs.append({PluginStatus::InitFailed, m});
        }
    }

    printErrorSummary(m_blkplgs);
    SettingManager::instance().setEnabledExtPlugins(successfulLoadedIds);
}

void PluginSystem::loadDevicePlugin() {
    const auto folder = QStringLiteral("devdrv");
    QDir udir(Utilities::getAppDataPath());
    if (!udir.exists(folder)) {
        udir.mkdir(folder);
    }

    const auto filter = QStringLiteral("*.wingdrv");
    const auto plgs =
        findPlugins(QCoreApplication::applicationDirPath() + QDir::separator() +
                        folder,
                    filter) +
        findPlugins(Utilities::getAppDataPath() + QDir::separator() + folder,
                    filter);
    const auto total = plgs.size();

    Logger::info(tr("Total number of device extension plugins discovered:") +
                 QString::number(total));

    auto plgset = QStringLiteral("drvset");
    if (!udir.exists(plgset)) {
        udir.mkdir(plgset);
    }
    bool udirOk = udir.cd(plgset);

    QStringList loaded;
    loaded.reserve(total);

    for (const auto &item : plgs) {
        if (item.isHidden()
#ifdef Q_OS_UNIX
            || !item.isExecutable()
#endif
        ) {
            m_blkdevs.append(
                {PluginStatus::InvalidPlugin, {item.baseName(), -1}});
            continue;
        }

        auto fileName = item.absoluteFilePath();
        QScopeGuard g([this]() { m_curLoadingPlg.clear(); });
        auto fName = item.fileName();
        m_curLoadingPlg = fName;

        PluginLoadGuard guard(item);
        auto loader = std::make_unique<QPluginLoader>(fileName);
        auto meta = readPluginMetaHeader(loader);
        if (!meta) {
            m_blkdevs.append(
                {PluginStatus::InvalidPlugin, {item.baseName(), -1}});
            continue;
        }

        PluginInfo m = meta.value();
        const auto cret = checkPluginMetadata(m);
        if (cret != PluginStatus::Valid) {
            m_blkdevs.append({cret, m});
            continue;
        }

        if (!checkPluginCert(item, m.certID)) {
            m_blkdevs.append({PluginStatus::CertNotPassed, m});
            continue;
        }

        if (!m_enabledDevIDs.contains(m.id)) {
            m_blkdevs.append({PluginStatus::Blocked, m});
            continue;
        }

        Logger::info(tr("Loading plugins:") + fName);
        auto p = qobject_cast<IWingDevice *>(loader->instance());
        if (Q_UNLIKELY(p == nullptr)) {
            m_blkdevs.append({PluginStatus::InvalidPlugin, m});
            loader->unload();
            Logger::critical(packLoadPlgMessage(fName, loader->errorString()));
            continue;
        } else {
            retranslateMetadata(p, m);
            if (loadPlugin(p, m,
                           udirOk ? std::make_optional(udir) : std::nullopt)) {
                loaded.append(meta->id);
            } else {
                m_blkdevs.append({PluginStatus::InitFailed, m});
            }
        }
    }

    printErrorSummary(m_blkdevs);
    SettingManager::instance().setEnabledDevPlugins(loaded);
}

QList<QFileInfo> PluginSystem::findPlugins(const QString &dir,
                                           const QString &filter) {
    const auto plgs = QDir(dir).entryInfoList(
        {filter}, QDir::Files | QDir::Readable | QDir::NoSymLinks, QDir::Name);
    return plgs;
}

bool PluginSystem::checkPluginCert(const QFileInfo &item,
                                   const QByteArray &certID) {
    if (!qEnvironmentVariableBool("WINGHEX_CERT_SKIP", false)) {
        bool ck = qEnvironmentVariableBool("WINGHEX_CERT_FORCE_ALL", false);
        if (!ck) {
            ck = FileAccessCheck::canStandardUserWriteFile(item) !=
                 FileAccessCheck::Status::Deny;
        }
        if (ck) {
            auto &ct = WingCert::instance();
            return ct.verify(item, certID);
        }
    }
    return true;
}

std::optional<PluginInfo> PluginSystem::readPluginMetaHeader(
    const std::unique_ptr<QPluginLoader> &loader) {
    Q_ASSERT(loader);
    auto lmeta = loader->metaData();
    if (lmeta.contains(QStringLiteral("MetaData"))) {
        return parsePluginMetadata(
            lmeta[QStringLiteral("MetaData")].toObject());
    }
    return std::nullopt;
}

QString PluginSystem::packLoadPlgMessage(const QString &header,
                                         const QString &content) {
    return QStringLiteral("{ %1 } %2").arg(header, content);
}

void PluginSystem::registerEvents(IWingPlugin *plg) {
    Q_ASSERT(plg);
    auto evs = plg->registeredEvents();
    using FlagInt = decltype(evs)::Int;
    if (FlagInt(evs) == FlagInt(IWingPlugin::RegisteredEvent::None)) {
        return;
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::SelectionChanged)) {
        m_evplgs[IWingPlugin::RegisteredEvent::SelectionChanged].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::CursorPositionChanged)) {
        m_evplgs[IWingPlugin::RegisteredEvent::CursorPositionChanged].append(
            plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileOpened)) {
        m_evplgs[IWingPlugin::RegisteredEvent::FileOpened].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileSaved)) {
        m_evplgs[IWingPlugin::RegisteredEvent::FileSaved].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileSwitched)) {
        m_evplgs[IWingPlugin::RegisteredEvent::FileSwitched].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::AppReady)) {
        m_evplgs[IWingPlugin::RegisteredEvent::AppReady].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::AppClosing)) {
        m_evplgs[IWingPlugin::RegisteredEvent::AppClosing].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileClosed)) {
        m_evplgs[IWingPlugin::RegisteredEvent::FileClosed].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::PluginFileOpened)) {
        m_evplgs[IWingPlugin::RegisteredEvent::PluginFileOpened].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::PluginFileClosed)) {
        m_evplgs[IWingPlugin::RegisteredEvent::PluginFileClosed].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::HexEditorViewPaint)) {
        m_evplgs[IWingPlugin::RegisteredEvent::HexEditorViewPaint].append(plg);
    }
}

void PluginSystem::registerHexContextMenu(IWingPlugin *plg) {
    if (!m_win) {
        return;
    }

    Q_ASSERT(plg);
    auto menu = plg->registeredHexContextMenu();
    if (menu) {
        menu->installEventFilter(this);
        m_win->m_hexContextMenu.append(menu);
        connect(menu, &QMenu::aboutToShow, this, [this, menu, plg]() {
            auto pp = menu->property("__CONTEXT__");
            auto ptr =
                reinterpret_cast<EditorViewContext *>(pp.value<quintptr>());
            if (ptr) {
                const auto &ws = m_win->m_editorViewWidgets;
                auto r = ws.find(plg);
                if (r == ws.end()) {
                    ptr->setCurrentPluginEditorWidget({});
                } else {
                    QStringList ids;
                    ids.reserve(r->size());
                    for (const auto &c : *r) {
                        ids.append(c->id());
                    }
                    ptr->setCurrentPluginEditorWidget(ids);
                }
                plg->prepareCallEditorContext(ptr);
            }
        });
        connect(menu, &QMenu::aboutToHide, this, [this, menu, plg]() {
            auto pp = menu->property("__CONTEXT__");
            auto ptr =
                reinterpret_cast<EditorViewContext *>(pp.value<quintptr>());
            if (ptr) {
                // QAction::triggered is processed later than QMenu::aboutToHide
                // so make a delay
                QTimer::singleShot(0, this, [plg, ptr]() {
                    plg->finishCallEditorContext(ptr);
                    ptr->setCurrentPluginEditorWidget({});
                });
            }
        });
    }
}

void PluginSystem::applyFunctionTables(QObject *plg) {
    auto plugin = qobject_cast<IWingPlugin *>(plg);
    if (plugin && !m_plgviewMap.contains(plugin)) {
        m_plgviewMap.insert(plugin, {});
        connect(plugin, &QObject::destroyed, this,
                [this, plugin] { cleanUpPluginHandles(plugin); });
    }
    CallTableEvent ev(&m_api);
    QApplication::sendEvent(plg, &ev);
}

void PluginSystem::cleanUpPluginHandles(IWingPlugin *plg) {
    const auto contexts = m_plgviewMap.value(plg).contexts;
    for (const auto &context : contexts) {
        closeHandle(plg, context->fid);
    }
    m_plgviewMap.remove(plg);
    for (auto &plugins : m_evplgs) {
        plugins.removeAll(plg);
    }
}

QString PluginSystem::getPUID(IWingPluginBase *p) {
    if (p) {
        return PluginSystem::instance().getPluginID(p);
    } else {
        return {};
    }
}

bool PluginSystem::isPluginLoaded(const WingDependency &d) {
    for (const auto &info : std::as_const(m_pinfos)) {
        if ((d.version.isNull() || info.version >= d.version) &&
            info.id == d.puid) {
            return true;
        }
    }
    return false;
}

bool PluginSystem::isPluginLoaded(const QString &id) {
    for (const auto &info : std::as_const(m_pinfos)) {
        if (info.id.compare(id, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

bool PluginSystem::loadPlugin(IWingPlugin *p, PluginInfo &meta,
                              const std::optional<QDir> &setdir) {
    QTranslator *pTr = nullptr;

    try {
        auto name = p->pluginName();
        if (name.trimmed().isEmpty()) {
            throw tr("Could not load the plugin: invalid plugin name.");
        }

        // dependencies had been checked

        Q_EMIT sigOnPluginLoading(name);

        pTr = LanguageManager::instance().try2LoadPluginLang(meta.id);

        applyFunctionTables(p);

        {
            std::unique_ptr<QSettings> setp(nullptr);
            if (setdir.has_value()) {
                setp = std::make_unique<QSettings>(
                    setdir->absoluteFilePath(meta.id),
                    QSettings::Format::IniFormat);
            }

            if (!p->init(setp)) {
                if (setp) {
                    setp->deleteLater();
                }
                throw tr("Could not initialize the plugin.");
            }
        }

        m_loadedplgs.append(p);
        m_pinfos.insert(p, meta);

        Logger::info(tr("Plugin name:") + p->pluginName());
        Logger::debug(tr("Plugin author:") + meta.author);
        Logger::debug(tr("Registering plugin objects..."));

        if (m_win) {
            // ensure call only once
            registerRibbonTools(p->registeredRibbonTools());
            registerPluginDockWidgets(p);
            registerHexContextMenu(p);

            {
                auto vieww = p->registeredEditorViewWidgets();
                if (!vieww.isEmpty()) {
                    m_win->m_editorViewWidgets.insert(p, vieww);
                }
            }

            registeredSettingPages(p);
        }
        registerEvents(p);

        // prepare for file contenxt
        m_plgviewMap.insert(p, {});
        connect(p, &QObject::destroyed, this,
                [this, p] { cleanUpPluginHandles(p); });
    } catch (const QString &error) {
        Logger::critical(error);
        if (pTr) {
            pTr->deleteLater();
        }
        return false;
    }
    return true;
}

bool PluginSystem::loadPlugin(IWingDevice *p, PluginInfo &meta,
                              const std::optional<QDir> &setdir) {
    QTranslator *pTr = nullptr;

    try {
        auto name = p->pluginName();
        if (name.trimmed().isEmpty()) {
            throw tr("Could not load the plugin: invalid plugin name.");
        }

        Logger::debug(tr("Device extension plugin author:") + meta.author);
        Logger::debug(tr("Registering device extension plugin objects..."));

        pTr = LanguageManager::instance().try2LoadPluginLang(meta.id);

        applyFunctionTables(p);

        {
            std::unique_ptr<QSettings> setp(nullptr);
            if (setdir.has_value()) {
                setp = std::make_unique<QSettings>(
                    setdir->absoluteFilePath(
                        QString::fromUtf8(p->metaObject()->className())),
                    QSettings::Format::IniFormat);
            }

            if (!p->init(setp)) {
                if (setp) {
                    setp->deleteLater();
                }
                throw tr("Could not initialize the device extension plugin.");
            }
        }

        m_loadeddevs.append(p);
        m_pinfos.insert(p, meta);

        if (m_win) {
            // ok register into menu open
            auto menu =
                m_win->m_toolBtneditors[MainWindow::ToolButtonIndex::OPEN_EXT]
                    ->menu();
            menu->addAction(m_win->newAction(
                p->supportedFileIcon(), p->supportedFileExtDisplayName(),
                [p, this]() {
                    auto file = p->onOpenFileBegin();
                    if (file.isNull()) {
                        return;
                    }

                    if (file.isEmpty()) {
                        // common dialog
                        file = WingFileDialog::getOpenFileName(
                            m_win, tr("Select file"), m_win->m_lastusedpath);
                    }

                    EditorView *view = nullptr;
                    auto ext = getPluginID(p);
                    auto res = m_win->openExtFile(ext, file, &view);

                    if (res == ErrFile::AlreadyOpened) {
                        view->raise();
                    }
                    if (m_win->reportErrFileError(res, {}, {}, {})) {
                        RecentFileManager::RecentInfo info;
                        info.url = WingHex::deviceUrl(ext, file);
                        m_win->m_recentmanager->addRecentFile(info);
                    }
                }));
        }

    } catch (const QString &error) {
        Logger::critical(error);
        if (pTr) {
            pTr->deleteLater();
        }
        return false;
    }
    return true;
}

void PluginSystem::registerPluginDockWidgets(IWingPluginBase *p) {
    if (!m_win) {
        return;
    }

    const auto dockWidgets = p->registeredDockWidgets();
    if (!dockWidgets.isEmpty()) {
        for (const auto &info : dockWidgets) {
            auto widgetName = info.widgetName.trimmed();
            auto displayName = info.displayName.trimmed();
            if (displayName.isEmpty()) {
                displayName = widgetName;
            }
            if (widgetName.isEmpty()) {
                Logger::warning(tr("Empty dock widget name:") +
                                QStringLiteral("{ ") + p->pluginName() +
                                QStringLiteral(" }"));
                continue;
            }

            auto inch = std::find_if_not(
                widgetName.begin(), widgetName.end(),
                [](const QChar &ch) { return ch.isLetterOrNumber(); });
            if (inch != widgetName.end()) {
                Logger::warning(tr("Invalid dock widget name:") +
                                QStringLiteral("{ ") + p->pluginName() +
                                QStringLiteral(" } ") + widgetName);
                continue;
            }

            if (info.widget == nullptr) {
                Logger::warning(
                    tr("Invalid empty dock widget:") + QStringLiteral("{ ") +
                    p->pluginName() + QStringLiteral(" } ") + widgetName +
                    QStringLiteral(" ( ") + displayName + QStringLiteral(" )"));
                continue;
            }

            auto dw =
                m_win->buildDockWidget(m_win->m_dock, widgetName, displayName,
                                       info.widget, MainWindow::PLUGIN_VIEWS);
            m_raisedw.insert(info.widget, dw);

            switch (info.area) {
            case Qt::LeftDockWidgetArea: {
                if (m_win->m_leftViewArea == nullptr) {
                    m_win->m_leftViewArea = m_win->m_dock->addDockWidget(
                        ads::LeftDockWidgetArea, dw);
                } else {
                    m_win->m_leftViewArea = m_win->m_dock->addDockWidget(
                        ads::CenterDockWidgetArea, dw, m_win->m_leftViewArea);
                }
            } break;
            case Qt::RightDockWidgetArea:
                m_win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                             m_win->m_rightViewArea);
                break;
            case Qt::TopDockWidgetArea: {
                if (m_win->m_topViewArea == nullptr) {
                    m_win->m_topViewArea = m_win->m_dock->addDockWidget(
                        ads::LeftDockWidgetArea, dw);
                } else {
                    m_win->m_topViewArea = m_win->m_dock->addDockWidget(
                        ads::CenterDockWidgetArea, dw, m_win->m_topViewArea);
                }
            } break;
            case Qt::BottomDockWidgetArea:
                m_win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                             m_win->m_bottomViewArea);
                break;
            case Qt::DockWidgetArea_Mask:
            case Qt::NoDockWidgetArea:
                m_win->m_dock->addDockWidget(ads::CenterDockWidgetArea, dw,
                                             m_win->m_rightViewArea);
                dw->hide();
                break;
            }
        }
    }
}

void PluginSystem::registerRibbonTools(
    const QList<WingRibbonToolBoxInfo> &tools) {
    if (!m_win) {
        return;
    }

    if (!tools.isEmpty()) {
        for (const auto &item : tools) {
            if (m_win->m_ribbonMaps.contains(item.catagory)) {
                if (!item.toolboxs.isEmpty()) {
                    auto &cat = m_win->m_ribbonMaps[item.catagory];
                    for (const auto &group : item.toolboxs) {
                        if (group.tools.isEmpty()) {
                            continue;
                        }
                        auto g = cat->addGroup(group.name);
                        for (const auto &tool : group.tools) {
                            g->addButton(tool);
                        }
                    }
                }
            } else {
                if (!item.toolboxs.isEmpty()) {
                    bool isSys = false;
                    RibbonTabContent *cat;
                    if (m_win->m_ribbonMaps.contains(item.catagory)) {
                        isSys = true;
                        cat = m_win->m_ribbonMaps.value(item.catagory);
                    } else {
                        cat = new RibbonTabContent;
                    }
                    bool hasAdded = false;
                    for (const auto &group : item.toolboxs) {
                        if (group.tools.isEmpty()) {
                            continue;
                        }
                        auto g = cat->addGroup(group.name);
                        for (const auto &tool : group.tools) {
                            g->addButton(tool);
                        }
                        hasAdded = true;
                    }
                    if (!isSys) {
                        if (hasAdded) {
                            m_win->m_ribbon->addTab(cat, item.displayName);
                            m_win->m_ribbonMaps[item.catagory] = cat;
                        } else {
                            delete cat;
                        }
                    }
                }
            }
        }
    }
}

void PluginSystem::registeredSettingPages(IWingPlugin *p) {
    if (!m_win) {
        return;
    }

    Q_ASSERT(p);
    const auto pages = p->registeredSettingPages();
    if (!pages.isEmpty()) {
        for (const auto &page : pages) {
            page->setProperty("__PLG__", quintptr(p));
            page->installEventFilter(this);
        }
        m_win->m_settingPages.append(pages);
    }
}

bool PluginSystem::checkThreadAff() {
    if (QThread::currentThread() != qApp->thread()) {
        qWarning("Operation is not allowed outside the GUI thread");
        return false;
    }
    return true;
}

QString PluginSystem::packLogMessage(const char *header, const QString &msg) {
    return QStringLiteral("{") + QString::fromUtf8(header) +
           QStringLiteral("} ") + msg;
}

PluginSystem &PluginSystem::instance() {
    static PluginSystem ins;
    return ins;
}

void PluginSystem::setMainWindow(MainWindow *win) { m_win = win; }

QWidget *PluginSystem::mainWindow() const { return m_win; }

void PluginSystem::loadAllPlugins() {
    Q_ASSERT(m_win || !ApplicationMode::hasMainWindow());

    auto &set = SettingManager::instance();
    bool enableSet = set.enablePlugin();

    auto disAll =
        qEnvironmentVariableBool("WINGHEX_DISABLE_PLUGIN_SYSTEM", false);
    auto marcoEnabled = !disAll;

    m_enabledExtIDs = set.enabledExtPlugins();
    m_enabledDevIDs = set.enabledDevPlugins();

    if (enableSet) {
        if (marcoEnabled) {
            constexpr bool enabled =
#ifdef QT_DEBUG
                false
#else
                true
#endif
                ;

            if (enabled &&
                qEnvironmentVariableBool("WINGHEX_CERT_SKIP", false)) {
                if (ApplicationMode::headless()) {
                    Logger::critical(tr("Certificate bypass requires "
                                        "interactive confirmation"));
                    throw CrashCode::PluginSetting;
                }
                if (WingMessageBox::critical(
                        m_win, qAppName(),
                        tr("Skipping certificate verification is dangerous. "
                           "Are you sure you want to continue?"),
                        QMessageBox::Yes | QMessageBox::No) ==
                    QMessageBox::No) {
                    throw CrashCode::PluginSetting;
                }
            }
            bool enabledrv = true, enableplg = true;
            auto disdrv =
                qEnvironmentVariableBool("WINGHEX_DISABLE_EXTDRV", false);
            if (disdrv) {
                enabledrv = false;
            }

            if (enabledrv) {
                loadDevicePlugin();
            }

            auto displg =
                qEnvironmentVariableBool("WINGHEX_DISABLE_PLUGIN", false);
            if (displg || !set.enablePlugin()) {
                enableplg = false;
            }

            if (Utilities::isRoot()) {
                auto dis = qEnvironmentVariableBool(
                    "WINGHEX_DISALLOW_ROOT_PLUGIN", true);
                if (dis || !set.enablePlgInRoot()) {
                    enableplg = false;
                }
            }

            if (enableplg) {
                loadExtPlugin();
            }
            Logger::info(tr("Plugins loaded!"));
        }
    }

    m_curLoadingPlg.clear();
}

void PluginSystem::destory() {
    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("plgset");
    if (!udir.exists(plgset)) {
        udir.mkdir(plgset);
    }
    m_unloading = true; // marked as unloading and disable cache

    bool ok = udir.cd(plgset);
    for (auto &plg : m_loadedplgs) {
        std::unique_ptr<QSettings> set;
        if (ok) {
            set = std::make_unique<QSettings>(
                udir.absoluteFilePath(m_pinfos[plg].id),
                QSettings::Format::IniFormat);
        }
        try {
            plg->unload(set);
        } catch (...) {
            qWarning("Plugin threw during unload: %s",
                     qUtf8Printable(m_pinfos.value(plg).id));
        }
        m_pinfos.remove(plg);
        delete plg;
    }
    m_loadedplgs.clear();

    for (auto &plg : m_loadeddevs) {
        std::unique_ptr<QSettings> set;
        if (ok) {
            set = std::make_unique<QSettings>(
                udir.absoluteFilePath(m_pinfos[plg].id),
                QSettings::Format::IniFormat);
        }
        try {
            plg->unload(set);
        } catch (...) {
            qWarning("Plugin threw during unload: %s",
                     qUtf8Printable(m_pinfos.value(plg).id));
        }
        m_pinfos.remove(plg);
        delete plg;
    }
    m_loadeddevs.clear();
}

EditorView *PluginSystem::pluginCurrentEditor(IWingPlugin *plg) const {
    if (plg) {
        auto fid = m_plgviewMap[plg].currentFID;
        if (fid < 0) {
            return m_win ? m_win->currentEditor() : nullptr;
        }

        auto ctx = pluginContextById(plg, fid);
        if (ctx) {
            return ctx->view;
        }
    }
    return nullptr;
}

QUndoCommand *PluginSystem::currentUndoCmd(EditorView *view) {
    if (m_viewBindings.contains(view)) {
        auto &vb = m_viewBindings[view];
        auto &undo = vb.undoStackPlg;
        if (!undo.isEmpty()) {
            return undo.last().first;
        }
    }
    return nullptr;
}

QSharedPointer<PluginSystem::PluginFileContext>
PluginSystem::pluginContextById(IWingPlugin *plg, int fid) const {
    if (fid < 0) {
        return nullptr;
    }
    auto &handles = m_plgviewMap[plg].contexts;
    auto r = std::find_if(handles.begin(), handles.end(),
                          [fid](const QSharedPointer<PluginFileContext> &d) {
                              return d->fid == fid;
                          });
    if (r == handles.end()) {
        return nullptr;
    }
    return *r;
}

int PluginSystem::closeFileHandle(const QObject *sender, int handle) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }
    auto plg = checkPluginAndReport(sender, __func__);
    if (!plg) {
        return ErrFile::Error;
    }
    auto view = closeHandle(plg, handle);
    if (!view) {
        return ErrFile::NotExist;
    }
    dispatchPluginFileClosedEvent(plg, m_win->getEditorViewFileType(view),
                                  view->fileNameUrl(), handle);
    return ErrFile::Success;
}

DocumentInfo PluginSystem::documentInfo(const QObject *sender, int handle) {
    DocumentInfo info;
    if (handle < 0) {
        return info;
    }
    auto plg = checkPluginAndReport(sender, __func__);
    auto view = handle2EditorView(plg, handle);
    if (!view || view->isClosed()) {
        return info;
    }
    info.fileNameUrl = view->fileNameUrl();
    if (!view->workSpaceName().isEmpty()) {
        info.workSpaceName = view->workSpaceName();
    }
    info.documentType = view->documentType();
    info.isSaved = view->isSaved();
    info.isReadOnly = view->isReadOnly(this);
    return info;
}
