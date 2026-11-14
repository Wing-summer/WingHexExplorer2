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

#include "terminalservices.h"

namespace {
std::atomic<std::sig_atomic_t> signalNumber{0};
static_assert(decltype(signalNumber)::is_always_lock_free);

void handleSignal(int number) { signalNumber = number; }
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
}

TerminalServices::~TerminalServices() {
    std::signal(SIGINT, m_private->previousInt);
    std::signal(SIGTERM, m_private->previousTerm);
}

int TerminalServices::pendingSignal() { return signalNumber; }

void TerminalServices::prepareStreams() {}

void TerminalServices::write(bool error, const QString &text) {
    auto stream = error ? stderr : stdout;

    const auto bytes = text.toUtf8();
    std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stream);
    std::fflush(stream);
}

bool TerminalServices::desktopConfigured() {
#ifdef Q_OS_MACOS
    return true;
#else
    return !qEnvironmentVariableIsEmpty("DISPLAY") ||
           !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");
#endif
}
