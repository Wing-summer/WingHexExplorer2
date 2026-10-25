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

#ifndef SCRIPTSERVICEAPI_H
#define SCRIPTSERVICEAPI_H

#include <QByteArray>
#include <QMetaType>
#include <QString>
#include <QVariant>

class QObject;
class PluginSystem;

bool invokeScriptService(PluginSystem &system, const QObject *sender,
                         const QString &puid, const QByteArray &method,
                         QMetaType returnType, const QVariantList &arguments,
                         QVariant &result);

#endif
