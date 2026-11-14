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

#ifndef TERMINALSERVICES_H
#define TERMINALSERVICES_H

#include <memory>

#include <QString>

// Terminal I/O and process signals are implemented by the selected platform
// backend.
class TerminalServices {
public:
    TerminalServices();
    ~TerminalServices();

public:
    static void prepareStreams();
    static bool desktopConfigured();
    static void write(bool error, const QString &text);
    static int pendingSignal();

private:
    struct Private;
    std::unique_ptr<Private> m_private;

private:
    Q_DISABLE_COPY(TerminalServices)
};
#endif // TERMINALSERVICES_H
