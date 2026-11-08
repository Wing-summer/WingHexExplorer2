#include "control/toast.h"

#include <QEnterEvent>
#include <QLineEdit>
#include <QTest>
#include <QWindow>

class ToastTest : public QObject {
    Q_OBJECT

private:
    static Toast *showToast(QWidget &parent,
                            const QString &text = QStringLiteral("Toast")) {
        Toast::toast(&parent, {}, text);
        return parent.findChild<Toast *>();
    }

    static void leaveToast(Toast *toast) {
        QEvent leave(QEvent::Leave);
        QApplication::sendEvent(toast, &leave);
    }

private slots:
    void overlayAndFocus() {
        QWidget host;
        host.resize(640, 480);
        QLineEdit edit(&host);
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));
        host.activateWindow();
        edit.setFocus();
        QCoreApplication::processEvents();
        auto focus = QApplication::focusWidget();
        auto toast = showToast(host);
        QVERIFY(toast);
        QVERIFY(toast->isVisible());
        QVERIFY(!toast->isWindow());
        QVERIFY(!toast->testAttribute(Qt::WA_NativeWindow));
        QCOMPARE(toast->parentWidget(), &host);
        QCOMPARE(QApplication::focusWidget(), focus);
        QCOMPARE(toast->focusPolicy(), Qt::NoFocus);
        QVERIFY(host.rect().contains(toast->geometry()));
    }

    void resizeAndPosition() {
        QWidget host;
        host.resize(640, 480);
        host.show();
        auto toast = showToast(host, QString(500, QLatin1Char('W')));
        QVERIFY(toast);
        for (auto position : {Toast::TOP, Toast::CENTER, Toast::BOTTOM}) {
            toast->setToastPos(position);
            for (const auto size :
                 {QSize(800, 600), QSize(180, 90), QSize(12, 10)}) {
                host.resize(size);
                QCoreApplication::processEvents();
                QCOMPARE(toast->lastToastPos(), position);
                QVERIFY(host.contentsRect().contains(toast->geometry()));
            }
        }
        host.resize(800, 600);
        host.setContentsMargins(20, 30, 40, 50);
        QCoreApplication::processEvents();
        QVERIFY(host.contentsRect().contains(toast->geometry()));
    }

    void nestedParentAndInput() {
        QWidget host;
        host.resize(640, 480);
        QWidget anchor(&host);
        anchor.setGeometry(80, 80, 400, 300);
        host.show();
        QPointer<Toast> toast = showToast(anchor);
        QVERIFY(toast);
        QVERIFY(QTest::qWaitForWindowExposed(&host));
        QTest::mouseClick(host.windowHandle(), Qt::LeftButton, Qt::NoModifier,
                          toast->mapTo(&host, toast->rect().center()));
        QVERIFY(toast->isVisible());
        QTest::mouseClick(toast, Qt::LeftButton, Qt::NoModifier,
                          toast->rect().center());
        QVERIFY(toast->isVisible());
        // Even events delivered directly to the anchor must dismiss it.
        QTest::mouseClick(&anchor, Qt::LeftButton, Qt::NoModifier,
                          QPoint(1, 1));
        QVERIFY(!toast->isVisible());
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(toast.isNull());
        toast = showToast(anchor);
        QTest::keyClick(&anchor, Qt::Key_A);
        QVERIFY(!toast->isVisible());
    }

    void ancestorHidden() {
        QWidget host;
        host.resize(640, 480);
        QWidget anchor(&host);
        anchor.resize(400, 300);
        host.show();
        QPointer<Toast> toast = showToast(anchor);
        host.hide();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(toast.isNull());
    }

    void replacementAndParentDestruction() {
        auto host = new QWidget;
        host->resize(640, 480);
        host->show();
        QPointer<Toast> first = showToast(*host);
        Toast::toast(host, {}, QStringLiteral("Replacement"));
        QVERIFY(!first->isVisible());
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(first.isNull());
        QPointer<Toast> replacement = host->findChild<Toast *>();
        QVERIFY(replacement);
        delete host;
        QVERIFY(replacement.isNull());
        QWidget nextHost;
        nextHost.show();
        QVERIFY(showToast(nextHost));
    }

    void fontIconAndStyle() {
        QWidget host;
        host.setStyleSheet(QStringLiteral("background: #aabbcc"));
        host.resize(640, 480);
        host.show();
        auto toast = showToast(host, QStringLiteral("A\r\nB\nC\rD"));
        const auto oldSize = toast->size();
        QPixmap icon(64, 64);
        icon.fill(Qt::red);
        icon.setDevicePixelRatio(2);
        toast->setIcon(icon);
        QVERIFY(toast->width() > oldSize.width());
        QFont font = toast->font();
        font.setPixelSize(24);
        toast->setFont(font);
        QVERIFY(toast->height() > oldSize.height());
        toast->setStyleSheet(
            QStringLiteral("Toast { qproperty-backcolor: #123456; "
                           "qproperty-textcolor: #abcdef; }"));
        QCOMPARE(toast->backColor(), QColor(QStringLiteral("#123456")));
        QCOMPARE(toast->textColor(), QColor(QStringLiteral("#abcdef")));
        const auto image = toast->grab().toImage();
        QCOMPARE(image.pixelColor(image.width() / 2, 1), toast->backColor());
        // Rounded corners reveal the parent, including with a styled host.
        QCOMPARE(image.pixelColor(0, 0), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(image.pixelColor(image.width() - 1, image.height() - 1),
                 QColor(QStringLiteral("#aabbcc")));
    }

    void multilineAndWrapping() {
        QWidget host;
        host.resize(640, 600);
        host.show();
        const auto snapshot = [&host](const QString &text) {
            auto toast = showToast(host, text);
            toast->setBackColor(Qt::black);
            toast->setTextColor(Qt::white);
            // Native Wayland may use fractional scaling; inspect logical
            // pixels.
            const auto pixmap = toast->grab();
            const auto image = pixmap.toImage().scaled(toast->size());
            delete toast;
            return image;
        };
        const auto single = snapshot(QStringLiteral("X"));
        const auto twoLines = snapshot(QStringLiteral("X\nX"));
        QVERIFY(twoLines.height() > single.height());
        QCOMPARE(twoLines.width(), single.width());
        QCOMPARE(snapshot(QStringLiteral("X\r\nX")), twoLines);
        QCOMPARE(snapshot(QStringLiteral("X\rX")), twoLines);
        const auto blankLine = snapshot(QStringLiteral("X\n\nX"));
        QVERIFY(blankLine.height() > twoLines.height());
        QCOMPARE(snapshot(QStringLiteral("\nX\n")).height(),
                 blankLine.height());

        // Verify that both lines contain text, not just extra blank space.
        const auto hasText = [](const QImage &image, int top, int bottom) {
            for (int y = top; y < bottom; ++y) {
                for (int x = 10; x < image.width() - 10; ++x) {
                    if (image.pixelColor(x, y).red() > 128) {
                        return true;
                    }
                }
            }
            return false;
        };
        const int middle = twoLines.height() / 2;
        QVERIFY(hasText(twoLines, 10, middle));
        QVERIFY(hasText(twoLines, middle, twoLines.height() - 10));
        const int lineHeight = single.height() - 20;
        QVERIFY(!hasText(blankLine, 10 + lineHeight, 10 + lineHeight * 2));

        const QString longWord(60, QLatin1Char('W'));
        const auto wide = snapshot(longWord);
        host.resize(200, 600);
        const auto narrow = snapshot(longWord);
        QVERIFY(narrow.height() > wide.height());
        QVERIFY(narrow.width() <= host.width());
        QVERIFY(hasText(narrow, 10, narrow.height() - 10));

        auto toast = showToast(host, QStringLiteral("First\nSecond"));
        const auto textOnlySize = toast->size();
        QPixmap icon(32, 32);
        icon.fill(Qt::red);
        toast->setIcon(icon);
        QVERIFY(toast->width() > textOnlySize.width());
        QCOMPARE(toast->height(), textOnlySize.height());
        QCOMPARE(
            toast->grab().toImage().scaled(toast->size()).pixelColor(11, 11),
            QColor(Qt::red));
        host.resize(100, 60);
        QCoreApplication::processEvents();
        QVERIFY(host.contentsRect().contains(toast->geometry()));
        QVERIFY(!toast->grab().isNull());
    }

    void pausedLifetimeAndFade() {
        QWidget host;
        host.setStyleSheet(QStringLiteral("background: white"));
        host.resize(640, 480);
        host.show();
        QPointer<Toast> toast = showToast(host);
        toast->setBackColor(Qt::black);
        QEnterEvent enter(QPointF(1, 1), QPointF(1, 1), QPointF(1, 1));
        QApplication::sendEvent(toast, &enter);
        QTest::qWait(3500);
        QVERIFY(toast && toast->isVisible());
        leaveToast(toast);
        QTest::qWait(3150);
        QVERIFY(toast && toast->isVisible());
        const auto image = toast->grab().toImage();
        const QPoint sample(image.width() / 2, 1);
        const auto color = image.pixelColor(sample);
        QCOMPARE(image.pixelColor(0, 0), QColor(Qt::white));
        QVERIFY2(color.red() > 0 && color.red() < 255,
                 qPrintable(color.name()));
        // Hover must pause the fade too, not just the initial stay time.
        QApplication::sendEvent(toast, &enter);
        QTest::qWait(500);
        QVERIFY(toast && toast->isVisible());
        QCOMPARE(toast->grab().toImage().pixelColor(sample), color);
        leaveToast(toast);
        QTRY_VERIFY_WITH_TIMEOUT(toast.isNull(), 1000);
    }

    void invalidParent() {
        Toast::toast(nullptr, {}, QStringLiteral("Null"));
        QWidget hidden;
        Toast::toast(&hidden, {}, QStringLiteral("Hidden"));
        QVERIFY(!hidden.findChild<Toast *>());
    }
};

QTEST_MAIN(ToastTest)
#include "tst_toast.moc"
