#ifndef WORKSPACEMANAGER_H
#define WORKSPACEMANAGER_H

#include "../../QHexView/document/qhexdocument.h"
#include "../../QHexView/document/qhexmetadata.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QObject>
#include <QStringList>

struct WorkSpaceInfo {
    bool showaddr = true;
    bool showheader = true;
    bool showstr = true;
    bool locked = true;
    bool keepsize = true;
    QString encoding = "ASCII";
    qulonglong base = 0;
    bool showmetafg = true;
    bool showmetabg = true;
    bool showmetacomment = true;
};

class WorkSpaceManager {

public:
    explicit WorkSpaceManager();
    bool static saveWorkSpace(QString filename, QString file,
                              QList<BookMarkStruct> bookmarks,
                              QList<QHexMetadataAbsoluteItem> metas,
                              WorkSpaceInfo infos);
    bool static loadWorkSpace(QString filename, QString &file,
                              QList<BookMarkStruct> &bookmarks,
                              QList<QHexMetadataAbsoluteItem> &metas,
                              WorkSpaceInfo &infos);
};

#endif // WORKSPACEMANAGER_H
