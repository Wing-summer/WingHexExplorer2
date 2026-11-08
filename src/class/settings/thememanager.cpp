/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "thememanager.h"

#include <cmath>

#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QIconEngine>
#include <QMetaEnum>
#include <QPainter>
#include <QPixmapCache>
#include <QProxyStyle>
#include <QRegularExpression>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <QStyleOption>
#include <QSvgRenderer>

#include "themestylesheet.h"

namespace {
// Standard icons are queried on demand rather than resolved for every QWidget
// while applying QSS. QSS defaults override enum-named resource aliases.
class ThemeProxyStyle final : public QProxyStyle {
public:
    explicit ThemeProxyStyle(QStyle *base) : QProxyStyle(base) {}

    QIcon standardIcon(StandardPixmap role, const QStyleOption *option,
                       const QWidget *widget) const override {
        const auto icon = themedIcon(role);
        return icon.isNull() ? QProxyStyle::standardIcon(role, option, widget)
                             : icon;
    }

    QPixmap standardPixmap(StandardPixmap role, const QStyleOption *option,
                           const QWidget *widget) const override {
        const auto size =
            QProxyStyle::pixelMetric(PM_SmallIconSize, option, widget);
        auto pixmap = themedIcon(role).pixmap(size, size);
        if (!pixmap.isNull()) {
            return pixmap;
        }
        return QProxyStyle::standardPixmap(role, option, widget);
    }

    void setIcons(const QHash<int, QString> &paths, const QString &iconPrefix) {
        m_paths = paths;
        m_iconPrefix = iconPrefix;
        m_icons.clear();
    }

private:
    QIcon themedIcon(StandardPixmap role) const {
        const auto found = m_icons.constFind(int(role));
        if (found != m_icons.cend()) {
            return found.value();
        }
        auto path = m_paths.value(int(role));
        if (path.isEmpty()) {
            // QSS cannot name every Qt standard role; QRC aliases cover the
            // rest.
            const auto roles = QMetaEnum::fromType<QStyle::StandardPixmap>();
            const auto key = roles.valueToKey(int(role));
            if (!key) {
                return {};
            }
            path = m_iconPrefix + QString::fromLatin1(key) +
                   QStringLiteral(".svg");
            if (!QFile::exists(path)) {
                return {};
            }
        }
        const QIcon icon(path);
        m_icons.insert(int(role), icon);
        return icon;
    }

    QHash<int, QString> m_paths;
    QString m_iconPrefix;
    mutable QHash<int, QIcon> m_icons;
};

using StandardIcon = ThemeManager::StandardIcon;

QString resourceName(StandardIcon icon) {
    const auto meta = QMetaEnum::fromType<StandardIcon>();
    const auto key = meta.valueToKey(int(icon));
    if (!key || icon == StandardIcon::None) {
        return {};
    }
    QString name = QString::fromLatin1(key);
    static QRegularExpression nameNormalize(
        QStringLiteral("(?|([A-Z]+)([A-Z][a-z])|([a-z0-9])([A-Z]))"));
    name.replace(nameNormalize, QStringLiteral("\\1-\\2"));
    return name.toLower();
}

QString roleColor(const QString &role, ThemeManager::Theme theme) {
    const bool dark = theme == ThemeManager::Theme::Dark;
    if (role == QStringLiteral("foreground")) {
        return dark ? QStringLiteral("#eff0f1") : QStringLiteral("#31363b");
    }
    if (role == QStringLiteral("muted")) {
        return dark ? QStringLiteral("#a8b0b6") : QStringLiteral("#606a73");
    }
    if (role == QStringLiteral("accent")) {
        return dark ? QStringLiteral("#3daee9") : QStringLiteral("#0075ad");
    }
    if (role == QStringLiteral("gold")) {
        return dark ? QStringLiteral("#e6c37c") : QStringLiteral("#94620b");
    }
    if (role == QStringLiteral("success")) {
        return dark ? QStringLiteral("#81ce7d") : QStringLiteral("#25803b");
    }
    if (role == QStringLiteral("danger")) {
        return dark ? QStringLiteral("#ff8175") : QStringLiteral("#be2d29");
    }
    if (role == QStringLiteral("purple")) {
        return dark ? QStringLiteral("#b295d6") : QStringLiteral("#704599");
    }
    return {};
}

QColor contrastColor(QColor color, ThemeManager::Theme theme) {
    const auto luminance = [](QColor value) {
        auto linear = [](qreal c) {
            return c <= 0.04045 ? c / 12.92
                                : std::pow((c + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * linear(value.redF()) + 0.7152 * linear(value.greenF()) +
               0.0722 * linear(value.blueF());
    };
    const QColor background(theme == ThemeManager::Theme::Dark
                                ? QStringLiteral("#202326")
                                : QStringLiteral("#eff0f1"));
    const auto back = luminance(background);
    for (int i = 0; i < 100; ++i) {
        const auto value = luminance(color);
        if ((qMax(back, value) + 0.05) / (qMin(back, value) + 0.05) >= 3.2) {
            break;
        }
        auto hsl = color.toHsl();
        const auto lightness =
            qBound(0.0,
                   hsl.lightnessF() +
                       (theme == ThemeManager::Theme::Dark ? 0.01 : -0.01),
                   1.0);
        color.setHslF(hsl.hslHueF(), hsl.hslSaturationF(), lightness,
                      color.alphaF());
    }
    return color;
}

void applyRoles(QDomElement element, ThemeManager::Theme theme) {
    for (const auto &property :
         {QStringLiteral("fill"), QStringLiteral("stroke")}) {
        const QString attribute = QStringLiteral("data-theme-") + property;
        const auto role = element.attribute(attribute);
        const auto color = roleColor(role, theme);
        if (role == QStringLiteral("auto")) {
            const QColor original(element.attribute(property));
            if (original.isValid()) {
                element.setAttribute(property,
                                     contrastColor(original, theme).name());
            }
        } else if (!color.isEmpty()) {
            element.setAttribute(property, color);
        }
        element.removeAttribute(attribute);
    }
    for (auto child = element.firstChildElement(); !child.isNull();
         child = child.nextSiblingElement()) {
        applyRoles(child, theme);
    }
}

QByteArray themedResourceData(const QString &path, ThemeManager::Theme theme) {
    if (path.isEmpty()) {
        return {};
    }
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        return {};
    }
    const auto data = file.readAll();
    if (!data.contains("data-theme-")) {
        return data;
    }
    QDomDocument document;
    if (!document.setContent(data)) {
        return {};
    }
    applyRoles(document.documentElement(), theme);
    return document.toByteArray(0);
}

class ThemeIconEngine final : public QIconEngine {
public:
    explicit ThemeIconEngine(StandardIcon icon) : m_icon(icon) {}

    explicit ThemeIconEngine(const QString &path) : m_path(path) {}

    QIconEngine *clone() const override { return new ThemeIconEngine(*this); }

    QString key() const override { return QStringLiteral("WingThemeIcon"); }

    QString iconName() override {
        return m_path.isEmpty() ? resourceName(m_icon) : m_path;
    }

    bool isNull() override {
        return m_path.isEmpty()
                   ? ThemeManager::instance().iconResource(m_icon).isEmpty()
                   : !QFile::exists(m_path);
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state) override {
        return render(size, mode, state, 1.0);
    }

    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode,
                         QIcon::State state, qreal scale) override {
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
        // QIcon passed physical pixels before Qt 6.8.
        return render(size, mode, state, scale);
#else
        return render(size * scale, mode, state, scale);
#endif
    }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode,
               QIcon::State state) override {
        const qreal ratio = painter->device()->devicePixelRatioF();
        const auto image = render(rect.size() * ratio, mode, state, ratio);
        painter->drawPixmap(rect, image);
    }

private:
    QPixmap render(const QSize &size, QIcon::Mode mode, QIcon::State state,
                   qreal ratio) const {
        if (size.isEmpty() || ratio <= 0) {
            return {};
        }
        auto &theme = ThemeManager::instance();
        // QIconEngine does not cache custom SVG rasterizations. Use Qt's
        // bounded GUI cache so copies and repeated sizes share their pixels.
        // Palette/style also affect disabled and selected icon generation.
        const auto palette = QApplication::palette();
        auto *style = QApplication::style();
        QString cacheKey = QStringLiteral("wing-theme-icon/");
        cacheKey += m_path.isEmpty() ? QString::number(int(m_icon)) : m_path;
        for (const auto &part :
             {QString::number(int(theme.currentTheme())),
              QString::number(theme.iconCacheRevision()),
              QString::number(size.width()), QString::number(size.height()),
              QString::number(int(mode)), QString::number(int(state)),
              QString::number(ratio, 'g', 17),
              QString::number(palette.cacheKey()),
              QString::number(quintptr(style), 16)}) {
            cacheKey += QLatin1Char('|');
            cacheKey += part;
        }
        QPixmap cached;
        if (QPixmapCache::find(cacheKey, &cached)) {
            return cached;
        }
        const auto data = m_path.isEmpty() ? theme.standardIconData(m_icon)
                                           : theme.resourceIconData(m_path);
        if (data.isEmpty()) {
            return {};
        }
        QPixmap image(size);
        image.fill(Qt::transparent);
        QSvgRenderer renderer(data);
        if (!renderer.isValid()) {
            return {};
        }
        renderer.setAspectRatioMode(Qt::KeepAspectRatio);
        {
            QPainter painter(&image);
            renderer.render(&painter, QRectF(QPointF(), QSizeF(size)));
        }
        image.setDevicePixelRatio(ratio);
        if (mode == QIcon::Disabled || mode == QIcon::Selected) {
            QStyleOption option;
            option.palette = palette;
            image = style->generatedIconPixmap(mode, image, &option);
            image.setDevicePixelRatio(ratio);
        }
        QPixmapCache::insert(cacheKey, image);
        return image;
    }

    StandardIcon m_icon = StandardIcon::None;
    QString m_path;
};
} // namespace

ThemeManager::ThemeManager() {
    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme scheme) {
                if (m_requestedTheme == Theme::System) {
                    applyTheme(scheme == Qt::ColorScheme::Dark ? Theme::Dark
                                                               : Theme::Light);
                }
            });
}

ThemeManager::~ThemeManager() = default;

ThemeManager &ThemeManager::instance() {
    static ThemeManager manager;
    return manager;
}

void ThemeManager::setTheme(Theme theme) {
    if (theme != Theme::System && theme != Theme::Dark &&
        theme != Theme::Light) {
        return;
    }
    m_requestedTheme = theme;
    if (theme == Theme::System) {
        theme = qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark
                    ? Theme::Dark
                    : Theme::Light;
    }
    applyTheme(theme);
}

void ThemeManager::applyTheme(Theme theme) {
    if (m_initialized && m_theme == theme) {
        return;
    }
    const QString directory =
        theme == Theme::Dark ? QStringLiteral("dark") : QStringLiteral("light");
    QFile sheet(QStringLiteral(":/") + directory +
                QStringLiteral("/stylesheet.qss"));
    if (!sheet.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("Cannot load the application theme stylesheet");
        return;
    }
    auto styleSheet = QString::fromUtf8(sheet.readAll());
    const auto icons = ThemeStyleSheet::extractStandardIcons(styleSheet);
    const auto iconPrefix =
        QStringLiteral(":/") + directory + QStringLiteral("/standard-icons/");
    auto *proxy = static_cast<ThemeProxyStyle *>(m_themeStyle.data());
    if (!proxy) {
#ifdef Q_OS_WIN
        proxy = new ThemeProxyStyle(
            QStyleFactory::create(QStringLiteral("windowsvista")));
#else
        proxy = new ThemeProxyStyle(
            QStyleFactory::create(QStringLiteral("fusion")));
#endif
        proxy->setIcons(icons, iconPrefix);
        m_themeStyle = proxy;
        qApp->setStyle(proxy);
    } else {
        proxy->setIcons(icons, iconPrefix);
    }
    m_theme = theme;
    ++m_iconCacheRevision;
    m_initialized = true;
    m_icons.clear();
    m_data.clear();
    m_pixmaps.clear();
    m_resourceData.clear();

    auto palette = qApp->style()->standardPalette();
    if (theme == Theme::Dark) {
        palette.setColor(QPalette::Window, QColor(QStringLiteral("#202326")));
        palette.setColor(QPalette::Base, QColor(QStringLiteral("#1b1e20")));
        palette.setColor(QPalette::Button, QColor(QStringLiteral("#31363b")));
        palette.setColor(QPalette::WindowText,
                         QColor(QStringLiteral("#eff0f1")));
        palette.setColor(QPalette::Text, QColor(QStringLiteral("#eff0f1")));
        palette.setColor(QPalette::ButtonText,
                         QColor(QStringLiteral("#eff0f1")));
        palette.setColor(QPalette::Link, QColor::fromRgb(88, 147, 248));
    }
    qApp->setPalette(palette);
    if (m_styleSheetDeferred) {
        m_pendingStyleSheet = styleSheet;
        m_styleSheetPending = true;
    } else {
        qApp->setStyleSheet(styleSheet);
    }
    Q_EMIT sigOnThemeChanged(theme);
    for (auto &widget : QApplication::allWidgets()) {
        widget->update();
    }
}

void ThemeManager::setStyleSheetDeferred(bool deferred) {
    m_styleSheetDeferred = deferred;
    if (!deferred && m_styleSheetPending) {
        m_styleSheetPending = false;
        ++m_iconCacheRevision;
        qApp->setStyleSheet(m_pendingStyleSheet);
        m_pendingStyleSheet.clear();
    }
}

quint64 ThemeManager::iconCacheRevision() const { return m_iconCacheRevision; }

QString ThemeManager::currentStyleSheet() const {
    return m_styleSheetPending ? m_pendingStyleSheet : qApp->styleSheet();
}

ThemeManager::Theme ThemeManager::currentTheme() const { return m_theme; }

ThemeManager::Theme ThemeManager::requestedTheme() const {
    return m_requestedTheme;
}

QList<ThemeManager::StandardIcon> ThemeManager::standardIcons() {
    QList<StandardIcon> result;
    const auto meta = QMetaEnum::fromType<StandardIcon>();
    result.reserve(meta.keyCount() - 1);
    for (int i = 0; i < meta.keyCount(); ++i) {
        const auto id = StandardIcon(meta.value(i));
        if (id != StandardIcon::None) {
            result.append(id);
        }
    }
    return result;
}

QString ThemeManager::iconResource(StandardIcon icon) const {
    const auto name = resourceName(icon);
    if (name.isEmpty()) {
        return {};
    }
    const QString directory = m_theme == Theme::Dark ? QStringLiteral("dark")
                                                     : QStringLiteral("light");
    const QString base = QStringLiteral(":/com.wingsummer.winghex/images/");
    const QString variant =
        base + directory + QLatin1Char('/') + name + QStringLiteral(".svg");
    if (QFile::exists(variant)) {
        return variant;
    }
    const QString svg = base + name + QStringLiteral(".svg");
    return QFile::exists(svg) ? svg : QString();
}

QIcon ThemeManager::standardIcon(StandardIcon icon) const {
    const int id = static_cast<int>(icon);
    auto cached = m_icons.constFind(id);
    if (cached != m_icons.cend()) {
        return cached.value();
    }
    if (iconResource(icon).isEmpty()) {
        return {};
    }
    QIcon result(new ThemeIconEngine(icon));
    m_icons.insert(id, result);
    return result;
}

QPixmap ThemeManager::standardPixmap(StandardIcon icon) const {
    const auto id = int(icon);
    auto cached = m_pixmaps.constFind(id);
    if (cached != m_pixmaps.cend()) {
        return cached.value();
    }
    const auto image = standardIcon(icon).pixmap(200, 200);
    if (!image.isNull()) {
        m_pixmaps.insert(id, image);
    }
    return image;
}

QIcon ThemeManager::resourceIcon(const QString &path) const {
    if (!path.startsWith(QStringLiteral(":/")) || !QFile::exists(path)) {
        return {};
    }
    return QIcon(new ThemeIconEngine(path));
}

QByteArray ThemeManager::resourceIconData(const QString &path) const {
    if (!path.startsWith(QStringLiteral(":/"))) {
        return {};
    }
    auto cached = m_resourceData.constFind(path);
    if (cached != m_resourceData.cend()) {
        return cached.value();
    }
    const auto data = themedResourceData(path, m_theme);
    m_resourceData.insert(path, data);
    return data;
}

QByteArray ThemeManager::resolveIconData(StandardIcon icon) const {
    return resourceIconData(iconResource(icon));
}

QByteArray ThemeManager::standardIconData(StandardIcon icon) const {
    const int id = int(icon);
    auto cached = m_data.constFind(id);
    if (cached != m_data.cend()) {
        return cached.value();
    }
    const auto data = resolveIconData(icon);
    m_data.insert(id, data);
    return data;
}

void ThemeManager::translate() {
    tr("System");
    tr("Dark");
    tr("Light");
}
