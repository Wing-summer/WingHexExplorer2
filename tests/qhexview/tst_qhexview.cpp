#include "QHexEdit2/chunks.h"
#include "qhexview.h"
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QImage>
#include <QSignalSpy>
#include <QStringEncoder>
#include <QTemporaryFile>
#include <QtTest>
#include <atomic>
#include <thread>
#include <vector>

class QHexViewTests : public QObject {
    Q_OBJECT
private:
    static QSharedPointer<QHexDocument> document() {
        auto doc = QSharedPointer<QHexDocument>(
            QHexDocument::fromInternalBuffer(false));
        if (!doc->insert(0, QByteArray("0123456789abcdef"))) {
            qFatal("Cannot prepare test document");
        }
        return doc;
    }

    static QColor backgroundAt(QHexView &view, int column = 0, int line = 0,
                               bool ascii = false, bool edge = false) {
        auto *renderer = view.renderer();
        QImage image(1600, 200, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer->render(&painter, line, line + 1, line);
        painter.end();
        const int x =
            qCeil((ascii ? renderer->getAsciiColumnX()
                         : renderer->getHexColumnX()) +
                  renderer->borderSize() +
                  column * (ascii ? 1 : 3) * renderer->getCellWidth()) +
            1;
        const int y =
            edge ? qFloor(renderer->headerLineCount() * renderer->lineHeight() +
                          0.5)
                 : qCeil(renderer->headerLineCount() * renderer->lineHeight()) +
                       2;
        return image.pixelColor(x, y);
    }

private slots:
    void unicodeCellsAvoidFontScan_data() {
        QTest::addColumn<int>("encoding");
        QTest::addColumn<QByteArray>("bytes");
        QTest::addColumn<QByteArray>("placeholderBytes");
        QTest::addColumn<bool>("hasGlyph");
        const QList<char32_t> characters = {0x0041, 0x4e2d,  0x0628,  0x0915,
                                            0x0780, 0x10400, 0x1f600, 0xffffd,
                                            0x0378, 0x202e};
        for (auto character : characters) {
            const auto text = QString::fromUcs4(&character, 1);
            bool hasGlyph = false;
            if (QChar::isPrint(character)) {
                for (const auto &family : QFontDatabase::families()) {
                    if (QRawFont::fromFont(QFont(family))
                            .supportsCharacter(character)) {
                        hasGlyph = true;
                        break;
                    }
                }
            }
            // An unassigned/noncharacter scalar of the same encoded size.
            const char32_t placeholder = character < 0x80      ? 0x00
                                         : character < 0x800   ? 0x0378
                                         : character < 0x10000 ? 0xffff
                                                               : 0x10ffff;
            for (auto encoding :
                 {QStringConverter::Utf8, QStringConverter::Utf16LE,
                  QStringConverter::Utf32LE}) {
                QStringEncoder encoder(encoding);
                const QByteArray bytes = encoder(text);
                const QByteArray placeholderBytes =
                    encoder(QString::fromUcs4(&placeholder, 1));
                const auto name =
                    QStringLiteral("U+%1-%2")
                        .arg(uint(character), 0, 16)
                        .arg(QString::fromLatin1(
                            QStringConverter::nameForEncoding(encoding)))
                        .toLatin1();
                QTest::newRow(name.constData())
                    << int(encoding) << bytes << placeholderBytes << hasGlyph;
            }
        }
    }

    void unicodeCellsAvoidFontScan() {
        QFETCH(int, encoding);
        QFETCH(QByteArray, bytes);
        QFETCH(QByteArray, placeholderBytes);
        QFETCH(bool, hasGlyph);
        QCOMPARE(bytes.size(), placeholderBytes.size());
        QHexView view;
        auto doc = QSharedPointer<QHexDocument>(
            QHexDocument::fromInternalBuffer(false));
        QVERIFY(doc->insert(0, bytes));
        view.setDocument(doc);
        auto *renderer = view.renderer();
        renderer->setStringEncoding(QStringConverter::Encoding(encoding));
        renderer->enableCursor(false);

        static int fontMessages = 0;
        static QtMessageHandler previousHandler = nullptr;
        fontMessages = 0;
        previousHandler = qInstallMessageHandler(
            [](QtMsgType type, const QMessageLogContext &context,
               const QString &message) {
                if (message.contains(
                        QStringLiteral("OpenType support missing"))) {
                    ++fontMessages;
                } else if (previousHandler) {
                    previousHandler(type, context, message);
                }
            });
        const auto render = [&] {
            QImage image(1600, 200, QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            renderer->render(&painter, 0, 1, 0);
            return image;
        };
        const auto first = render();
        const auto second = render();
        // Application fonts can be installed or removed while a view exists.
        QMetaObject::invokeMethod(qGuiApp, "fontDatabaseChanged");
        const auto afterInvalidation = render();
        qInstallMessageHandler(previousHandler);
        QCOMPARE(fontMessages, 0);
        QCOMPARE(first, second);
        QCOMPARE(first, afterInvalidation);

        QVERIFY(doc->replace(0, placeholderBytes));
        const auto placeholder = render();
        const QRect asciiRect(
            qCeil(renderer->getAsciiColumnX() + renderer->borderSize()),
            qCeil(renderer->headerLineCount() * renderer->lineHeight()),
            qFloor(renderer->getCellWidth() * bytes.size()),
            qFloor(renderer->lineHeight()));
        if (hasGlyph) {
            QVERIFY(first.copy(asciiRect) != placeholder.copy(asciiRect));
        } else {
            QCOMPARE(first.copy(asciiRect), placeholder.copy(asciiRect));
        }
    }

    void concurrentChunkReadsKeepDeviceCursorConsistent() {
        QTemporaryFile file;
        QVERIFY(file.open());
        QByteArray expected(64 * 1024, 'a');
        for (qsizetype i = 0; i < expected.size(); ++i) {
            expected[i] = char(i % 251);
        }
        QCOMPARE(file.write(expected), qint64(expected.size()));
        QVERIFY(file.flush());
        Chunks chunks(&file);
        QVERIFY(chunks.overwrite(0, 'X'));
        expected[0] = 'X';
        const auto needle = expected.mid(123, 16);
        std::atomic<bool> correct{true};
        std::vector<std::thread> readers;
        for (int thread = 0; thread < 8; ++thread) {
            readers.emplace_back([&, thread] {
                for (int i = 0; i < 100; ++i) {
                    const auto offset = (thread * 7919 + i * 113) % 65000;
                    if (chunks.data(offset, 97) != expected.mid(offset, 97) ||
                        chunks.indexOf(needle, 0) != expected.indexOf(needle) ||
                        chunks.lastIndexOf(needle, expected.size()) !=
                            expected.lastIndexOf(needle)) {
                        correct = false;
                    }
                }
            });
        }
        for (auto &reader : readers) {
            reader.join();
        }
        QVERIFY(correct.load());
    }
    void rebindingDoesNotDuplicateCursorConnections() {
        QHexView view;
        auto doc = document();
        for (int i = 0; i < 4; ++i) {
            view.setDocument(doc);
        }
        QSignalSpy location(&view, &QHexView::cursorLocationChanged);
        view.cursor()->moveTo(qsizetype(2));
        QCOMPARE(location.count(), 1);
        QSignalSpy selection(&view, &QHexView::cursorSelectionChanged);
        view.cursor()->select(qsizetype(2));
        QCOMPARE(selection.count(), 1);
    }

    void sharedCursorKeepsOtherViewsConnected() {
        QHexCursor cursor;
        QHexView first, second;
        auto doc = document();
        first.setDocument(doc, &cursor);
        second.setDocument(doc, &cursor);
        QSignalSpy firstChanges(&first, &QHexView::cursorLocationChanged);
        QSignalSpy secondChanges(&second, &QHexView::cursorLocationChanged);
        second.setDocument(doc, &cursor);
        cursor.moveTo(qsizetype(3));
        QCOMPARE(firstChanges.count(), 1);
        QCOMPARE(secondChanges.count(), 1);
    }

    void displaySettingsDoNotChangeDocument() {
        QHexView view;
        auto doc = document();
        view.setDocument(doc);
        QSignalSpy documentChanges(doc.data(), &QHexDocument::documentChanged);
        QSignalSpy layout(view.renderer(), &QHexRenderer::layoutChanged);
        QSignalSpy repaint(view.renderer(), &QHexRenderer::repaintRequested);
        view.setAsciiVisible(false);
        view.setAsciiVisible(false);
        view.setAddressVisible(false);
        view.setHeaderVisible(false);
        view.renderer()->setStringEncoding(QStringConverter::Utf8);
        view.setCursorSync(false);
        QCOMPARE(documentChanges.count(), 0);
        QCOMPARE(layout.count(), 3);
        QCOMPARE(repaint.count(), 2);
    }

    void metadataStaysWithinDocumentAndUpdates() {
        QHexView first, second;
        auto firstDoc = document();
        auto secondDoc = document();
        QVERIFY(firstDoc->metadata()->background(0, 2, Qt::red));
        first.setDocument(firstDoc);
        second.setDocument(secondDoc);
        QCOMPARE(backgroundAt(first), QColor(Qt::red));
        QCOMPARE(backgroundAt(second),
                 second.renderer()->bytesAlterBackground());
        firstDoc->metadata()->clear();
        QVERIFY(firstDoc->metadata()->background(0, 2, Qt::green));
        QCOMPARE(backgroundAt(first), QColor(Qt::green));
        QCOMPARE(backgroundAt(first, 3),
                 first.renderer()->bytesAlterBackground());
        first.setDocument(secondDoc);
        QCOMPARE(backgroundAt(first), first.renderer()->bytesAlterBackground());
    }

    void metadataBackgroundRemainsVisible_data() {
        QTest::addColumn<QColor>("base");
        QTest::addColumn<QColor>("metadata");
        QTest::newRow("white") << QColor(Qt::white) << QColor(Qt::white);
        QTest::newRow("black") << QColor(Qt::black) << QColor(Qt::black);
        QTest::newRow("near-light") << QColor("#fafafa") << QColor("#f8f9fa");
        QTest::newRow("near-dark") << QColor("#202020") << QColor("#242322");
        QTest::newRow("translucent")
            << QColor(Qt::white) << QColor(255, 0, 0, 5);
        QTest::newRow("middle") << QColor("#808080") << QColor("#808080");
    }

    void metadataBackgroundRemainsVisible() {
        QFETCH(QColor, base);
        QFETCH(QColor, metadata);
        QHexView view;
        auto doc = document();
        QVERIFY(doc->insert(doc->length(), QByteArray("0123456789abcdef")));
        QVERIFY(doc->metadata()->color(0, 31, Qt::black, metadata));
        view.setDocument(doc);
        auto *renderer = view.renderer();
        renderer->enableCursor(false);
        renderer->setBytesAlterBackground(base);
        renderer->setBytesBackground(base);
        for (int line : {0, 1}) {
            for (bool ascii : {false, true}) {
                const auto edge = backgroundAt(view, 2, line, ascii, true);
                QVERIFY(QHexMetadata::calculateContrastRatio(edge, base) >=
                        1.3);
                QImage expected(1, 1, QImage::Format_ARGB32);
                expected.fill(base);
                QPainter painter(&expected);
                painter.fillRect(expected.rect(), metadata);
                painter.end();
                QCOMPARE(backgroundAt(view, 2, line, ascii),
                         expected.pixelColor(0, 0));
            }
        }
        QCOMPARE(doc->metadata()->getAllMetadata().constFirst().background,
                 metadata);
        // A theme change must use the new row background immediately.
        renderer->setBytesAlterBackground(Qt::red);
        const auto edge = backgroundAt(view, 2, 0, false, true);
        // Clearly contrasting fills do not acquire a border after switching.
        if (metadata.alpha() == 255 &&
            QHexMetadata::calculateContrastRatio(metadata, Qt::red) >= 1.3) {
            QCOMPARE(edge, metadata);
        }
        doc->setMetabgVisible(false);
        QCOMPARE(backgroundAt(view), QColor(Qt::red));
        doc->setMetabgVisible(true);
        renderer->setSelBackgroundColor(QColor("#3874d9"));
        view.cursor()->moveTo(qsizetype(0));
        view.cursor()->select(qsizetype(1));
        QCOMPARE(backgroundAt(view), renderer->selBackgroundColor());
    }

    void metadataOutlineLeavesUnderlineClear() {
        for (const auto base : {QColor(Qt::white), QColor(Qt::black)}) {
            for (int pointSize : {10, 18}) {
                QHexView view;
                auto font = view.font();
                font.setPointSize(pointSize);
                view.setFont(font);
                auto doc = document();
                QVERIFY(doc->metadata()->metadata(0, 7, Qt::black, base,
                                                  QStringLiteral("comment")));
                view.setDocument(doc);
                auto *renderer = view.renderer();
                renderer->enableCursor(false);
                renderer->setBytesAlterBackground(base);
                const auto render = [&] {
                    QImage image(1600, 200, QImage::Format_ARGB32);
                    image.fill(base);
                    QPainter painter(&image);
                    renderer->render(&painter, 0, 1, 0);
                    return image;
                };
                const auto framed = render();
                doc->setMetabgVisible(false);
                const auto unframed = render();
                const auto top =
                    renderer->headerLineCount() * renderer->lineHeight();
                // The full lower band must match rendering without a frame,
                // including the outlined underline and both of its ends.
                const auto y = qFloor(top + renderer->lineHeight() - 3);
                const auto bottom = qFloor(top + renderer->lineHeight());
                for (bool ascii : {false, true}) {
                    const auto left = (ascii ? renderer->getAsciiColumnX()
                                             : renderer->getHexColumnX()) +
                                      renderer->borderSize();
                    const auto width =
                        8 * (ascii ? 1 : 3) * renderer->getCellWidth();
                    const QRect band(qFloor(left), y, qCeil(width), bottom - y);
                    QCOMPARE(framed.copy(band), unframed.copy(band));
                    const auto x = qCeil(left) + 2;
                    const auto edgeY = qFloor(top + 0.5);
                    QVERIFY(framed.pixelColor(x, edgeY) !=
                            unframed.pixelColor(x, edgeY));
                }
            }
        }
    }

    void multibyteFallbackHonorsCursorBlink() {
        QHexView view;
        auto doc = document();
        QVERIFY(doc->replace(0, QByteArray::fromHex("c2a2")));
        QVERIFY(doc->metadata()->background(0, 0, Qt::red));
        QVERIFY(doc->metadata()->background(1, 1, Qt::green));
        view.setDocument(doc);
        auto *renderer = view.renderer();
        renderer->setStringEncoding(QStringConverter::Utf8);
        const auto sample = [&] {
            QImage image(1600, 200, QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            renderer->render(&painter, 0, 1, 0);
            painter.end();
            return image.pixelColor(
                qCeil(renderer->getAsciiColumnX() + renderer->borderSize()) + 1,
                qCeil(renderer->headerLineCount() * renderer->lineHeight()) +
                    1);
        };
        renderer->enableCursor(false);
        QCOMPARE(sample(), QColor(Qt::red));
        renderer->enableCursor(true);
        QCOMPARE(sample(), renderer->bytesColor());
    }

    void indexedSelectionUsesItsOwnLength() {
        QHexView view;
        view.setDocument(document());
        auto *cursor = view.cursor();
        cursor->moveTo(qsizetype(0));
        cursor->select(qsizetype(2));
        cursor->moveTo(qsizetype(6));
        cursor->select(qsizetype(3), QHexCursor::SelectionAdd);
        QCOMPARE(cursor->selectionCount(), 2);
        QCOMPARE(cursor->currentSelectionLength(), 5);
        for (int i = 0; i < cursor->selectionCount(); ++i) {
            QCOMPARE(view.selectedBytes(i).size(), cursor->selectionLength(i));
        }
        QCOMPARE(view.selectedBytes().join().size(), 5);
    }

    void multipleSelectionsRemoveFromHighestOffset() {
        for (bool undoable : {false, true}) {
            QHexView view;
            auto doc = document();
            view.setDocument(doc);
            view.cursor()->select(qsizetype(2));
            view.cursor()->moveTo(qsizetype(6));
            view.cursor()->select(qsizetype(3), QHexCursor::SelectionAdd);
            QCOMPARE(view.selectionCount(), 2);
            QVERIFY(undoable ? view.RemoveSelection() : view.removeSelection());
            QCOMPARE(doc->read(0), QByteArray("23459abcdef"));
            if (undoable) {
                doc->undo();
                QCOMPARE(doc->read(0), QByteArray("0123456789abcdef"));
            }
        }
    }

    void offsetMoveForwardsClearSelection() {
        QHexCursor cursor;
        cursor.select(qsizetype(3));
        cursor.moveTo(qsizetype(5), true);
        QVERIFY(!cursor.hasSelection());
        QCOMPARE(cursor.currentNibble(), 1);
        QCOMPARE(cursor.position().offset(), 5);
    }

    void lineWidthPreservesByteOffsets() {
        QHexView view;
        auto doc = document();
        doc->setHexLineWidth(8);
        view.setDocument(doc);
        auto *cursor = view.cursor();
        cursor->moveTo(qsizetype(10));
        QCOMPARE(cursor->currentLine(), 1);
        QCOMPARE(cursor->currentColumn(), 2);
        cursor->select(qsizetype(3));
        doc->setHexLineWidth(4);
        QCOMPARE(cursor->position().offset(), 10);
        QCOMPARE(cursor->selectionStart(0).offset(), 10);
        QCOMPARE(cursor->selectionEnd(0).offset(), 12);
        QCOMPARE(cursor->currentSelectionLength(), 3);
        doc->setHexLineWidth(0);
        QCOMPARE(doc->hexLineWidth(), 4);
    }

    void failedPasteKeepsCursorPosition() {
        QHexView view;
        auto doc = document();
        view.setDocument(doc);
        view.cursor()->moveTo(qsizetype(2));
        view.cursor()->setInsertionMode(QHexCursor::InsertMode);
        QVERIFY(doc->setLockedFile(true));
        qApp->clipboard()->setText(QStringLiteral("AB"));
        QVERIFY(!view.Paste());
        QCOMPARE(view.currentOffset(), 2);
        QVERIFY(!view.paste());
        QCOMPARE(view.currentOffset(), 2);
        QCOMPARE(doc->read(0), QByteArray("0123456789abcdef"));
    }

    void cutReportsCopyLimitOnce() {
        QHexView view;
        view.setDocument(document());
        view.cursor()->select(qsizetype(2));
        view.setCopyLimit(0);
        QSignalSpy limit(&view, &QHexView::copyLimitRaised);
        QVERIFY(!view.Cut());
        QCOMPARE(limit.count(), 1);
        QVERIFY(!view.cut(false));
        QCOMPARE(limit.count(), 2);
    }

    void clipboardPasteKeepsUndoSemantics() {
        QHexView view;
        auto doc = document();
        view.setDocument(doc);
        view.cursor()->setInsertionMode(QHexCursor::InsertMode);
        qApp->clipboard()->setText(QStringLiteral("41 42"));
        QVERIFY(view.Paste(true));
        QCOMPARE(doc->read(0, 2), QByteArray("AB"));
        QVERIFY(doc->canUndo());
        doc->undo();
        QCOMPARE(doc->read(0), QByteArray("0123456789abcdef"));
        view.cursor()->moveTo(qsizetype(0));
        QVERIFY(view.paste(true));
        QCOMPARE(doc->read(0, 2), QByteArray("AB"));
        QVERIFY(!doc->canUndo());
    }
};

QTEST_MAIN(QHexViewTests)
#include "tst_qhexview.moc"
