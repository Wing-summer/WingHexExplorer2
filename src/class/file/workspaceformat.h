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

#ifndef WORKSPACEFORMAT_H
#define WORKSPACEFORMAT_H

#include "workspacemanager.h"

#include <QDataStream>

namespace WorkspaceFormat {

inline constexpr char Magic[] = "WINGPRO";
inline constexpr quint16 Version = 1;

struct BookMark {
    quint64 pos = 0;
    QString comment;
};

struct MetaData {
    quint64 begin = 0;
    quint64 end = 0;
    QColor foreground;
    QColor background;
    QString comment;
};

QDataStream &operator<<(QDataStream &stream, const BookMark &item);
QDataStream &operator>>(QDataStream &stream, BookMark &item);
QDataStream &operator<<(QDataStream &stream, const MetaData &item);
QDataStream &operator>>(QDataStream &stream, MetaData &item);

} // namespace WorkspaceFormat

QDataStream &operator<<(QDataStream &stream, const WorkSpaceInfo &info);
QDataStream &operator>>(QDataStream &stream, WorkSpaceInfo &info);

#endif // WORKSPACEFORMAT_H
