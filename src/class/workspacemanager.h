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

#ifndef WORKSPACEMANAGER_H
#define WORKSPACEMANAGER_H

#include "QHexView/document/qhexmetadata.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVector>

struct WorkSpaceInfo {
    quint64 base = 0;
    QByteArray checkSum;
    QMap<QString, QByteArray> pluginData;
};

struct WorkSpaceData {
    QUrl file;
    QMap<qsizetype, QString> bookmarks;
    QVector<QHexMetadataItem> metas;
    WorkSpaceInfo infos;
    bool corrupted = false;
};

class WorkSpaceManager {

public:
    explicit WorkSpaceManager();
    static bool saveWorkSpace(const QString &filename,
                              const WorkSpaceData &data);
    static bool loadWorkSpace(const QString &filename, WorkSpaceData &data,
                              QJsonDocument doc = {});

    static QJsonDocument loadWorkSpace(const QString &filename);
    static QUrl loadWorkSpaceDocFile(const QString &filename,
                                     const QJsonDocument &doc);

private:
    QString static getColorString(const QColor &color);

    static std::optional<quint64> readUInt64(const QJsonValue &value);
    static std::optional<QColor> readColor(const QJsonValue &value);

    static void readMetas(WorkSpaceData &data, const QJsonValue &values);
    static void readBookmarks(WorkSpaceData &data, const QJsonValue &values);
    static void loadPluginData(WorkSpaceData &data, const QJsonValue &values);

    static QString packupMessage(const QUrl &file, const QJsonValue &values,
                                 const QString &reason);
};

#endif // WORKSPACEMANAGER_H
