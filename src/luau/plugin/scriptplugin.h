/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#ifndef SCRIPTPLUGIN_H
#define SCRIPTPLUGIN_H

#include "WingPlugin/iwingdevice.h"
#include "WingPlugin/iwingplugin.h"

#include "scriptpluginruntime.h"

class ScriptPlugin final : public WingHex::IWingPlugin {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingPlugin)
public:
    explicit ScriptPlugin(std::shared_ptr<ScriptPluginPackage> package);

public:
    inline ScriptPluginRuntime *runtime() const { return m_runtime.get(); }

public:
    ScriptPluginResult prepare();
    void onRegisterScriptObj(WingScriptBind::ScopeBuilder scope) override;
    bool init(const std::unique_ptr<QSettings> &settings) override;
    void unload(std::unique_ptr<QSettings> &settings) override;
    const QString pluginName() const override;
    const QString pluginComment() const override;
    QIcon pluginIcon() const override;
    QString retranslate(const QString &text) override;
    RegisteredEvents registeredEvents() const override;
    void eventReady() override;
    bool eventClosing() override;
    void eventSelectionChanged(const QByteArrayList &selections,
                               bool preview) override;
    void
    eventCursorPositionChanged(const WingHex::HexPosition &position) override;
    void eventPluginFile(PluginFileEvent event, FileType type, const QUrl &url,
                         int handle, const QUrl &oldUrl) override;
    QMenu *registeredHexContextMenu() const override;
    void onPaintHexEditorView(QPainter *painter, QWidget *widget,
                              WingHex::HexEditorContext *context) override;
    QList<WingHex::SettingPage *> registeredSettingPages() const override;
    QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>>
    registeredEditorViewWidgets() const override;
    QList<WingHex::WingRibbonToolBoxInfo>
    registeredRibbonTools() const override;
    QList<WingHex::WingDockWidgetInfo> registeredDockWidgets() const override;

private:
    std::shared_ptr<ScriptPluginRuntime> m_runtime;
    QString m_name, m_comment;
    QIcon m_icon;
    bool m_registered = false;
    QPointer<QMenu> m_menu;
    QList<WingHex::WingRibbonToolBoxInfo> m_ribbon;
    QList<WingHex::WingDockWidgetInfo> m_docks;
    QList<WingHex::SettingPage *> m_pages;
    QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>> m_views;
};

class ScriptDevicePlugin final : public WingHex::IWingDevice {
    Q_OBJECT
    Q_INTERFACES(WingHex::IWingDevice)
public:
    explicit ScriptDevicePlugin(std::shared_ptr<ScriptPluginPackage> package);

public:
    inline ScriptPluginRuntime *runtime() const { return m_runtime.get(); }

public:
    ScriptPluginResult prepare();
    void onRegisterScriptObj(WingScriptBind::ScopeBuilder scope) override;
    bool init(const std::unique_ptr<QSettings> &settings) override;
    void unload(std::unique_ptr<QSettings> &settings) override;
    const QString pluginName() const override;
    const QString pluginComment() const override;
    QIcon pluginIcon() const override;
    QString retranslate(const QString &text) override;
    QString supportedFileExtDisplayName() const override;
    QIcon supportedFileIcon() const override;
    QString onOpenFileBegin() override;
    WingHex::WingIODevice *onOpenFile(const QString &path) override;
    bool onCloseFile(WingHex::WingIODevice *device) override;

private:
    std::shared_ptr<ScriptPluginRuntime> m_runtime;
    QString m_name, m_comment;
    QIcon m_icon;
    QString m_deviceName;
    QIcon m_deviceIcon;
};

ScriptPluginRuntime *scriptPluginRuntime(const QObject *object);

#endif // SCRIPTPLUGIN_H
