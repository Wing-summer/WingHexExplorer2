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

#include "commandhistorymanager.h"

#include "utilities.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

constexpr auto COMMAND_FILE_SIZE_LIMIT = 4 * 1024 * 1024;

QStringList CommandHistoryManager::load() {
    QStringList ret;
    QFile f(getHistoryPath());
    if (f.size() > COMMAND_FILE_SIZE_LIMIT) {
        return {};
    }
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        const auto contents = f.readAll();
        const auto document = QJsonDocument::fromJson(f.readAll());
        if (!document.isArray()) {
            return {};
        }
        for (const auto &entry : document.array()) {
            if (!entry.isString()) {
                return {};
            }
            ret.append(entry.toString());
        }
    }
    return ret;
}

void CommandHistoryManager::save(const QStringList &strings) {
    QJsonArray entries;
    // Load feeds entries into History::add, which prepends the newest entry.
    for (auto it = strings.crbegin(); it != strings.crend(); ++it) {
        entries.append(*it);
    }
    const auto contents = QJsonDocument(entries).toJson();
    QSaveFile f(getHistoryPath());
    if (f.open(QFile::WriteOnly)) {
        if (f.write(contents) != contents.size()) {
            f.cancelWriting();
            return;
        }
        f.commit();
    }
}

QString CommandHistoryManager::getHistoryPath() {
    QDir dir(Utilities::getAppDataPath());
    return dir.absoluteFilePath(QStringLiteral(".command_history.lst"));
}
