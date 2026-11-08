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

#include "layoutmanager.h"

#include "dbghelper.h"
#include "utilities.h"

LayoutManager &LayoutManager::instance() {
    static LayoutManager ins;
    return ins;
}

const QHash<QString, QByteArray> &LayoutManager::layouts() const {
    return m_layouts;
}

QStringList LayoutManager::layoutNames() const { return m_caches; }

void LayoutManager::addLayout(const QString &name, const QByteArray &data) {
    m_layouts.insert(name, data);
    m_caches.append(name);
}

bool LayoutManager::contains(const QString &name) {
    return m_caches.contains(name, Qt::CaseInsensitive);
}

QByteArray LayoutManager::layout(const QString &v) const {
    return m_layouts.value(v);
}

qsizetype LayoutManager::layoutCount() const { return m_layouts.size(); }

QDir LayoutManager::layoutDir() {
    QDir pdir(Utilities::getAppDataPath());
    auto lname = QStringLiteral("layouts");
    return pdir.absoluteFilePath(lname);
}

LayoutManager::LayoutManager() {
    ASSERT_SINGLETON;
    QDir pdir(Utilities::getAppDataPath());
    auto lname = QStringLiteral("layouts");
    if (!pdir.exists(lname)) {
        pdir.mkdir(lname);
        return;
    }
    if (!pdir.cd(lname)) {
        return;
    }

    process(pdir);
}

void LayoutManager::process(const QDir &dir) {
    const auto files = dir.entryInfoList({"*.wing-layout"}, QDir::Files);
    for (const auto &l : files) {
        QString k = l.baseName();

        QFile f(l.absoluteFilePath());
        if (f.open(QFile::ReadOnly)) {
            auto b = f.readAll();
            if (!b.isEmpty()) {
                auto n = k.trimmed();
                m_caches.append(n);
                m_layouts.insert(n, b);
            }
        }
    }
}
