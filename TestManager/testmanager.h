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

#ifndef TESTMANAGER_H
#define TESTMANAGER_H

#include "WingPlugin/iwingmanager.h"

#include <QCheckBox>
#include <QVBoxLayout>

class TestManager : public WingHex::IWingManager {
    Q_OBJECT

    // 这两行是必须，只有后面的 "TestManager.json" 你根据需要修改，
    // 具体可见 QT 文档，剩下直接 CV 大法
    Q_PLUGIN_METADATA(IID "com.wingsummer.iwingmanager" FILE "TestManager.json")
    Q_INTERFACES(WingHex::IWingManager)
public:
    explicit TestManager();
    virtual ~TestManager();

    // IWingPluginCoreBase interface
public:
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;

public:
    virtual const QString comment() const override;

    virtual QList<WingHex::SettingPage *>
    registeredSettingPages() const override;

private:
    class TestPage : public WingHex::SettingPage {
    public:
        TestPage() {
            auto layout = new QVBoxLayout(this);
            _cbblk = new QCheckBox(QStringLiteral("Disable msg*"), this);
            _cbblk->setChecked(false);
            layout->addWidget(_cbblk);
            layout->addStretch();
        }
        // PageBase interface
    public:
        virtual QIcon categoryIcon() const override {
            return QIcon(WingHex::HOSTRESPIMG("monitor"));
        }
        virtual QString name() const override {
            return QStringLiteral("TestManager");
        }
        virtual QString id() const override {
            return QStringLiteral("TestManager");
        }

        // SettingInterface interface
    public:
        virtual void apply() override { _isDisabled = _cbblk->isChecked(); }
        virtual void reset() override {
            _isDisabled = false;
            _cbblk->setChecked(false);
        }
        virtual void cancel() override { _cbblk->setChecked(_isDisabled); }

    public:
        bool isDisableMsg() const { return _isDisabled; }

    private:
        QCheckBox *_cbblk;
        bool _isDisabled = false;
    };

public slots:
    virtual bool enterGuard(const QMetaObject *sender, const QString &sig,
                            const QVariantList &params) override;

private:
    TestPage *content;
};

#endif // TESTMANAGER_H
