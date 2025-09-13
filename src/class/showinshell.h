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

#ifndef SHOWINSHELL_H
#define SHOWINSHELL_H

#include <QString>
#include <QWidget>

struct HostOsInfo {
#ifdef Q_OS_MAC
    static constexpr bool isMacHost() { return true; }
#else
    static constexpr bool isMacHost() { return false; }
#endif
#ifdef Q_OS_WIN
    static constexpr bool isWindowsHost() { return true; }
#else
    static constexpr bool isWindowsHost() { return false; }
#endif
};

class ShowInShell {
public:
    static bool showInGraphicalShell(QWidget *parent, const QString &pathIn,
                                     bool deselect);

private:
    static bool showInWindowsShell(const QString &filePath, bool deselect);
};

#endif // SHOWINSHELL_H
