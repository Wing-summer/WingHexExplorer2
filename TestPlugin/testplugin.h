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

#include "WingPlugin/iwingplugin.h"

#include <QSharedMemory>

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
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;
    virtual const QString pluginName() const override;
    virtual const QString pluginComment() const override;

    virtual QString retranslate(const QString &str) override;

    // IWingPlugin interface (可选)
public:
    // 有关注册开头（register 开头）的函数，插件系统不保证有且只调用一次
    // 所以最好不要为了偷懒，在实现里面 new 和做相应操作
    virtual QList<WingHex::WingDockWidgetInfo>
    registeredDockWidgets() const override;
    virtual QMenu *registeredHexContextMenu() const override;
    virtual QList<WingHex::WingRibbonToolBoxInfo>
    registeredRibbonTools() const override;
    virtual QList<WingHex::SettingPage *>
    registeredSettingPages() const override;
    virtual QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>>
    registeredEditorViewWidgets() const override;

    virtual RegisteredEvents registeredEvents() const override;
    virtual void eventReady() override;

    // IWingPluginCoreBase interface
public:
    virtual void onRegisterScriptObj(WingHex::IWingAngel *o) override;

    virtual void
    onPaintHexEditorView(QPainter *painter, QWidget *w,
                         WingHex::HexEditorContext *context) override;

private:
    QVariant test_a(const QVariantList &params);
    QVariant test_b(const QVariantList &params);
    QVariant test_c(const QVariantList &params);
    QVariant test_d(const QVariantList &params);

    QVariant test_e(const QVariantList &params);
    QVariant test_f(const QVariantList &params);
    QVariant test_g(const QVariantList &params);
    QVariant test_h(const QVariantList &params);

    WingHex::UNSAFE_RET colorTable(const QList<void *> &params);

    QVariant createTestShareMem(const QVariantList &params);
    QVariant destoryTestShareMem(const QVariantList &params);
    QVariant printLogTestSharedMemData(const QVariantList &params);

    QVariant setPluginMetaTestEnabled(const QVariantList &params);
    QVariant pluginMetaTestEnabled(const QVariantList &params);

    QVariant testCrash(const QVariantList &params);

    static void testGenericAdd(WingHex::asIWingGeneric *param);

    static void testRaiseScriptException(WingHex::asIWingGeneric *);

private:
    void test_a();
    void test_b(const QString &b);
    void test_c(const QVector<int> &c);
    void test_d(const QVariantHash &d);

    bool test_e();
    QByteArray test_f();
    QString test_g();
    QVariantHash test_h();

    QVector<QColor> colorTable();
    bool createTestShareMem(const QString &nameID);
    void destoryTestShareMem();
    void printLogTestSharedMemData();

    void setPluginMetaTestEnabled(bool b);
    bool pluginMetaTestEnabled();

    void testCrash();

private:
    QDialog *_tform = nullptr;
    QMenu *_tmenu = nullptr;

    QSharedMemory *_tsharemem = nullptr;

    bool ENABLE_META = false;

    QList<WingHex::WingDockWidgetInfo> _winfo;
    QList<WingHex::WingRibbonToolBoxInfo> _rtbinfo;
    QList<WingHex::SettingPage *> _setpages;
    QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>> _evws;
};

#endif // TESTPLUGIN_H
