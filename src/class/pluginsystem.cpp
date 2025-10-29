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

#include "pluginsystem.h"
#include "Qt-Advanced-Docking-System/src/DockAreaWidget.h"
#include "class/languagemanager.h"
#include "class/logger.h"
#include "class/settingmanager.h"
#include "class/skinmanager.h"
#include "class/wingangel.h"
#include "class/wingcstruct.h"
#include "class/wingfiledialog.h"
#include "class/winggeneric.h"
#include "class/winginputdialog.h"
#include "class/wingmessagebox.h"
#include "control/toast.h"
#include "define.h"
#include "dialog/colorpickerdialog.h"
#include "dialog/framelessdialogbase.h"
#include "dialog/mainwindow.h"
#include "predefgen.h"

#include <QDir>
#include <QFileInfoList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPluginLoader>
#include <QScopeGuard>
#include <QStringListModel>

#include <private/qmetaobject_p.h>

PluginSystem::PluginSystem(QObject *parent) : QObject(parent) {
    auto mobj = PluginSystem::metaObject();
    auto total = mobj->methodCount();

    // all public slots
    for (int i = mobj->methodOffset(); i < total; ++i) {
        auto m = mobj->method(i);
        if (m.methodType() == QMetaMethod::Slot &&
            m.access() == QMetaMethod::Public) {
            WingHex::FunctionSig msig;
            msig.fnName = m.name();

            auto total = m.parameterCount();
            msig.types.reserve(total);

            for (int i = 1; i < total; ++i) {
                auto mt = m.parameterType(i);
                msig.types.append(mt);
            }

            _plgFns.insert(msig, m);
        }
    }
}

PluginSystem::~PluginSystem() {}

// =========================================================================

void PluginSystem::toast(const QObject *sender, const QPixmap &icon,
                         const QString &message) {
    Q_UNUSED(sender);
    if (!checkThreadAff()) {
        return;
    }

    if (message.isEmpty()) {
        return;
    }

    if (passByFailedGuard(sender, __func__, {icon, message})) {
        return;
    }

    Toast::toast(_win, icon, message);
}

void PluginSystem::logTrace(const QObject *sender, const QString &message) {
    if (passByFailedGuard(sender, __func__, {message})) {
        return;
    }
    Logger::trace(packLogMessage(sender->metaObject()->className(), message));
}

void PluginSystem::logDebug(const QObject *sender, const QString &message) {
    if (passByFailedGuard(sender, __func__, {message})) {
        return;
    }
    Logger::debug(packLogMessage(sender->metaObject()->className(), message));
}

void PluginSystem::logInfo(const QObject *sender, const QString &message) {
    if (passByFailedGuard(sender, __func__, {message})) {
        return;
    }
    Logger::info(packLogMessage(sender->metaObject()->className(), message));
}

void PluginSystem::logError(const QObject *sender, const QString &message) {
    if (passByFailedGuard(sender, __func__, {message})) {
        return;
    }
    Logger::critical(
        packLogMessage(sender->metaObject()->className(), message));
}

bool PluginSystem::raiseDockWidget(const QObject *sender, QWidget *w) {
    Q_UNUSED(sender);
    if (!checkThreadAff()) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {QVariant::fromValue(w)})) {
        return false;
    }

    if (_raisedw.contains(w)) {
        _raisedw.value(w)->raise();
        return true;
    }
    return false;
}

QDialog *PluginSystem::createDialog(const QObject *sender, QWidget *content) {
    Q_UNUSED(sender);
    if (!checkThreadAff()) {
        return nullptr;
    }

    if (passByFailedGuard(sender, __func__, {QVariant::fromValue(content)})) {
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

void PluginSystem::msgAboutQt(const QObject *sender, QWidget *parent,
                              const QString &title) {
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title})) {
            return;
        }

        WingMessageBox::aboutQt(parent, title);
    }
}

QMessageBox::StandardButton
PluginSystem::msgInformation(const QObject *sender, QWidget *parent,
                             const QString &title, const QString &text,
                             QMessageBox::StandardButtons buttons,
                             QMessageBox::StandardButton defaultButton) {
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, text,
                               QVariant::fromValue(buttons),
                               QVariant::fromValue(defaultButton)})) {
            return QMessageBox::StandardButton::NoButton;
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, text,
                               QVariant::fromValue(buttons),
                               QVariant::fromValue(defaultButton)})) {
            return QMessageBox::StandardButton::NoButton;
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, text,
                               QVariant::fromValue(buttons),
                               QVariant::fromValue(defaultButton)})) {
            return QMessageBox::StandardButton::NoButton;
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, text,
                               QVariant::fromValue(buttons),
                               QVariant::fromValue(defaultButton)})) {
            return QMessageBox::StandardButton::NoButton;
        }

        return WingMessageBox::critical(parent, title, text, buttons,
                                        defaultButton);
    }
    return QMessageBox::StandardButton::NoButton;
}

void PluginSystem::msgAbout(const QObject *sender, QWidget *parent,
                            const QString &title, const QString &text) {
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, text})) {
            return;
        }

        WingMessageBox::about(parent, title, text);
    }
}

QMessageBox::StandardButton
PluginSystem::msgbox(const QObject *sender, QWidget *parent,
                     QMessageBox::Icon icon, const QString &title,
                     const QString &text, QMessageBox::StandardButtons buttons,
                     QMessageBox::StandardButton defaultButton) {
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, text,
                               QVariant::fromValue(buttons),
                               QVariant::fromValue(defaultButton)})) {
            return QMessageBox::StandardButton::NoButton;
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, label,
                               QVariant::fromValue(echo), text,
                               QVariant::fromValue(ok),
                               QVariant::fromValue(inputMethodHints)})) {
            return {};
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, label, text,
                               QVariant::fromValue(ok),
                               QVariant::fromValue(inputMethodHints)})) {
            return {};
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, label, items,
                               current, editable, QVariant::fromValue(ok),
                               QVariant::fromValue(inputMethodHints)})) {
            return {};
        }

        return WingInputDialog::getItem(parent, title, label, items, current,
                                        editable, ok, inputMethodHints);
    }
    return {};
}

int PluginSystem::dlgGetInt(const QObject *sender, QWidget *parent,
                            const QString &title, const QString &label,
                            int value, int minValue, int maxValue, int step,
                            bool *ok) {
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, label, value,
                               minValue, maxValue, step,
                               QVariant::fromValue(ok)})) {
            return 0;
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), title, label, value,
                               minValue, maxValue, decimals,
                               QVariant::fromValue(ok), step})) {
            return 0;
        }

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
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), caption, dir,
                               QVariant::fromValue(options)})) {
            return {};
        }

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
    if (checkErrAllAllowAndReport(sender, __func__)) {
        return {};
    }
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), caption, dir,
                               filter, QVariant::fromValue(selectedFilter),
                               QVariant::fromValue(options)})) {
            return {};
        }

        return WingFileDialog::getOpenFileName(parent, caption, dir, filter,
                                               selectedFilter, options);
    }
    return {};
}

QStringList PluginSystem::dlgGetOpenFileNames(
    const QObject *sender, QWidget *parent, const QString &caption,
    const QString &dir, const QString &filter, QString *selectedFilter,
    QFileDialog::Options options) {
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), caption, dir,
                               filter, QVariant::fromValue(selectedFilter),
                               QVariant::fromValue(options)})) {
            return {};
        }

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
    if (checkErrAllAllowAndReport(sender, __func__)) {
        return {};
    }
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {QVariant::fromValue(parent), caption, dir,
                               filter, QVariant::fromValue(selectedFilter),
                               QVariant::fromValue(options)})) {
            return {};
        }

        return WingFileDialog::getSaveFileName(parent, caption, dir, filter,
                                               selectedFilter, options);
    }
    return {};
}

QColor PluginSystem::dlgGetColor(const QObject *sender, const QString &caption,
                                 QWidget *parent) {
    Q_UNUSED(sender);
    if (checkThreadAff()) {
        if (passByFailedGuard(sender, __func__,
                              {caption, QVariant::fromValue(parent)})) {
            return {};
        }

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
    if (passByFailedGuard(sender, __func__, {puid})) {
        return false;
    }

    return std::find_if(_loadedplgs.begin(), _loadedplgs.end(),
                        [puid, this](IWingPlugin *plg) {
                            return getPluginID(plg) == puid;
                        }) != _loadedplgs.end();
}

bool PluginSystem::invokeServiceImpl(const QObject *sender, const QString &puid,
                                     const WingHex::MetaCallInfo &infos) {
    auto p = checkPluginAndReport(sender, __func__);
    if (p == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__,
                          {puid, QVariant::fromValue(infos)})) {
        return false;
    }

    QObject *obj = nullptr;
    if (puid.compare(QStringLiteral("[MAN]"), Qt::CaseInsensitive) == 0) {
        obj = _manager;
    } else if (puid.compare(QStringLiteral("[HEXE]"), Qt::CaseInsensitive) ==
               0) {
        obj = _hexExt;
    } else {
        QString rpuid;
        auto question = puid.indexOf('?');
        enum { None, MustPlugin, MustExt } State = None;

        if (question >= 0) {
            rpuid = puid.sliced(0, question);
            auto equest = puid.sliced(question + 1);
            if (equest.compare(QStringLiteral("[PLG]"))) {
                State = MustPlugin;
            } else if (equest.compare(QStringLiteral("[EXT]"))) {
                State = MustExt;
            } else {
                // warn and ignored
                qWarning().noquote()
                    << QStringLiteral(
                           "[PluginSystem::invokeServiceImpl] Cannot "
                           "parsing filter: '%1'")
                           .arg(equest);
            }
        } else {
            rpuid = puid;
            State = None;
        }
        auto r = std::find_if(
            _loadedplgs.begin(), _loadedplgs.end(), [=](IWingPlugin *plg) {
                return rpuid.compare(getPUID(plg), Qt::CaseInsensitive) == 0;
            });

        if (r == _loadedplgs.end()) {
            return false;
        }

        auto rc = *r;
        switch (State) {
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

        if (met.name() == method) {
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
        qCritical(
            "[PluginSystem::invokeServiceImpl] Invalid MetaMethod (Not-Found)");
        return false;
    }

    SenderInfo info;
    info.plgcls = meta->className();
    info.puid = getPUID(p);

    auto meta_name = "WING_META";
    // property first
    auto var = sender->property(meta_name);
    if (var.isValid()) {
        info.meta = var;
    } else {
        auto iidx = meta->indexOfClassInfo(meta_name);
        if (iidx >= 0) {
            info.meta = QString(meta->classInfo(iidx).value());
        }
    }

    // prepend the 'SenderInfo' parameter
    // note: index 0 is return value, index 1 is the real first param
    auto data = QtPrivate::Invoke::dataHelper(info);
    auto tn = QtPrivate::Invoke::typenameHelper(info);
    auto inf = QtPrivate::Invoke::metaTypeHelper(info);

    auto nparamCount = paramCount + 1;
    QVarLengthArray<decltype(data)> nparameters;
    QVarLengthArray<decltype(tn)> ntypeNames;
    QVarLengthArray<decltype(inf)> nmetaTypes;
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

    auto cstr = [](QMetaMethodInvoker::InvokeFailReason r) -> const char * {
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

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    return pluginCurrentEditor(plg);
}

QString PluginSystem::currentDocFile(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->fileNameUrl().url();
    }
    return {};
}

bool PluginSystem::isReadOnly(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->cursor()->insertionMode() ==
               QHexCursor::InsertMode;
    }
    return false;
}

bool PluginSystem::isInsertionMode(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->isLocked();
    }
    return false;
}

bool PluginSystem::isKeepSize(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->isKeepSize();
    }
    return false;
}

bool PluginSystem::isLocked(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->document()->isDocSaved();
    }
    return false;
}

qsizetype PluginSystem::documentLines(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->renderer()->documentLines();
    }
    return 0;
}

qsizetype PluginSystem::documentBytes(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->documentBytes();
    }
    return 0;
}

qsizetype PluginSystem::currentRow(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->currentRow();
    }
    return 0;
}

qsizetype PluginSystem::currentColumn(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->currentColumn();
    }
    return 0;
}

qsizetype PluginSystem::currentOffset(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->cursor()->position().offset();
    }
    return 0;
}

qsizetype PluginSystem::selectedLength(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->cursor()->currentSelectionLength();
    }
    return 0;
}

QByteArray PluginSystem::selectedBytes(const QObject *sender, qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {index})) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e && index >= 0 && index < e->hexEditor()->cursor()->selectionCount()) {
        return e->hexEditor()->selectedBytes(index);
    }
    return {};
}

QByteArrayList PluginSystem::selectionBytes(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->selectedBytes();
    }
    return {};
}

qsizetype PluginSystem::selectionLength(const QObject *sender,
                                        qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {index})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {

        auto cursor = e->hexEditor()->cursor();
        if (index >= 0 && index < cursor->selectionCount()) {
            return cursor->selectionLength(index);
        }
    }
    return 0;
}

qsizetype PluginSystem::selectionCount(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto cursor = e->hexEditor()->cursor();
        return cursor->selectionCount();
    }
    return 0;
}

bool PluginSystem::stringVisible(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->asciiVisible();
    }
    return false;
}

bool PluginSystem::addressVisible(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->headerVisible();
    }
    return false;
}

bool PluginSystem::headerVisible(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->addressVisible();
    }
    return false;
}

quintptr PluginSystem::addressBase(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->addressBase();
    }
    return 0;
}

bool PluginSystem::isModified(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return !e->hexEditor()->isSaved();
    }
    return 0;
}

qint8 PluginSystem::readInt8(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<qint8>(plg, offset);
}

qint16 PluginSystem::readInt16(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<qint16>(plg, offset);
}

qint32 PluginSystem::readInt32(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<qint32>(plg, offset);
}

qint64 PluginSystem::readInt64(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<qint64>(plg, offset);
}

quint8 PluginSystem::readUInt8(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<quint8>(plg, offset);
}

quint16 PluginSystem::readUInt16(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<quint16>(plg, offset);
}

quint32 PluginSystem::readUInt32(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<quint32>(plg, offset);
}

quint64 PluginSystem::readUInt64(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<quint64>(plg, offset);
}

float PluginSystem::readFloat(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return qQNaN();
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return 0;
    }

    return readBasicTypeContent<float>(plg, offset);
}

double PluginSystem::readDouble(const QObject *sender, qsizetype offset) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return qQNaN();
    }

    if (passByFailedGuard(sender, __func__, {offset})) {
        return qQNaN();
    }

    return readBasicTypeContent<double>(plg, offset);
}

QString PluginSystem::readString(const QObject *sender, qsizetype offset,
                                 const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {offset, encoding})) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        _rwlock.lockForRead();
        auto hexeditor = e->hexEditor();
        auto doc = hexeditor->document();
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
    return QString();
}

QByteArray PluginSystem::readBytes(const QObject *sender, qsizetype offset,
                                   qsizetype count) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {offset, count})) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        _rwlock.lockForRead();
        auto ret = e->hexEditor()->document()->read(offset, count);
        _rwlock.unlock();
        return ret;
    }
    return {};
}

qsizetype PluginSystem::findNext(const QObject *sender, qsizetype begin,
                                 const QByteArray &ba) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {begin, ba})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->document()->findNext(begin, ba);
    }
    return qsizetype(-1);
}

qsizetype PluginSystem::findPrevious(const QObject *sender, qsizetype begin,
                                     const QByteArray &ba) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return 0;
    }

    if (passByFailedGuard(sender, __func__, {begin, ba})) {
        return 0;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->document()->findPrevious(begin, ba);
    }
    return qsizetype(-1);
}

QString PluginSystem::bookMarkComment(const QObject *sender, qsizetype pos) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {pos})) {
        return {};
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->document()->bookMark(pos);
    }
    return {};
}

bool PluginSystem::existBookMark(const QObject *sender, qsizetype pos) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {pos})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->document()->existBookMark(pos);
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

    if (passByFailedGuard(sender, __func__, {handle})) {
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
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {handle})) {
        return false;
    }

    if (handle < 0) {
        auto cur = _win->m_curEditor;
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->document()->setLockedFile(b);
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        return e->hexEditor()->document()->setKeepSize(b);
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        e->hexEditor()->setAsciiVisible(b);
        return true;
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        e->hexEditor()->setAddressVisible(b);
        return true;
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        e->hexEditor()->setHeaderVisible(b);
        return true;
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

    if (passByFailedGuard(sender, __func__, {base})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        e->hexEditor()->setAddressBase(base);
        return true;
    }
    return false;
}

bool PluginSystem::beginMarco(const QObject *sender, const QString &txt) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {txt})) {
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
            rtxt = _pinfos.value(plg).id;
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

    if (passByFailedGuard(sender, __func__, {})) {
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
                Logger::warning(tr("UnexpectedUndoCmdPushDetected"));
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

    if (passByFailedGuard(sender, __func__, {})) {
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

    if (passByFailedGuard(sender, __func__, {})) {
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

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeInt16(const QObject *sender, qsizetype offset,
                              qint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeInt32(const QObject *sender, qsizetype offset,
                              qint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeInt64(const QObject *sender, qsizetype offset,
                              qint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeUInt8(const QObject *sender, qsizetype offset,
                              quint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeUInt16(const QObject *sender, qsizetype offset,
                               quint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeUInt32(const QObject *sender, qsizetype offset,
                               quint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeUInt64(const QObject *sender, qsizetype offset,
                               quint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeFloat(const QObject *sender, qsizetype offset,
                              float value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeDouble(const QObject *sender, qsizetype offset,
                               double value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return writeBasicTypeContent(plg, offset, value);
}

bool PluginSystem::writeString(const QObject *sender, qsizetype offset,
                               const QString &value, const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value, encoding})) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        auto editor = e->hexEditor();
        auto doc = editor->document();

        auto unicode = Utilities::encodingString(value, encoding);
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeReplace(uc, editor->cursor(), offset, unicode);
        if (uc == nullptr && cmd) {
            _rwlock.lockForWrite();
            doc->pushMakeUndo(cmd);
            _rwlock.unlock();
            return true;
        }
    }
    return false;
}

bool PluginSystem::writeBytes(const QObject *sender, qsizetype offset,
                              const QByteArray &data) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, data})) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        auto editor = e->hexEditor();
        auto doc = editor->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeReplace(uc, editor->cursor(), offset, data);
        if (uc == nullptr && cmd) {
            _rwlock.lockForWrite();
            doc->pushMakeUndo(cmd);
            _rwlock.unlock();
            return true;
        }
    }
    return false;
}

bool PluginSystem::insertInt8(const QObject *sender, qsizetype offset,
                              qint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertInt16(const QObject *sender, qsizetype offset,
                               qint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertInt32(const QObject *sender, qsizetype offset,
                               qint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertInt64(const QObject *sender, qsizetype offset,
                               qint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertUInt8(const QObject *sender, qsizetype offset,
                               quint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertUInt16(const QObject *sender, qsizetype offset,
                                quint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertUInt32(const QObject *sender, qsizetype offset,
                                quint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertUInt64(const QObject *sender, qsizetype offset,
                                quint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertFloat(const QObject *sender, qsizetype offset,
                               float value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertDouble(const QObject *sender, qsizetype offset,
                                double value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value})) {
        return false;
    }

    return insertBasicTypeContent(plg, offset, value);
}

bool PluginSystem::insertString(const QObject *sender, qsizetype offset,
                                const QString &value, const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, value, encoding})) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        auto editor = e->hexEditor();
        auto doc = editor->document();

        auto unicode = Utilities::encodingString(value, encoding);
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeInsert(uc, editor->cursor(), offset, unicode);
        if (uc == nullptr && cmd) {
            _rwlock.lockForWrite();
            doc->pushMakeUndo(cmd);
            _rwlock.unlock();
            return true;
        }
    }
    return false;
}

bool PluginSystem::insertBytes(const QObject *sender, qsizetype offset,
                               const QByteArray &data) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, data})) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        auto editor = e->hexEditor();
        auto doc = editor->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeInsert(uc, editor->cursor(), offset, data);
        if (uc == nullptr && cmd) {
            _rwlock.lockForWrite();
            doc->pushMakeUndo(cmd);
            _rwlock.unlock();
            return true;
        }
    }
    return false;
}

bool PluginSystem::appendInt8(const QObject *sender, qint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendInt16(const QObject *sender, qint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendInt32(const QObject *sender, qint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendInt64(const QObject *sender, qint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendUInt8(const QObject *sender, quint8 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendUInt16(const QObject *sender, quint16 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendUInt32(const QObject *sender, quint32 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendUInt64(const QObject *sender, quint64 value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }
    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendFloat(const QObject *sender, float value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendDouble(const QObject *sender, double value) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value})) {
        return false;
    }

    return appendBasicTypeContent(plg, value);
}

bool PluginSystem::appendString(const QObject *sender, const QString &value,
                                const QString &encoding) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {value, encoding})) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        auto editor = e->hexEditor();
        auto doc = editor->document();
        auto offset = doc->length();

        auto unicode = Utilities::encodingString(value, encoding);
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeInsert(uc, editor->cursor(), offset, unicode);
        if (uc == nullptr && cmd) {
            _rwlock.lockForWrite();
            doc->pushMakeUndo(cmd);
            _rwlock.unlock();
            return true;
        }
    }
    return false;
}

bool PluginSystem::appendBytes(const QObject *sender, const QByteArray &data) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {data})) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        auto editor = e->hexEditor();
        auto doc = editor->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeAppend(uc, editor->cursor(), data);
        if (uc == nullptr && cmd) {
            _rwlock.lockForWrite();
            doc->pushMakeUndo(cmd);
            _rwlock.unlock();
            return true;
        }
    }
    return false;
}

bool PluginSystem::removeBytes(const QObject *sender, qsizetype offset,
                               qsizetype len) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {offset, len})) {
        return false;
    }

    auto e = getCurrentPluginView(plg);
    if (e) {
        auto editor = e->hexEditor();
        auto doc = editor->document();

        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeRemove(uc, editor->cursor(), offset, len);
        if (uc == nullptr && cmd) {
            _rwlock.lockForWrite();
            doc->pushMakeUndo(cmd);
            _rwlock.unlock();
            return true;
        }
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

    if (passByFailedGuard(sender, __func__,
                          {line, column, nibbleindex, clearSelection})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        e->hexEditor()->cursor()->moveTo(line, column, nibbleindex,
                                         clearSelection);
        return true;
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

    if (passByFailedGuard(sender, __func__, {offset, clearSelection})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        e->hexEditor()->cursor()->moveTo(offset, clearSelection);
        return true;
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

    if (passByFailedGuard(sender, __func__,
                          {offset, length, QVariant::fromValue(mode)})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto cursor = e->hexEditor()->cursor();
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

    if (passByFailedGuard(sender, __func__, {isinsert})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        e->hexEditor()->cursor()->setInsertionMode(
            isinsert ? QHexCursor::InsertMode : QHexCursor::OverwriteMode);
        return true;
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

    if (passByFailedGuard(sender, __func__,
                          {begin, length, fgcolor, bgcolor, comment})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();

        auto uc = currentUndoCmd(e);
        auto cmd = doc->metadata()->MakeMetadata(uc, begin, begin + length - 1,
                                                 fgcolor, bgcolor, comment);
        if (uc == nullptr && cmd) {
            doc->pushMakeUndo(cmd);
            return true;
        }
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

    if (passByFailedGuard(sender, __func__, {offset})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->metadata()->MakeRemoveMetadata(uc, offset);
        if (uc == nullptr && cmd) {
            doc->pushMakeUndo(cmd);
            return true;
        }
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

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->metadata()->MakeClear(uc);
        if (uc == nullptr && cmd) {
            doc->pushMakeUndo(cmd);
            return true;
        }
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        doc->setMetafgVisible(b);
        doc->setMetabgVisible(b);
        doc->setMetaCommentVisible(b);
        return true;
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        doc->setMetafgVisible(b);
        return true;
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        doc->setMetabgVisible(b);
        return true;
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

    if (passByFailedGuard(sender, __func__, {b})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        doc->setMetaCommentVisible(b);
        return true;
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

    if (passByFailedGuard(sender, __func__, {pos, comment})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeAddBookMark(uc, pos, comment);
        if (uc == nullptr && cmd) {
            doc->pushMakeUndo(cmd);
            return true;
        }
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

    if (passByFailedGuard(sender, __func__, {pos, comment})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeModBookMark(uc, pos, comment);
        if (uc == nullptr && cmd) {
            doc->pushMakeUndo(cmd);
            return true;
        }
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

    if (passByFailedGuard(sender, __func__, {pos})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeRemoveBookMark(uc, pos);
        if (uc == nullptr && cmd) {
            doc->pushMakeUndo(cmd);
            return true;
        }
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

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto doc = e->hexEditor()->document();
        auto uc = currentUndoCmd(e);
        auto cmd = doc->MakeClearBookMark(uc);
        if (uc == nullptr && cmd) {
            doc->pushMakeUndo(cmd);
            return true;
        }
    }
    return false;
}

bool PluginSystem::closeAllFiles(const QObject *sender) {
    if (!checkThreadAff()) {
        return false;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return false;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return false;
    }

    auto &map = m_plgviewMap[plg];
    auto &maps = map.contexts;
    for (auto &item : maps) {
        closeEditor(plg, getUIDHandle(item->fid), true);
    }
    map.currentFID = -1;
    maps.clear();
    return true;
}

void PluginSystem::__raiseContextException(const QObject *sender,
                                           const QString &exception,
                                           bool allowCatch) {
    if (passByFailedGuard(sender, __func__, {})) {
        return;
    }

    auto ctx = asGetActiveContext();
    if (ctx) {
        auto info = exception.toUtf8();
        ctx->SetException(info, allowCatch);
    }
}

IWingGeneric *PluginSystem::__createParamContext(const QObject *sender,
                                                 void *ctx) {
    if (passByFailedGuard(sender, __func__, {})) {
        return nullptr;
    }

    auto gen = reinterpret_cast<asIScriptGeneric *>(ctx);
    if (gen) {
        return new WingGeneric(gen);
    }
    return nullptr;
}

bool PluginSystem::passByFailedGuard(const QObject *sender, const char *func,
                                     const QVariantList &params) {
    if (_manager && sender != _manager) {
        auto ret = !_manager->enterGuard(sender->metaObject(), func, params);
        if (ret) {
            qCritical("[GuardBlock] '%s' was blocked", func);
        }
        return ret;
    }
    return false;
}

bool PluginSystem::checkErrAllAllowAndReport(const QObject *sender,
                                             const char *func) {
    QObject *s = const_cast<QObject *>(sender);
    auto p = qobject_cast<WingHex::IWingPlugin *>(s);
    if (p == nullptr) {
        auto p = qobject_cast<WingHex::IWingDevice *>(s);
        if (p == nullptr) {
            qCritical("[EvilCall] Invalid sender called '%s'", func);
            return true;
        }
    }
    return false;
}

const std::optional<PluginInfo> &PluginSystem::hexEditorExtensionInfo() const {
    return _manHexInfo;
}

IWingHexEditorPlugin *PluginSystem::hexEditorExtension() const {
    return _hexExt;
}

QMap<PluginSystem::BlockReason, QList<PluginInfo>>
PluginSystem::blockedDevPlugins() const {
    return _blkdevs;
}

QMap<PluginSystem::BlockReason, QList<PluginInfo>>
PluginSystem::blockedPlugins() const {
    return _blkplgs;
}

void PluginSystem::doneRegisterScriptObj() {
    Q_ASSERT(_angelplg);
    // ok, then, we will register all script objects
    auto api =
        QScopedPointer<WingAngel>(new WingAngel(_angelplg, _scriptMarcos));

    // don't register evalutors for internal types
    auto &m = ScriptMachine::instance();
    auto engine = m.engine();
    auto total = engine->GetObjectTypeCount();
    QList<int> excludeTypeId;
    excludeTypeId.reserve(total);
    for (asUINT i = 0; i < total; ++i) {
        auto obj = engine->GetObjectTypeByIndex(i);
        auto typeId = obj->GetTypeId();
        typeId &= asTYPEID_MASK_OBJECT | asTYPEID_MASK_SEQNBR;
        if (typeId) {
            excludeTypeId.append(typeId);
        }
    }
    api->setExcludeEvalIDs(excludeTypeId);

    auto ptr = api.data();
    if (_manager) {
        ptr->setCurrentPluginSession(_manInfo->id.toUtf8());
        _manager->onRegisterScriptObj(ptr);
    }

    for (auto &p : _loadedplgs) {
        auto puid = getPUID(p);
        ptr->setCurrentPluginSession(puid.toUtf8());
        p->onRegisterScriptObj(ptr);
    }

    for (auto &p : _loadeddevs) {
        auto puid = getPUID(p);
        ptr->setCurrentPluginSession(puid.toUtf8());
        p->onRegisterScriptObj(ptr);
    }

    auto evals = api->customEvals();
    m.setCustomEvals(evals);

    ptr->setCurrentPluginSession({});
    generateScriptPredefined(ScriptMachine::instance().engine(),
                             Utilities::getASPredefPath());
}

const std::optional<PluginInfo> &PluginSystem::monitorManagerInfo() const {
    return _manInfo;
}

IWingManager *PluginSystem::monitorManager() const { return _manager; }

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

ErrFile PluginSystem::saveAsCurrent(const QObject *sender,
                                    const QString &savename) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {savename})) {
        return ErrFile::Permission;
    }

    auto view = getCurrentPluginView(plg);
    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    if (view) {
        return _win->saveEditor(view, savename);
    }
    return ErrFile::Error;
}

ErrFile PluginSystem::exportCurrent(const QObject *sender,
                                    const QString &savename) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {savename})) {
        return ErrFile::Permission;
    }

    auto view = getCurrentPluginView(plg);
    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    if (view) {
        auto ws = _win->m_views.value(view);
        return view->save(ws, savename, true,
                          EditorView::SaveWorkSpaceAttr::AutoWorkSpace);
    }
    return ErrFile::Error;
}

ErrFile PluginSystem::saveCurrent(const QObject *sender) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return ErrFile::Permission;
    }

    auto view = getCurrentPluginView(plg);
    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    if (view) {
        auto ws = _win->m_views.value(view);
        return view->save(ws, {}, false,
                          EditorView::SaveWorkSpaceAttr::AutoWorkSpace);
    }
    return ErrFile::Error;
}

ErrFile PluginSystem::closeCurrent(const QObject *sender, bool force) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {force})) {
        return ErrFile::Permission;
    }

    auto view = getCurrentPluginView(plg);
    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    if (view == nullptr) {
        return ErrFile::NotExist;
    }

    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    return _win->closeEditor(view, force);
}

int PluginSystem::openCurrent(const QObject *sender) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return ErrFile::Permission;
    }

    auto view = getCurrentPluginView(plg);
    if (view) {
        if (checkPluginHasAlreadyOpened(plg, view)) {
            return ErrFile::AlreadyOpened;
        }

        auto id = assginHandleForOpenPluginView(plg, view);

        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::PluginFileOpened,
            {quintptr(plg), view->fileNameUrl(), id,
             QVariant::fromValue(_win->getEditorViewFileType(view))});

        return id;
    }
    return ErrFile::Error;
}

ErrFile PluginSystem::saveAsFile(const QObject *sender, int handle,
                                 const QString &savename) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {handle, savename})) {
        return ErrFile::Permission;
    }

    auto view = handle2EditorView(plg, handle);
    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    if (view) {
        return _win->saveEditor(view, savename);
    }
    return ErrFile::NotExist;
}

ErrFile PluginSystem::exportFile(const QObject *sender, int handle,
                                 const QString &savename) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {handle, savename})) {
        return ErrFile::Permission;
    }

    auto view = handle2EditorView(plg, handle);
    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    if (view) {
        return _win->saveEditor(view, savename, true);
    }
    return ErrFile::NotExist;
}

int PluginSystem::openWorkSpace(const QObject *sender,
                                const QString &filename) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {filename})) {
        return ErrFile::Permission;
    }

    EditorView *view = nullptr;
    if (!checkPluginCanOpenedFile(plg)) {
        return ErrFile::TooManyOpenedFile;
    }
    auto ret = _win->openWorkSpace(filename, &view);
    if (view) {
        if (ret == ErrFile::AlreadyOpened) {
            return ErrFile::AlreadyOpened;
        }
        auto id = assginHandleForOpenPluginView(plg, view);

        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::PluginFileOpened,
            {quintptr(plg), filename, id,
             QVariant::fromValue(_win->getEditorViewFileType(view))});

        return id;
    } else {
        return ret;
    }
}

ErrFile PluginSystem::saveFile(const QObject *sender, int handle) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {handle})) {
        return ErrFile::Permission;
    }

    auto view = handle2EditorView(plg, handle);
    if (!checkPluginHasAlreadyOpened(plg, view)) {
        return ErrFile::Error;
    }

    if (view) {
        return _win->saveEditor(view, {});
    }
    return ErrFile::NotExist;
}

ErrFile PluginSystem::closeFile(const QObject *sender, int handle, bool force) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {handle, force})) {
        return ErrFile::Permission;
    }

    if (closeEditor(plg, handle, force)) {
        return ErrFile::Success;
    }
    return ErrFile::NotExist;
}

ErrFile PluginSystem::closeHandle(const QObject *sender, int handle) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {handle})) {
        return ErrFile::Permission;
    }

    if (closeHandle(plg, handle)) {
        return WingHex::ErrFile::Success;
    }
    return WingHex::ErrFile::NotExist;
}

int PluginSystem::openExtFile(const QObject *sender, const QString &ext,
                              const QString &file) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {ext, file})) {
        return ErrFile::Permission;
    }

    EditorView *view = nullptr;
    if (!checkPluginCanOpenedFile(plg)) {
        return ErrFile::TooManyOpenedFile;
    }
    auto ret = _win->openExtFile(ext, file, &view);
    if (view) {
        if (ret == ErrFile::AlreadyOpened) {
            return ErrFile::AlreadyOpened;
        }
        auto id = assginHandleForOpenPluginView(plg, view);

        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::PluginFileOpened,
            {quintptr(plg), file, id,
             QVariant::fromValue(_win->getEditorViewFileType(view))});

        return id;
    } else {
        return ret;
    }
}

int PluginSystem::openFile(const QObject *sender, const QString &filename) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {filename})) {
        return ErrFile::Permission;
    }

    EditorView *view = nullptr;
    if (!checkPluginCanOpenedFile(plg)) {
        return ErrFile::TooManyOpenedFile;
    }
    auto ret = _win->openFile(filename, &view);
    if (view) {
        if (ret == ErrFile::AlreadyOpened) {
            return ErrFile::AlreadyOpened;
        }
        auto id = assginHandleForOpenPluginView(plg, view);
        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::PluginFileOpened,
            {quintptr(plg), filename, id,
             QVariant::fromValue(_win->getEditorViewFileType(view))});

        return id;
    } else {
        return ret;
    }
}

int PluginSystem::newFile(const QObject *sender) {
    if (!checkThreadAff()) {
        return ErrFile::NotAllowedInNoneGUIThread;
    }

    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return ErrFile::Error;
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return ErrFile::Permission;
    }

    if (!checkPluginCanOpenedFile(plg)) {
        return ErrFile::TooManyOpenedFile;
    }
    auto view = _win->newfileGUI();
    if (view) {
        auto id = assginHandleForOpenPluginView(plg, view);
        m_plgviewMap[plg].currentFID = id;

        PluginSystem::instance().dispatchEvent(
            IWingPlugin::RegisteredEvent::PluginFileOpened,
            {quintptr(plg),
             {},
             id,
             QVariant::fromValue(_win->getEditorViewFileType(view))});

        return id;
    } else {
        return ErrFile::Error;
    }
}

HexPosition PluginSystem::selectionEnd(const QObject *sender, qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {index})) {
        return {};
    }

    HexPosition pos;
    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto cursor = e->hexEditor()->cursor();
        if (index >= 0 && index < cursor->selectionCount()) {
            auto qpos = cursor->selectionEnd(index);
            pos.line = qpos.line;
            pos.column = qpos.column;
            pos.lineWidth = qpos.lineWidth;
            pos.nibbleindex = qpos.nibbleindex;
        }
    }
    return pos;
}

HexPosition PluginSystem::selectionStart(const QObject *sender,
                                         qsizetype index) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {index})) {
        return {};
    }

    HexPosition pos;
    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto cursor = e->hexEditor()->cursor();
        if (index >= 0 && index < cursor->selectionCount()) {
            auto qpos = cursor->selectionStart(index);
            pos.line = qpos.line;
            pos.column = qpos.column;
            pos.lineWidth = qpos.lineWidth;
            pos.nibbleindex = qpos.nibbleindex;
        }
    }
    return pos;
}

HexPosition PluginSystem::currentPos(const QObject *sender) {
    auto plg = checkPluginAndReport(sender, __func__);
    if (plg == nullptr) {
        return {};
    }

    if (passByFailedGuard(sender, __func__, {})) {
        return {};
    }

    HexPosition pos;
    auto e = pluginCurrentEditor(plg);
    if (e) {
        auto cursor = e->hexEditor()->cursor();
        pos.line = cursor->currentLine();
        pos.column = cursor->currentColumn();
        pos.lineWidth = e->hexEditor()->document()->hexLineWidth();
        pos.nibbleindex = cursor->currentNibble();
    }
    return pos;
}

AppTheme PluginSystem::currentAppTheme(const QObject *sender) {
    if (passByFailedGuard(sender, __func__, {})) {
        return WingHex::AppTheme::Invalid;
    }

    auto theme = SkinManager::instance().currentTheme();
    switch (theme) {
    case SkinManager::Theme::Dark:
        return WingHex::AppTheme::Dark;
    case SkinManager::Theme::Light:
        return WingHex::AppTheme::Light;
    }
    return WingHex::AppTheme::Invalid;
}

void PluginSystem::logWarn(const QObject *sender, const QString &message) {
    if (passByFailedGuard(sender, __func__, {message})) {
        return;
    }

    Logger::warning(packLogMessage(sender->metaObject()->className(), message));
}

// =========================================================================

QString PluginSystem::currentLoadingPlugin() const { return _curLoadingPlg; }

QStringList PluginSystem::scriptMarcos() const { return _scriptMarcos; }

void PluginSystem::scriptPragmaBegin() { _pragmaedPlg.clear(); }

qsizetype PluginSystem::pluginAPICount() const { return _plgFns.size(); }

const QList<IWingPlugin *> &PluginSystem::plugins() const {
    return _loadedplgs;
}

IWingPlugin *PluginSystem::plugin(qsizetype index) const {
    return _loadedplgs.at(index);
}

const QList<IWingDevice *> &PluginSystem::devices() const {
    return _loadeddevs;
}

IWingDevice *PluginSystem::device(qsizetype index) const {
    return _loadeddevs.at(index);
}

template <typename T>
std::optional<PluginInfo> PluginSystem::loadPlugin(const QFileInfo &fileinfo,
                                                   const QDir &setdir) {
    Q_ASSERT(_win);

    if (fileinfo.exists()) {
        auto fileName = fileinfo.absoluteFilePath();
        _curLoadingPlg =
            QDir(qApp->applicationDirPath()).relativeFilePath(fileName);

        QScopeGuard g([this]() { _curLoadingPlg.clear(); });
        QPluginLoader loader(fileName, this);
        Logger::info(tr("LoadingPlugin") + fileinfo.fileName());

        auto lmeta = loader.metaData();
        PluginStatus cret;
        std::optional<PluginInfo> meta;
        if (lmeta.contains("MetaData")) {
            auto m = parsePluginMetadata(lmeta["MetaData"].toObject());
            cret = checkPluginMetadata(m, std::is_same_v<T, IWingPlugin>);
            meta = m;
        } else {
            cret = PluginStatus::BrokenVersion;
        }

        switch (cret) {
        case PluginStatus::Valid:
            // OK and success
            break;
        case PluginStatus::BrokenVersion:
            Logger::critical(tr("InvalidPluginBrokenInfo"));
            return std::nullopt;
        case PluginStatus::LackDependencies: {
            _lazyplgs.append(fileName);
            return meta;
        } break;
        case PluginStatus::InvalidID:
            Logger::critical(tr("InvalidPluginID"));
            return std::nullopt;
        case PluginStatus::DupID:
            Logger::critical(tr("InvalidDupPlugin"));
            return std::nullopt;
        case PluginStatus::SDKVersion:
            Logger::critical(tr("ErrLoadPluginSDKVersion"));
            return std::nullopt;
        }

        auto m = meta.value();

        if (_manager) {
            if (!_manager->onLoadingPlugin(fileName, m)) {
                if constexpr (std::is_same_v<T, IWingPlugin>) {
                    _blkplgs[BlockReason::BlockedByManager].append(m);
                } else if constexpr (std::is_same_v<T, IWingDevice>) {
                    _blkdevs[BlockReason::BlockedByManager].append(m);
                }
                qCritical(
                    "[PluginSystem::loadPlugin] '%s' was blocked by manager",
                    qUtf8Printable(m.id));
                return std::nullopt;
            }
        }

        if constexpr (std::is_same_v<T, IWingPlugin>) {
            if (!_enabledExtIDs.contains(m.id)) {
                _blkplgs[BlockReason::Disabled].append(m);
                return std::nullopt;
            }
        } else if constexpr (std::is_same_v<T, IWingDevice>) {
            if (!_enabledDevIDs.contains(m.id)) {
                _blkdevs[BlockReason::Disabled].append(m);
                return std::nullopt;
            }
        }

        auto p = qobject_cast<T *>(loader.instance());
        if (Q_UNLIKELY(p == nullptr)) {
            Logger::critical(loader.errorString());
        } else {
            retranslateMetadata(p, m);
            loadPlugin(p, m, setdir);
        }
        Logger::newLine();
    }

    return std::nullopt;
}

WingAngelAPI *PluginSystem::angelApi() const { return _angelplg; }

EditorView *PluginSystem::getCurrentPluginView(IWingPlugin *plg) {
    if (plg == nullptr) {
        return nullptr;
    }

    auto fid = m_plgviewMap[plg].currentFID;
    auto ctx = pluginContextById(plg, fid);
    if (ctx) {
        return ctx->view;
    }
    return _win->m_curEditor;
}

EditorView *PluginSystem::handle2EditorView(IWingPlugin *plg, int handle) {
    if (handle < 0) {
        return getCurrentPluginView(plg);
    }

    auto ctx = pluginContextById(plg, handle);
    if (ctx) {
        return ctx->view;
    }

    return nullptr;
}

bool PluginSystem::equalCompareHandle(const SharedUniqueId &id, int handle) {
    return (*id) == handle;
}

int PluginSystem::getUIDHandle(const SharedUniqueId &id) {
    Q_ASSERT(id.data());
    return int(*id);
}

void PluginSystem::retranslateMetadata(IWingPluginBase *plg, PluginInfo &meta) {
    meta.author = plg->retranslate(meta.author);
    meta.vendor = plg->retranslate(meta.vendor);
    meta.license = plg->retranslate(meta.license);
}

int PluginSystem::assginHandleForOpenPluginView(IWingPlugin *plg,
                                                EditorView *view) {
    if (plg == nullptr || view == nullptr) {
        return {};
    }

    auto id = m_idGen.get();

    auto context = QSharedPointer<PluginFileContext>::create();
    context->fid = id;
    context->linkedplg = plg;
    context->view = view;

    auto handle = getUIDHandle(id);

    m_plgviewMap[plg].currentFID = handle;
    m_plgviewMap[plg].contexts.append(context);
    m_viewBindings[view].linkedplg.append(plg);

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
                }
            }
        }
    }

    return info;
}

PluginSystem::PluginStatus
PluginSystem::checkPluginMetadata(const PluginInfo &meta, bool isPlg) {
    constexpr auto puid_limit = 36; // same as uuid length, so enough

    if (meta.SDKVersion != SDKVERSION) {
        return PluginStatus::SDKVersion;
    }

    if (meta.id.length() > puid_limit) {
        return PluginStatus::InvalidID;
    }

    if (!Utilities::isValidIdentifier(meta.id)) {
        return PluginStatus::InvalidID;
    }

    if (isPluginLoaded(meta.id)) {
        return PluginStatus::DupID;
    }
    if (meta.version.isNull()) {
        return PluginStatus::BrokenVersion;
    }
    if (isPlg) {
        for (auto &d : meta.dependencies) {
            if (!isPluginLoaded(d)) {
                return PluginStatus::LackDependencies;
            }
        }
    }
    return PluginStatus::Valid;
}

bool PluginSystem::checkPluginCanOpenedFile(IWingPlugin *plg) {
    if (plg == nullptr) {
        return false;
    }
    return m_plgviewMap[plg].contexts.size() <= UCHAR_MAX;
}

bool PluginSystem::checkPluginHasAlreadyOpened(IWingPlugin *plg,
                                               EditorView *view) {
    auto &maps = m_plgviewMap[plg].contexts;
    auto ret =
        std::find_if(maps.begin(), maps.end(),
                     [view](const QSharedPointer<PluginFileContext> &content) {
                         return content->view == view;
                     });
    return ret != maps.end();
}

void PluginSystem::cleanUpEditorViewHandle(EditorView *view) {
    if (m_viewBindings.contains(view)) {
        auto v = m_viewBindings[view];

        // clean up
        for (auto &plg : v.linkedplg) {
            auto &handles = m_plgviewMap[plg];
            auto id = handles.currentFID;
            handles.contexts.removeIf(
                [view, this, id,
                 plg](const QSharedPointer<PluginFileContext> &v) {
                    if (v->view == view) {
                        if (equalCompareHandle(v->fid, id)) {
                            m_plgviewMap[plg].currentFID = -1;
                        }
                        dispatchEvent(
                            IWingPlugin::RegisteredEvent::PluginFileClosed,
                            {quintptr(v->linkedplg), v->view->fileNameUrl(),
                             getUIDHandle(v->fid),
                             QVariant::fromValue(
                                 _win->getEditorViewFileType(view))});
                        return true;
                    }
                    return false;
                });
        }
        m_viewBindings.remove(view);
    }
}

bool PluginSystem::closeEditor(IWingPlugin *plg, int handle, bool force) {
    if (handle >= 0) {
        auto &vm = m_plgviewMap[plg];
        auto &handles = vm.contexts;
        auto r = std::find_if(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            handles.cbegin(), handles.cend(),
#else
            handles.begin(), handles.end(),
#endif
            [handle](const QSharedPointer<PluginFileContext> &d) {
                return equalCompareHandle(d->fid, handle);
            });
        if (r == handles.end()) {
            return false;
        }
        auto sharedc = *r;

        auto &vb = m_viewBindings[sharedc->view];
        auto &bind = vb.linkedplg;
        bind.removeOne(plg);
        if (bind.isEmpty()) {
            _win->closeEditor(sharedc->view, force);
            m_viewBindings.remove(sharedc->view);
        }

        if (vm.currentFID == handle) {
            vm.currentFID = -1;
        }

        handles.erase(r);
    } else {
        return false;
    }

    return true;
}

bool PluginSystem::closeHandle(IWingPlugin *plg, int handle) {
    if (handle < 0) {
        return false;
    }
    auto &vm = m_plgviewMap[plg];
    auto &handles = vm.contexts;
    auto r = std::find_if(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        handles.cbegin(), handles.cend(),
#else
        handles.begin(), handles.end(),
#endif
        [handle](const QSharedPointer<PluginFileContext> &d) {
            return equalCompareHandle(d->fid, handle);
        });
    if (r == handles.end()) {
        return false;
    }
    auto sharedc = *r;

    auto &bind = m_viewBindings[sharedc->view].linkedplg;
    bind.removeOne(plg);
    if (bind.isEmpty()) {
        m_viewBindings.remove(sharedc->view);
    }

    if (vm.currentFID == handle) {
        vm.currentFID = -1;
    }
    handles.erase(r);
    return true;
}

bool PluginSystem::dispatchEvent(IWingPlugin::RegisteredEvent event,
                                 const QVariantList &params) {
    switch (event) {
    case WingHex::IWingPlugin::RegisteredEvent::SelectionChanged: {
        Q_ASSERT(params.size() == 2 &&
                 params.at(0).canConvert<QByteArrayList>() &&
                 params.at(1).canConvert<bool>());
        auto buffers = params.first().value<QByteArrayList>();
        auto isPreview = params.at(1).toBool();
        for (auto &plg : _evplgs[event]) {
            plg->eventSelectionChanged(buffers, isPreview);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::CursorPositionChanged: {
        Q_ASSERT(params.size() == 1 && params.at(0).canConvert<QHexPosition>());
        auto cursor = params.at(0).value<QHexPosition>();
        HexPosition pos;
        pos.line = cursor.line;
        pos.column = cursor.column;
        pos.nibbleindex = cursor.nibbleindex;
        pos.lineWidth = cursor.lineWidth;
        for (auto &plg : _evplgs[event]) {
            plg->eventCursorPositionChanged(pos);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileOpened: {
        Q_ASSERT(params.size() == 2);
        auto fileName = params.at(0).toUrl();
        auto fileType = params.at(1).value<WingHex::IWingPlugin::FileType>();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Opened, fileType,
                                 fileName, -1, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileSaved: {
        Q_ASSERT(params.size() == 4);
        auto newFileName = params.at(0).toUrl();
        auto oldFileName = params.at(1).toUrl();
        auto isExported = params.at(2).toBool();
        auto fileType = params.at(3).value<WingHex::IWingPlugin::FileType>();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(isExported
                                     ? IWingPlugin::PluginFileEvent::Exported
                                     : IWingPlugin::PluginFileEvent::Saved,
                                 fileType, oldFileName, -1, newFileName);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileSwitched: {
        Q_ASSERT(params.size() == 2);
        auto newFileName = params.at(0).toUrl();
        auto oldFileName = params.at(1).toUrl();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Switched,
                                 IWingPlugin::FileType::Invalid, oldFileName,
                                 -1, newFileName);
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::AppReady: {
        Q_ASSERT(params.isEmpty());
        for (auto &plg : _evplgs[event]) {
            plg->eventReady();
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::FileClosed: {
        Q_ASSERT(params.size() == 2);
        auto fileName = params.first().toString();
        auto fileType = params.at(1).value<WingHex::IWingPlugin::FileType>();
        for (auto &plg : _evplgs[event]) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::Closed, fileType,
                                 fileName, -1, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::ScriptPragma: {
        Q_ASSERT(false);
        // should not go there, call processScriptPragma instead
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::ScriptPragmaInit: {
        Q_ASSERT(false);
        // should not go there, lazy calling instead
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::PluginFileOpened: {
        Q_ASSERT(params.size() == 4);
        auto plg =
            reinterpret_cast<IWingPlugin *>(params.at(0).value<quintptr>());
        auto fileName = params.at(1).toString();
        auto id = params.at(2).toInt();
        auto fileType = params.at(3).value<WingHex::IWingPlugin::FileType>();
        if (_evplgs[event].contains(plg)) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::PluginOpened,
                                 fileType, fileName, id, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::PluginFileClosed: {
        Q_ASSERT(params.size() == 4);
        auto plg =
            reinterpret_cast<IWingPlugin *>(params.at(0).value<quintptr>());
        auto fileName = params.at(1).toString();
        auto id = params.at(2).toInt();
        auto fileType = params.at(3).value<WingHex::IWingPlugin::FileType>();
        if (_evplgs[event].contains(plg)) {
            plg->eventPluginFile(IWingPlugin::PluginFileEvent::PluginClosed,
                                 fileType, fileName, id, {});
        }
    } break;
    case WingHex::IWingPlugin::RegisteredEvent::AppClosing:
        for (auto &plg : _evplgs[event]) {
            auto ret = plg->eventClosing();
            if (!ret) {
                Logger::warning(tr("AppClosingCanceled:") + plg->pluginName() +
                                QStringLiteral(" (") + getPluginID(plg) +
                                QStringLiteral(")"));
                return ret;
            }
        }
        break;
    case WingHex::IWingPlugin::RegisteredEvent::HexEditorViewPaint: {
        auto painter =
            reinterpret_cast<QPainter *>(params.at(0).value<quintptr>());
        auto w = reinterpret_cast<QWidget *>(params.at(1).value<quintptr>());
        auto palette = reinterpret_cast<HexEditorContext *>(
            params.at(2).value<quintptr>());
        for (auto &plg : _evplgs[event]) {
            plg->onPaintHexEditorView(painter, w, palette);
        }
    } break;
    default:
        return false;
    }
    return true;
}

std::optional<PragmaResult>
PluginSystem::processPragma(const QString &section, const QString &plgId,
                            const QStringList &params) {

    auto &es = _evplgs[WingHex::IWingPlugin::RegisteredEvent::ScriptPragma];
    auto r =
        std::find_if(es.constBegin(), es.constEnd(), [plgId](IWingPlugin *p) {
            return plgId.compare(getPUID(p), Qt::CaseInsensitive) == 0;
        });
    if (r == es.constEnd()) {
        PragmaResult res;
        res.error.append(QStringLiteral("Unknown pragma command %1 with %2")
                             .arg(params.join(' '), plgId));
        return res;
    }
    auto plg = *r;
    if (!_pragmaedPlg.contains(plg)) {
        plg->eventOnScriptPragmaInit();
        _pragmaedPlg.append(plg);
    }
    return plg->eventOnScriptPragma(section, params);
}

IWingDevice *PluginSystem::ext2Device(const QString &ext) {
    auto r =
        std::find_if(_loadeddevs.begin(), _loadeddevs.end(),
                     [=](IWingDevice *dev) { return _pinfos[dev].id == ext; });
    if (r == _loadeddevs.end()) {
        return nullptr;
    }
    return *r;
}

PluginInfo PluginSystem::getPluginInfo(IWingPluginBase *plg) const {
    Q_ASSERT(plg);
    return _pinfos.value(plg);
}

QString PluginSystem::getPluginID(IWingPluginBase *plg) const {
    Q_ASSERT(plg);
    return _pinfos.value(plg).id;
}

void PluginSystem::loadExtPlugin() {
    QDir plugindir(QCoreApplication::applicationDirPath() + QDir::separator() +
                   QStringLiteral("plugin"));
    plugindir.setNameFilters({"*.wingplg"});

    checkDirRootSafe(plugindir);

    auto plgs = plugindir.entryInfoList();
    Logger::info(tr("FoundPluginCount") + QString::number(plgs.count()));

    if (!plgs.isEmpty()) {
        Logger::newLine();
    }

    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("plgset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        throw CrashCode::PluginSetting;
    }

    for (auto &item : plgs) {
        loadPlugin<IWingPlugin>(item, udir);
    }

    QList<PluginInfo> errorplg;
    if (!_lazyplgs.isEmpty()) {
        QStringList lazyplgs;
        lazyplgs.swap(_lazyplgs);

        for (auto &item : lazyplgs) {
            auto r = loadPlugin<IWingPlugin>(QFileInfo(item), udir);
            if (r) {
                errorplg.append(r.value());
            }
        }
    }

    if (!_lazyplgs.isEmpty()) {
        Logger::critical(tr("PluginLoadingFailedSummary"));
        Logger::newLine();

        for (auto &lplg : errorplg) {
            Logger::critical(tr("- PluginID:") + lplg.id);
            Logger::critical(tr("- Dependencies:"));
            for (auto &d : lplg.dependencies) {
                Logger::critical(QString(4, ' ') + tr("PUID:") + d.puid);
                Logger::critical(QString(4, ' ') + tr("Version:") +
                                 d.version.toString());
            }
        }
        _lazyplgs.clear();
    }

    Logger::info(tr("PluginLoadingFinished"));
}

void PluginSystem::loadDevicePlugin() {
    QDir devdir(QCoreApplication::applicationDirPath() + QDir::separator() +
                QStringLiteral("devdrv"));
    devdir.setNameFilters({"*.wingdrv"});

    checkDirRootSafe(devdir);

    auto plgs = devdir.entryInfoList();
    Logger::info(tr("FoundDrvPluginCount") + QString::number(plgs.count()));
    if (!plgs.isEmpty()) {
        Logger::newLine();
    }

    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("drvset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        throw CrashCode::PluginSetting;
    }

    for (auto &item : plgs) {
        loadPlugin<IWingDevice>(item, udir);
    }
}

void PluginSystem::checkDirRootSafe(const QDir &dir) {
    if (!Utilities::isRoot()) {
        auto testFileName =
            dir.absoluteFilePath(QUuid::createUuid().toString(QUuid::Id128));

        QFile f(testFileName);
        if (f.open(QFile::WriteOnly)) {
            f.close();
            f.remove();
            Logger::warning(QStringLiteral("<i><u>") + tr("UnsafePluginDir") +
                            QStringLiteral("</u></i>"));
        }
    }
}

void PluginSystem::try2LoadManagerPlugin() {
    QDir dir(qApp->applicationDirPath());

    auto mplgs = dir.entryInfoList({"*.wingman"}, QDir::Files);
    if (mplgs.isEmpty()) {
        return;
    }

    if (mplgs.size() > 1) {
        Logger::warning(tr("PluginManagerNeedSingleton"));
        return;
    }

    auto mplg = mplgs.front();
    QPluginLoader loader(mplg.absoluteFilePath());

    auto lmeta = loader.metaData();
    auto m = parsePluginMetadata(lmeta["MetaData"].toObject());

    auto cret = checkPluginMetadata(m, false);

    switch (cret) {
    case PluginStatus::Valid:
        break;
    case PluginStatus::SDKVersion:
        Logger::critical(tr("ErrLoadPluginSDKVersion"));
        return;
    case PluginStatus::InvalidID:
        Logger::critical(tr("InvalidPluginID"));
        return;
    case PluginStatus::BrokenVersion:
        Logger::critical(tr("InvalidPluginBrokenInfo"));
        return;
    case PluginStatus::DupID:
    case PluginStatus::LackDependencies:
        // monitor is the first plugin to load and
        // should not have any dependency
        Q_ASSERT(false);
        return;
    }

    auto p = qobject_cast<IWingManager *>(loader.instance());
    if (p) {
        QDir udir(Utilities::getAppDataPath());
        auto plgset = QStringLiteral("plgset");
        udir.mkdir(plgset);
        if (!udir.cd(plgset)) {
            throw CrashCode::PluginSetting;
        }

        auto setp = std::make_unique<QSettings>(udir.absoluteFilePath(m.id),
                                                QSettings::Format::IniFormat);

        if (!p->init(setp)) {
            setp->deleteLater();
            Logger::critical(tr("ErrLoadInitPlugin"));
            return;
        }

        applyFunctionTables(p, _plgFns);
        _manager = p;
        registerPluginDetectMarco(m.id);

        // translate the meta-data
        m.author = p->retranslate(m.author);
        m.vendor = p->retranslate(m.vendor);
        m.license = p->retranslate(m.license);

        _manInfo = m;

        registerRibbonTools(p->registeredRibbonTools());
        registeredSettingPages(QVariant::fromValue(p),
                               p->registeredSettingPages());
    }
}

void PluginSystem::try2LoadHexExtPlugin() {
    QDir dir(qApp->applicationDirPath());

    auto mplgs = dir.entryInfoList({"*.winghexe"}, QDir::Files);
    if (mplgs.isEmpty()) {
        return;
    }

    if (mplgs.size() > 1) {
        Logger::warning(tr("HexExtNeedSingleton"));
        return;
    }

    auto mplg = mplgs.front();
    auto path = mplg.absoluteFilePath();
    QPluginLoader loader(path);

    auto lmeta = loader.metaData();
    auto m = parsePluginMetadata(lmeta["MetaData"].toObject());

    auto cret = checkPluginMetadata(m, false);

    switch (cret) {
    case PluginStatus::Valid:
        break;
    case PluginStatus::SDKVersion:
        Logger::critical(tr("ErrLoadPluginSDKVersion"));
        return;
    case PluginStatus::InvalidID:
        Logger::critical(tr("InvalidPluginID"));
        return;
    case PluginStatus::BrokenVersion:
        Logger::critical(tr("InvalidPluginBrokenInfo"));
        return;
    case PluginStatus::DupID:
    case PluginStatus::LackDependencies:
        // monitor is the first plugin to load and
        // should not have any dependency
        Q_ASSERT(false);
        return;
    }

    auto p = qobject_cast<IWingHexEditorPlugin *>(loader.instance());
    if (p) {
        QDir udir(Utilities::getAppDataPath());
        auto plgset = QStringLiteral("plgset");
        udir.mkdir(plgset);
        if (!udir.cd(plgset)) {
            throw CrashCode::PluginSetting;
        }

        auto setp = std::make_unique<QSettings>(udir.absoluteFilePath(m.id),
                                                QSettings::Format::IniFormat);

        if (!p->init(setp)) {
            setp->deleteLater();
            Logger::critical(tr("ErrLoadInitPlugin"));
            return;
        }

        applyFunctionTables(p, _plgFns);
        _hexExt = p;
        registerPluginDetectMarco(m.id);

        // translate the meta-data
        m.author = p->retranslate(m.author);
        m.vendor = p->retranslate(m.vendor);
        m.license = p->retranslate(m.license);

        _manHexInfo = m;

        registerHexContextMenu(p);
        registerRibbonTools(p->registeredRibbonTools());
        registeredSettingPages(QVariant::fromValue(p),
                               p->registeredSettingPages());
    }
}

void PluginSystem::registerPluginDetectMarco(const QString &id) {
    static auto sep = QStringLiteral("_");
    _scriptMarcos.append(sep + id + sep);
}

void PluginSystem::registerEvents(IWingPlugin *plg) {
    Q_ASSERT(plg);
    auto evs = plg->registeredEvents();
    using FlagInt = decltype(evs)::Int;
    if (FlagInt(evs) == FlagInt(IWingPlugin::RegisteredEvent::None)) {
        return;
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::SelectionChanged)) {
        _evplgs[IWingPlugin::RegisteredEvent::SelectionChanged].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::CursorPositionChanged)) {
        _evplgs[IWingPlugin::RegisteredEvent::CursorPositionChanged].append(
            plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileOpened)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileOpened].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileSaved)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileSaved].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileSwitched)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileSwitched].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::AppReady)) {
        _evplgs[IWingPlugin::RegisteredEvent::AppReady].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::AppClosing)) {
        _evplgs[IWingPlugin::RegisteredEvent::AppClosing].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::FileClosed)) {
        _evplgs[IWingPlugin::RegisteredEvent::FileClosed].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::ScriptPragma)) {
        _evplgs[IWingPlugin::RegisteredEvent::ScriptPragma].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::ScriptPragmaInit)) {
        _evplgs[IWingPlugin::RegisteredEvent::ScriptPragmaInit].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::PluginFileOpened)) {
        _evplgs[IWingPlugin::RegisteredEvent::PluginFileOpened].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::PluginFileClosed)) {
        _evplgs[IWingPlugin::RegisteredEvent::PluginFileClosed].append(plg);
    }

    if (evs.testFlag(IWingPlugin::RegisteredEvent::HexEditorViewPaint)) {
        _evplgs[IWingPlugin::RegisteredEvent::HexEditorViewPaint].append(plg);
    }
}

void PluginSystem::registerHexContextMenu(IWingHexEditorInterface *inter) {
    Q_ASSERT(inter);
    auto menu = inter->registeredHexContextMenu();
    if (menu) {
        _win->m_hexContextMenu.append(menu);
        connect(menu, &QMenu::aboutToShow, this, [menu, inter]() {
            auto pp = menu->property("__CONTEXT__");
            auto ptr =
                reinterpret_cast<HexEditorContext *>(pp.value<quintptr>());
            if (ptr) {
                inter->prepareCallEditorContext(ptr);
            }
        });
        connect(menu, &QMenu::triggered, this, [menu, inter]() {
            auto pp = menu->property("__CONTEXT__");
            auto ptr =
                reinterpret_cast<HexEditorContext *>(pp.value<quintptr>());
            if (ptr) {
                inter->finishCallEditorContext(ptr);
            }
        });
    }
}

void PluginSystem::applyFunctionTables(QObject *plg, const CallTable &fns) {
    plg->setProperty("__CALL_TABLE__", QVariant::fromValue(fns));
    plg->setProperty("__CALL_POINTER__", quintptr(this));
}

QString PluginSystem::getPUID(IWingPluginBase *p) {
    if (p) {
        return PluginSystem::instance().getPluginID(p);
    } else {
        return {};
    }
}

bool PluginSystem::isPluginLoaded(const WingDependency &d) {
    for (auto &info : _pinfos) {
        if ((d.version.isNull() || info.version >= d.version) &&
            info.id == d.puid) {
            return true;
        }
    }
    return false;
}

bool PluginSystem::isPluginLoaded(const QString &id) {
    for (auto &info : _pinfos) {
        if (info.id.compare(id, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

void PluginSystem::loadPlugin(IWingPlugin *p, PluginInfo &meta,
                              const std::optional<QDir> &setdir) {
    QTranslator *p_tr = nullptr;

    try {
        if (!p->pluginName().trimmed().length()) {
            throw tr("ErrLoadPluginNoName");
        }

        // dependencies had been checked

        Q_EMIT pluginLoading(p->pluginName());

        p_tr = LanguageManager::instance().try2LoadPluginLang(meta.id);

        applyFunctionTables(p, _plgFns);

        {
            std::unique_ptr<QSettings> setp(nullptr);
            if (setdir.has_value()) {
                setp = std::make_unique<QSettings>(
                    setdir->absoluteFilePath(meta.id),
                    QSettings::Format::IniFormat);
            }

            if (!p->init(setp)) {
                setp->deleteLater();
                throw tr("ErrLoadInitPlugin");
            }
        }

        _loadedplgs.append(p);
        _pinfos.insert(p, meta);

        registerPluginDetectMarco(meta.id);

        Logger::info(tr("PluginName :") + p->pluginName());
        Logger::debug(tr("PluginAuthor :") + meta.author);
        Logger::debug(tr("PluginWidgetRegister"));

        // ensure call only once
        registerRibbonTools(p->registeredRibbonTools());
        registerPluginDockWidgets(p);
        registerHexContextMenu(p);

        {
            auto vieww = p->registeredEditorViewWidgets();
            if (!vieww.isEmpty()) {
                _win->m_editorViewWidgets.insert(p, vieww);
            }
        }

        registeredSettingPages(QVariant::fromValue(p),
                               p->registeredSettingPages());

        registerEvents(p);

        // prepare for file contenxt
        m_plgviewMap.insert(p, {});
    } catch (const QString &error) {
        Logger::critical(error);
        if (p_tr) {
            p_tr->deleteLater();
        }
    }
}

void PluginSystem::loadPlugin(IWingDevice *p, PluginInfo &meta,
                              const std::optional<QDir> &setdir) {
    QTranslator *p_tr = nullptr;

    try {
        Logger::debug(tr("ExtPluginAuthor :") + meta.author);
        Logger::debug(tr("ExtPluginWidgetRegister"));

        p_tr = LanguageManager::instance().try2LoadPluginLang(meta.id);

        applyFunctionTables(p, _plgFns);

        {
            std::unique_ptr<QSettings> setp(nullptr);
            if (setdir.has_value()) {
                setp = std::make_unique<QSettings>(
                    setdir->absoluteFilePath(p->metaObject()->className()),
                    QSettings::Format::IniFormat);
            }

            if (!p->init(setp)) {
                setp->deleteLater();
                throw tr("ErrLoadInitExtPlugin");
            }
        }

        _loadeddevs.append(p);
        _pinfos.insert(p, meta);
        registerPluginDetectMarco(meta.id);

        // ok register into menu open
        auto menu =
            _win->m_toolBtneditors[MainWindow::ToolButtonIndex::OPEN_EXT]
                ->menu();
        menu->addAction(_win->newAction(
            p->supportedFileIcon(), p->supportedFileExtDisplayName(),
            [p, this]() {
                auto file = p->onOpenFileBegin();
                if (file.isNull()) {
                    return;
                }

                if (file.isEmpty()) {
                    // common dialog
                    file = WingFileDialog::getOpenFileName(
                        _win, tr("ChooseFile"), _win->m_lastusedpath);
                }

                EditorView *view = nullptr;
                auto ext = getPluginID(p);
                auto res = _win->openExtFile(ext, file, &view);

                if (res == ErrFile::AlreadyOpened) {
                    Q_ASSERT(view);
                    view->raise();
                    view->setFocus();
                    return;
                } else {
                    if (_win->reportErrFileError(res, {}, {}, {})) {
                        RecentFileManager::RecentInfo info;
                        info.url = Utilities::getDeviceFileName(ext, file);
                        _win->m_recentmanager->addRecentFile(info);
                    }
                }
            }));

    } catch (const QString &error) {
        Logger::critical(error);
        if (p_tr) {
            p_tr->deleteLater();
        }
    }
}

void PluginSystem::registerPluginDockWidgets(IWingPluginBase *p) {
    auto dockWidgets = p->registeredDockWidgets();
    if (!dockWidgets.isEmpty()) {
        for (auto &info : dockWidgets) {
            auto widgetName = info.widgetName.trimmed();
            auto displayName = info.displayName.trimmed();
            if (displayName.isEmpty()) {
                displayName = widgetName;
            }
            if (widgetName.isEmpty()) {
                Logger::warning(tr("EmptyNameDockWidget:") +
                                QStringLiteral("{ ") + p->pluginName() +
                                QStringLiteral(" }"));
                continue;
            }

            auto inch = std::find_if_not(
                widgetName.begin(), widgetName.end(),
                [](const QChar &ch) { return ch.isLetterOrNumber(); });
            if (inch != widgetName.end()) {
                Logger::warning(tr("InvalidNameDockWidget:") +
                                QStringLiteral("{ ") + p->pluginName() +
                                QStringLiteral(" } ") + widgetName);
                continue;
            }

            if (info.widget == nullptr) {
                Logger::warning(
                    tr("InvalidNullDockWidget:") + QStringLiteral("{ ") +
                    p->pluginName() + QStringLiteral(" } ") + widgetName +
                    QStringLiteral(" ( ") + displayName + QStringLiteral(" )"));
                continue;
            }

            auto dw =
                _win->buildDockWidget(_win->m_dock, widgetName, displayName,
                                      info.widget, MainWindow::PLUGIN_VIEWS);
            _raisedw.insert(info.widget, dw);

            switch (info.area) {
            case Qt::LeftDockWidgetArea: {
                if (_win->m_leftViewArea == nullptr) {
                    _win->m_leftViewArea = _win->m_dock->addDockWidget(
                        ads::LeftDockWidgetArea, dw);
                } else {
                    _win->m_leftViewArea = _win->m_dock->addDockWidget(
                        ads::CenterDockWidgetArea, dw, _win->m_leftViewArea);
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
}

void PluginSystem::registerRibbonTools(
    const QList<WingRibbonToolBoxInfo> &tools) {
    if (!tools.isEmpty()) {
        for (auto &item : tools) {
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
                    bool isSys = false;
                    RibbonTabContent *cat;
                    if (_win->m_ribbonMaps.contains(item.catagory)) {
                        isSys = true;
                        cat = _win->m_ribbonMaps.value(item.catagory);
                    } else {
                        cat = new RibbonTabContent;
                    }
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
                    if (!isSys) {
                        if (hasAdded) {
                            _win->m_ribbon->addTab(cat, item.displayName);
                            _win->m_ribbonMaps[item.catagory] = cat;
                        } else {
                            delete cat;
                        }
                    }
                }
            }
        }
    }
}

void PluginSystem::registeredSettingPages(const QVariant &itptr,
                                          const QList<SettingPage *> &pages) {
    if (!pages.isEmpty()) {
        for (auto &page : pages) {
            page->setProperty("__plg__", itptr);
        }
        _win->m_settingPages.append(pages);
    }
}

bool PluginSystem::checkThreadAff() {
    if (QThread::currentThread() != qApp->thread()) {
        Logger::warning(tr("Not allowed operation in non-UI thread"));
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

void PluginSystem::loadAllPlugins() {
    Q_ASSERT(_win);

    auto &set = SettingManager::instance();
    bool enableSet = set.enablePlugin();

    bool ok = false;
    auto disAll =
        qEnvironmentVariableIntValue("WING_DISABLE_PLUGIN_SYSTEM", &ok);
    auto marco_Enabled = !ok || (ok && !disAll);

    if (marco_Enabled) {
        if (enableSet) {
            auto dis =
                qEnvironmentVariableIntValue("WING_DISABLE_MONITOR", &ok);
            if (set.enableMonitor() && (!ok || (ok && !dis))) {
                try2LoadManagerPlugin();
            }
            dis = qEnvironmentVariableIntValue("WING_DISABLE_HEXEXT", &ok);
            if (set.enableHexExt() && (!ok || (ok && !dis))) {
                try2LoadHexExtPlugin();
            }
        }
    }

    _enabledExtIDs = set.enabledExtPlugins();
    _enabledDevIDs = set.enabledDevPlugins();

    // manager plugin can be blocked by settings only
    if (set.scriptEnabled()) {
        _angelplg = new WingAngelAPI;

        QFile angeljson(QStringLiteral(
            ":/com.wingsummer.winghex/src/class/WingAngelAPI.json"));
        auto ret = angeljson.open(QFile::ReadOnly);
        Q_ASSERT(ret);
        Q_UNUSED(ret);
        auto angelapi = angeljson.readAll();
        angeljson.close();

        QJsonDocument doc = QJsonDocument::fromJson(angelapi);
        auto meta = parsePluginMetadata(doc.object());
        Q_ASSERT(checkPluginMetadata(meta, true) == PluginStatus::Valid);
        retranslateMetadata(_angelplg, meta);
        loadPlugin(_angelplg, meta, std::nullopt);
    }

    Logger::newLine();

    if (marco_Enabled) {
        QFile cstructjson(QStringLiteral(
            ":/com.wingsummer.winghex/src/class/WingCStruct.json"));
        auto ret = cstructjson.open(QFile::ReadOnly);
        Q_ASSERT(ret);
        Q_UNUSED(ret);
        auto cstruct = cstructjson.readAll();
        cstructjson.close();

        QJsonDocument doc = QJsonDocument::fromJson(cstruct);
        auto meta = parsePluginMetadata(doc.object());
        Q_ASSERT(checkPluginMetadata(meta, true) == PluginStatus::Valid);

        // internal plugin has no filename
        if (_manager == nullptr ||
            (_manager && _manager->onLoadingPlugin({}, meta))) {
            if (_enabledExtIDs.contains(meta.id)) {
                auto cstructplg = new WingCStruct;
                QDir setd(Utilities::getAppDataPath());
                auto plgset = QStringLiteral("plgset");
                setd.mkdir(plgset);
                retranslateMetadata(cstructplg, meta);
                loadPlugin(cstructplg, meta, setd);
            } else {
                _blkplgs[BlockReason::Disabled].append(meta);
            }
        } else {
            _blkplgs[BlockReason::BlockedByManager].append(meta);
            Logger::critical(QStringLiteral("{ ") + meta.id +
                             QStringLiteral(" } ") +
                             tr("PluginBlockByManager"));
        }
    }

    Logger::newLine();

    if (enableSet) {
        if (marco_Enabled) {
            bool enabledrv = true, enableplg = true;
            auto disdrv =
                qEnvironmentVariableIntValue("WING_DISABLE_EXTDRV", &ok);
            if (ok && disdrv) {
                enabledrv = false;
            }

            if (enabledrv) {
                loadDevicePlugin();
            }

            auto displg =
                qEnvironmentVariableIntValue("WING_DISABLE_PLUGIN", &ok);
            if ((ok && displg) || !set.enablePlugin()) {
                enableplg = false;
            }

            if (Utilities::isRoot()) {
                if (!set.enablePlgInRoot()) {
                    enableplg = false;
                }
            }

            if (enableplg) {
                loadExtPlugin();
            }
        }
        Logger::newLine();
    }
}

void PluginSystem::destory() {
    QDir udir(Utilities::getAppDataPath());
    auto plgset = QStringLiteral("plgset");
    udir.mkdir(plgset);
    if (!udir.cd(plgset)) {
        qApp->exit(int(CrashCode::PluginSetting));
    }

    for (auto &item : _loadedplgs) {
        auto set =
            std::make_unique<QSettings>(udir.absoluteFilePath(_pinfos[item].id),
                                        QSettings::Format::IniFormat);
        item->unload(set);
        delete item;
    }
    _loadedplgs.clear();

    for (auto &item : _loadeddevs) {
        auto set =
            std::make_unique<QSettings>(udir.absoluteFilePath(_pinfos[item].id),
                                        QSettings::Format::IniFormat);
        item->unload(set);
        delete item;
    }
    _loadeddevs.clear();

    if (_manager && _manInfo) {
        auto set = std::make_unique<QSettings>(
            udir.absoluteFilePath(_manInfo->id), QSettings::Format::IniFormat);
        _manager->unload(set);
        delete _manager;
        _manInfo.reset();
    }

    if (_hexExt && _manHexInfo) {
        auto set =
            std::make_unique<QSettings>(udir.absoluteFilePath(_manHexInfo->id),
                                        QSettings::Format::IniFormat);
        _hexExt->unload(set);
        delete _hexExt;
        _manHexInfo.reset();
    }
}

EditorView *PluginSystem::pluginCurrentEditor(IWingPlugin *plg) const {
    if (plg) {
        auto fid = m_plgviewMap[plg].currentFID;
        if (fid < 0) {
            return _win->currentEditor();
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
                              return equalCompareHandle(d->fid, fid);
                          });
    if (r == handles.end()) {
        return nullptr;
    }
    return *r;
}
