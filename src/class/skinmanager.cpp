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

#include "skinmanager.h"

#include "dbghelper.h"
#include "settingmanager.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QScreen>
#include <QStyleFactory>

SkinManager::SkinManager() {
    ASSERT_SINGLETON;

    int theme = SettingManager::instance().themeID();

    QFile qss;
    switch (Theme(theme)) {
    case Theme::Dark:
        setTheme(Theme::Dark);
        qss.setFileName("://dark/stylesheet.qss");
        break;
    case Theme::Light:
        setTheme(Theme::Light);
        qss.setFileName("://light/stylesheet.qss");
        break;
    }
    qss.open(QFile::ReadOnly | QFile::Text);
#ifdef Q_OS_WIN
    qApp->setStyle(QStyleFactory::create("windowsvista"));
#else
    qApp->setStyle(QStyleFactory::create("fusion"));
#endif
    qApp->setStyleSheet(qss.readAll());
    qss.close();
}

SkinManager::~SkinManager() {}

SkinManager &SkinManager::instance() {
    static SkinManager instance;
    return instance;
}

void SkinManager::setTheme(SkinManager::Theme theme) {
    if (m_theme != theme) {
        m_theme = theme;
        m_cache.clear();
    }
}

QIcon SkinManager::themeIcon(const QString &name) {
    auto picon = m_cache.find(name);
    if (picon == m_cache.end()) {
        switch (m_theme) {
        case Theme::Dark: {
            QIcon icon(QStringLiteral("://dark/") + name +
                       QStringLiteral(".svg"));
            m_cache.insert(name, icon);
            return icon;
        }
        case Theme::Light: {
            QIcon icon(QStringLiteral("://light/") + name +
                       QStringLiteral(".svg"));
            m_cache.insert(name, icon);
            return icon;
        }
        default:
            return {};
        }
    } else {
        return *picon;
    }
}

SkinManager::Theme SkinManager::currentTheme() const { return m_theme; }

void SkinManager::translate() {
    tr("Dark");
    tr("Light");
}
