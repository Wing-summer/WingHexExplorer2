/*==============================================================================
 ** Copyright (C) 2026-2029 WingSummer
 **
 ** This program is free software: you can redistribute it and/or modify it
 ** under the terms of the GNU Affero General Public License as published by the
 ** Free Software Foundation, version 3.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 ** for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program. If not, see <https://www.gnu.org/licenses/>.
 ** =============================================================================
 */

#include "scriptserviceapi.h"

#include <tuple>

#include <QList>

#include "class/plugin/pluginsystem.h"
#include "luau/plugin/scriptplugin.h"

bool invokeScriptService(PluginSystem &system, const QObject *sender,
                         const QString &puid, const QByteArray &method,
                         QMetaType returnType, const QVariantList &arguments,
                         QVariant &result) {
    QList<const void *> data;
    QList<const char *> names;
    QList<const QtPrivate::QMetaTypeInterface *> types;
    data.append(result.isValid() ? result.data() : nullptr);
    names.append(returnType.name());
    types.append(returnType.iface());
    for (const auto &argument : arguments) {
        data.append(argument.constData());
        names.append(argument.metaType().name());
        types.append(argument.metaType().iface());
    }
    const qsizetype count = data.size();
    data.append(nullptr);
    names.append("void");
    types.append(QMetaType::fromType<void>().iface());
    return system.invokeServiceImpl(
        sender, puid,
        std::make_tuple(method.constData(), Qt::DirectConnection, count,
                        data.constData(), names.constData(),
                        types.constData()));
}

bool invokeScriptPluginService(ScriptPluginRuntime &runtime,
                               const QObject *sender,
                               const WingHex::MetaCallInfo &info) {
    const auto &[method, connection, count, parameters, names, types] = info;
    if (!method || count < 1 || !parameters || !names || !types ||
        (connection != Qt::DirectConnection &&
         connection != Qt::AutoConnection)) {
        return false;
    }
    QVariantList arguments;
    for (qsizetype i = 1; i < count; ++i) {
        if (!types[i] || !parameters[i]) {
            return false;
        }
        arguments.append(QVariant(QMetaType(types[i]), parameters[i]));
    }
    try {
        if (auto *caller = scriptPluginRuntime(sender)) {
            for (auto &argument : arguments) {
                argument = caller->serviceValue(argument);
            }
        }
    } catch (const std::exception &error) {
        qWarning("Script service argument transfer failed: %s", error.what());
        return false;
    }
    QVariant result;
    if (!runtime.invokeExport(QByteArray(method), arguments, result)) {
        return false;
    }
    const QMetaType returnType(types[0]);
    if (returnType.id() == QMetaType::Void) {
        return true;
    }
    if (!parameters[0]) {
        return false;
    }
    auto *destination = const_cast<void *>(parameters[0]);
    if (returnType == QMetaType::fromType<QVariant>()) {
        *static_cast<QVariant *>(destination) = result;
        return true;
    }
    if (!result.convert(returnType)) {
        return false;
    }
    returnType.destruct(destination);
    returnType.construct(destination, result.constData());
    return true;
}
