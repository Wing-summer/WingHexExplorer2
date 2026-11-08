#include <cstdio>
#include <random>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "class/diff/binarydiff.h"
#ifdef Q_OS_UNIX
#include <sys/resource.h>
#endif
using namespace WingDiff;

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    if (argc < 4) {
        return 2;
    }
    const auto requestedSize =
        QString::fromLocal8Bit(argv[1]).toULongLong() * MiB;
    const auto pattern = QString::fromLocal8Bit(argv[2]);
    const auto size =
        requestedSize - (pattern == QStringLiteral("insert") ? 128 * 1024 : 0);
    QTemporaryDir directory(QString::fromLocal8Bit(argv[3]) +
                            QStringLiteral("/diff-bench-XXXXXX"));
    if (!directory.isValid()) {
        return 3;
    }
    std::mt19937 random(131);
    QByteArray block(int(MiB), Qt::Uninitialized);
    for (auto &c : block) {
        c = char(random());
    }
    const bool reuse = argc >= 5 && QString::fromLocal8Bit(argv[4]) ==
                                        QStringLiteral("--reuse");
    const auto inputDirectory =
        reuse ? QString::fromLocal8Bit(argv[3]) : directory.path();
    QFile left(inputDirectory + (reuse && pattern == QStringLiteral("delete")
                                     ? QStringLiteral("/right.bin")
                                     : QStringLiteral("/left.bin")));
    QFile right(inputDirectory + (reuse && pattern == QStringLiteral("delete")
                                      ? QStringLiteral("/left.bin")
                                      : QStringLiteral("/right.bin")));
    if (!reuse) {
        if (!left.open(QIODevice::WriteOnly) ||
            !right.open(QIODevice::WriteOnly)) {
            return 4;
        }
        const auto edited = (size / 2 / MiB) * MiB;
        for (quint64 offset = 0; offset < size; offset += MiB) {
            QByteArray target =
                block.first(qsizetype(std::min(MiB, size - offset)));
            if (left.write(target) != target.size()) {
                return 5;
            }
            if (pattern == QStringLiteral("unrelated")) {
                for (auto &c : target) {
                    c ^= char(0xAA);
                }
            } else if (pattern == QStringLiteral("dense")) {
                for (qsizetype i = 0; i < target.size(); i += 128) {
                    target[i] ^= 0x55;
                }
            } else if (pattern == QStringLiteral("sparse") &&
                       offset % (64 * MiB) == 0) {
                target[100] ^= 0x55;
            }
            if (offset == edited && pattern == QStringLiteral("insert")) {
                if (right.write(QByteArray(128 * 1024, 'I')) != 128 * 1024) {
                    return 6;
                }
            }
            if (offset == edited && pattern == QStringLiteral("delete")) {
                target.remove(0, 128 * 1024);
            }
            if (right.write(target) != target.size()) {
                return 7;
            }
        }
        left.close();
        right.close();
    }
    if (!left.open(QIODevice::ReadOnly) || !right.open(QIODevice::ReadOnly)) {
        return 8;
    }
    Options options;
    options.fileLimit = 65536 * MiB;
    ResultStore store(23 * MiB);
    QElapsedTimer timer;
    timer.start();
    const auto outcome = BinaryDiff().compare(&left, &right, options, store);
    QJsonObject report;
    report.insert(QStringLiteral("sizeMiB"), qint64(requestedSize / MiB));
    report.insert(QStringLiteral("pattern"), pattern);
    report.insert(QStringLiteral("leftBytes"), left.size());
    report.insert(QStringLiteral("rightBytes"), right.size());
    report.insert(QStringLiteral("elapsedMs"), timer.elapsed());
    report.insert(QStringLiteral("firstResultMs"), outcome.firstResultMs);
    report.insert(QStringLiteral("bytesRead"), qint64(outcome.bytesRead));
    report.insert(QStringLiteral("seeks"), qint64(outcome.seeks));
    report.insert(QStringLiteral("resultBytes"),
                  qint64(store.allocatedBytes()));
    report.insert(QStringLiteral("intervals"), qint64(store.differences()));
    report.insert(QStringLiteral("coarse"), qint64(store.coarseCount()));
    report.insert(QStringLiteral("status"), int(outcome.status));
    report.insert(QStringLiteral("error"), int(outcome.error));
#ifdef Q_OS_UNIX
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    report.insert(QStringLiteral("maxRssKiB"), qint64(usage.ru_maxrss));
#endif
    const auto json = QJsonDocument(report).toJson(QJsonDocument::Compact);
    std::puts(json.constData());
    return outcome.status == Status::Complete ? 0 : 1;
}
