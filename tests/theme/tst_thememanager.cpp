#include "class/application/startupfeedback.h"
#include "class/settings/thememanager.h"
#include "class/settings/themestylesheet.h"
#include "dialog/splashdialog.h"

#include "TestPlugin/testform.h"
#include "WingPlugin/iwingplugin.h"
#include "WingPlugin/iwingpluginbasecalls.h"
#include "WingPlugin/wingcore.h"
#include "WingPlugin/wingplugincalls.h"
#include "WingPlugin/wingplugincalls_p.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QDomDocument>
#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QPixmapCache>
#include <QPointer>
#include <QProgressBar>
#include <QProxyStyle>
#include <QPushButton>
#include <QSignalSpy>
#include <QStyleFactory>
#include <QStyleHints>
#include <QSvgRenderer>
#include <QToolButton>
#include <QtTest>
#include <cmath>
#include <memory>

using Icon = WingHex::StandardIcon;

namespace {
double relativeLuminance(const QColor &color) {
    const auto linear = [](double c) {
        return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * linear(color.redF()) + 0.7152 * linear(color.greenF()) +
           0.0722 * linear(color.blueF());
}

double contrastRatio(const QColor &first, const QColor &second) {
    const auto a = relativeLuminance(first);
    const auto b = relativeLuminance(second);
    return (qMax(a, b) + 0.05) / (qMin(a, b) + 0.05);
}
} // namespace

class FormProbePlugin final : public WingHex::IWingPlugin {
public:
    bool init(const std::unique_ptr<QSettings> &) override { return true; }
    void unload(std::unique_ptr<QSettings> &) override {}
    const QString pluginName() const override {
        return QStringLiteral("style-probe");
    }
    const QString pluginComment() const override { return {}; }
};

class ThemeEndpoint : public QObject {
    Q_OBJECT
public slots:
    QPixmap appAuthorPixmap(const QObject *sender) {
        Q_UNUSED(sender);
        QPixmap image(1, 1);
        image.fill(Qt::transparent);
        return image;
    }

    QIcon standardIcon(const QObject *sender, WingHex::StandardIcon icon) {
        Q_UNUSED(sender);
        return ThemeManager::instance().standardIcon(icon);
    }
    QPixmap standardPixmap(const QObject *sender, WingHex::StandardIcon icon) {
        Q_UNUSED(sender);
        return ThemeManager::instance().standardPixmap(icon);
    }
    QByteArray standardIconData(const QObject *sender,
                                WingHex::StandardIcon icon) {
        Q_UNUSED(sender);
        return ThemeManager::instance().standardIconData(icon);
    }
    WingHex::AppTheme currentAppTheme(const QObject *sender) {
        Q_UNUSED(sender);
        return ThemeManager::instance().currentTheme() ==
                       ThemeManager::Theme::Dark
                   ? WingHex::AppTheme::Dark
                   : WingHex::AppTheme::Light;
    }
};

class ThemeClient : public WingHex::IWingPluginBaseCalls {
public:
    explicit ThemeClient(ThemeEndpoint &endpoint) : m_endpoint(endpoint) {
        auto add = [this](auto function, const char *name,
                          const char *signature) {
            const auto index = m_endpoint.metaObject()->indexOfSlot(signature);
            Q_ASSERT(index >= 0);
            m_table.insert(WingHex::getFunctionSig(function, name),
                           m_endpoint.metaObject()->method(index));
        };
        add(&WingHex::IWingPluginBaseCalls::standardIcon, "standardIcon",
            "standardIcon(const QObject*,WingHex::StandardIcon)");
        add(&WingHex::IWingPluginBaseCalls::standardPixmap, "standardPixmap",
            "standardPixmap(const QObject*,WingHex::StandardIcon)");
        add(&WingHex::IWingPluginBaseCalls::standardIconData,
            "standardIconData",
            "standardIconData(const QObject*,WingHex::StandardIcon)");
        add(&WingHex::IWingPluginBaseCalls::currentAppTheme, "currentAppTheme",
            "currentAppTheme(const QObject*)");
    }
    WingHex::CallTable callTable() const override { return m_table; }
    QObject *callReceiver() const override { return &m_endpoint; }

protected:
    const QObject *getSender() const override { return &m_endpoint; }

private:
    ThemeEndpoint &m_endpoint;
    WingHex::CallTable m_table;
};

class ThemeTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        const auto output = qEnvironmentVariable("WING_THEME_PREVIEWS");
        if (output.isEmpty()) {
            return;
        }
        QDir().mkpath(output);
        auto &theme = ThemeManager::instance();
        QList<Icon> ids;
        for (const auto id : theme.standardIcons()) {
            if (theme.iconResource(id).contains(QStringLiteral("/images/"))) {
                ids.append(id);
            }
        }
        for (int page = 0; page * 32 < ids.size(); ++page) {
            QImage sheet(1120, 8 * 105, QImage::Format_ARGB32_Premultiplied);
            sheet.fill(QColor(QStringLiteral("#dddddd")));
            QPainter painter(&sheet);
            for (int index = 0; index < 32 && page * 32 + index < ids.size();
                 ++index) {
                const auto id = ids.at(page * 32 + index);
                const int x = index % 4 * 280;
                const int y = index / 4 * 105;
                const auto relative = theme.iconResource(id).section(
                    QStringLiteral("/images/"), 1);
                const auto name = relative.chopped(4);
                const auto original =
                    qEnvironmentVariable("WING_THEME_ORIGINALS") +
                    QLatin1Char('/') + name + QStringLiteral(".png");
                painter.fillRect(x + 4, y + 3, 80, 76,
                                 QColor(QStringLiteral("#ededed")));
                QPixmap before(original);
                if (!before.isNull()) {
                    painter.drawPixmap(x + 12, y + 9, 64, 64, before);
                }
                for (int variant = 0; variant < 2; ++variant) {
                    const auto scheme = variant == 0
                                            ? ThemeManager::Theme::Light
                                            : ThemeManager::Theme::Dark;
                    theme.setTheme(scheme);
                    painter.fillRect(x + 90 + variant * 90, y + 3, 80, 76,
                                     variant == 0
                                         ? QColor(QStringLiteral("#f5f5f5"))
                                         : QColor(QStringLiteral("#202326")));
                    painter.drawPixmap(x + 98 + variant * 90, y + 9,
                                       theme.standardIcon(id).pixmap(64, 64));
                }
                painter.setPen(Qt::black);
                painter.drawText(x + 5, y + 96,
                                 name +
                                     QStringLiteral("  | PNG / Light / Dark"));
            }
            painter.end();
            QVERIFY(sheet.save(output +
                               QStringLiteral("/icons-%1.png").arg(page + 1)));
        }
    }

    void init() {
        ThemeManager::instance().setTheme(ThemeManager::Theme::Dark);
    }

    void pluginGearIsTransparent() {
        auto &theme = ThemeManager::instance();
        for (auto scheme :
             {ThemeManager::Theme::Dark, ThemeManager::Theme::Light}) {
            theme.setTheme(scheme);
            const auto image =
                theme.standardIcon(Icon::Plugin).pixmap(200, 200).toImage();
            // The hub and overlapping puzzle area must both remain transparent.
            QCOMPARE(image.pixelColor(133, 133).alpha(), 0);
            QCOMPARE(image.pixelColor(105, 133).alpha(), 0);
            QVERIFY(image.pixelColor(133, 114).alpha() > 0);
        }
    }

    void crashArtworkHasTransparentTear() {
        auto &theme = ThemeManager::instance();
        for (auto scheme :
             {ThemeManager::Theme::Dark, ThemeManager::Theme::Light}) {
            theme.setTheme(scheme);
            const auto image =
                theme.standardIcon(Icon::Crash).pixmap(171, 171).toImage();
            QCOMPARE(image.pixelColor(112, 28).alpha(), 0);
            // Muted colors remain distinct from a monochrome rendering.
            const auto iris = image.pixelColor(100, 72);
            QVERIFY(iris.blue() > iris.red());
        }
    }

    void systemThemeFollowsNotifications() {
        auto &theme = ThemeManager::instance();
        theme.setTheme(ThemeManager::Theme::System);
        QCOMPARE(theme.requestedTheme(), ThemeManager::Theme::System);
        for (auto scheme : {Qt::ColorScheme::Dark, Qt::ColorScheme::Light,
                            Qt::ColorScheme::Unknown}) {
            QVERIFY(QMetaObject::invokeMethod(
                qApp->styleHints(), "colorSchemeChanged", Qt::DirectConnection,
                Q_ARG(Qt::ColorScheme, scheme)));
            QCOMPARE(theme.currentTheme(), scheme == Qt::ColorScheme::Dark
                                               ? ThemeManager::Theme::Dark
                                               : ThemeManager::Theme::Light);
            QCOMPARE(theme.requestedTheme(), ThemeManager::Theme::System);
        }
        theme.setTheme(ThemeManager::Theme::Dark);
        QVERIFY(QMetaObject::invokeMethod(
            qApp->styleHints(), "colorSchemeChanged", Qt::DirectConnection,
            Q_ARG(Qt::ColorScheme, Qt::ColorScheme::Light)));
        QCOMPARE(theme.currentTheme(), ThemeManager::Theme::Dark);
        QCOMPARE(theme.requestedTheme(), ThemeManager::Theme::Dark);
    }

    void pixmapCacheFollowsTheme() {
        auto &theme = ThemeManager::instance();
        const auto dark = theme.standardPixmap(Icon::Undo);
        QVERIFY(!dark.isNull());
        QCOMPARE(dark.size(), QSize(200, 200));
        QCOMPARE(dark.cacheKey(), theme.standardPixmap(Icon::Undo).cacheKey());
        theme.setTheme(ThemeManager::Theme::Light);
        QVERIFY(dark.toImage() != theme.standardPixmap(Icon::Undo).toImage());
        QVERIFY(theme.standardPixmap(Icon::None).isNull());
    }

    void resourceIconsFollowTheme() {
        auto &theme = ThemeManager::instance();
        const auto icon = theme.resourceIcon(
            QStringLiteral(":/com.wingsummer.winghex/images/search.svg"));
        QVERIFY(!icon.isNull());
        const auto dark = icon.pixmap(32, 32).toImage();
        theme.setTheme(ThemeManager::Theme::Light);
        QVERIFY(dark != icon.pixmap(32, 32).toImage());
        QVERIFY(theme.resourceIcon(QStringLiteral(":/missing.svg")).isNull());
    }

    void actualTestFormStyleCombo() {
        ThemeEndpoint endpoint;
        FormProbePlugin plugin;
        WingHex::WingPluginCallsCorePrivate calls;
        calls._fnCaller = &endpoint;
        const auto index = endpoint.metaObject()->indexOfSlot(
            "appAuthorPixmap(const QObject*)");
        QVERIFY(index >= 0);
        calls._fnTable.insert(
            WingHex::getFunctionSig(
                &WingHex::IWingPluginBaseCalls::appAuthorPixmap,
                "appAuthorPixmap"),
            endpoint.metaObject()->method(index));
        WingHex::CallTableEvent event(&calls);
        QCoreApplication::sendEvent(&plugin, &event);
        const auto roles = QMetaEnum::fromType<QStyle::StandardPixmap>();
        int expectedCount = 0;
        for (int i = 0; i < roles.keyCount(); ++i) {
            if (roles.value(i) >= 0 &&
                qstrcmp(roles.key(i), "NStandardPixmap") != 0) {
                ++expectedCount;
            }
        }
        QTest::failOnWarning(
            QRegularExpression(QStringLiteral("^cbToastIcon")));
        for (auto scheme :
             {ThemeManager::Theme::Light, ThemeManager::Theme::Dark}) {
            ThemeManager::instance().setTheme(scheme);
            TestForm form(&plugin);
            auto combo =
                form.findChild<QComboBox *>(QStringLiteral("cbToastIcon"));
            QVERIFY(combo);
            QCOMPARE(combo->count(), expectedCount);
            for (int row = 0; row < combo->count(); ++row) {
                QVERIFY2(!combo->itemIcon(row).isNull(),
                         qPrintable(combo->itemText(row)));
                QVERIFY2(!combo->itemData(row).value<QPixmap>().isNull(),
                         qPrintable(combo->itemText(row)));
            }
        }
    }

    void testFormStyleComboIcons() {
        auto &theme = ThemeManager::instance();
        const auto roles = QMetaEnum::fromType<QStyle::StandardPixmap>();
        QList<int> values;
        for (int i = 0; i < roles.keyCount(); ++i) {
            if (roles.value(i) >= 0 &&
                QByteArray(roles.key(i)) != "NStandardPixmap") {
                values.append(roles.value(i));
            }
        }
        auto missingIcons = [&](QStyle *style) {
            QStringList missing;
            for (const auto value : values) {
                // Match TestForm::initStyleCombo: no option or widget argument.
                if (style->standardIcon(QStyle::StandardPixmap(value))
                        .isNull()) {
                    missing.append(
                        QString::fromLatin1(roles.valueToKey(value)));
                }
            }
            return missing;
        };
        QWidget widget;
        for (const auto scheme :
             {ThemeManager::Theme::Light, ThemeManager::Theme::Dark}) {
            theme.setTheme(scheme);
#ifdef Q_OS_WIN
            const auto styleName = QStringLiteral("windowsvista");
#else
            const auto styleName = QStringLiteral("fusion");
#endif
            const std::unique_ptr<QStyle> native(
                QStyleFactory::create(styleName));
            QVERIFY(native);
            QWidget reference;
            reference.setStyle(native.get());
            const auto prefix = scheme == ThemeManager::Theme::Dark
                                    ? QStringLiteral(":/dark/")
                                    : QStringLiteral(":/light/");
            QFile source(prefix + QStringLiteral("stylesheet.qss"));
            QVERIFY(source.open(QFile::ReadOnly));
            reference.setStyleSheet(QString::fromUtf8(source.readAll()));
            const auto nativeMissing = missingIcons(reference.style());
            const auto themedMissing = missingIcons(widget.style());
            qInfo() << "original QSS missing:" << nativeMissing;
            qInfo() << "ThemeProxyStyle missing:" << themedMissing;
            QVERIFY2(themedMissing.isEmpty(),
                     qPrintable(themedMissing.join(QStringLiteral(", "))));
            auto style = widget.style();
            for (const auto value : values) {
                QVERIFY2(!style->standardPixmap(QStyle::StandardPixmap(value))
                              .isNull(),
                         roles.valueToKey(value));
            }
        }
    }

    void qtStandardIcons_data() {
        QTest::addColumn<int>("role");
        const auto roles = QMetaEnum::fromType<QStyle::StandardPixmap>();
        for (int i = 0; i < roles.keyCount(); ++i) {
            const auto value = roles.value(i);
            if (value < 0 || QByteArray(roles.key(i)) == "NStandardPixmap") {
                continue;
            }
            QTest::newRow(roles.key(i)) << value;
        }
    }

    void qtStandardIcons() {
        QFETCH(int, role);
        auto &theme = ThemeManager::instance();
        QWidget widget;
        for (auto scheme :
             {ThemeManager::Theme::Light, ThemeManager::Theme::Dark}) {
            theme.setTheme(scheme);
            const auto optimized = theme.currentStyleSheet();
            const auto prefix = scheme == ThemeManager::Theme::Dark
                                    ? QStringLiteral(":/dark/")
                                    : QStringLiteral(":/light/");
            QFile source(prefix + QStringLiteral("stylesheet.qss"));
            QVERIFY(source.open(QFile::ReadOnly));
            qApp->setStyleSheet(QString::fromUtf8(source.readAll()));
            const auto expectedIcon =
                widget.style()
                    ->standardIcon(QStyle::StandardPixmap(role), nullptr,
                                   &widget)
                    .pixmap(24, 24)
                    .toImage();
            const auto expectedPixmap =
                widget.style()
                    ->standardPixmap(QStyle::StandardPixmap(role), nullptr,
                                     &widget)
                    .toImage();
            qApp->setStyleSheet(optimized);
            QCOMPARE(widget.style()
                         ->standardIcon(QStyle::StandardPixmap(role), nullptr,
                                        &widget)
                         .pixmap(24, 24)
                         .toImage(),
                     expectedIcon);
            QCOMPARE(widget.style()
                         ->standardPixmap(QStyle::StandardPixmap(role), nullptr,
                                          &widget)
                         .toImage(),
                     expectedPixmap);
        }
    }

    void resourceStandardIcons_data() {
        QTest::addColumn<int>("role");
        QFile source(QStringLiteral(":/light/stylesheet.qss"));
        QVERIFY(source.open(QFile::ReadOnly));
        auto sheet = QString::fromUtf8(source.readAll());
        const auto paths = ThemeStyleSheet::extractStandardIcons(sheet);
        const auto roles = QMetaEnum::fromType<QStyle::StandardPixmap>();
        for (int i = 0; i < roles.keyCount(); ++i) {
            const auto value = roles.value(i);
            if (value < 0 || QByteArray(roles.key(i)) == "NStandardPixmap" ||
                paths.contains(value)) {
                continue;
            }
            QTest::newRow(roles.key(i)) << value;
        }
    }

    void resourceStandardIcons() {
        QFETCH(int, role);
        auto &theme = ThemeManager::instance();
        QWidget widget;
        for (auto scheme :
             {ThemeManager::Theme::Light, ThemeManager::Theme::Dark}) {
            theme.setTheme(scheme);
            const auto prefix = scheme == ThemeManager::Theme::Dark
                                    ? QStringLiteral(":/dark/standard-icons/")
                                    : QStringLiteral(":/light/standard-icons/");
            const auto key =
                QMetaEnum::fromType<QStyle::StandardPixmap>().valueToKey(role);
            const auto path =
                prefix + QString::fromLatin1(key) + QStringLiteral(".svg");
            QVERIFY2(QFile::exists(path), qPrintable(path));
            const QIcon expected(path);
            const auto image = expected.pixmap(24, 24).toImage();
            QVERIFY(!image.isNull());
            QCOMPARE(widget.style()
                         ->standardIcon(QStyle::StandardPixmap(role))
                         .pixmap(24, 24)
                         .toImage(),
                     image);
            const auto size =
                widget.style()->pixelMetric(QStyle::PM_SmallIconSize);
            QCOMPARE(widget.style()
                         ->standardPixmap(QStyle::StandardPixmap(role))
                         .toImage(),
                     expected.pixmap(size, size).toImage());
        }
    }

    void standardIconExtractionPreservesOtherRules() {
        const auto commented = QStringLiteral(
            "/* QWidget { dialog-ok-icon: url(:/light/stop.svg); } */");
        const auto scoped = QStringLiteral(
            "QWidget:hover { dialog-ok-icon: url(:/light/pause.svg); }");
        const auto grouped = QStringLiteral(
            "QWidget, QDialog { dialog-ok-icon: url(:/light/stop.svg); }");
        const auto unknown =
            QStringLiteral(" unknown-icon: url(:/light/play.svg);"
                           " dialog-cancel-icon: url(:/missing.svg);"
                           " dialog-help-icon: none;"
                           " qproperty-text: \"} QWidget { ignored; }\";");
        auto sheet =
            commented +
            QStringLiteral(" QWidget { color: red; dialog-ok-icon: "
                           "url(:/light/dialog-ok.svg); }") +
            scoped + grouped +
            QStringLiteral(
                " QWidget { dialog-ok-icon: url(\":/light/play.svg\");") +
            unknown + QLatin1Char('}');
        const auto icons = ThemeStyleSheet::extractStandardIcons(sheet);
        QCOMPARE(icons.size(), 1);
        QCOMPARE(icons.value(QStyle::SP_DialogOkButton),
                 QStringLiteral(":/light/play.svg"));
        QVERIFY(sheet.contains(commented));
        QVERIFY(sheet.contains(scoped));
        QVERIFY(sheet.contains(grouped));
        QVERIFY(sheet.contains(unknown));
        QVERIFY(sheet.contains(QStringLiteral("color: red;")));
    }

    void standardIconQssCascade() {
        auto &theme = ThemeManager::instance();
        for (auto scheme :
             {ThemeManager::Theme::Light, ThemeManager::Theme::Dark}) {
            theme.setTheme(scheme);
            const auto prefix = scheme == ThemeManager::Theme::Dark
                                    ? QStringLiteral(":/dark/")
                                    : QStringLiteral(":/light/");
            QFile source(prefix + QStringLiteral("stylesheet.qss"));
            QVERIFY(source.open(QFile::ReadOnly));
            const auto original = QString::fromUtf8(source.readAll());
            auto optimized = original;
            const auto paths = ThemeStyleSheet::extractStandardIcons(optimized);
            QCOMPARE(paths.size(), 54);
            QCOMPARE(optimized, theme.currentStyleSheet());
            QVERIFY(optimized.contains(
                QStringLiteral("titlebar-close-icon: url(") + prefix +
                QStringLiteral("transparent.svg)")));
            QWidget generic;
            generic.setStyleSheet(
                QStringLiteral("QWidget { icon-size: 32px; }"));
            QDockWidget dock;
            QWidget custom;
            custom.setStyleSheet(
                QStringLiteral("QWidget { dialog-ok-icon: url(") + prefix +
                QStringLiteral("play.svg); }"));
            QDialogButtonBox box(QDialogButtonBox::Ok |
                                 QDialogButtonBox::Cancel);
            auto capture = [&] {
                QList<QImage> images;
                for (auto widget : {&generic, static_cast<QWidget *>(&dock),
                                    &custom, static_cast<QWidget *>(&box)}) {
                    for (auto role : {QStyle::SP_DialogOkButton,
                                      QStyle::SP_TitleBarCloseButton,
                                      QStyle::SP_DockWidgetCloseButton}) {
                        images.append(widget->style()
                                          ->standardIcon(role, nullptr, widget)
                                          .pixmap(24, 24)
                                          .toImage());
                        images.append(
                            widget->style()
                                ->standardPixmap(role, nullptr, widget)
                                .toImage());
                    }
                }
                box.ensurePolished();
                images.append(box.button(QDialogButtonBox::Ok)
                                  ->icon()
                                  .pixmap(24, 24)
                                  .toImage());
                return images;
            };
            qApp->setStyleSheet(original);
            const auto before = capture();
            qApp->setStyleSheet(optimized);
            QCOMPARE(capture(), before);
            QCOMPARE(
                custom.style()
                    ->standardIcon(QStyle::SP_DialogOkButton, nullptr, &custom)
                    .pixmap(24, 24)
                    .toImage(),
                QIcon(prefix + QStringLiteral("play.svg"))
                    .pixmap(24, 24)
                    .toImage());
        }
    }

    void startupDurationHistory() {
        using namespace StartupFeedback;
        QCOMPARE(estimate({}), UnknownDurationMs);
        QCOMPARE(estimate({-1, 0, QStringLiteral("invalid"), 60001}),
                 UnknownDurationMs);
        QCOMPARE(estimate({400, 390, 410, 20000, 380}), qint64(400));
        QCOMPARE(estimate({100, 200}), qint64(150));
        const auto history = remember({10, 20, 30, 40, 50}, 60);
        QCOMPARE(history.size(), HistoryLimit);
        QCOMPARE(history.first().toLongLong(), qint64(20));
        QCOMPARE(history.last().toLongLong(), qint64(60));
        QCOMPARE(remember(history, -1), history);
    }

    void automaticSplashFastStartupStaysHidden() {
        auto splash =
            new SplashDialog(SplashDialog::DisplayMode::Automatic, 70);
        QPointer<SplashDialog> alive(splash);
        QVERIFY(!splash->isVisible());
        QVERIFY(splash->styleSheet().isEmpty());
        splash->setInfoText(QStringLiteral("Fast startup"));
        splash->prepareForBlockingPhase();
        QVERIFY(!splash->isVisible());
        splash->finish();
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(alive.isNull());
        QTest::qWait(StartupFeedback::ShowDelayMs + 30);
        // No queued timer may resurrect a completed startup splash.
        for (auto widget : QApplication::topLevelWidgets()) {
            QVERIFY(!qobject_cast<SplashDialog *>(widget));
        }
    }

    void automaticSplashShowsOnDeadline() {
        auto splash =
            new SplashDialog(SplashDialog::DisplayMode::Automatic, 70);
        QVERIFY(!splash->isVisible());
        QTRY_VERIFY_WITH_TIMEOUT(splash->isVisible(), 1000);
        QVERIFY(!splash->styleSheet().isEmpty());
        splash->finish();
        QVERIFY(!splash->isVisible());
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void automaticSplashShowsBeforePredictedSlowPhase() {
        auto splash =
            new SplashDialog(SplashDialog::DisplayMode::Automatic, 400);
        QVERIFY(!splash->isVisible());
        splash->prepareForBlockingPhase();
        QVERIFY(splash->isVisible());
        splash->finish();
        // Completion must not add any artificial minimum display duration.
        QVERIFY(!splash->isVisible());
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void automaticSplashCloseCancelsPendingShow() {
        auto splash =
            new SplashDialog(SplashDialog::DisplayMode::Automatic, 70);
        QPointer<SplashDialog> alive(splash);
        splash->close();
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(alive.isNull());
        QTest::qWait(StartupFeedback::ShowDelayMs + 30);
    }

    void splashBeforeGlobalStyleSheet_data() {
        QTest::addColumn<bool>("dark");
        QTest::newRow("light") << false;
        QTest::newRow("dark") << true;
    }

    void splashBeforeGlobalStyleSheet() {
        QFETCH(bool, dark);
        auto &theme = ThemeManager::instance();
        theme.setStyleSheetDeferred(false);
        theme.setTheme(dark ? ThemeManager::Theme::Light
                            : ThemeManager::Theme::Dark);
        qApp->setStyleSheet(QString());
        theme.setStyleSheetDeferred(true);
        theme.setTheme(dark ? ThemeManager::Theme::Dark
                            : ThemeManager::Theme::Light);
        QVERIFY(qApp->styleSheet().isEmpty());
        const auto pending = theme.currentStyleSheet();
        QVERIFY(!pending.isEmpty());
        auto splash = new SplashDialog;
        QPointer<SplashDialog> alive(splash);
        auto progress =
            splash->findChild<QProgressBar *>(QStringLiteral("progressBar"));
        QVERIFY(progress);
        // Stabilize the busy animation for pixel-for-pixel comparison.
        progress->setRange(0, 100);
        progress->setValue(50);
        splash->setInfoText(QStringLiteral("Initializing interface"));
        QCOMPARE(splash->styleSheet(), pending);
        QVERIFY(splash->isVisible());
        auto label = splash->findChild<QLabel *>(QStringLiteral("lblinfo"));
        QVERIFY(label);
        QCOMPARE(label->text(), QStringLiteral("Initializing interface..."));
        QApplication::processEvents();
        const auto early = splash->grab().toImage();
        QVERIFY(!early.isNull());
        if (qEnvironmentVariableIsSet("WING_SPLASH_PREVIEWS")) {
            QDir output(qEnvironmentVariable("WING_SPLASH_PREVIEWS"));
            QVERIFY(QDir().mkpath(output.absolutePath()));
            QVERIFY(early.save(
                output.filePath(dark ? QStringLiteral("dark.png")
                                     : QStringLiteral("light.png"))));
        }
        theme.setStyleSheetDeferred(false);
        QApplication::processEvents();
        QCOMPARE(qApp->styleSheet(), pending);
        QCOMPARE(splash->grab().toImage(), early);
        // Existing splash windows must also follow subsequent theme changes.
        theme.setStyleSheetDeferred(true);
        theme.setTheme(dark ? ThemeManager::Theme::Light
                            : ThemeManager::Theme::Dark);
        QCOMPARE(splash->styleSheet(), theme.currentStyleSheet());
        QVERIFY(splash->styleSheet() != pending);
        theme.setStyleSheetDeferred(false);
        splash->close();
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(alive.isNull());
    }

    void startupStyleSheetBatch() {
        auto &theme = ThemeManager::instance();
        theme.setStyleSheetDeferred(false);
        theme.setTheme(ThemeManager::Theme::Light);
        const auto before = qApp->styleSheet();
        const auto icon = theme.standardIcon(Icon::Undo);
        theme.setStyleSheetDeferred(true);
        theme.setTheme(ThemeManager::Theme::Dark);
        QCOMPARE(theme.currentTheme(), ThemeManager::Theme::Dark);
        QCOMPARE(qApp->styleSheet(), before);
        const auto beforeFlush = icon.pixmap(24, 24);
        const auto revision = theme.iconCacheRevision();
        theme.setStyleSheetDeferred(false);
        QVERIFY(qApp->styleSheet() != before);
        QVERIFY(theme.iconCacheRevision() > revision);
        const auto afterFlush = icon.pixmap(24, 24);
        QVERIFY(beforeFlush.cacheKey() != afterFlush.cacheKey());
        QCOMPARE(beforeFlush.toImage(), afterFlush.toImage());
        const auto flushedRevision = theme.iconCacheRevision();
        theme.setStyleSheetDeferred(false);
        QCOMPARE(theme.iconCacheRevision(), flushedRevision);
    }

    void repeatedRasterizationAndEviction() {
        auto &theme = ThemeManager::instance();
        const auto icon = theme.standardIcon(Icon::Undo);
        QPixmapCache::clear();
        const auto first = icon.pixmap(24, 24);
        const auto repeated = icon.pixmap(24, 24);
        QCOMPARE(first.cacheKey(), repeated.cacheKey());
        const auto otherSize = icon.pixmap(32, 32);
        QVERIFY(otherSize.cacheKey() != first.cacheKey());
        QCOMPARE(icon.pixmap(24, 24).cacheKey(), first.cacheKey());
        const QIcon copied = icon;
        QCOMPARE(copied.pixmap(24, 24).cacheKey(), first.cacheKey());
        QPixmapCache::clear();
        const auto regenerated = icon.pixmap(24, 24);
        QVERIFY(regenerated.cacheKey() != first.cacheKey());
        QCOMPARE(regenerated.toImage(), first.toImage());
        const auto originalPalette = QApplication::palette();
        auto changed = originalPalette;
        changed.setColor(QPalette::Disabled, QPalette::WindowText, Qt::red);
        const auto disabled = icon.pixmap(24, 24, QIcon::Disabled);
        QApplication::setPalette(changed);
        const auto recolored = icon.pixmap(24, 24, QIcon::Disabled);
        QVERIFY(recolored.cacheKey() != disabled.cacheKey());
        QApplication::setPalette(originalPalette);
    }

    void catalogAndModes() {
        auto &theme = ThemeManager::instance();
        const auto meta = QMetaEnum::fromType<Icon>();
        QCOMPARE(theme.standardIcons().size(), meta.keyCount() - 1);
        for (auto scheme :
             {ThemeManager::Theme::Dark, ThemeManager::Theme::Light}) {
            theme.setTheme(scheme);
            for (const auto id : theme.standardIcons()) {
                const auto path = theme.iconResource(id);
                QVERIFY2(QFile::exists(path),
                         meta.valueToKey(static_cast<int>(id)));
                QVERIFY(path.endsWith(QStringLiteral(".svg")));
                const auto data = theme.standardIconData(id);
                QVERIFY2(!data.isEmpty(), qPrintable(path));
                QVERIFY(!data.contains("<image"));
                if (path.endsWith(QStringLiteral(".svg"))) {
                    QDomDocument doc;
                    QVERIFY(doc.setContent(data));
                    QCOMPARE(
                        doc.elementsByTagName(QStringLiteral("svg")).size(), 1);
                    QSvgRenderer renderer(data);
                    QVERIFY2(renderer.isValid(), qPrintable(path));
                }
                const auto icon = theme.standardIcon(id);
                QVERIFY2(!icon.isNull(), qPrintable(path));
                for (int size : {16, 24, 32, 48, 64}) {
                    for (qreal ratio : {1.0, 1.25, 1.5, 2.0}) {
                        for (auto mode : {QIcon::Normal, QIcon::Disabled,
                                          QIcon::Active, QIcon::Selected}) {
                            const auto pixmap =
                                icon.pixmap(QSize(size, size), ratio, mode);
                            QVERIFY2(!pixmap.isNull(), qPrintable(path));
                            QCOMPARE(pixmap.size(), QSize(size, size) * ratio);
                            QCOMPARE(pixmap.devicePixelRatio(), ratio);
                        }
                    }
                }
            }
        }
    }

    void existingIconsFollowTheme() {
        auto &theme = ThemeManager::instance();
        const auto icon = theme.standardIcon(Icon::Undo);
        const auto darkData = theme.standardIconData(Icon::Undo);
        const auto dark = icon.pixmap(64, 64).toImage();
        QSignalSpy spy(&theme, &ThemeManager::themeChanged);
        theme.setTheme(ThemeManager::Theme::Light);
        QCOMPARE(spy.count(), 1);
        const auto light = icon.pixmap(64, 64).toImage();
        QVERIFY(darkData != theme.standardIconData(Icon::Undo));
        QVERIFY(dark != light);
        QCOMPARE(light,
                 theme.standardIcon(Icon::Undo).pixmap(64, 64).toImage());
        theme.setTheme(ThemeManager::Theme::Light);
        QCOMPARE(spy.count(), 1);
        theme.setTheme(static_cast<ThemeManager::Theme>(99));
        QCOMPARE(theme.currentTheme(), ThemeManager::Theme::Light);
        QCOMPARE(spy.count(), 1);
        theme.setTheme(ThemeManager::Theme::Dark);
        QCOMPARE(icon.pixmap(64, 64).toImage(), dark);
    }

    void primaryColorsHaveContrast() {
        auto &theme = ThemeManager::instance();
        for (auto scheme :
             {ThemeManager::Theme::Dark, ThemeManager::Theme::Light}) {
            theme.setTheme(scheme);
            const auto background = scheme == ThemeManager::Theme::Dark
                                        ? QColor(QStringLiteral("#202326"))
                                        : QColor(QStringLiteral("#f5f5f5"));
            for (auto id : {Icon::Undo, Icon::GoTo, Icon::New}) {
                QDomDocument doc;
                QVERIFY(doc.setContent(theme.standardIconData(id)));
                const auto paths =
                    doc.elementsByTagName(QStringLiteral("path"));
                bool checked = false;
                for (int i = 0; i < paths.size(); ++i) {
                    const auto element = paths.at(i).toElement();
                    for (const auto &attribute :
                         {QStringLiteral("fill"), QStringLiteral("stroke")}) {
                        const auto value = element.attribute(attribute);
                        const QColor color(value);
                        if (!color.isValid() ||
                            value == QStringLiteral("none")) {
                            continue;
                        }
                        const double contrast =
                            contrastRatio(color, background);
                        QVERIFY2(contrast >= 3.0, qPrintable(value));
                        checked = true;
                    }
                }
                QVERIFY(checked);
            }
        }
    }

    void buttonStateIconsKeepContrast() {
        auto &theme = ThemeManager::instance();
        for (auto scheme :
             {ThemeManager::Theme::Dark, ThemeManager::Theme::Light}) {
            theme.setTheme(scheme == ThemeManager::Theme::Dark
                               ? ThemeManager::Theme::Light
                               : ThemeManager::Theme::Dark);
            theme.setTheme(scheme);
            for (auto checked : {true, false}) {
                QToolButton plain;
                QToolButton withIcon;
                for (auto *button : {&plain, &withIcon}) {
                    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
                    button->setCheckable(true);
                    button->setFocusPolicy(Qt::NoFocus);
                    button->resize(40, 40);
                    button->show();
                }
                if (checked) {
                    plain.setChecked(true);
                    withIcon.setChecked(true);
                } else {
                    plain.setDown(true);
                    withIcon.setDown(true);
                }
                withIcon.setIcon(theme.standardIcon(Icon::CompareHorizontal));
                QApplication::processEvents();

                const auto background = plain.grab().toImage();
                const auto foreground = withIcon.grab().toImage();
                QCOMPARE(foreground.size(), background.size());

                bool iconPainted = false;
                double maximumContrast = 0.0;
                for (int y = 0; y < foreground.height(); ++y) {
                    for (int x = 0; x < foreground.width(); ++x) {
                        const auto before = background.pixelColor(x, y);
                        const auto after = foreground.pixelColor(x, y);
                        if (before == after) {
                            continue;
                        }
                        iconPainted = true;
                        maximumContrast =
                            qMax(maximumContrast, contrastRatio(before, after));
                    }
                }
                QVERIFY(iconPainted);
                QVERIFY2(
                    maximumContrast >= 3.0,
                    qPrintable(QStringLiteral("%1 contrast on %2")
                                   .arg(maximumContrast)
                                   .arg(checked ? QStringLiteral("checked")
                                                : QStringLiteral("pressed"))));
                plain.close();
                withIcon.close();
            }
        }
    }

    void rejectUnknownResources() {
        auto &theme = ThemeManager::instance();
        for (auto id :
             {Icon::None, static_cast<Icon>(-1), static_cast<Icon>(999999)}) {
            QVERIFY(theme.standardIcon(id).isNull());
            QVERIFY(theme.standardIconData(id).isEmpty());
            QVERIFY(theme.iconResource(id).isEmpty());
        }
    }

    void sdkUsesHostTheme() {
        ThemeEndpoint endpoint;
        ThemeClient client(endpoint);
        auto &theme = ThemeManager::instance();
        const auto icon = client.standardIcon(Icon::OpenProject);
        QVERIFY(!icon.isNull());
        for (auto scheme :
             {ThemeManager::Theme::Light, ThemeManager::Theme::Dark}) {
            theme.setTheme(scheme);
            QCOMPARE(client.standardPixmap(Icon::OpenProject).toImage(),
                     theme.standardPixmap(Icon::OpenProject).toImage());
            QCOMPARE(client.standardIconData(Icon::OpenProject),
                     theme.standardIconData(Icon::OpenProject));
            QCOMPARE(
                icon.pixmap(48, 48).toImage(),
                theme.standardIcon(Icon::OpenProject).pixmap(48, 48).toImage());
            QCOMPARE(client.currentAppTheme(),
                     scheme == ThemeManager::Theme::Dark
                         ? WingHex::AppTheme::Dark
                         : WingHex::AppTheme::Light);
        }
        QVERIFY(client.standardIcon(static_cast<Icon>(999999)).isNull());
        QVERIFY(client.standardIconData(Icon::None).isEmpty());
    }
};

QTEST_MAIN(ThemeTests)
#include "tst_thememanager.moc"
