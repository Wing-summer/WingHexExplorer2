// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-only
#include <cstdio>

#include <QSettings>
#include <QString>

#include "modeprobe.h"

int wingApplicationMain(int argc, char **argv);

int main(int argc, char **argv) {
    const auto config = qEnvironmentVariable("WINGHEX_HEADLESS_TEST_CONFIG");
    if (config.isEmpty()) {
        std::fputs("The process probe requires an isolated settings path\n",
                   stderr);
        return 99;
    }
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, config);
    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, config);
    const bool verifyEnvironment =
        qEnvironmentVariableIsSet("WINGHEX_HEADLESS_TEST_CHECK_ENV");
    const auto result = wingApplicationMain(argc, argv);
    if (verifyEnvironment && (qgetenv("WINGHEX_DISALLOW_ROOT_PLUGIN") != "1" ||
                              qgetenv("WINGHEX_CERT_SKIP") != "0" ||
                              qgetenv("WINGHEX_CERT_FORCE_ALL") != "0")) {
        std::fputs("Environment setup did not precede argument rejection\n",
                   stderr);
        return 98;
    }
    return modeProbeFailed ? 97 : result;
}
