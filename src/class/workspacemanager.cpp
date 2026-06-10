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

#include "logger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>

constexpr qsizetype MAX_BYTES = std::numeric_limits<qsizetype>::max();

bool WorkSpaceManager::loadWorkSpace(const QString &filename,
                                     WorkSpaceData &data, QJsonDocument doc) {
    if (doc.isEmpty()) {
        doc = loadWorkSpace(filename);
    }

    if (doc.isEmpty()) {
        return false;
    }

    auto file = loadWorkSpaceDocFile(filename, doc);
    if (file.isValid()) {
        data.file = file;
        auto jobj = doc.object();
        auto values = jobj.value(QLatin1String("base"));
        if (!values.isUndefined()) {
            auto base = readUInt64(values);
            if (base) {
                data.infos.base = base.value();
            } else {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    file, values, QStringLiteral("Invalid address base")));
            }
        }

        values = jobj.value(QLatin1String("checksum"));
        if (!values.isUndefined()) {
            if (values.isString()) {
                auto cs = values.toString();
                data.infos.checkSum = QByteArray::fromHex(cs.toUtf8());
            } else {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    file, values, QStringLiteral("Invalid checksum")));
            }
        }

        values = jobj.value(QLatin1String("metas"));
        readMetas(data, values);

        values = jobj.value(QLatin1String("bookmarks"));
        readBookmarks(data, values);

        values = jobj.value(QLatin1String("plugindata"));
        loadPluginData(data, values);

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
            auto t = jobj.value(QLatin1String("type"));
            if (t.isString()) {
                auto type = t.toString();
                if (!QString::compare(type, QLatin1String("workspace"),
                                      Qt::CaseInsensitive)) {
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
    auto ff = jobj.value(QLatin1String("file"));
    QUrl file;
    if (ff.isString()) {
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

std::optional<quint64> WorkSpaceManager::readUInt64(const QJsonValue &value) {
    if (value.isString()) {
        auto v = value.toString();
        bool b;
        auto rv = v.toULongLong(&b, 0);
        if (rv < 0) {
            return -1;
        }
        return rv;
    }

    return std::nullopt;
}

std::optional<QColor> WorkSpaceManager::readColor(const QJsonValue &value) {
    if (value.isUndefined()) {
        return QColor{};
    }

    if (value.isString()) {
        auto v = value.toString();
        if (v.isEmpty()) {
            return QColor{};
        }
        QColor c(v);
        if (c.isValid()) {
            return c;
        }
    }
    return std::nullopt;
}

void WorkSpaceManager::readMetas(WorkSpaceData &data,
                                 const QJsonValue &values) {
    if (values.isUndefined()) {
        return;
    }

    if (values.isArray()) {
        const auto metaitems = values.toArray();
        qsizetype counter = 0;
        for (auto p = metaitems.begin(); p != metaitems.end(); ++p, ++counter) {
            auto item = *p;

            if (counter > QHEXVIEW_METADATA_LIMIT) {
                data.corrupted = true;
                Logger::warning(
                    packupMessage(data.file, item,
                                  QStringLiteral("Too many metadata to load")));
                break;
            }

            if (!item.isObject()) {
                data.corrupted = true;
                Logger::warning(
                    packupMessage(data.file, item,
                                  QStringLiteral("Invalid metadata object")));
                continue;
            }

            auto linem = item.toObject();
            QHexMetadataItem metaitem;
            auto begin = readUInt64(linem.value(QLatin1String("begin")));
            if (begin) {
                auto v = begin.value();
                if (v >= MAX_BYTES) {
                    data.corrupted = true;
                    Logger::warning(packupMessage(
                        data.file, item,
                        QStringLiteral("Invalid metadata 'begin' tag")));
                    continue;
                }
                metaitem.begin = v;
            } else {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Invalid metadata 'begin' tag")));
                continue;
            }

            auto end = readUInt64(linem.value(QLatin1String("end")));
            if (end) {
                auto v = end.value();
                if (v >= MAX_BYTES) {
                    data.corrupted = true;
                    Logger::warning(packupMessage(
                        data.file, item,
                        QStringLiteral("Invalid metadata 'end' tag")));
                    continue;
                }
                if (begin > end) {
                    data.corrupted = true;
                    Logger::warning(packupMessage(
                        data.file, item,
                        QStringLiteral(
                            "Invalid metadata 'end' tag for 'begin > end'")));
                    continue;
                }
                metaitem.end = v;
            } else {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Invalid metadata 'end' tag")));
                continue;
            }

            auto fg = readColor(linem.value(QLatin1String("fgcolor")));
            if (fg) {
                metaitem.foreground = fg.value();
            } else {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Invalid metadata 'fgcolor' tag")));
                continue;
            }

            auto bg = readColor(linem.value(QLatin1String("bgcolor")));
            if (bg) {
                metaitem.background = bg.value();
            } else {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Invalid metadata 'bgcolor' tag")));
                continue;
            }

            auto comment = linem.value(QLatin1String("comment"));
            if (!comment.isUndefined()) {
                if (comment.isString()) {
                    auto cmt = comment.toString();
                    if (cmt.length() > QHEXVIEW_COMMENT_LIMIT) {
                        cmt.slice(QHEXVIEW_COMMENT_LIMIT);
                        data.corrupted = true;
                        Logger::warning(packupMessage(
                            data.file, item,
                            QStringLiteral("Length is too long for metadata "
                                           "'comment' tag")));
                    }
                    metaitem.comment = cmt;
                } else {
                    data.corrupted = true;
                    Logger::warning(packupMessage(
                        data.file, item,
                        QStringLiteral("Invalid metadata 'comment' tag")));
                    continue;
                }
            }
            data.metas.append(metaitem);
        }
    } else {
        data.corrupted = true;
        Logger::warning(packupMessage(data.file, values,
                                      QStringLiteral("Invalid metadata")));
    }
}

void WorkSpaceManager::readBookmarks(WorkSpaceData &data,
                                     const QJsonValue &values) {
    if (values.isUndefined()) {
        return;
    }

    if (values.isArray()) {
        const auto array = values.toArray();
        qsizetype counter = 0;
        for (auto p = array.begin(); p != array.end(); ++p, counter++) {
            auto item = *p;
            if (counter > QHEXVIEW_BOOKMARK_LIMIT) {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Too many bookmarks to load")));
                break;
            }

            if (!p->isObject()) {
                data.corrupted = true;
                Logger::warning(
                    packupMessage(data.file, item,
                                  QStringLiteral("Invalid bookmark object")));
                continue;
            }

            auto sitem = p->toObject();
            auto pos = readUInt64(sitem.value(QLatin1String("pos")));
            if (!pos) {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Invalid bookmark 'pos' tag")));
                continue;
            }

            auto ipos = pos.value();
            if (ipos >= MAX_BYTES) {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Invalid bookmark 'pos' tag")));
                continue;
            }

            auto comment = sitem.value(QLatin1String("comment"));
            if (!comment.isString()) {
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Invalid bookmark 'comment' tag")));
                continue;
            }

            auto cmt = comment.toString();
            if (cmt.length() > QHEXVIEW_COMMENT_LIMIT) {
                cmt.slice(QHEXVIEW_COMMENT_LIMIT);
                data.corrupted = true;
                Logger::warning(packupMessage(
                    data.file, item,
                    QStringLiteral("Length is too long for bookmark "
                                   "'comment' tag")));
            }

            data.bookmarks.insert(ipos, cmt);
        }
    } else {
        data.corrupted = true;
        Logger::warning(packupMessage(data.file, values,
                                      QStringLiteral("Invalid bookmarks")));
    }
}

void WorkSpaceManager::loadPluginData(WorkSpaceData &data,
                                      const QJsonValue &values) {
    if (values.isUndefined()) {
        return;
    }
    if (values.isArray()) {
        const auto array = values.toArray();
        for (const auto &&item : array) {
            if (!item.isObject()) {
                data.corrupted = true;
                Logger::warning(
                    packupMessage(data.file, item,
                                  QStringLiteral("Invalid plugindata object")));
                continue;
            }
            auto sitem = item.toObject();
            auto plgobj = sitem.value(QLatin1String("key"));
            auto valueobj = sitem.value(QLatin1String("value"));
            if (plgobj.isString() && valueobj.isString()) {
                auto plg = plgobj.toString();
                auto value =
                    QByteArray::fromBase64(valueobj.toString().toLatin1());
                if (plg.isEmpty()) {
                    data.corrupted = true;
                    Logger::warning(packupMessage(
                        data.file, item,
                        QStringLiteral("Invalid plugindata 'key' tag")));
                    continue;
                }
                data.infos.pluginData.insert(plg, value);
            } else {
                data.corrupted = true;
                Logger::warning(
                    packupMessage(data.file, item,
                                  QStringLiteral("Invalid plugindata object")));
                continue;
            }
        }
    } else {
        data.corrupted = true;
        Logger::warning(packupMessage(data.file, values,
                                      QStringLiteral("Invalid plugindata")));
    }
}

QString WorkSpaceManager::packupMessage(const QUrl &file,
                                        const QJsonValue &values,
                                        const QString &reason) {
    auto json = QString::fromUtf8(values.toJson(QJsonDocument::Compact));
    return QStringLiteral("[%1] %2 -> %3")
        .arg(file.fileName(), reason, json.left(80));
}

bool WorkSpaceManager::saveWorkSpace(const QString &filename,
                                     const WorkSpaceData &data) {
    QFile f(filename);
    if (f.open(QFile::WriteOnly)) {
        QJsonObject jobj;
        jobj.insert(QLatin1String("type"), QLatin1String("workspace"));

        QString ff;
        auto file = data.file;
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

        jobj.insert(QLatin1String("file"), ff);
        jobj.insert(QLatin1String("base"),
                    QString(QStringLiteral("0x") +
                            QString::number(data.infos.base, 16)));
        jobj.insert(QLatin1String("checksum"),
                    QString::fromUtf8(data.infos.checkSum.toHex()));

        QJsonArray metas;
        for (const auto &meta : data.metas) {
            QJsonObject obj;
            obj.insert(QLatin1String("begin"), QString::number(meta.begin));
            obj.insert(QLatin1String("end"), QString::number(meta.end));
            obj.insert(QLatin1String("comment"), meta.comment);
            obj.insert(QLatin1String("fgcolor"),
                       getColorString(meta.foreground));
            obj.insert(QLatin1String("bgcolor"),
                       getColorString(meta.background));
            metas.append(obj);
        }
        jobj.insert("metas", metas);

        QJsonArray bookmarks;
        const auto &bookmarklist = data.bookmarks;
        for (auto p = bookmarklist.cbegin(); p != bookmarklist.cend(); ++p) {
            QJsonObject i;
            i.insert(QLatin1String("pos"), QString::number(p.key()));
            i.insert(QLatin1String("comment"), p.value());
            bookmarks.append(i);
        }

        jobj.insert(QLatin1String("bookmarks"), bookmarks);

        // plugin data
        QJsonArray plugindata;
        const auto &plgdata = data.infos.pluginData;
        for (auto p = plgdata.begin(); p != plgdata.end(); p++) {
            QJsonObject i;
            i.insert(QLatin1String("key"), p.key());
            i.insert(QLatin1String("value"),
                     QString::fromLatin1(p.value().toBase64()));
            plugindata.append(i);
        }

        jobj.insert(QLatin1String("plugindata"), plugindata);

        QJsonDocument jdoc(jobj);
        if (f.write(jdoc.toJson(QJsonDocument::JsonFormat::Indented)) >= 0) {
            f.close();
            return true;
        }
    }
    return false;
}
