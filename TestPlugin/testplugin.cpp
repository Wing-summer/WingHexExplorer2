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

TestPlugin::TestPlugin() : puid(QStringLiteral("TestPlugin2")) {
    // 在构造函数中，所有的 API 都无法调用。插件的翻译文件也不会自动加载。
    // 在构造函数中，仅适合做一些为初始化准备的操作。

    // 插件的语言文件会在初始化前自动加载，如果初始化失败则会被卸载
    // 初始化会传递一个配置类，插件系统会统一管理放到统一的地方，使用 INI 保存
    // 你可以自行管理，但不建议，统一管理方便使用者备份和转移插件配置
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

    return true;
}

void TestPlugin::unload(std::unique_ptr<QSettings> &set) {
    // 设个数字，那就是 5 测试一下配置是否正常工作
    set->setValue("Test", 5);

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
    return {};
}

QMenu *TestPlugin::registeredHexContextMenu() const { return _tmenu; }

QList<WingHex::WingRibbonToolBoxInfo>
TestPlugin::registeredRibbonTools() const {
    return _rtbinfo;
}

QHash<WingHex::SettingPage *, bool> TestPlugin::registeredSettingPages() const {
    return {};
}

QList<WingHex::PluginPage *> TestPlugin::registeredPages() const { return {}; }

QList<WingHex::WingEditorViewWidget *>
TestPlugin::registeredEditorViewWidgets() const {
    return {};
}

QString TestPlugin::getPuid() const { return puid; }

QHash<QString, WingHex::IWingPlugin::ScriptFnInfo>
TestPlugin::registeredScriptFn() {
    return {};
}
