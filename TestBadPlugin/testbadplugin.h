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

#ifndef TESTBADPLUGIN_H
#define TESTBADPLUGIN_H

#include "WingPlugin/iwingplugin.h"

class TestBadPlugin : public WingHex::IWingPlugin {
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "com.wingsummer.iwingplugin" FILE
                          "TestBadPlugin.json")
    Q_INTERFACES(WingHex::IWingPlugin)
public:
    TestBadPlugin();

    // IWingPluginCoreBase interface
public:
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;

    // IWingPluginBase interface
public:
    virtual const QString pluginName() const override;
    virtual const QString pluginComment() const override;
};

#endif // TESTBADPLUGIN_H
