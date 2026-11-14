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

#include <atomic>
#include <csignal>
#include <cstdio>

#include <fcntl.h>
#include <io.h>
#include <windows.h>

#include "terminalservices.h"

namespace {
std::atomic<std::sig_atomic_t> signalNumber{0};
static_assert(decltype(signalNumber)::is_always_lock_free);

void handleSignal(int number) { signalNumber = number; }

BOOL WINAPI consoleControl(DWORD event) {
    if (event == CTRL_C_EVENT || event == CTRL_BREAK_EVENT) {
        signalNumber = SIGINT;
        return TRUE;
    }
    return FALSE;
}

static void connectTerminalStreams() {
    auto valid = [](HANDLE handle) {
        return handle && handle != INVALID_HANDLE_VALUE;
    };
    if (!valid(GetStdHandle(STD_OUTPUT_HANDLE))) {
        AttachConsole(ATTACH_PARENT_PROCESS);
    }
    const auto bind = [valid](DWORD identifier, FILE *stream,
                              const char *mode) {
        const auto original = GetStdHandle(identifier);
        if (!valid(original)) {
            return;
        }
        HANDLE copy = nullptr;
        if (!DuplicateHandle(GetCurrentProcess(), original, GetCurrentProcess(),
                             &copy, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            return;
        }
        const int descriptor =
            _open_osfhandle(reinterpret_cast<intptr_t>(copy), _O_BINARY);
        if (descriptor < 0) {
            CloseHandle(copy);
            return;
        }
        if (_fileno(stream) < 0) {
            std::freopen("NUL", mode, stream);
        }
        _dup2(descriptor, _fileno(stream));
        _close(descriptor);
    };
    bind(STD_OUTPUT_HANDLE, stdout, "w");
    bind(STD_ERROR_HANDLE, stderr, "w");
}
} // namespace

struct TerminalServices::Private {
    using Handler = void (*)(int);
    Handler previousInt;
    Handler previousTerm;
};

TerminalServices::TerminalServices() : m_private(std::make_unique<Private>()) {
    signalNumber = 0;
    m_private->previousInt = std::signal(SIGINT, handleSignal);
    m_private->previousTerm = std::signal(SIGTERM, handleSignal);
    SetConsoleCtrlHandler(consoleControl, TRUE);
}

TerminalServices::~TerminalServices() {
    SetConsoleCtrlHandler(consoleControl, FALSE);
    std::signal(SIGINT, m_private->previousInt);
    std::signal(SIGTERM, m_private->previousTerm);
}

int TerminalServices::pendingSignal() { return signalNumber; }

void TerminalServices::prepareStreams() { connectTerminalStreams(); }

void TerminalServices::write(bool error, const QString &text) {
    auto stream = error ? stderr : stdout;

    const auto handle =
        GetStdHandle(stream == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(handle, &mode)) {
        DWORD count = 0;
        WriteConsoleW(handle, text.utf16(), DWORD(text.size()), &count,
                      nullptr);
        return;
    }

    const auto bytes = text.toUtf8();
    std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stream);
    std::fflush(stream);
}

bool TerminalServices::desktopConfigured() { return true; }
