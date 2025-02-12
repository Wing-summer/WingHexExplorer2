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

#include "testplugin.h"
#include "testform.h"
#include "testpluginpage.h"
#include "testsettingpage.h"
#include "testwingeditorviewwidget.h"

#include <QApplication>

// 注意：所有提供的脚本接口函数都不是线程安全的，只是测试

TestPlugin::TestPlugin() : WingHex::IWingPlugin() {
    // 在构造函数中，所有的 API 都无法调用。插件的翻译文件也不会自动加载。
    // 在构造函数中，仅适合做一些为初始化准备的操作。

    // 插件的语言文件会在初始化前自动加载，如果初始化失败则会被卸载
    // 初始化会传递一个配置类，插件系统会统一管理放到统一的地方，使用 INI 保存
    // 你可以自行管理，但不建议，统一管理方便使用者备份和转移插件配置

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_a),
                      this, std::placeholders::_1);
        info.ret = MetaType::Void;
        _scriptInfo.insert(QStringLiteral("test_a"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_b),
                      this, std::placeholders::_1);
        info.ret = MetaType::Void;
        info.params.append(qMakePair(MetaType::String, QStringLiteral("info")));
        _scriptInfo.insert(QStringLiteral("test_b"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_c),
                      this, std::placeholders::_1);
        info.ret = MetaType::Void;
        info.params.append(qMakePair(MetaType(MetaType::Int | MetaType::Array),
                                     QStringLiteral("c")));
        _scriptInfo.insert(QStringLiteral("test_c"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_d),
                      this, std::placeholders::_1);
        info.ret = MetaType::Void;
        info.params.append(qMakePair(MetaType::Hash, QStringLiteral("d")));
        _scriptInfo.insert(QStringLiteral("test_d"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_e),
                      this, std::placeholders::_1);
        info.ret = MetaType::Bool;
        _scriptInfo.insert(QStringLiteral("test_e"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_f),
                      this, std::placeholders::_1);
        info.ret = MetaType(MetaType::Byte | MetaType::Array);
        _scriptInfo.insert(QStringLiteral("test_f"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_g),
                      this, std::placeholders::_1);
        info.ret = MetaType::String;
        _scriptInfo.insert(QStringLiteral("test_g"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn =
            std::bind(QOverload<const QVariantList &>::of(&TestPlugin::test_h),
                      this, std::placeholders::_1);
        info.ret = MetaType::Hash;
        _scriptInfo.insert(QStringLiteral("test_h"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(QOverload<const QVariantList &>::of(
                                &TestPlugin::createTestShareMem),
                            this, std::placeholders::_1);
        info.ret = MetaType::Bool;
        info.params.append(
            qMakePair(MetaType::String, QStringLiteral("nameID")));
        _scriptInfo.insert(QStringLiteral("createTestShareMem"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(QOverload<const QVariantList &>::of(
                                &TestPlugin::destoryTestShareMem),
                            this, std::placeholders::_1);
        info.ret = MetaType::Void;
        _scriptInfo.insert(QStringLiteral("destoryTestShareMem"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(QOverload<const QVariantList &>::of(
                                &TestPlugin::printLogTestSharedMemData),
                            this, std::placeholders::_1);
        info.ret = MetaType::Void;
        _scriptInfo.insert(QStringLiteral("printLogTestSharedMemData"), info);
    }

    {
        _scriptUnsafe.insert(QStringLiteral("array<color>@ colorTable()"),
                             std::bind(QOverload<const QList<void *> &>::of(
                                           &TestPlugin::colorTable),
                                       this, std::placeholders::_1));
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(QOverload<const QVariantList &>::of(
                                &TestPlugin::setPluginMetaTestEnabled),
                            this, std::placeholders::_1);
        info.ret = MetaType::Void;
        info.params.append(qMakePair(MetaType::Bool, QStringLiteral("b")));
        _scriptInfo.insert(QStringLiteral("setPluginMetaTestEnabled"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(QOverload<const QVariantList &>::of(
                                &TestPlugin::pluginMetaTestEnabled),
                            this, std::placeholders::_1);
        info.ret = MetaType::Bool;
        _scriptInfo.insert(QStringLiteral("pluginMetaTestEnabled"), info);
    }

    {
        WingHex::IWingPlugin::ScriptFnInfo info;
        info.fn = std::bind(
            QOverload<const QVariantList &>::of(&TestPlugin::testCrash), this,
            std::placeholders::_1);
        info.ret = MetaType::Void;
        _scriptInfo.insert(QStringLiteral("testCrash"), info);
    }
}

TestPlugin::~TestPlugin() { destoryTestShareMem(); }

int TestPlugin::sdkVersion() const { return WingHex::SDKVERSION; }

const QString TestPlugin::signature() const { return WingHex::WINGSUMMER; }

bool TestPlugin::init(const std::unique_ptr<QSettings> &set) {
    auto v = set->value("Test", 0).toInt();
    // 如果你之前启动过且正常推出，这个值一定是 5
    qDebug() << v;

    // 和日志与 UI 相关的接口此时可用，剩余的 API 初始化成功才可用
    _tform = emit createDialog(new TestForm(this));
    if (_tform == nullptr) {
        return false;
    }
    _tform->setWindowFlag(Qt::WindowStaysOnTopHint);
    _tform->setMaximumHeight(500);

    using TBInfo = WingHex::WingRibbonToolBoxInfo;
    TBInfo::RibbonCatagories cats;
    auto tb = new QToolButton;

    // 这里有一个约定，对于含有图片的，前缀应为：/images/插件的 PUID
    // 其他资源，前缀应为：/res/插件的 PUID 或 /resources/插件的 PUID
    // 如果想省劲，可以统一为 /插件的 PUID，但别一个 / 完事。
    // 这很容易发生资源文件冲突！！！
    tb->setIcon(QIcon(QStringLiteral(":/images/TestPlugin/images/test.png")));
    tb->setText(tr("Test"));
    connect(tb, &QToolButton::clicked, this, [=] {
        if (_tform) {
            _tform->show();
            _tform->raise();
        }
    });

    {
        WingHex::WingRibbonToolBoxInfo rtinfo;
        rtinfo.catagory = cats.PLUGIN;
        TBInfo::Toolbox tbtb;
        tbtb.name = tr("TestPlugin");
        tbtb.tools = {tb};
        rtinfo.toolboxs = {tbtb};
        _rtbinfo.append(rtinfo);
    }

    {
        WingHex::WingRibbonToolBoxInfo rtinfo;
        rtinfo.catagory = QStringLiteral("TestPlugin");
        rtinfo.displayName = tr("TestPlugin");

        QIcon btnIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));
        for (int i = 0; i < 3; ++i) {
            TBInfo::Toolbox tbtb;
            tbtb.name = tr("TestPlugin") + QStringLiteral("(%1)").arg(i);
            for (int y = 0; y < 5; ++y) {
                auto tb = new QToolButton;
                tb->setIcon(btnIcon);
                tb->setText(tr("Button - ") +
                            QStringLiteral("(%1, %2)").arg(i).arg(y));
                connect(tb, &QToolButton::clicked, this, [this] {
                    auto tb = qobject_cast<QToolButton *>(sender());
                    emit msgbox.information(nullptr, tr("Click"), tb->text());
                });
                tbtb.tools.append(tb);
            }
            rtinfo.toolboxs.append(tbtb);
        }
        _rtbinfo.append(rtinfo);
    }

    {
        auto sp = new TestSettingPage(QStringLiteral("Test1"),
                                      QStringLiteral("This is a Test1"));
        _setpages.insert(sp, true);
    }

    {
        auto sp = new TestSettingPage(QStringLiteral("Test2"),
                                      QStringLiteral("This is a Test2"));
        _setpages.insert(sp, false);
    }

    {
        WingHex::WingDockWidgetInfo info;
        auto lbl = new QLabel(QStringLiteral("DockTest1"));
        lbl->setAlignment(Qt::AlignCenter);
        info.widget = lbl;
        info.widgetName = QStringLiteral("DockTest1");
        info.area = Qt::LeftDockWidgetArea;
        _winfo.append(info);
    }

    {
        auto ev = QSharedPointer<TestWingEditorViewWidget::Creator>::create();
        _evws.append(ev);
    }

    {
        auto pp = new TestPluginPage;
        _plgps.append(pp);
    }

    _tmenu = new QMenu(QStringLiteral("TestPlugin"));
    auto micon = QIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));
    _tmenu->setIcon(micon);
    for (int i = 0; i < 5; ++i) {
        auto a = new QAction(
            micon, QStringLiteral("Test - ") + QString::number(i), _tmenu);
        connect(a, &QAction::triggered, this, [this, a]() {
            emit msgbox.information(nullptr, QStringLiteral("Test"), a->text());
        });
        _tmenu->addAction(a);
    }

    return true;
}

void TestPlugin::unload(std::unique_ptr<QSettings> &set) {
    // 设个数字，那就是 5 测试一下配置是否正常工作
    set->setValue("Test", 5);

    for (auto p = _setpages.constKeyValueBegin();
         p != _setpages.constKeyValueEnd(); ++p) {
        p->first->deleteLater();
    }

    for (auto &item : _winfo) {
        item.widget->deleteLater();
    }

    _tform->close();
    _tform->deleteLater();
}

const QString TestPlugin::pluginName() const { return tr("TestPlugin"); }

const QString TestPlugin::pluginComment() const {
    return tr("A Test Plugin for WingHexExplorer2.");
}

QString TestPlugin::retranslate(const QString &str) {
    return QApplication::tr(str.toLatin1());
}

QList<WingHex::WingDockWidgetInfo> TestPlugin::registeredDockWidgets() const {
    return _winfo;
}

QMenu *TestPlugin::registeredHexContextMenu() const { return _tmenu; }

QList<WingHex::WingRibbonToolBoxInfo>
TestPlugin::registeredRibbonTools() const {
    return _rtbinfo;
}

QHash<WingHex::SettingPage *, bool> TestPlugin::registeredSettingPages() const {
    return _setpages;
}

QList<WingHex::PluginPage *> TestPlugin::registeredPages() const {
    return _plgps;
}

QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>>
TestPlugin::registeredEditorViewWidgets() const {
    return _evws;
}

QHash<QString, WingHex::IWingPlugin::UNSAFE_SCFNPTR>
TestPlugin::registeredScriptUnsafeFns() const {
    return _scriptUnsafe;
}

QVariant TestPlugin::test_a(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    test_a();
    return {};
}

QVariant TestPlugin::test_b(const QVariantList &params) {
    if (params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto arg0 = params.first().toString();
    test_b(arg0);
    return {};
}

QVariant TestPlugin::test_c(const QVariantList &params) {
    if (params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto arg0 = params.first();

    // note: passed QVector<QVariant>
    if (arg0.canConvert<QVector<QVariant>>()) {
        QVector<int> param;
        for (auto &v : arg0.value<QVector<QVariant>>()) {
            param.append(v.toInt()); // must be int
        }

        test_c(param);
    } else {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    return {};
}

QVariant TestPlugin::test_d(const QVariantList &params) {
    if (params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }

    auto arg0 = params.first();

    // note: passed QVariantHash
    if (arg0.canConvert<QVariantHash>()) {
        test_d(arg0.value<QVariantHash>());
    } else {
        return getScriptCallError(-2, tr("InvalidParam"));
    }
    return {};
}

QVariant TestPlugin::test_e(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return test_e();
}

QVariant TestPlugin::test_f(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return test_f();
}

QVariant TestPlugin::test_g(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return test_g();
}

QVariant TestPlugin::test_h(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return test_h();
}

WingHex::IWingPlugin::UNSAFE_RET
TestPlugin::colorTable(const QList<void *> &params) {
    if (!params.isEmpty()) {
        return generateScriptCallError(-1, tr("InvalidParamsCount"));
    }

    void *array;
    QVector<void *> colors;
    for (auto &c : colorTable()) {
        colors.append(new QColor(c));
    }

    auto invoked =
        invokeService(QStringLiteral("WingAngelAPI"), "vector2AsArray",
                      WINGAPI_RETURN_ARG(void *, array),
                      WINGAPI_ARG(MetaType, MetaType::Color),
                      WINGAPI_ARG(QVector<void *>, colors));
    if (invoked) {
        if (array) {
            qDeleteAll(colors);
            return array;
        }
    }

    qDeleteAll(colors);
    return generateScriptCallError(-2, tr("AllocArrayFailed"));
}

QVector<QColor> TestPlugin::colorTable() {
    return {Qt::red, Qt::green, Qt::blue};
}

QVariant TestPlugin::createTestShareMem(const QVariantList &params) {
    if (params.size() != 1) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto arg0 = params.first().toString();
    return createTestShareMem(arg0);
}

QVariant TestPlugin::destoryTestShareMem(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    destoryTestShareMem();
    return {};
}

QVariant TestPlugin::printLogTestSharedMemData(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    printLogTestSharedMemData();
    return {};
}

QVariant TestPlugin::setPluginMetaTestEnabled(const QVariantList &params) {
    if (params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    auto arg0 = params.first().toBool();
    setPluginMetaTestEnabled(arg0);
    return {};
}

QVariant TestPlugin::pluginMetaTestEnabled(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    return pluginMetaTestEnabled();
}

QVariant TestPlugin::testCrash(const QVariantList &params) {
    if (!params.isEmpty()) {
        return getScriptCallError(-1, tr("InvalidParamsCount"));
    }
    testCrash();
    return {};
}

void TestPlugin::test_a() { emit debug(__FUNCTION__); }

void TestPlugin::test_b(const QString &b) {
    emit warn(__FUNCTION__ + QStringLiteral(" : ") % b);
}

void TestPlugin::test_c(const QVector<int> &c) {
    auto content = __FUNCTION__ + QStringLiteral(" : { ");
    if (!c.isEmpty()) {
        content += QString::number(c.first());

        for (QVector<int>::size_type i = 1; i < c.size(); ++i) {
            content += (QStringLiteral(", ") + QString::number(c.at(i)));
        }
    }

    content += QStringLiteral(" }");

    emit warn(content);
}

void TestPlugin::test_d(const QVariantHash &d) {
    auto content = __FUNCTION__ + QStringLiteral(" : { ");
    if (!d.isEmpty()) {
        QStringList hash;
        for (auto p = d.constKeyValueBegin(); p != d.constKeyValueEnd(); p++) {
            hash.append(QStringLiteral("{ ") + p->first + QStringLiteral(", ") +
                        p->second.toString() + QStringLiteral(" }"));
        }

        content += hash.join(", ");
    }
    content += QStringLiteral(" }");
    emit warn(content);
}

bool TestPlugin::test_e() {
    emit warn(__FUNCTION__);
    return true;
}

QByteArray TestPlugin::test_f() {
    emit warn(__FUNCTION__);
    return WingHex::WINGSUMMER.toLatin1();
}

QString TestPlugin::test_g() {
    emit warn(__FUNCTION__);
    return WingHex::WINGSUMMER;
}

QVariantHash TestPlugin::test_h() {
    QVariantHash hash;
    auto t = WingHex::WINGSUMMER.length();
    for (int i = 0; i < t; ++i) {
        hash.insert(WingHex::WINGSUMMER.at(i), i);
    }
    return hash;
}

bool TestPlugin::createTestShareMem(const QString &nameID) {
    if (_tsharemem) {
        return false;
    }

    if (nameID.isEmpty()) {
        return false;
    }

    _tsharemem = new QSharedMemory(nameID, this);

    if (!_tsharemem->create(20)) {
        return false;
    }

    auto buffer = _tsharemem->data();
    std::memset(buffer, 0, 20);
    const char data[] = "WingSummer";
    std::memcpy(buffer, data, sizeof(data));

    return true;
}

void TestPlugin::destoryTestShareMem() {
    if (_tsharemem) {
        _tsharemem->detach();
        _tsharemem->deleteLater();
        _tsharemem = nullptr;
    }
}

void TestPlugin::printLogTestSharedMemData() {
    emit warn(QByteArray(reinterpret_cast<const char *>(_tsharemem->data()), 20)
                  .toHex(' '));
}

void TestPlugin::setPluginMetaTestEnabled(bool b) {
    if (b != ENABLE_META) {
        ENABLE_META = b;
        for (auto &i : TestWingEditorViewWidget::instances()) {
            i->setEnableMeta(b);
            emit i->docSaved(false);
        }
    }
}

bool TestPlugin::pluginMetaTestEnabled() { return ENABLE_META; }

void TestPlugin::testCrash() {
    // if you want to reproduce nullptr deferenced,
    // you can use this example. 'volatile' is important in release mode
    // volatile int *a = nullptr;
    // (*a)++;

    abort();
}

QHash<QString, WingHex::IWingPlugin::ScriptFnInfo>
TestPlugin::registeredScriptFns() const {
    return _scriptInfo;
}

WingHex::IWingPlugin::RegisteredEvents TestPlugin::registeredEvents() const {
    RegisteredEvents evs;
    evs.setFlag(RegisteredEvent::AppReady);
    evs.setFlag(RegisteredEvent::ScriptUnSafeFnRegistering);
    return evs;
}

void TestPlugin::eventReady() {
    bool ret;
    emit invokeService(
        QStringLiteral("WingAngelAPI"), "execCode", Qt::AutoConnection,
        WINGAPI_RETURN_ARG(bool, ret),
        WINGAPI_ARG(QString, R"(print("Hello, this is TestPlugin!");)"));
}
