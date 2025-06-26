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

#include "testmanager.h"

TestManager::TestManager() : WingHex::IWingManager() { content = new TestPage; }

TestManager::~TestManager() {
    // no need to manual delete content
    // the host will take the ownership
}

bool TestManager::init(const std::unique_ptr<QSettings> &set) {
    Q_UNUSED(set);
    return true;
}

void TestManager::unload(std::unique_ptr<QSettings> &set) { Q_UNUSED(set); }

const QString TestManager::comment() const {
    return QStringLiteral("Hello world!");
}

QList<WingHex::SettingPage *> TestManager::registeredSettingPages() const {
    return {content};
}

bool TestManager::enterGuard(const QMetaObject *sender, const QString &sig,
                             const QVariantList &params) {
    if (content->isDisableMsg()) {
        if (sig.startsWith("msg")) {
            // block all msg-prefix function
            logWarn(QStringLiteral("Blocking: (%1) {%2}")
                        .arg(sender->className(), sig));
            return false;
        }
    }

    return true;
}
