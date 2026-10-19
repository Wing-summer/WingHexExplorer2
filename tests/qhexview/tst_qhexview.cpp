#include "qhexview.h"
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QSignalSpy>
#include <QtTest>

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

    static QColor backgroundAt(QHexView &view, int column = 0) {
        auto *renderer = view.renderer();
        QImage image(1600, 200, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer->render(&painter, 0, 1, 0);
        painter.end();
        const int x = qCeil(renderer->getHexColumnX() + renderer->borderSize() +
                            column * 3 * renderer->getCellWidth()) +
                      1;
        const int y =
            qCeil(renderer->headerLineCount() * renderer->lineHeight()) + 1;
        return image.pixelColor(x, y);
    }

private slots:
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
