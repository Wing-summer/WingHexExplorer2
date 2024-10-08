#include "workspacemanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>

WorkSpaceManager::WorkSpaceManager() {}

bool WorkSpaceManager::loadWorkSpace(QString filename, QString &file,
                                     QList<BookMarkStruct> &bookmarks,
                                     QList<QHexMetadataAbsoluteItem> &metas,
                                     WorkSpaceInfo &infos) {
    bool b = false;
    QFile f(filename);
    if (f.exists()) {
        QJsonParseError err;
        if (!f.open(QFile::ReadOnly))
            return false;
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
        f.close();
        if (err.error == QJsonParseError::NoError) {
            auto jobj = doc.object();
            auto t = jobj.value("type");
            if (!t.isUndefined() && t.isString()) {
                auto type = t.toString();
                if (!QString::compare(type, "workspace", Qt::CaseInsensitive)) {
                    auto ff = jobj.value("file");
                    if (!ff.isUndefined() && ff.isString()) {
                        auto fi = ff.toString();
                        QFileInfo finfo(fi);
                        file = finfo.isAbsolute()
                                   ? fi
                                   : QFileInfo(filename).absolutePath() +
                                         QDir::separator() + fi;
                        auto values = jobj.value("encoding");
                        if (!values.isUndefined() && values.isString()) {
                            infos.encoding = values.toString();
                        }
                        values = jobj.value("base");
                        if (!values.isUndefined() && values.isString()) {
                            auto ba = values.toString();
                            auto nbase = ba.toULongLong(&b);
                            if (b)
                                infos.base = nbase;
                        }

                        auto maxbytes =
                            QFileInfo(file).size(); // 简单排除非法标记

                        values = jobj.value("metas");
                        if (!values.isUndefined() && values.isArray()) {
                            auto metaitems = values.toArray();
                            for (auto item : metaitems) {
                                auto linem = item.toObject();
                                auto begin = linem.value("begin");
                                auto end = linem.value("end");
                                auto comment = linem.value("comment");
                                auto fgcolor = linem.value("fgcolor");
                                auto bgcolor = linem.value("bgcolor");
                                if (!begin.isUndefined() && begin.isString() &&
                                    !end.isUndefined() && end.isString() &&
                                    !comment.isUndefined() &&
                                    comment.isString() &&
                                    !fgcolor.isUndefined() &&
                                    fgcolor.isString() &&
                                    !bgcolor.isUndefined() &&
                                    bgcolor.isString()) {
                                    auto nbegin =
                                        begin.toString().toLongLong(&b);
                                    if (!b || nbegin >= maxbytes || nbegin < 0)
                                        continue;
                                    auto nend = end.toString().toLongLong(&b);
                                    if (!b || nend >= maxbytes || nend < 0)
                                        continue;
                                    auto nf = fgcolor.toString().toUInt(&b, 16);
                                    if (!b)
                                        continue;
                                    auto nb = bgcolor.toString().toUInt(&b, 16);
                                    if (!b)
                                        continue;
                                    auto fcolor = QColor::fromRgba(nf);
                                    auto bcolor = QColor::fromRgba(nb);

                                    QHexMetadataAbsoluteItem metaitem;
                                    metaitem.begin = nbegin;
                                    metaitem.end = nend;
                                    metaitem.comment = comment.toString();
                                    metaitem.foreground = fcolor.alpha() > 0
                                                              ? fcolor.toHsv()
                                                              : fcolor;
                                    metaitem.background = bcolor.alpha() > 0
                                                              ? fcolor.toHsv()
                                                              : bcolor;
                                    metas.append(metaitem);
                                }
                            }
                        }
                        values = jobj.value("bookmarks");
                        if (!values.isUndefined() && values.isArray()) {
                            for (auto item : values.toArray()) {
                                if (!item.isUndefined() && item.isObject()) {
                                    auto sitem = item.toObject();
                                    auto pos = sitem.value("pos");
                                    auto comment = sitem.value("comment");
                                    if (!pos.isUndefined() && pos.isString() &&
                                        !comment.isUndefined() &&
                                        comment.isString()) {
                                        auto b = false;
                                        auto ipos =
                                            pos.toString().toLongLong(&b);
                                        if (!b || ipos < 0 || ipos >= maxbytes)
                                            continue;
                                        BookMarkStruct book;
                                        book.pos = ipos;
                                        book.comment = comment.toString();
                                        bookmarks.append(book);
                                    }
                                }
                            }
                        }

                        values = jobj.value("plugindata");
                        if (!values.isUndefined() && values.isArray()) {
                            for (auto item : values.toArray()) {
                                if (!item.isUndefined() && item.isObject()) {
                                    auto sitem = item.toObject();
                                    auto plgobj = sitem.value("key");
                                    auto valueobj = sitem.value("value");
                                    if (!plgobj.isUndefined() &&
                                        plgobj.isString() &&
                                        !valueobj.isUndefined() &&
                                        valueobj.isString()) {
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
                }
            }
        }
    }
    return false;
}

bool WorkSpaceManager::saveWorkSpace(QString filename, QString file,
                                     QList<BookMarkStruct> bookmarklist,
                                     QList<QHexMetadataAbsoluteItem> metalist,
                                     WorkSpaceInfo infos) {
    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        QJsonObject jobj;
        jobj.insert("type", "workspace");

        QFileInfo fileInfo(file);
        if (fileInfo.isAbsolute()) {
            QDir dir(QFileInfo(f).absoluteDir());
            QFileInfo fi(file);
            file = dir.relativeFilePath(fi.absoluteFilePath());
        }

        jobj.insert("file", file);
        jobj.insert("encoding", infos.encoding);
        jobj.insert("base", QString::number(infos.base));

        QJsonArray metas;
        for (auto meta : metalist) {
            QJsonObject obj;
            obj.insert("begin", QString::number(meta.begin));
            obj.insert("end", QString::number(meta.end));
            obj.insert("comment", meta.comment);
            obj.insert("fgcolor", QString::number(meta.foreground.rgba(), 16));
            obj.insert("bgcolor", QString::number(meta.background.rgba(), 16));
            metas.append(obj);
        }
        jobj.insert("metas", metas);

        QJsonArray bookmarks;
        for (auto item : bookmarklist) {
            QJsonObject i;
            i.insert("pos", QString::number(item.pos));
            i.insert("comment", item.comment);
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
