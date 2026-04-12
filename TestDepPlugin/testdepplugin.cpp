/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
** =============================================================================
*/

#include "testdepplugin.h"

#include <QTimer>

TestDepPlugin::TestDepPlugin() : WingHex::IWingPlugin() {}

bool TestDepPlugin::init(const std::unique_ptr<QSettings> &set) {
    invokeService(QStringLiteral("TestPlugin"), "test_a");
    return true;
}

void TestDepPlugin::unload(std::unique_ptr<QSettings> &set) {
    invokeService(QStringLiteral("TestPlugin"), "test_a");
}

const QString TestDepPlugin::pluginName() const {
    return QStringLiteral("TestDepPlugin");
}

const QString TestDepPlugin::pluginComment() const {
    return QStringLiteral("Test");
}

void TestDepPlugin::eventReady() {
    // This may be unsuccessful because TestPlugin will call
    // WingAngelAPI::execCode as well and it will be busy.
    // The script running is timer event driven so wait for a moment.
    QTimer::singleShot(100, this, [this]() {
        bool ret;
        invokeService(QStringLiteral("WingAngelAPI"), "execCode",
                      Qt::AutoConnection, qReturnArg(ret),
                      QStringLiteral(R"(println("Hello? TestDepPlugin!");)"));
    });
}

WingHex::IWingPlugin::RegisteredEvents TestDepPlugin::registeredEvents() const {
    return WingHex::IWingPlugin::RegisteredEvent::AppReady;
}
