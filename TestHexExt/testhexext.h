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

#ifndef TESTHEXEXT_H
#define TESTHEXEXT_H

#include "WingPlugin/iwinghexeditorplugin.h"

/**
 * @brief The TestHexExt class
 * @brief This plugin will draw a line number panel
 */
class TestHexExt : public WingHex::IWingHexEditorPlugin {
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "com.wingsummer.iwinghexext" FILE "TestHexExt.json")
    Q_INTERFACES(WingHex::IWingHexEditorPlugin)

public:
    TestHexExt();

    // IWingPluginCoreBase interface
public:
    virtual bool init(const std::unique_ptr<QSettings> &set) override;
    virtual void unload(std::unique_ptr<QSettings> &set) override;

public:
    virtual const QString comment() const override;

    virtual QMenu *registeredHexContextMenu() const override;
    virtual QList<WingHex::WingRibbonToolBoxInfo>
    registeredRibbonTools() const override;

    // additional offset that applies HexEditor
    virtual QMargins
    contentMargins(WingHex::HexEditorContext *context) const override;

public:
    virtual void
    prepareCallEditorContext(WingHex::HexEditorContext *context) override;
    virtual void
    finishCallEditorContext(WingHex::HexEditorContext *context) override;

public:
    virtual void onPaintEvent(QPainter *painter, const QWidget *w,
                              WingHex::HexEditorContext *context) override;

private:
    bool isShowLinePannel(WingHex::HexEditorContext *context);

private slots:
    void setColVisible(bool b);

private:
    QMenu *m_context;
    QAction *m_aVisCol;
    WingHex::HexEditorContext *_curContext = nullptr;
};

#endif // TESTHEXEXT_H
