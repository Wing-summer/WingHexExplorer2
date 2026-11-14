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

#ifndef SCRIPTRPCERROR_H
#define SCRIPTRPCERROR_H

#include <variant>

#include <QJsonValue>
#include <QString>

// Stable JSON-RPC wire codes; application errors occupy the server-error range.
enum class ScriptRpcError : int {
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,
    UnknownTask = -32001,
    DuplicateTask = -32002,
    InvalidTaskState = -32003,
    TaskCapacity = -32004,
    InvalidScript = -32005,
    ServiceStopping = -32006
};

struct ScriptRpcFailure {
    ScriptRpcError code;
    QString message;
};

using ScriptRpcResult = std::variant<QJsonValue, ScriptRpcFailure>;
#endif // SCRIPTRPCERROR_H
