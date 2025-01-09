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

TestPlugin::TestPlugin()
    : WingHex::IWingPlugin(), puid(QStringLiteral("TestPlugin2")) {
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
}

TestPlugin::~TestPlugin() {}

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

const QString TestPlugin::pluginAuthor() const { return WingHex::WINGSUMMER; }

uint TestPlugin::pluginVersion() const { return WingHex::SDKVERSION; }

const QString TestPlugin::pluginComment() const {
    return tr("A Test Plugin for WingHexExplorer2.");
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

QString TestPlugin::getPuid() const { return puid; }

QVariant TestPlugin::test_a(const QVariantList &) {
    test_a();
    return {};
}

QVariant TestPlugin::test_b(const QVariantList &params) {
    if (params.isEmpty()) {
        return {};
    }
    auto arg0 = params.first().toString();
    test_b(arg0);
    return {};
}

void TestPlugin::test_a() { emit debug(__FUNCTION__); }

void TestPlugin::test_b(const QString &a) {
    emit warn(__FUNCTION__ + QStringLiteral(" : ") % a);
}

QHash<QString, WingHex::IWingPlugin::ScriptFnInfo>
TestPlugin::registeredScriptFns() const {
    return _scriptInfo;
}

WingHex::IWingPlugin::RegisteredEvents TestPlugin::registeredEvents() const {
    return RegisteredEvent::AppReady;
}

void TestPlugin::eventReady() {
    bool ret;
    emit invokeService(
        QStringLiteral("WingAngelAPI"), "execCode", Qt::AutoConnection,
        WINGAPI_RETURN_ARG(bool, ret),
        WINGAPI_ARG(QString, R"(print("Hello, this is TestPlugin!");)"));
}
