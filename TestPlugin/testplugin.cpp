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
#include "testsettingpage.h"
#include "testwingeditorviewwidget.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>

// 注意：所有提供的脚本接口函数都不是线程安全的，只是测试

TestPlugin::TestPlugin() : WingHex::IWingPlugin() {
    // 在构造函数中，所有的 API 都无法调用。插件的翻译文件也不会自动加载。
    // 在构造函数中，仅适合做一些为初始化准备的操作。

    // 插件的语言文件会在初始化前自动加载，如果初始化失败则会被卸载
    // 初始化会传递一个配置类，插件系统会统一管理放到统一的地方，使用 INI 保存
    // 你可以自行管理，但不建议，统一管理方便使用者备份和转移插件配置
}

TestPlugin::~TestPlugin() { destoryTestShareMem(); }

bool TestPlugin::init(const std::unique_ptr<QSettings> &set) {
    auto v = set->value("Test", 0).toInt();
    // 如果你之前启动过且正常推出，这个值一定是 5
    qDebug() << v;

    // 和日志与 UI 相关的接口此时可用，剩余的 API 初始化成功才可用
    _tform = createDialog(new TestForm(this));
    if (_tform == nullptr) {
        return false;
    }

    _tform->setMaximumHeight(500);

    using TBInfo = WingHex::WingRibbonToolBoxInfo;
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

    QIcon btnIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));

    _rtbinfo << createRibbonToolBox(
        WingHex::WingRibbonCatagories::PLUGIN,
        WingHex::createToolBox(tr("TestPlugin"), tb));

    auto rtb = WingHex::createRibbonToolBox(QStringLiteral("TestPlugin"),
                                            tr("TestPlugin"));
    for (int i = 0; i < 3; ++i) {
        TBInfo::Toolbox tbtb;
        tbtb.name = tr("TestPlugin") + QStringLiteral("(%1)").arg(i);
        for (int y = 0; y < 5; ++y) {
            tbtb.tools << createToolButton(
                btnIcon,
                tr("Button - ") + QStringLiteral("(%1, %2)").arg(i).arg(y),
                this, [this] {
                    auto tb = qobject_cast<QToolButton *>(sender());
                    msgInformation(nullptr, tr("Click"), tb->text());
                });
        }
        rtb.toolboxs.append(tbtb);
    }
    _rtbinfo.append(rtb);

    _setpages = {new TestSettingPage(QStringLiteral("Test1"),
                                     QStringLiteral("This is a Test1"), true),
                 new TestSettingPage(QStringLiteral("Test2"),
                                     QStringLiteral("This is a Test2"), false)};

    // DockWidget test
    auto lbl = new QLabel(QStringLiteral("DockTest1"));
    lbl->setAlignment(Qt::AlignCenter);
    _winfo << WingHex::createWingDockWidget(QStringLiteral("DockTest1"), lbl,
                                            Qt::LeftDockWidgetArea);

    auto ev = QSharedPointer<TestWingEditorViewWidget::Creator>::create();
    _evws.append(ev);

    _tmenu = new QMenu(QStringLiteral("TestPlugin"));
    auto micon = QIcon(QStringLiteral(":/images/TestPlugin/images/btn.png"));
    _tmenu->setIcon(micon);
    _tmenu->addAction(QStringLiteral("Raise Plugin View"), this, [this]() {
        if (_curContext) {
            auto ws = _curContext->currentPluginEditorWidgets();
            if (!ws.isEmpty()) {
                auto w = ws.first();
                w->raiseView();
            }
        }
    });
    _tmenu->addSeparator();
    for (int i = 0; i < 5; ++i) {
        auto a = new QAction(
            micon, QStringLiteral("Test - ") + QString::number(i), _tmenu);
        connect(a, &QAction::triggered, this, [this, a]() {
            msgInformation(nullptr, QStringLiteral("Test"), a->text());
        });
        _tmenu->addAction(a);
    }

    return true;
}

void TestPlugin::unload(std::unique_ptr<QSettings> &set) {
    // 设个数字，那就是 5 测试一下配置是否正常工作
    set->setValue("Test", 5);

    for (auto &p : _setpages) {
        p->deleteLater();
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

QList<WingHex::SettingPage *> TestPlugin::registeredSettingPages() const {
    return _setpages;
}

QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>>
TestPlugin::registeredEditorViewWidgets() const {
    return _evws;
}

QVector<QColor> TestPlugin::colorTable() {
    return {Qt::red, Qt::green, Qt::blue};
}

void TestPlugin::test_a() { logDebug(__FUNCTION__); }

void TestPlugin::test_b(const QString &b) {
    logWarn(__FUNCTION__ + QStringLiteral(" : ") % b);
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

    logWarn(content);
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
    logWarn(content);
}

bool TestPlugin::test_e() {
    logWarn(__FUNCTION__);
    return true;
}

QByteArray TestPlugin::test_f() {
    logWarn(__FUNCTION__);
    return "wingsummer";
}

QString TestPlugin::test_g() {
    logWarn(__FUNCTION__);
    return "wingsummer";
}

QVariantHash TestPlugin::test_h() {
    QVariantHash hash;
    constexpr auto str = "wingsummer";
    auto t = qstrlen(str);
    for (int i = 0; i < t; ++i) {
        hash.insert(QChar(str[i]), i);
    }
    return hash;
}

void TestPlugin::test_a(const WingHex::SenderInfo &sender) {
    Q_UNUSED(sender);
    test_a();
}

void TestPlugin::test_b(const WingHex::SenderInfo &sender, const QString &b) {
    Q_UNUSED(sender);
    test_b(b);
}

QString TestPlugin::test_g(const WingHex::SenderInfo &sender) {
    Q_UNUSED(sender);
    return test_g();
}

QString TestPlugin::test_i(const WingHex::SenderInfo &sender,
                           const QString &i) {
    Q_UNUSED(sender);
    logWarn(__FUNCTION__);
    logWarn(i);
    return i;
}

QString TestPlugin::test_j(const WingHex::SenderInfo &sender, int a, int b) {
    return QString::number(a + b);
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

bool TestPlugin::destoryTestShareMem() {
    if (_tsharemem) {
        // don't try to destory the share memory
        // if it's opened, it will cause freeze
        // but it's ok with `shmem`
        bool used = true;
        auto ret = invokeService(QStringLiteral("shmem"), "isShareMemUsed",
                                 qReturnArg(used));
        if (!ret || ret && !used) {
            _tsharemem->detach();
            delete _tsharemem;
            _tsharemem = nullptr;
            return true;
        }
    }
    return false;
}

void TestPlugin::printLogTestSharedMemData() {
    logWarn(QByteArray(reinterpret_cast<const char *>(_tsharemem->data()), 20)
                .toHex(' '));
}

void TestPlugin::setPluginMetaTestEnabled(bool b) {
    if (b != ENABLE_META) {
        ENABLE_META = b;
        for (auto &i : TestWingEditorViewWidget::instances()) {
            i->setEnableMeta(b);
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

WingHex::IWingPlugin::RegisteredEvents TestPlugin::registeredEvents() const {
    return packupEvent(RegisteredEvent::AppReady,
                       RegisteredEvent::HexEditorViewPaint);
}

void TestPlugin::eventReady() {
    bool ret;
    invokeService(QStringLiteral("WingAngelAPI"), "execCode",
                  Qt::AutoConnection, qReturnArg(ret),
                  QStringLiteral(R"(println("Hello, this is TestPlugin!");)"));
}

void TestPlugin::onRegisterScriptObj(WingScriptBind::ScopeBuilder scope) {
    scope.addFunction("test_a", [this] { test_a(); });
    scope.addFunction("test_b",
                      [this](const QString &value) { test_b(value); });
    scope.addFunction("test_c",
                      [this](const QVector<int> &value) { test_c(value); });
    scope.addFunction("test_d",
                      [this](const QVariantHash &value) { test_d(value); });
    scope.addFunction("test_e", [this] { return test_e(); });
    scope.addFunction("test_f", [this] { return test_f(); });
    scope.addFunction("test_g", [this] { return test_g(); });
    scope.addFunction("test_h", [this] { return test_h(); });
    scope.addFunction("createTestShareMem", [this](const QString &name) {
        return createTestShareMem(name);
    });
    scope.addFunction("destoryTestShareMem",
                      [this] { return destoryTestShareMem(); });
    scope.addFunction("printLogTestSharedMemData",
                      [this] { printLogTestSharedMemData(); });
    scope.addFunction("colorTable", [this] { return colorTable(); });
    scope.addFunction("colorTableNew", [this] { return colorTable(); });
    scope.addFunction("setPluginMetaTestEnabled", [this](bool enabled) {
        setPluginMetaTestEnabled(enabled);
    });
    scope.addFunction("pluginMetaTestEnabled",
                      [this] { return pluginMetaTestEnabled(); });
    scope.addFunction("testCrash", [this] { testCrash(); });
    scope.addFunction("testAdd",
                      [](int left, int right) { return left + right; });
    scope.addFunction("raiseScriptException", [] {
        throw WingScriptBind::ScriptException(
            WingScriptBind::Exception::RuntimeError,
            QStringLiteral("raiseScriptException"));
    });

    struct TestObj {
        TestObj() = default;
        explicit TestObj(int value) : _v(value) {}
        int getV() const { return _v; }
        int _v = 0;
    };
    scope.beginClass<TestObj>("TestObj")
        .addConstructor<void (*)(), void (*)(int)>()
        .addFunction("getV", &TestObj::getV)
        .addData("value", &TestObj::_v);
}

void TestPlugin::onPaintHexEditorView(QPainter *painter, QWidget *w,
                                      WingHex::HexEditorContext *context) {
    Q_UNUSED(context);
    painter->save();
    auto font = painter->font();
    auto metric = QFontMetrics(font);
    auto str = QStringLiteral("TestPlugin");
    auto len = metric.horizontalAdvance(str);
    auto height = metric.height();
    auto pen = painter->pen();
    auto color = pen.color();
    color.setAlpha(180);
    pen.setColor(color);
    painter->setPen(pen);
    constexpr auto padding = 2;
    auto rect = w->rect();
    painter->drawText(rect.right() - len - padding, height + padding, str);
    painter->restore();
}

void TestPlugin::prepareCallEditorContext(WingHex::HexEditorContext *context) {
    _curContext = context;
}

void TestPlugin::finishCallEditorContext(WingHex::HexEditorContext *) {
    _curContext = nullptr;
}
