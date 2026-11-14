// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: AGPL-3.0-only
#include <stdexcept>

#include <QApplication>
#include <QFile>

#include "WingPlugin/iwingdevice.h"
#include "WingPlugin/iwingplugin.h"

namespace {
void record(const char *stage) {
    QFile file(qEnvironmentVariable("WINGHEX_HEADLESS_TEST_TRACE"));
    if (!file.fileName().isEmpty() &&
        file.open(QIODevice::WriteOnly | QIODevice::Append)) {
#ifdef HEADLESS_DEVICE_PROBE
        file.write("device:");
#else
        file.write("plugin:");
#endif
        file.write(stage);
        file.write("\n");
    }
}
} // namespace
#ifdef HEADLESS_DEVICE_PROBE
class HeadlessProbe : public WingHex::IWingDevice {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.wingsummer.iwingdevice" FILE
                          "HeadlessDeviceProbe.json")
    Q_INTERFACES(WingHex::IWingDevice)
public:
    QString supportedFileExtDisplayName() const override {
        return QStringLiteral("Headless probe");
    }

    WingHex::WingIODevice *onOpenFile(const QString &) override {
        return nullptr;
    }

    bool onCloseFile(WingHex::WingIODevice *) override { return true; }
#else
class HeadlessProbe : public WingHex::IWingPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.wingsummer.iwingplugin" FILE
                          "HeadlessProbe.json")
    Q_INTERFACES(WingHex::IWingPlugin)
public:
    RegisteredEvents registeredEvents() const override {
        return packupEvent(RegisteredEvent::AppReady,
                           RegisteredEvent::AppClosing);
    }

    QList<WingHex::WingRibbonToolBoxInfo>
    registeredRibbonTools() const override {
        record("ui");
        return {};
    }

    void eventReady() override {
        m_ready = true;
        record("ready");
    }

    bool eventClosing() override {
        record("closing");
        return false;
    }
#endif
public:
    const QString pluginName() const override {
        return QStringLiteral("Headless probe");
    }

    const QString pluginComment() const override {
        return QStringLiteral("Headless regression fixture");
    }

    bool init(const std::unique_ptr<QSettings> &) override {
        record("init");
        if (qEnvironmentVariableIsSet("WINGHEX_MODE_PLUGIN")) {
            return true;
        }
        bool ok = true;
        const auto text =
            dlgGetText(nullptr, QStringLiteral("Probe"),
                       QStringLiteral("Input"), QLineEdit::Normal, {}, &ok);
        return !ok && text.isEmpty() &&
               msgQuestion(nullptr, QStringLiteral("Probe"),
                           QStringLiteral("Continue?")) ==
                   QMessageBox::NoButton &&
               QApplication::topLevelWidgets().isEmpty();
    }

    void unload(std::unique_ptr<QSettings> &) override { record("unload"); }

    void onRegisterScriptObj(WingScriptBind::ScopeBuilder scope) override {
        record("register");
        if (qEnvironmentVariableIsSet(
                "WINGHEX_HEADLESS_TEST_REGISTRATION_FAIL")) {
            throw std::runtime_error("Intentional registration failure");
        }
        scope.addFunction("answer", [] { return 42; });
        scope.addFunction("ready", [this] { return m_ready; });
    }

private:
    bool m_ready = false;
};
#include "probe.moc"
