#include "QHexView/qhexview.h"
#include "class/diff/binarydiff.h"
#include "class/diff/diffsource.h"
#include "control/diffrenderer.h"
#include "model/diffresultmodel.h"
#include <QBuffer>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QtConcurrent>
#include <cstring>
#include <random>

using namespace WingDiff;
class DiffTests : public QObject {
    Q_OBJECT
private:
    void compare(const QByteArray &a, const QByteArray &b) {
        QBuffer left, right;
        left.setData(a);
        right.setData(b);
        QVERIFY(left.open(QIODevice::ReadOnly));
        QVERIFY(right.open(QIODevice::ReadOnly));
        ResultStore results(24 * MiB);
        auto outcome = BinaryDiff().compare(&left, &right, Options(), results);
        QCOMPARE(outcome.status, Status::Complete);
        quint64 x = 0, y = 0;
        const auto differences = results.page(0, int(results.differences()));
        for (const auto &r : differences) {
            QVERIFY(r.leftOffset >= x);
            QVERIFY(r.rightOffset >= y);
            QCOMPARE(a.mid(qsizetype(x), qsizetype(r.leftOffset - x)),
                     b.mid(qsizetype(y), qsizetype(r.rightOffset - y)));
            QVERIFY(r.leftLength || r.rightLength);
            if (r.type == Type::Insert) {
                QCOMPARE(r.leftLength, 0);
            }
            if (r.type == Type::Delete) {
                QCOMPARE(r.rightLength, 0);
            }
            x = r.leftOffset + r.leftLength;
            y = r.rightOffset + r.rightLength;
        }
        QCOMPARE(a.mid(qsizetype(x)), b.mid(qsizetype(y)));
        QVERIFY(results.allocatedBytes() <= 24 * MiB);
    }
private slots:
    void edits() {
        compare({}, {});
        compare({}, "a");
        compare("a", {});
        compare("abc", "abc");
        compare("abcdef", "abXYcdef");
        compare("abcdef", "abef");
        compare("abcdef", "abXYef");
        compare("abcdef", "XYcdefZZ");
    }
    void randomEdits() {
        std::mt19937 generator(17);
        for (int run = 0; run < 100; ++run) {
            QByteArray left(300, Qt::Uninitialized);
            for (auto &c : left) {
                c = char(generator() % 13);
            }
            auto right = left;
            for (int edit = 0; edit < 8; ++edit) {
                const auto pos = int(generator() % unsigned(right.size() + 1));
                right.insert(pos, QByteArray(1, char(generator() % 13)));
                if (pos < right.size() && generator() % 2) {
                    right.remove(pos, 2);
                }
            }
            compare(left, right);
        }
    }
    void resynchronize() {
        std::mt19937 generator(11);
        QByteArray left(5 * int(MiB), Qt::Uninitialized);
        for (auto &c : left) {
            c = char(generator());
        }
        auto right = left;
        right.insert(17, "inserted");
        compare(left, right);
        right = left;
        right.remove(1000, 128 * 1024);
        compare(left, right);
    }

    void localMinimality() {
        std::mt19937 generator(77);
        for (int attempt = 0; attempt < 60; ++attempt) {
            QByteArray a(40, Qt::Uninitialized), b(40, Qt::Uninitialized);
            for (auto &c : a) {
                c = char(generator() % 5);
            }
            for (auto &c : b) {
                c = char(generator() % 5);
            }
            std::array<std::array<int, 41>, 41> lcs{};
            for (int i = 1; i <= 40; ++i) {
                for (int j = 1; j <= 40; ++j) {
                    lcs[i][j] = a[i - 1] == b[j - 1]
                                    ? lcs[i - 1][j - 1] + 1
                                    : std::max(lcs[i - 1][j], lcs[i][j - 1]);
                }
            }
            QBuffer left(&a), right(&b);
            QVERIFY(left.open(QIODevice::ReadOnly));
            QVERIFY(right.open(QIODevice::ReadOnly));
            ResultStore store(24 * MiB);
            QCOMPARE(
                BinaryDiff().compare(&left, &right, Options(), store).status,
                Status::Complete);
            quint64 cost = 0;
            for (const auto &range : store.page(0)) {
                QVERIFY(!range.coarse);
                cost += range.leftLength + range.rightLength;
            }
            QCOMPARE(cost, quint64(80 - 2 * lcs[40][40]));
        }
    }

    void globalSearchBudgetFallsBackWithoutUnboundedResults() {
        class GeneratedDevice final : public QIODevice {
        public:
            explicit GeneratedDevice(char value) : m_value(value) {
                open(QIODevice::ReadOnly | QIODevice::Unbuffered);
            }
            qint64 size() const override { return 80 * qint64(MiB); }
            bool isSequential() const override { return false; }
            qint64 readData(char *data, qint64 count) override {
                const auto length = std::min(count, size() - pos());
                std::memset(data, m_value, size_t(length));
                return length;
            }
            qint64 writeData(const char *, qint64) override { return -1; }

        private:
            char m_value;
        };
        GeneratedDevice a('A'), b('B');
        ResultStore store(24 * MiB);
        const auto outcome = BinaryDiff().compare(&a, &b, Options(), store);
        QCOMPARE(outcome.status, Status::Complete);
        QVERIFY(store.searchLimited.load());
        QVERIFY(store.summarizing.load());
        QVERIFY(store.differences() < 100);
        QCOMPARE(store.processed.load(), 160 * MiB);
        QVERIFY(store.coarseCount() > 0);
        QVERIFY(outcome.firstResultMs >= 0);
    }
    void cancelled() {
        QBuffer a, b;
        a.setData("abc");
        b.setData("def");
        a.open(QIODevice::ReadOnly);
        b.open(QIODevice::ReadOnly);
        ResultStore store(24 * MiB);
        QCOMPARE(BinaryDiff()
                     .compare(&a, &b, Options(), store, [] { return true; })
                     .status,
                 Status::Cancelled);
        class LargeDevice final : public QIODevice {
        public:
            LargeDevice() { open(QIODevice::ReadOnly); }
            qint64 size() const override { return 64 * 1024 * qint64(MiB); }
            bool isSequential() const override { return false; }
            qint64 readData(char *, qint64) override { return -1; }
            qint64 writeData(const char *, qint64) override { return -1; }
        } largeA, largeB;
        ResultStore largeStore(MiB);
        QTest::ignoreMessage(
            QtWarningMsg, "An input exceeds the configured file size limit.");
        QCOMPARE(
            BinaryDiff().compare(&largeA, &largeB, Options(), largeStore).error,
            Error::FileLimitExceeded);
        Options largeOptions;
        largeOptions.fileLimit = 64 * 1024 * MiB;
        largeOptions.normalize();
        QCOMPARE(BinaryDiff()
                     .compare(&largeA, &largeB, largeOptions, largeStore,
                              [] { return true; })
                     .status,
                 Status::Cancelled);
    }
    void contentNotificationsExcludePresentationChanges() {
        QHexView view;
        auto doc = view.document();
        QSignalSpy contents(doc.data(), &QHexDocument::contentsChanged);
        QVERIFY(doc->insert(0, QByteArray("abc")));
        QCOMPARE(contents.count(), 1);
        doc->setBaseAddress(0x1000);
        doc->setMetabgVisible(!doc->metabgVisible());
        QVERIFY(doc->addBookMark(1, QStringLiteral("Bookmark")));
        QCOMPARE(contents.count(), 1);
        QVERIFY(doc->replace(0, QByteArray("Q")));
        QVERIFY(doc->remove(2, 1));
        QCOMPARE(contents.count(), 3);
        QVERIFY(doc->Replace(view.cursor(), 0, QByteArray("Z")));
        QCOMPARE(contents.count(), 4);
        doc->undo();
        QCOMPARE(contents.count(), 5);
        doc->redo();
        QCOMPARE(contents.count(), 6);
        QVERIFY(doc->AddBookMark(0, QStringLiteral("Undoable bookmark")));
        doc->undo();
        doc->redo();
        QCOMPARE(contents.count(), 6);
    }
    void sharedReadOnlyView() {
        QSharedPointer<QHexDocument> doc(
            QHexDocument::fromInternalBuffer(false));
        QVERIFY(doc->insert(0, "abcdef"));
        QHexView first, second;
        first.setDocument(doc);
        second.setDocument(doc);
        second.setReadOnly(true);
        second.Replace(0, QByteArray("XX"));
        QCOMPARE(doc->read(0, 6), QByteArray("abcdef"));
        QVERIFY(!first.isReadOnly());
        QVERIFY(second.isReadOnly());
        QVERIFY(doc->replace(1, QByteArray("Z")));
        QCOMPARE(second.document()->read(0, 6), QByteArray("aZcdef"));
        second.cursor()->moveTo(qsizetype(4));
        QVERIFY(first.cursor()->position().offset() != 4);
    }

    void viewDecorationsAreLocalAndSupportEmptyBoundary() {
        QSharedPointer<QHexDocument> doc(
            QHexDocument::fromInternalBuffer(false));
        QHexView first, second;
        first.setDocument(doc);
        second.setDocument(doc);
        first.resize(640, 180);
        second.resize(640, 180);
        first.show();
        second.show();
        QApplication::processEvents();
        first.renderer()->enableCursor(false);
        second.renderer()->enableCursor(false);
        const auto original = first.grab().toImage();
        QSignalSpy changed(doc.data(), &QHexDocument::documentChanged);
        auto renderer =
            new DiffRenderer(doc.data(), first.cursor(),
                             QFontMetricsF(first.font()), &first, false);
        QPointer<QHexRenderer> previous(first.renderer());
        QVERIFY(first.setRenderer(renderer));
        QTRY_VERIFY(previous.isNull());
        auto results = std::make_shared<ResultStore>(MiB);
        QVERIFY(results->append({0, 0, 0, 1, Type::Insert, false}));
        results->publish();
        renderer->setResults(results);
        const auto decorated = first.grab().toImage();
        QVERIFY(original != decorated);
        QCOMPARE(changed.count(), 0);
        QVERIFY(!doc->metadata()->hasMetadata());
        QCOMPARE(second.grab().toImage(), original);
    }
    void rendererReplacementPreservesViewState() {
        QHexView view, other;
        auto document = view.document();
        QVERIFY(document->insert(0, QByteArray("abcdefghijklmnop")));
        other.setDocument(document);
        view.setAsciiVisible(false);
        view.setAddressVisible(false);
        view.setHeaderColor(Qt::cyan);
        auto cursor = view.cursor();
        cursor->moveTo(qsizetype(3));
        QPointer<QHexRenderer> old(view.renderer());
        auto renderer =
            new DiffRenderer(other.document().data(), other.cursor(),
                             QFontMetricsF(other.font()), nullptr, false);
        QVERIFY(view.setRenderer(renderer));
        QTRY_VERIFY(old.isNull());
        QCOMPARE(renderer->parent(), &view);
        QCOMPARE(view.cursor(), cursor);
        QVERIFY(!view.asciiVisible());
        QVERIFY(!view.addressVisible());
        QCOMPARE(view.headerColor(), QColor(Qt::cyan));
        QVERIFY(other.asciiVisible());
        QSignalSpy repaint(renderer, &QHexRenderer::repaintRequested);
        QSharedPointer<QHexDocument> replacement(
            QHexDocument::fromInternalBuffer(false));
        view.setDocument(replacement, cursor);
        QCOMPARE(view.renderer(), renderer);
        QVERIFY(view.document()->insert(0, QByteArray("new")));
        QVERIFY(view.setRenderer(renderer));
        QCOMPARE(view.cursor()->position().offset(), qsizetype(3));
    }
    void selectionOverridesDiffInHexAndAscii() {
        QSharedPointer<QHexDocument> doc(
            QHexDocument::fromInternalBuffer(false));
        QVERIFY(doc->insert(0, QByteArray("abcdefgh")));
        QHexView a, b;
        a.setDocument(doc);
        b.setDocument(doc);
        a.resize(640, 180);
        b.resize(640, 180);
        a.show();
        b.show();
        QApplication::processEvents();
        a.renderer()->enableCursor(false);
        b.renderer()->enableCursor(false);
        auto renderer = new DiffRenderer(doc.data(), a.cursor(),
                                         QFontMetricsF(a.font()), &a, false);
        QVERIFY(a.setRenderer(renderer));
        auto store = std::make_shared<ResultStore>(MiB);
        QVERIFY(store->append({0, 8, 0, 8, Type::Replace, false}));
        store->publish();
        renderer->setResults(store);
        QVERIFY(a.grab().toImage() != b.grab().toImage());
        a.cursor()->moveTo(qsizetype(0));
        a.cursor()->select(qsizetype(8));
        b.cursor()->moveTo(qsizetype(0));
        b.cursor()->select(qsizetype(8));
        QCOMPARE(a.grab().toImage(), b.grab().toImage());
    }
    void continuousMillionRows() {
        auto store = std::make_shared<ResultStore>(64 * MiB);
        DiffResultModel model;
        model.setStore(store);
        QElapsedTimer timer;
        timer.start();
        for (quint64 i = 0; i < 1000000; ++i) {
            QVERIFY(store->append({i * 2, 1, i * 2, 1, Type::Replace, false}));
            if (i == 99999) {
                store->publish();
                model.refresh();
                QCOMPARE(model.rowCount(), 100000);
            }
        }
        store->publish();
        model.refresh();
        QCOMPARE(model.rowCount(), 1000000);
        QCOMPARE(model.range(999999).leftOffset, quint64(1999998));
        qInfo() << "Million rows: elapsed ms" << timer.elapsed()
                << "compact storage bytes" << store->allocatedBytes();
    }
    void cancellationDuringReadHasNoError() {
        QTest::failOnWarning();
        bool cancelled = false;
        class CancellingDevice final : public QIODevice {
        public:
            bool *cancelled;
            explicit CancellingDevice(bool *token) : cancelled(token) {
                open(ReadOnly);
            }
            qint64 size() const override { return 100; }
            bool isSequential() const override { return false; }
            qint64 readData(char *, qint64) override {
                *cancelled = true;
                return -1;
            }
            qint64 writeData(const char *, qint64) override { return -1; }
        } a(&cancelled), b(&cancelled);
        ResultStore store(MiB);
        const auto outcome = BinaryDiff().compare(&a, &b, Options(), store,
                                                  [&] { return cancelled; });
        QCOMPARE(outcome.status, Status::Cancelled);
        QCOMPARE(outcome.error, Error::None);
    }
    void readErrorIsNotEndOfFile() {
        class BrokenDevice final : public QIODevice {
        public:
            qint64 size() const override { return 100; }
            bool isSequential() const override { return false; }
            qint64 readData(char *, qint64) override { return -1; }
            qint64 writeData(const char *, qint64) override { return -1; }
        };
        BrokenDevice a, b;
        QVERIFY(a.open(QIODevice::ReadOnly));
        QVERIFY(b.open(QIODevice::ReadOnly));
        ResultStore store(24 * MiB);
        const auto result = BinaryDiff().compare(&a, &b, Options(), store);
        QCOMPARE(result.status, Status::Failed);
        QVERIFY(result.error != Error::None);
    }
    void publishedRangesRemainImmutable() {
        ResultStore store(24 * MiB);
        QVERIFY(store.append({0, 2, 0, 2, Type::Replace, true}));
        store.publish();
        QVERIFY(store.append({2, 3, 2, 3, Type::Replace, true}));
        Range first;
        QVERIFY(store.difference(0, &first));
        QCOMPARE(first.leftLength, 2);
        QCOMPARE(store.differences(), 2);
    }

    void resultPagesAndLargeOffsets() {
        auto results = std::make_shared<ResultStore>(24 * MiB);
        for (quint64 i = 0; i < 5000; ++i) {
            QVERIFY(
                results->append({i * 2, 1, i * 2, 1, Type::Replace, false}));
            QVERIFY(results->append(
                {i * 2 + 1, 1, i * 2 + 1, 1, Type::Equal, false}));
        }
        QCOMPARE(results->differences(), 5000);
        QCOMPARE(results->coveredOffsets(),
                 qMakePair(quint64(10000), quint64(10000)));
        DiffResultModel model;
        results->publish();
        model.setStore(results);
        QCOMPARE(model.rowCount(), 5000);
        QCOMPARE(model.globalRow(2048), 2048);
        QCOMPARE(model.range(2048).leftOffset, 4096);
        QSignalSpy inserted(&model, &QAbstractItemModel::rowsInserted);
        QVERIFY(results->append({10000, 1, 10000, 1, Type::Replace, false}));
        results->publish();
        model.refresh();
        QCOMPARE(model.rowCount(), 5001);
        QCOMPARE(inserted.count(), 1);
        ResultStore large(MiB);
        QVERIFY(large.append(
            {0, 64 * 1024 * MiB, 0, 48 * 1024 * MiB, Type::Replace, true}));
        QCOMPARE(large.mapOffset(32 * 1024 * MiB, false), 24 * 1024 * MiB);
    }
    void memoryAndDetailDegradation() {
        QByteArray a(2 * int(MiB), Qt::Uninitialized);
        std::mt19937 generator(99);
        for (auto &c : a) {
            c = char(generator());
        }
        auto b = a;
        for (int i = 0; i < b.size(); i += 128) {
            b[i] ^= 0x55;
        }
        QBuffer left(&a), right(&b);
        left.open(QIODevice::ReadOnly);
        right.open(QIODevice::ReadOnly);
        Options options;
        options.detailLimit = 1000;
        ResultStore results(8 * MiB);
        auto outcome = BinaryDiff().compare(&left, &right, options, results);
        QCOMPARE(outcome.status, Status::Complete);
        QVERIFY(results.summarizing.load());
        QVERIFY(results.coarseCount() > 0);
        QVERIFY(results.allocatedBytes() <= 8 * MiB);
        QVERIFY(results.differences() < 2000);
        QVERIFY(outcome.bytesRead < 20 * MiB);
    }
    void sourceMemoryAndOwnerCancellation() {
        QTest::failOnWarning();
        QSharedPointer<QHexDocument> doc(
            QHexDocument::fromInternalBuffer(false));
        QVERIFY(doc->insert(0, QByteArray(1000, 'Q')));
        const auto source = Source::capture(doc.data(), MiB).source;
        QVERIFY(source.valid());
        auto token = std::make_shared<std::atomic_bool>(false);
        auto future = QtConcurrent::run([source, token] {
            auto device = source.open(token).device;
            return device ? device->readAll() : QByteArray();
        });
        QTRY_VERIFY(future.isFinished());
        QCOMPARE(future.result(), doc->read(0, 1000));
        // Force the bounded owner-thread path by denying mapping storage.
        const auto owner = Source::capture(doc.data(), 0).source;
        auto pending = QtConcurrent::run([owner, token] {
            auto device = owner.open(token).device;
            return device ? device->read(100) : QByteArray();
        });
        *token = true;
        QTRY_VERIFY(pending.isFinished());
        QVERIFY(pending.result().isEmpty());
    }
    void largeEditedPayloadDoesNotPinAnUnboundedVersion() {
        QSharedPointer<QHexDocument> doc(
            QHexDocument::fromInternalBuffer(false));
        QVERIFY(doc->insert(0, QByteArray(2 * int(MiB), 'A')));
        QVERIFY(!doc->buffer()->readSession(MiB).session);
        const auto source = Source::capture(doc.data(), MiB).source;
        QVERIFY(source.valid());
        auto token = std::make_shared<std::atomic_bool>(false);
        auto future = QtConcurrent::run([source, token] {
            auto device = source.open(token).device;
            if (!device || !device->seek(2 * qint64(MiB) - 16)) {
                return QByteArray();
            }
            return device->read(16);
        });
        QTRY_VERIFY(future.isFinished());
        QCOMPARE(future.result(), QByteArray(16, 'A'));
    }

    void typedReadErrors() {
        QTest::ignoreMessage(QtWarningMsg, "Cannot capture a null document");
        QCOMPARE(Source::capture(nullptr, MiB).error,
                 QHexReadError::InvalidDevice);
        QTemporaryDir dir;
        const auto path = dir.filePath(QStringLiteral("source.bin"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write("ABCD"), 4);
        file.close();
        QSharedPointer<QHexDocument> doc(QHexDocument::fromFile(path, false));
        const auto captured = doc->buffer()->readSession(MiB);
        QCOMPARE(captured.error, QHexReadError::None);
        QVERIFY(captured.session);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write("changed"), 7);
        file.close();
        QTest::ignoreMessage(
            QtWarningMsg,
            QRegularExpression(
                "Cannot open read session.*source.bin.*source file changed"));
        auto opened = captured.session->createDevice();
        QCOMPARE(opened.error, QHexReadError::SourceChanged);
        QVERIFY(!opened.device);
    }
    void readSessionEdits() {
        QTemporaryDir dir;
        const auto path = dir.filePath("input.bin");
        QByteArray original(20000, 'A');
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(original), original.size());
        file.close();
        QSharedPointer<QHexDocument> doc(QHexDocument::fromFile(path, false));
        QVERIFY(doc);
        QVERIFY(doc->insert(300, "INSERT"));
        QVERIFY(doc->remove(8200, 200));
        QVERIFY(doc->replace(12000, QByteArray("REPLACED")));
        auto expected = doc->read(0, doc->length());
        auto session = doc->buffer()->readSession(MiB).session;
        QVERIFY(session);
        auto future = QtConcurrent::run([session] {
            auto device = session->createDevice().device;
            return device ? device->readAll() : QByteArray();
        });
        future.waitForFinished();
        QCOMPARE(future.result(), expected);
        QCOMPARE(doc->buffer()->ioDevice()->pos(), 0);
        QVERIFY(doc->replace(0, QByteArray("B")));
        auto device = session->createDevice().device;
        QVERIFY(device);
        QCOMPARE(device->read(1), expected.first(1));
    }
};
QTEST_MAIN(DiffTests)
#include "tst_diff.moc"
