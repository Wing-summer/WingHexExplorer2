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

#include <QTextStream>

QStringList CommandHistoryManager::load() {
    QStringList ret;
    QFile f(getHistoryPath());
    if (f.open(QFile::ReadOnly)) {
        QTextStream is(&f);
        while (!is.atEnd()) {
            ret.append(is.readLine());
        }
    }
    return ret;
}

void CommandHistoryManager::save(const QStringList &strings) {
    QFile f(getHistoryPath());
    if (f.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream os(&f);
        int n = strings.size();
        while (n > 0)
            os << strings.at(--n) << Qt::endl;
    }
}

QString CommandHistoryManager::getHistoryPath() {
    QDir dir(Utilities::getAppDataPath());
    return dir.absoluteFilePath(QStringLiteral(".command_history.lst"));
}
