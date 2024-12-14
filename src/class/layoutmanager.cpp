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
#include "languagemanager.h"
#include "utilities.h"

LayoutManager &LayoutManager::instance() {
    static LayoutManager ins;
    return ins;
}

const QHash<QString, QByteArray> LayoutManager::layouts() const {
    return _layouts;
}

QByteArray LayoutManager::layout(const QString &v) const {
    return _layouts.value(v);
}

qsizetype LayoutManager::layoutCount() const { return _layouts.size(); }

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

    // read translation names
    auto trf = QStringLiteral("metatr.ini");
    if (pdir.exists(trf)) {
        process(pdir, std::make_unique<QSettings>(pdir.absoluteFilePath(trf),
                                                  QSettings::IniFormat));
    } else {
        process(pdir, nullptr);
    }
}

void LayoutManager::process(const QDir &dir,
                            const std::unique_ptr<QSettings> &set) {
    auto name = LanguageManager::instance().defaultLocale().name();
    auto sep = QStringLiteral("/");

    for (auto &l : dir.entryInfoList({"*.wing-layout"}, QDir::Files)) {
        QString k;
        if (set) {
            k = set->value(name + sep + l.baseName()).toString();
        }

        if (k.isEmpty()) {
            k = l.baseName();
        }

        QFile f(l.absoluteFilePath());
        if (f.open(QFile::ReadOnly)) {
            auto b = f.readAll();
            if (!b.isEmpty()) {
                _layouts.insert(k.trimmed(), b);
            }
        }
    }
}
