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

#ifndef WORKSPACEMANAGER_H
#define WORKSPACEMANAGER_H

#include <QByteArray>
#include <QMap>
#include <QUrl>
#include <QVariantMap>
#include <QVector>

#include "QHexView/document/qhexmetadata.h"

struct WorkSpaceInfo {
    quint64 base = 0;
    QByteArray checkSum;
};

struct WorkSpaceData {
    QUrl file;
    QMap<qsizetype, QString> bookmarks;
    QVector<QHexMetadataItem> metas;
    WorkSpaceInfo infos;
    QVariantMap plugindata;
};

class WorkSpaceManager {
    Q_DISABLE_COPY_MOVE(WorkSpaceManager)

public:
    static bool saveWorkSpace(const QString &filename,
                              const WorkSpaceData &data,
                              QString *error = nullptr);
    static bool loadWorkSpace(const QString &filename, WorkSpaceData &data,
                              QString *error = nullptr);
    static bool validateWorkSpace(const WorkSpaceData &data,
                                  QString *error = nullptr);

    static QMap<QString, QByteArray> pluginData(const WorkSpaceData &data);
    static void setPluginData(WorkSpaceData &data,
                              const QMap<QString, QByteArray> &values);
    static void setPluginData(WorkSpaceData &data, const QString &id,
                              const QByteArray &value);
};

#endif // WORKSPACEMANAGER_H
