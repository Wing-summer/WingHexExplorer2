/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
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

#include "testbadplugin.h"

TestBadPlugin::TestBadPlugin() {}

bool TestBadPlugin::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    msgCritical(
        nullptr, QStringLiteral("TestBadPlugin"),
        QStringLiteral("Hello, pals! I'm a evil MESSAGEBOX! PLEASE BAN ME!"));
    return true;
}

void TestBadPlugin::unload(std::unique_ptr<QSettings> &set) { Q_UNUSED(set); }

const QString TestBadPlugin::pluginName() const {
    return QStringLiteral("TestBadPlugin");
}

const QString TestBadPlugin::pluginComment() const {
    return QStringLiteral("TestBadPlugin: popup a messagebox when startup!");
}
