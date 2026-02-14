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

#include "workspacemanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>

WorkSpaceManager::WorkSpaceManager() {}

bool WorkSpaceManager::loadWorkSpace(const QString &filename, QUrl &file,
                                     QMap<qsizetype, QString> &bookmarks,
                                     QVector<QHexMetadataItem> &metas,
                                     WorkSpaceInfo &infos, QJsonDocument doc) {
    if (doc.isEmpty()) {
        doc = loadWorkSpace(filename);
    }

    if (doc.isEmpty()) {
        return false;
    }

    bool b;
    file = loadWorkSpaceDocFile(filename, doc);
    if (file.isValid()) {
        auto jobj = doc.object();
        qsizetype maxbytes = std::numeric_limits<qsizetype>::max();
        auto values = jobj.value("base");
        if (!values.isUndefined() && values.isString()) {
            auto ba = values.toString();
            auto nbase = ba.toULongLong(&b);
            if (b)
                infos.base = nbase;
        }

        values = jobj.value("checksum");
        if (values.isString()) {
            auto cs = values.toString();
            infos.checkSum = QByteArray::fromHex(cs.toUtf8());
        }

        values = jobj.value("metas");
        if (!values.isUndefined() && values.isArray()) {
            auto metaitems = values.toArray();
            for (auto &&item : metaitems) {
                auto linem = item.toObject();
                auto begin = linem.value("begin");
                auto end = linem.value("end");
                auto comment = linem.value("comment");
                auto fgcolor = linem.value("fgcolor");
                auto bgcolor = linem.value("bgcolor");
                if (!begin.isUndefined() && begin.isString() &&
                    !end.isUndefined() && end.isString() &&
                    !comment.isUndefined() && comment.isString() &&
                    !fgcolor.isUndefined() && fgcolor.isString() &&
                    !bgcolor.isUndefined() && bgcolor.isString()) {
                    auto nbegin = begin.toString().toLongLong(&b);
                    if (!b || nbegin >= maxbytes || nbegin < 0)
                        continue;
                    auto nend = end.toString().toLongLong(&b);
                    if (!b || nend >= maxbytes || nend < 0)
                        continue;
                    if (nbegin > nend)
                        continue;

                    QColor fcolor, bcolor;
                    auto fgn = fgcolor.toString();
                    fcolor = QColor(fgn);

                    auto bgn = bgcolor.toString();
                    bcolor = QColor(bgn);

                    QHexMetadataItem metaitem;
                    metaitem.begin = nbegin;
                    metaitem.end = nend;
                    metaitem.comment = comment.toString();
                    metaitem.foreground = fcolor;
                    metaitem.background = bcolor;
                    metas.append(metaitem);
                }
            }
        }
        values = jobj.value("bookmarks");
        if (!values.isUndefined() && values.isArray()) {
            auto array = values.toArray();
            for (auto &&item : array) {
                if (!item.isUndefined() && item.isObject()) {
                    auto sitem = item.toObject();
                    auto pos = sitem.value("pos");
                    auto comment = sitem.value("comment");
                    if (!pos.isUndefined() && pos.isString() &&
                        !comment.isUndefined() && comment.isString()) {
                        auto b = false;
                        auto ipos = pos.toString().toLongLong(&b);
                        if (!b || ipos < 0 || ipos >= maxbytes)
                            continue;
                        bookmarks.insert(ipos, comment.toString());
                    }
                }
            }
        }

        values = jobj.value("plugindata");
        if (!values.isUndefined() && values.isArray()) {
            auto array = values.toArray();
            for (auto &&item : array) {
                if (!item.isUndefined() && item.isObject()) {
                    auto sitem = item.toObject();
                    auto plgobj = sitem.value("key");
                    auto valueobj = sitem.value("value");
                    if (!plgobj.isUndefined() && plgobj.isString() &&
                        !valueobj.isUndefined() && valueobj.isString()) {
                        auto plg = plgobj.toString();
                        auto value = QByteArray::fromBase64(
                            valueobj.toString().toLatin1());
                        if (plg.isEmpty() || value.isEmpty()) {
                            continue;
                        }
                        infos.pluginData.insert(plg, value);
                    }
                }
            }
        }

        return true;
    }

    return false;
}

QJsonDocument WorkSpaceManager::loadWorkSpace(const QString &filename) {
    bool b = false;
    QFile f(filename);
    if (f.exists()) {
        QJsonParseError err;
        if (!f.open(QFile::ReadOnly))
            return {};
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
        f.close();
        if (err.error == QJsonParseError::NoError) {
            auto jobj = doc.object();
            auto t = jobj.value("type");
            if (!t.isUndefined() && t.isString()) {
                auto type = t.toString();
                if (!QString::compare(type, "workspace", Qt::CaseInsensitive)) {
                    return doc;
                }
            }
        }
    }
    return {};
}

QUrl WorkSpaceManager::loadWorkSpaceDocFile(const QString &filename,
                                            const QJsonDocument &doc) {
    auto jobj = doc.object();
    auto ff = jobj.value("file");
    QUrl file;
    if (!ff.isUndefined() && ff.isString()) {
        auto fi = ff.toString();
        auto curDir = QFileInfo(filename).absolutePath();
        auto url = QUrl::fromUserInput(fi, curDir, QUrl::AssumeLocalFile);
        if (url.isLocalFile()) {
            auto fi = url.toLocalFile();
            QFileInfo finfo(url.toLocalFile());
            auto path =
                finfo.isAbsolute() ? fi : curDir + QDir::separator() + fi;
            finfo.setFile(path);
            file = QUrl::fromLocalFile(path);
        } else {
            file = url;
        }
    }
    return file;
}

QString WorkSpaceManager::getColorString(const QColor &color) {
    if (color.isValid()) {
        return color.name();
    }
    return {};
}

bool WorkSpaceManager::saveWorkSpace(
    const QString &filename, const QUrl &file,
    const QMap<qsizetype, QString> &bookmarklist,
    const QVector<QHexMetadataItem> &metalist, const WorkSpaceInfo &infos) {
    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        QJsonObject jobj;
        jobj.insert("type", "workspace");

        QString ff;
        if (file.isLocalFile()) {
            ff = file.toLocalFile();
            QFileInfo fileInfo(ff);
            if (fileInfo.isAbsolute()) {
                QDir dir(QFileInfo(f).absoluteDir());
                QFileInfo fi(ff);
                ff = dir.relativeFilePath(fi.absoluteFilePath());
            }
        } else {
            ff = file.url();
        }

        jobj.insert("file", ff);
        jobj.insert("base", QString::number(infos.base));
        jobj.insert("checksum", QString::fromUtf8(infos.checkSum.toHex()));

        QJsonArray metas;
        for (auto &meta : metalist) {
            QJsonObject obj;
            obj.insert("begin", QString::number(meta.begin));
            obj.insert("end", QString::number(meta.end));
            obj.insert("comment", meta.comment);
            obj.insert("fgcolor", getColorString(meta.foreground));
            obj.insert("bgcolor", getColorString(meta.background));
            metas.append(obj);
        }
        jobj.insert("metas", metas);

        QJsonArray bookmarks;
        for (auto p = bookmarklist.cbegin(); p != bookmarklist.cend(); ++p) {
            QJsonObject i;
            i.insert("pos", QString::number(p.key()));
            i.insert("comment", p.value());
            bookmarks.append(i);
        }

        jobj.insert("bookmarks", bookmarks);

        // plugin data
        QJsonArray plugindata;
        for (auto p = infos.pluginData.begin(); p != infos.pluginData.end();
             p++) {
            QJsonObject i;
            i.insert("key", p.key());
            i.insert("value", QString::fromLatin1(p.value().toBase64()));
            plugindata.append(i);
        }

        jobj.insert("plugindata", plugindata);

        QJsonDocument jdoc(jobj);
        if (f.write(jdoc.toJson(QJsonDocument::JsonFormat::Indented)) >= 0) {
            f.close();
            return true;
        }
    }
    return false;
}
