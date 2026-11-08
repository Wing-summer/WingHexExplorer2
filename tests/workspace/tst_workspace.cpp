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

#include <limits>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>

#include "class/file/workspaceformat.h"
#include "legacyworkspace.h"

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

class WorkspaceTests : public QObject {
    Q_OBJECT

    static QByteArray read(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return {};
        }
        return file.readAll();
    }

    static bool write(const QString &path, const QByteArray &bytes) {
        QFile file(path);
        return file.open(QIODevice::WriteOnly) &&
               file.write(bytes) == bytes.size();
    }

    static WorkSpaceData sample(const QString &source) {
        WorkSpaceData data;
        data.file = QUrl::fromLocalFile(source);
        data.infos.base = std::numeric_limits<quint64>::max();
        data.infos.checkSum = QByteArray::fromHex("0001ff");
        data.bookmarks.insert(0, QStringLiteral("开始"));
        data.bookmarks.insert(42, QString());
        data.metas.append(QHexMetadataItem(0, 42, QColor(1, 2, 3, 40), QColor(),
                                           QStringLiteral("元数据")));
        WorkSpaceManager::setPluginData(
            data, {{QStringLiteral("plugin.a"), QByteArray("\0\xff", 2)}});
        return data;
    }

    static QJsonObject legacy() {
        return {{QStringLiteral("type"), QStringLiteral("workspace")},
                {QStringLiteral("file"), QStringLiteral("../源文件.bin")},
                {QStringLiteral("base"), QStringLiteral("0xffffffffffffffff")},
                {QStringLiteral("checksum"), QStringLiteral("0001ff")},
                {QStringLiteral("metas"),
                 QJsonArray{QJsonObject{
                     {QStringLiteral("begin"), QStringLiteral("0")},
                     {QStringLiteral("end"), QStringLiteral("42")},
                     {QStringLiteral("comment"), QStringLiteral("元数据")},
                     {QStringLiteral("fgcolor"), QStringLiteral("#28010203")},
                     {QStringLiteral("bgcolor"), QString()}}}},
                {QStringLiteral("bookmarks"),
                 QJsonArray{QJsonObject{
                     {QStringLiteral("pos"), QStringLiteral("0")},
                     {QStringLiteral("comment"), QStringLiteral("开始")}}}},
                {QStringLiteral("plugindata"),
                 QJsonArray{QJsonObject{
                     {QStringLiteral("key"), QStringLiteral("plugin.a")},
                     {QStringLiteral("value"), QStringLiteral("AP8=")}}}},
                {QStringLiteral("future"),
                 QJsonObject{{QStringLiteral("enabled"), true}}}};
    }

    static QByteArray
    binary(const QList<WorkspaceFormat::BookMark> &bookmarks,
           const QList<WorkspaceFormat::MetaData> &metas = {},
           const QVariantMap &plugins = {},
           const QString &source = QStringLiteral("source.bin")) {
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_6_0);
        stream.setByteOrder(QDataStream::BigEndian);
        stream.writeRawData(WorkspaceFormat::Magic, 7);
        stream << WorkspaceFormat::Version << source << WorkSpaceInfo{}
               << bookmarks << metas << plugins;
        return bytes;
    }

private slots:

    void nativeLists() {
        const QList<WorkspaceFormat::BookMark> bookmarks{
            {quint64(1) << 48, QStringLiteral("bookmark")}, {0, QString()}};
        const QList<WorkspaceFormat::MetaData> metas{
            {0, quint64(1) << 48, QColor(1, 2, 3, 40), QColor(),
             QStringLiteral("meta")}};
        QByteArray bytes;
        QDataStream out(&bytes, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out << bookmarks << metas;
        QCOMPARE(out.status(), QDataStream::Ok);
        QList<WorkspaceFormat::BookMark> restoredBookmarks;
        QList<WorkspaceFormat::MetaData> restoredMetas;
        QDataStream in(bytes);
        in.setVersion(QDataStream::Qt_6_0);
        in >> restoredBookmarks >> restoredMetas;
        QCOMPARE(in.status(), QDataStream::Ok);
        QCOMPARE(restoredBookmarks.size(), bookmarks.size());
        QCOMPARE(restoredBookmarks[0].pos, bookmarks[0].pos);
        QCOMPARE(restoredBookmarks[0].comment, bookmarks[0].comment);
        QCOMPARE(restoredMetas[0].end, metas[0].end);
        QCOMPARE(restoredMetas[0].foreground, metas[0].foreground);
        QVERIFY(!restoredMetas[0].background.isValid());
    }

    void roundTrip() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const auto path = dir.filePath(QStringLiteral("project.wingpro"));
        auto data = sample(dir.filePath(QStringLiteral("不存在.bin")));
        if (sizeof(qsizetype) >= 8) {
            data.bookmarks.insert(qsizetype(quint64(1) << 48),
                                  QStringLiteral("wide"));
        }
        QString error;
        QVERIFY2(WorkSpaceManager::saveWorkSpace(path, data, &error),
                 qPrintable(error));
        QCOMPARE(read(path).left(9), QByteArray("WINGPRO\0\1", 9));
        WorkSpaceData restored;
        QVERIFY2(WorkSpaceManager::loadWorkSpace(path, restored, &error),
                 qPrintable(error));
        QCOMPARE(restored.file, data.file);
        QCOMPARE(restored.infos.base, data.infos.base);
        QCOMPARE(restored.infos.checkSum, data.infos.checkSum);
        QCOMPARE(restored.bookmarks, data.bookmarks);
        QCOMPARE(restored.metas, data.metas);
        QCOMPARE(restored.plugindata, data.plugindata);
        WorkSpaceManager::setPluginData(restored, QStringLiteral("plugin.b"),
                                        QByteArray("new"));
        QCOMPARE(WorkSpaceManager::pluginData(restored).size(), 2);
        QCOMPARE(WorkSpaceManager::pluginData(restored).value(
                     QStringLiteral("plugin.a")),
                 QByteArray("\0\xff", 2));
        restored.infos.checkSum = QByteArray("updated");
        QVERIFY(WorkSpaceManager::saveWorkSpace(path, restored));
        WorkSpaceData again;
        QVERIFY(WorkSpaceManager::loadWorkSpace(path, again));
        QCOMPARE(again.plugindata, restored.plugindata);
        // Full replacement must persist explicit plugin-data cleanup.
        WorkSpaceManager::setPluginData(again, {});
        QVERIFY(WorkSpaceManager::pluginData(again).isEmpty());
    }

    void pathsAndDevices() {
        QTemporaryDir dir;
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("output")));
        const auto path = dir.filePath(QStringLiteral("output/p.wingpro"));
        auto data = sample(dir.filePath(QStringLiteral("source.bin")));
        QVERIFY(WorkSpaceManager::saveWorkSpace(path, data));
        QDataStream stream(read(path).mid(9));
        stream.setVersion(QDataStream::Qt_6_0);
        QString source;
        stream >> source;
        QCOMPARE(source, QStringLiteral("../source.bin"));
        WorkSpaceData restored;
        QVERIFY(WorkSpaceManager::loadWorkSpace(path, restored));
        QCOMPARE(restored.file, data.file);
        data.file = QUrl(QStringLiteral("wdrv://device/path%20with%20spaces"));
        QVERIFY(WorkSpaceManager::saveWorkSpace(path, data));
        QVERIFY(WorkSpaceManager::loadWorkSpace(path, restored));
        QCOMPARE(restored.file, data.file);
        data.file.setScheme(QStringLiteral("wingpro"));
        QVERIFY(!WorkSpaceManager::saveWorkSpace(path, data));
        data.file = QUrl(QStringLiteral("wdrv://device"));
        QVERIFY(!WorkSpaceManager::saveWorkSpace(path, data));
    }

    void rejectsMalformed() {
        QTemporaryDir dir;
        const auto path = dir.filePath(QStringLiteral("p.wingpro"));
        auto data = sample(dir.filePath(QStringLiteral("source.bin")));
        const auto good = binary({{0, QString()}});
        auto badVersion = good;
        badVersion[8] = 2;
        auto badMagic = good;
        badMagic[0] = 'X';
        const QList<QByteArray> invalid{
            QByteArray("{}"),
            badVersion,
            badMagic,
            good + 'x',
            binary({{0, QString()}, {0, QStringLiteral("duplicate")}}),
            binary({{std::numeric_limits<quint64>::max(), QString()}}),
            binary({}, {{5, 4, QColor(), QColor(), QString()}}),
            binary({}, {}, {{QString(), QByteArray()}}),
            binary({}, {}, {{QStringLiteral("p"), QStringLiteral("bad")}}),
            binary({}, {}, {}, QString()),
            binary({}, {}, {}, QStringLiteral("wingpro:///nested.wingpro")),
            binary(
                {{0, QString(QHEXVIEW_COMMENT_LIMIT + 1, QLatin1Char('x'))}})};
        QString error;
        for (const auto &bytes : invalid) {
            QVERIFY(write(path, bytes));
            QVERIFY(!WorkSpaceManager::loadWorkSpace(path, data, &error));
            QVERIFY(!error.isEmpty());
            QCOMPARE(data.infos.base, std::numeric_limits<quint64>::max());
            QCOMPARE(data.bookmarks.size(), 2);
        }
        for (qsizetype i = 0; i < good.size(); ++i) {
            QVERIFY(write(path, good.left(i)));
            QVERIFY(!WorkSpaceManager::loadWorkSpace(path, data));
        }
        // A malicious count is rejected before QList reserves storage.
        QByteArray oversized;
        QDataStream out(&oversized, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);
        out.writeRawData(WorkspaceFormat::Magic, 7);
        out << WorkspaceFormat::Version << QStringLiteral("source.bin")
            << WorkSpaceInfo{} << quint32(0xfffffffe);
        QVERIFY(write(path, oversized));
        QVERIFY(!WorkSpaceManager::loadWorkSpace(path, data));
    }

    void limitsAndAtomicSave() {
        QTemporaryDir dir;
        const auto path = dir.filePath(QStringLiteral("p.wingpro"));
        WorkSpaceData data;
        data.file =
            QUrl::fromLocalFile(dir.filePath(QStringLiteral("source.bin")));
        for (qsizetype i = 0; i < QHEXVIEW_BOOKMARK_LIMIT; ++i) {
            data.bookmarks.insert(
                i, QString(QHEXVIEW_COMMENT_LIMIT, QLatin1Char('a')));
        }
        for (qsizetype i = 0; i < QHEXVIEW_METADATA_LIMIT; ++i) {
            data.metas.append(
                QHexMetadataItem(i, i, QColor(), QColor(), QString()));
        }
        QVERIFY(WorkSpaceManager::saveWorkSpace(path, data));
        const auto original = read(path);
        data.bookmarks.insert(QHEXVIEW_BOOKMARK_LIMIT, QString());
        QVERIFY(!WorkSpaceManager::saveWorkSpace(path, data));
        QCOMPARE(read(path), original);
        data.bookmarks.remove(QHEXVIEW_BOOKMARK_LIMIT);
        data.metas.append(
            QHexMetadataItem(0, 1, QColor(), QColor(), QString()));
        QVERIFY(!WorkSpaceManager::saveWorkSpace(path, data));
        QCOMPARE(read(path), original);
        data.metas.clear();
        QVERIFY(!WorkSpaceManager::saveWorkSpace(
            dir.filePath(QStringLiteral("missing/p.wingpro")), data));
        QCOMPARE(read(path), original);
    }

    void legacyConversion() {
        QTemporaryDir dir;
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("old")));
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("new")));
        const auto input = dir.filePath(QStringLiteral("old/p.wingpro"));
        const auto output = dir.filePath(QStringLiteral("new/p.wingpro"));
        const auto bytes = QJsonDocument(legacy()).toJson();
        QVERIFY(write(input, bytes));
        WorkSpaceData data;
        QString error;
        QVERIFY2(LegacyWorkspace::load(input, data, &error), qPrintable(error));
        QCOMPARE(data.file, QUrl::fromLocalFile(
                                dir.filePath(QStringLiteral("源文件.bin"))));
        QCOMPARE(data.infos.base, std::numeric_limits<quint64>::max());
        QCOMPARE(data.plugindata.size(), 1);
        QCOMPARE(data.metas.first().foreground.alpha(), 40);
        QCOMPARE(WorkSpaceManager::pluginData(data).value(
                     QStringLiteral("plugin.a")),
                 QByteArray("\0\xff", 2));
        QVERIFY(WorkSpaceManager::saveWorkSpace(output, data));
        WorkSpaceData restored;
        QVERIFY(WorkSpaceManager::loadWorkSpace(output, restored));
        QCOMPARE(restored.file, data.file);
        QCOMPARE(restored.plugindata, data.plugindata);
        QCOMPARE(read(input), bytes);
        QVERIFY(!WorkSpaceManager::loadWorkSpace(input, restored));
        const QJsonObject minimal{
            {QStringLiteral("type"), QStringLiteral("workspace")},
            {QStringLiteral("file"), QStringLiteral("wdrv://plugin/device")}};
        QVERIFY(write(input, QJsonDocument(minimal).toJson()));
        QVERIFY(LegacyWorkspace::load(input, data, &error));
        QCOMPARE(data.file, QUrl(QStringLiteral("wdrv://plugin/device")));
        QVERIFY(data.bookmarks.isEmpty());
        QVERIFY(data.metas.isEmpty());
    }

    void legacyInvalid() {
        QTemporaryDir dir;
        const auto input = dir.filePath(QStringLiteral("old.wingpro"));
        QList<QJsonObject> invalid;
        for (const auto &text :
             {QStringLiteral("garbage"), QStringLiteral("-1"),
              QStringLiteral("18446744073709551616")}) {
            auto object = legacy();
            object.insert(QStringLiteral("base"), text);
            invalid.append(object);
        }
        for (const auto &text : {QStringLiteral("a"), QStringLiteral("gg")}) {
            auto object = legacy();
            object.insert(QStringLiteral("checksum"), text);
            invalid.append(object);
        }
        auto object = legacy();
        object.insert(
            QStringLiteral("plugindata"),
            QJsonArray{QJsonObject{
                {QStringLiteral("key"), QStringLiteral("p")},
                {QStringLiteral("value"), QStringLiteral("!invalid!")}}});
        invalid.append(object);
        object = legacy();
        object.insert(QStringLiteral("metas"),
                      QJsonArray{QJsonObject{
                          {QStringLiteral("begin"), QStringLiteral("9")},
                          {QStringLiteral("end"), QStringLiteral("1")}}});
        invalid.append(object);
        object = legacy();
        object.insert(
            QStringLiteral("bookmarks"),
            QJsonArray{QJsonObject{
                {QStringLiteral("pos"), QStringLiteral("0")},
                {QStringLiteral("comment"),
                 QString(QHEXVIEW_COMMENT_LIMIT + 1, QLatin1Char('x'))}}});
        invalid.append(object);
        object = legacy();
        object.insert(QStringLiteral("file"),
                      QStringLiteral("wingpro:///nested.wingpro"));
        invalid.append(object);
        WorkSpaceData data =
            sample(dir.filePath(QStringLiteral("original.bin")));
        for (const auto &value : invalid) {
            QVERIFY(write(input, QJsonDocument(value).toJson()));
            QString error;
            QVERIFY(!LegacyWorkspace::load(input, data, &error));
            QVERIFY(!error.isEmpty());
            QCOMPARE(data.infos.base, std::numeric_limits<quint64>::max());
            QCOMPARE(data.bookmarks.size(), 2);
        }
    }

#ifdef WORKSPACE_MIGRATOR_PATH
    void commandLine() {
        QTemporaryDir dir;
        const auto input = dir.filePath(QStringLiteral("old.wingpro"));
        const auto output = dir.filePath(QStringLiteral("new.wingpro"));
        const auto original = QJsonDocument(legacy()).toJson();
        QVERIFY(write(input, original));
        auto run = [](const QStringList &args) {
            QProcess process;
            process.start(QString::fromLocal8Bit(WORKSPACE_MIGRATOR_PATH),
                          args);
            if (!process.waitForFinished(10000) ||
                process.exitStatus() != QProcess::NormalExit) {
                return -1;
            }
            return process.exitCode();
        };
        QCOMPARE(run({input, output}), 0);
        const auto converted = read(output);
        QCOMPARE(converted.left(7), QByteArray("WINGPRO"));
        QCOMPARE(run({input, output}), 2);
        QCOMPARE(run({input, input, QStringLiteral("--force")}), 2);
#ifdef Q_OS_UNIX
        const auto alias = dir.filePath(QStringLiteral("alias.wingpro"));
        QVERIFY(QFile::link(input, alias));
        QCOMPARE(run({input, alias, QStringLiteral("--force")}), 2);
        const auto hardLink = dir.filePath(QStringLiteral("hardlink.wingpro"));
        QCOMPARE(::link(QFile::encodeName(input).constData(),
                        QFile::encodeName(hardLink).constData()),
                 0);
        QCOMPARE(run({input, hardLink, QStringLiteral("--force")}), 2);
#endif
        QCOMPARE(run({input, output, QStringLiteral("--force")}), 0);
        QVERIFY(write(input, QByteArray("invalid JSON")));
        QCOMPARE(run({input, output, QStringLiteral("--force")}), 1);
        QCOMPARE(read(output), converted);
        QVERIFY(write(input, original));
        QCOMPARE(read(input), original);
    }
#endif
};

QTEST_GUILESS_MAIN(WorkspaceTests)
#include "tst_workspace.moc"
