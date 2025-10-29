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

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVector>

struct WorkSpaceInfo {
    qulonglong base = 0;
    QHash<QString, QByteArray> pluginData;
};

class WorkSpaceManager {

public:
    explicit WorkSpaceManager();
    bool static saveWorkSpace(const QString &filename, const QUrl &file,
                              const QMap<qsizetype, QString> &bookmarks,
                              const QVector<QHexMetadataItem> &metas,
                              const WorkSpaceInfo &infos);
    bool static loadWorkSpace(const QString &filename, QUrl &file,
                              QMap<qsizetype, QString> &bookmarks,
                              QVector<QHexMetadataItem> &metas,
                              WorkSpaceInfo &infos);

private:
    QString static getColorString(const QColor &color);
};

#endif // WORKSPACEMANAGER_H
