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

#include "class/plugin/pluginsystem.h"
#include <QList>
#include <tuple>

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
