#include "class/settings/thememanager.h"

#include "WingPlugin/iwingpluginbasecalls.h"
#include "WingPlugin/wingcore.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QPainter>
#include <QSignalSpy>
#include <QStyleHints>
#include <QSvgRenderer>
#include <QtTest>
#include <cmath>

using Icon = WingHex::StandardIcon;

class ThemeEndpoint : public QObject {
    Q_OBJECT
public slots:
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
        const auto luminance = [](QColor color) {
            auto linear = [](double c) {
                return c <= 0.04045 ? c / 12.92
                                    : std::pow((c + 0.055) / 1.055, 2.4);
            };
            return 0.2126 * linear(color.redF()) +
                   0.7152 * linear(color.greenF()) +
                   0.0722 * linear(color.blueF());
        };
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
                        const double a = luminance(color),
                                     b = luminance(background);
                        const double contrast =
                            (qMax(a, b) + 0.05) / (qMin(a, b) + 0.05);
                        QVERIFY2(contrast >= 3.0, qPrintable(value));
                        checked = true;
                    }
                }
                QVERIFY(checked);
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
