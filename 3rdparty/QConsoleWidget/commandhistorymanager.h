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

#ifndef COMMANDHISTORYMANAGER_H
#define COMMANDHISTORYMANAGER_H

#include <QStringList>

class CommandHistoryManager {
public:
    static QStringList load();

    static void save(const QStringList &strings);

private:
    static QString getHistoryPath();
};

#endif // COMMANDHISTORYMANAGER_H
