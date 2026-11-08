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

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "WingPlugin/wingplugin_global.h"

#include <QByteArray>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QString>

class QStyle;

class ThemeManager : public QObject {
    Q_OBJECT
public:
    enum class Theme { System = 0, Dark = 1, Light = 2 };
    Q_ENUM(Theme)
    using StandardIcon = WingHex::StandardIcon;

    static ThemeManager &instance();
    ~ThemeManager() override;

public:
    void setTheme(Theme theme);
    // Resolved appearance; System is exposed by requestedTheme().
    Theme currentTheme() const;
    Theme requestedTheme() const;
    QString currentStyleSheet() const;
    // Batch initial widget construction before applying the application QSS.
    void setStyleSheetDeferred(bool deferred);
    quint64 iconCacheRevision() const;

    QIcon standardIcon(StandardIcon icon) const;
    QIcon resourceIcon(const QString &path) const;
    QByteArray resourceIconData(const QString &path) const;
    QPixmap standardPixmap(StandardIcon icon) const;
    QByteArray standardIconData(StandardIcon icon) const;
    QString iconResource(StandardIcon icon) const;

    static QList<StandardIcon> standardIcons();

signals:
    void themeChanged(ThemeManager::Theme theme);

private:
    explicit ThemeManager();
    void translate();
    void applyTheme(Theme theme);
    QByteArray resolveIconData(StandardIcon icon) const;

private:
    Theme m_requestedTheme = Theme::System;
    Theme m_theme = Theme::Light;
    bool m_initialized = false;
    bool m_styleSheetDeferred = false;
    bool m_styleSheetPending = false;
    quint64 m_iconCacheRevision = 0;
    QString m_pendingStyleSheet;
    QPointer<QStyle> m_themeStyle;

private:
    mutable QHash<int, QIcon> m_icons;
    mutable QHash<int, QByteArray> m_data;
    mutable QHash<int, QPixmap> m_pixmaps;
    mutable QHash<QString, QByteArray> m_resourceData;

    Q_DISABLE_COPY_MOVE(ThemeManager)
};

#endif // THEMEMANAGER_H
