#pragma once
#include <QString>

// Only the persisted language setting is substituted. The production loader,
// package validation, Qt translators and embedded resources run unchanged.
class SettingManager {
public:
    static SettingManager &instance() {
        static SettingManager settings;
        return settings;
    }
    QString defaultLang() const {
        return qEnvironmentVariable("WING_TEST_LANGUAGE");
    }
};
