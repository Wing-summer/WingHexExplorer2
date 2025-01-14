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

#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#include "iwingplugin.h"

class TestPlugin final : public WingHex::IWingPlugin {
    Q_OBJECT

    // 这两行是必须，只有后面的 "TestPlugin.json" 你根据需要修改，
    // 具体可见 QT 文档，剩下直接 CV 大法
    Q_PLUGIN_METADATA(IID "com.wingsummer.iwingplugin" FILE "TestPlugin.json")
    Q_INTERFACES(WingHex::IWingPlugin)

public:
    explicit TestPlugin();

    virtual ~TestPlugin();

    // IWingPlugin interface (必须)
public:
    virtual int sdkVersion() const override;
    virtual const QString signature() const override;
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginComment() const override;

    // IWingPlugin interface (可选)
public:
    // 有关注册开头（register 开头）的函数，插件系统不保证有且只调用一次
    // 所以最好不要为了偷懒，在实现里面 new 和做相应操作
    virtual QList<WingHex::WingDockWidgetInfo>
    registeredDockWidgets() const override;
    virtual QMenu *registeredHexContextMenu() const override;
    virtual QList<WingHex::WingRibbonToolBoxInfo>
    registeredRibbonTools() const override;
    virtual QHash<WingHex::SettingPage *, bool>
    registeredSettingPages() const override;
    virtual QList<WingHex::PluginPage *> registeredPages() const override;
    virtual QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>>
    registeredEditorViewWidgets() const override;
    virtual QHash<QString, ScriptFnInfo> registeredScriptFns() const override;

    virtual RegisteredEvents registeredEvents() const override;
    virtual void eventReady() override;

private:
    QString getPuid() const;

    QVariant test_a(const QVariantList &);
    QVariant test_b(const QVariantList &params);

private:
    void test_a();
    void test_b(const QString &a);

private:
    QDialog *_tform = nullptr;
    QMenu *_tmenu = nullptr;
    const QString puid;

    QHash<QString, WingHex::IWingPlugin::ScriptFnInfo> _scriptInfo;
    QList<WingHex::WingDockWidgetInfo> _winfo;
    QList<WingHex::WingRibbonToolBoxInfo> _rtbinfo;
    QHash<WingHex::SettingPage *, bool> _setpages;
    QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>> _evws;
    QList<WingHex::PluginPage *> _plgps;
};

#endif // TESTPLUGIN_H
