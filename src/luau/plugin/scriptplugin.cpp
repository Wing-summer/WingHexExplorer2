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

#include "scriptplugin.h"

#include <QMenu>
#include <QPainter>
#include <QThread>
#include <QToolButton>
#include <QVBoxLayout>

#include "WingPlugin/hexeditorcontext.h"
#include "WingPlugin/settingpage.h"
#include "qtluau/value_conversion.hpp"
#include "qtluau/widget_wrappers.hpp"

#include "pluginpaintcontext.h"
#include "qt_extensions.hpp"

namespace {
template <class T>
std::optional<T> decode(const luabridge::LuaRef &value) {
    try {
        return qtluau::qtFromLua<T>(value);
    } catch (const std::exception &) {
        return std::nullopt;
    }
}

QString text(ScriptPluginRuntime &runtime, const char *method) {
    const auto result = runtime.call(method);
    if (result.isNil()) {
        return {};
    }
    if (const auto value = decode<QString>(result)) {
        return *value;
    }
    runtime.fail(QStringLiteral("Invalid string result: %1")
                     .arg(QString::fromUtf8(method)));
    return {};
}

QIcon icon(ScriptPluginRuntime &runtime, const char *method) {
    const auto result = runtime.call(method);
    if (result.isNil()) {
        return {};
    }
    if (const auto path = decode<QString>(result)) {
        return QIcon(*path);
    }
    if (const auto value = decode<QIcon>(result)) {
        return *value;
    }
    runtime.fail(QStringLiteral("Invalid icon result"));
    return {};
}

bool boolean(ScriptPluginRuntime &runtime, const luabridge::LuaRef &result,
             bool fallback = false) {
    if (result.isNil()) {
        return fallback;
    }
    if (const auto value = result.cast<bool>()) {
        return *value;
    }
    runtime.fail(QStringLiteral("Invalid boolean callback result"));
    return false;
}

QWidget *widget(ScriptPluginRuntime &runtime, const luabridge::LuaRef &value) {
    const auto result = value.cast<qtluau::QtWidgetRef>();
    if (!result) {
        throw std::runtime_error("Expected a Qt widget");
    }
    auto *object = result->get();
    if (!object || object->thread() != QThread::currentThread()) {
        throw std::runtime_error("Invalid plugin widget");
    }
    runtime.trackWidget(object);
    return object;
}

QString stringField(ScriptPluginRuntime &runtime,
                    const luabridge::LuaRef &object, const char *name) {
    const auto value = decode<QString>(runtime.field(object, name));
    if (!value) {
        throw std::runtime_error("Missing plugin description string");
    }
    return *value;
}

const QList<luabridge::LuaRef> entries(const luabridge::LuaRef &array) {
    QList<luabridge::LuaRef> result;
    if (array.isNil()) {
        return result;
    }
    if (!array.isTable() || array.length() > 4096) {
        throw std::runtime_error("Invalid plugin description list");
    }
    for (int i = 1; i <= array.length(); ++i) {
        luabridge::LuaRef item = array[i];
        result.append(item);
    }
    return result;
}

class ScriptSettingPage final : public WingHex::SettingPage {
public:
    ScriptSettingPage(std::shared_ptr<ScriptPluginRuntime> runtime,
                      luabridge::LuaRef description)
        : m_runtime(std::move(runtime)), m_description(std::move(description)) {
        m_id = stringField(*m_runtime, m_description, "id");
        m_name = stringField(*m_runtime, m_description, "name");
        auto *content =
            widget(*m_runtime, m_runtime->field(m_description, "widget"));
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(content);
        m_runtime->trackWidget(this);
    }

    QString name() const override { return m_name; }

    QString id() const override { return m_id; }

    QIcon categoryIcon() const override { return {}; }

    void restore() override { m_runtime->callObject(m_description, "restore"); }

    void discard() override { m_runtime->callObject(m_description, "discard"); }

    bool containUnsavedChanges() const override {
        return boolean(
            *m_runtime,
            m_runtime->callObject(m_description, "contain_unsaved_changes"));
    }

    void highlightUnsavedChange() override {
        m_runtime->callObject(m_description, "highlight_unsaved_change");
    }

private:
    std::shared_ptr<ScriptPluginRuntime> m_runtime;
    luabridge::LuaRef m_description;
    QString m_id, m_name;
};
class ScriptViewCreator;

class ScriptView final : public WingHex::WingEditorViewWidget {
public:
    ScriptView(QSharedPointer<const ScriptViewCreator> creator,
               std::shared_ptr<ScriptPluginRuntime> runtime,
               luabridge::LuaRef object, QWidget *parent);
    WingHex::WingEditorViewWidget *clone() override;

    void toggled(bool visible) override {
        m_runtime->callObject(m_object, "toggled", {m_runtime->value(visible)});
    }

    void loadState(const QByteArray &state) override {
        m_runtime->callObject(m_object, "load_state",
                              {qtluau::qtToLua(m_runtime->state(), state)});
    }

    QByteArray saveState() override {
        const auto value =
            decode<QByteArray>(m_runtime->callObject(m_object, "save_state"));
        return value ? *value : QByteArray();
    }

    bool hasUnsavedState() override {
        return boolean(*m_runtime,
                       m_runtime->callObject(m_object, "has_unsaved_state"));
    }

    void setSaved() override { m_runtime->callObject(m_object, "set_saved"); }

    bool onClosing() override {
        return boolean(*m_runtime,
                       m_runtime->callObject(m_object, "on_closing"), true);
    }

    void onWorkSpaceNotify(bool workspace) override {
        m_runtime->callObject(m_object, "on_work_space_notify",
                              {m_runtime->value(workspace)});
    }

private:
    QSharedPointer<const ScriptViewCreator> m_creator;
    std::shared_ptr<ScriptPluginRuntime> m_runtime;
    luabridge::LuaRef m_object;
};

class ScriptViewCreator final
    : public WingHex::WingEditorViewWidget::Creator,
      public QEnableSharedFromThis<ScriptViewCreator> {
public:
    ScriptViewCreator(std::shared_ptr<ScriptPluginRuntime> runtime,
                      luabridge::LuaRef description)
        : m_runtime(std::move(runtime)), m_description(std::move(description)) {
        m_id = stringField(*m_runtime, m_description, "id");
        m_name = stringField(*m_runtime, m_description, "name");
    }

    QIcon icon() const override { return {}; }

    QString name() const override { return m_name; }

    QString id() const override { return m_id; }

    WingHex::WingEditorViewWidget *create(QWidget *parent) const override {
        const auto object = m_runtime->callObject(m_description, "create");
        if (!object.isTable()) {
            m_runtime->fail(QStringLiteral("View factory must return a table"));
            return nullptr;
        }
        try {
            return new ScriptView(sharedFromThis(), m_runtime, object, parent);
        } catch (const std::exception &error) {
            m_runtime->fail(QString::fromUtf8(error.what()));
            return nullptr;
        }
    }

private:
    std::shared_ptr<ScriptPluginRuntime> m_runtime;
    luabridge::LuaRef m_description;
    QString m_id, m_name;
};

ScriptView::ScriptView(QSharedPointer<const ScriptViewCreator> creator,
                       std::shared_ptr<ScriptPluginRuntime> runtime,
                       luabridge::LuaRef object, QWidget *parent)
    : WingEditorViewWidget(creator.data(), parent),
      m_creator(std::move(creator)), m_runtime(std::move(runtime)),
      m_object(std::move(object)) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widget(*m_runtime, m_runtime->field(m_object, "widget")));
}

WingHex::WingEditorViewWidget *ScriptView::clone() {
    return m_creator->create(parentWidget());
}

class ScriptDevice final : public WingHex::WingIODevice {
public:
    ScriptDevice(std::shared_ptr<ScriptPluginRuntime> runtime,
                 luabridge::LuaRef object)
        : m_runtime(std::move(runtime)), m_object(std::move(object)) {
        const auto writable =
            m_runtime->field(m_object, "writable").cast<bool>();
        const auto sequential =
            m_runtime->field(m_object, "sequential").cast<bool>();
        m_sequential = sequential && *sequential;
        for (const auto *name : {"read", "write", "seek", "size", "close"}) {
            const auto method = m_runtime->field(m_object, name);
            if (!method.isNil() && !method.isFunction()) {
                throw std::runtime_error("Device callback is not a function");
            }
            if (method.isNil() &&
                (QByteArray(name) == "read" || QByteArray(name) == "size" ||
                 (QByteArray(name) == "write" && writable && *writable) ||
                 (QByteArray(name) == "seek" && !m_sequential))) {
                throw std::runtime_error("Missing device callback");
            }
            m_methods.insert(name, method);
        }
        open(writable && *writable ? QIODevice::ReadWrite
                                   : QIODevice::ReadOnly);
    }

    luabridge::LuaRef object() const { return m_object; }

    bool belongsTo(const std::shared_ptr<ScriptPluginRuntime> &runtime) const {
        return runtime == m_runtime;
    }

    bool closeDevice() {
        if (!m_runtime->active()) {
            close();
            return true;
        }
        const auto result = call("close");
        const bool accepted = boolean(*m_runtime, result, true);
        if (accepted) {
            close();
        }
        return accepted;
    }

    bool isSequential() const override { return m_sequential; }

    bool keepSize() const override {
        return boolean(*m_runtime, m_runtime->field(m_object, "keep_size"));
    }

    bool defaultIsKeepSize() const override {
        return boolean(*m_runtime, m_runtime->field(m_object, "keep_size"));
    }

    bool defaultIsLocked() const override {
        return boolean(*m_runtime, m_runtime->field(m_object, "locked"));
    }

    bool defaultIsOverwrite() const override {
        return boolean(*m_runtime, m_runtime->field(m_object, "overwrite"));
    }

    qint64 size() const override {
        if (!m_runtime->active()) {
            return 0;
        }
        const auto result = call("size");
        const auto value = decode<qint64>(result);
        if (!value || *value < 0) {
            m_runtime->fail(QStringLiteral("Invalid device size"));
            return 0;
        }
        return *value;
    }

    bool seek(qint64 position) override {
        if (!m_runtime->active() || m_sequential || position < 0) {
            return false;
        }
        const auto result =
            call("seek", {qtluau::qtToLua(m_runtime->state(), position)});
        return boolean(*m_runtime, result) && QIODevice::seek(position);
    }

protected:
    qint64 readData(char *destination, qint64 maximum) override {
        if (!m_runtime->active()) {
            setErrorString(m_runtime->error());
            return -1;
        }
        const auto result =
            call("read", {qtluau::qtToLua(m_runtime->state(), maximum)});
        if (result.isNil() && m_runtime->active()) {
            const auto message =
                decode<QString>(m_runtime->field(m_object, "error_string"));
            setErrorString(message ? *message
                                   : QStringLiteral("Device read failed"));
            return -1;
        }
        const auto bytes = decode<QByteArray>(result);
        if (!bytes || bytes->size() > maximum) {
            m_runtime->fail(QStringLiteral("Invalid device read result"));
            setErrorString(m_runtime->error());
            return -1;
        }
        std::memcpy(destination, bytes->constData(), size_t(bytes->size()));
        return bytes->size();
    }

    qint64 writeData(const char *source, qint64 length) override {
        if (!m_runtime->active()) {
            setErrorString(m_runtime->error());
            return -1;
        }
        const auto result = call(
            "write",
            {qtluau::qtToLua(m_runtime->state(), QByteArray(source, length))});
        const auto written = decode<qint64>(result);
        if (!written || *written < -1 || *written > length) {
            m_runtime->fail(QStringLiteral("Invalid device write result"));
            setErrorString(m_runtime->error());
            return -1;
        }
        return *written;
    }

private:
    luabridge::LuaRef
    call(const char *name,
         const QList<luabridge::LuaRef> &arguments = {}) const {
        return m_runtime->callFunction(
            m_object,
            m_methods.value(name, luabridge::LuaRef(m_runtime->state())),
            arguments);
    }

    std::shared_ptr<ScriptPluginRuntime> m_runtime;
    QHash<QByteArray, luabridge::LuaRef> m_methods;
    luabridge::LuaRef m_object;
    bool m_sequential = false;
};
} // namespace

ScriptPlugin::ScriptPlugin(std::shared_ptr<ScriptPluginPackage> package)
    : m_runtime(
          std::make_shared<ScriptPluginRuntime>(std::move(package), this)) {}

ScriptPluginResult ScriptPlugin::prepare() {
    QString error;
    const auto prepared = m_runtime->prepare();
    if (!prepared) {
        return prepared;
    }
    m_name = text(*m_runtime, "plugin_name");
    m_comment = text(*m_runtime, "plugin_comment");
    m_icon = icon(*m_runtime, "plugin_icon");
    if (!m_runtime->active() || m_name.trimmed().isEmpty()) {
        error = QStringLiteral("Invalid plugin metadata");
        return {ScriptPluginError::Initialization, error, {}};
    }
    return {};
}

bool ScriptPlugin::init(const std::unique_ptr<QSettings> &settings) {
    if (!m_runtime->initialize(settings.get())) {
        return false;
    }
    try {
        m_ribbon = registeredRibbonTools();
        m_docks = registeredDockWidgets();
        m_menu = registeredHexContextMenu();
        m_pages = registeredSettingPages();
        m_views = registeredEditorViewWidgets();
        m_registered = true;
        return m_runtime->active();
    } catch (const std::exception &error) {
        m_runtime->fail(QString::fromUtf8(error.what()));
        m_runtime->stop();
        return false;
    }
}

void ScriptPlugin::unload(std::unique_ptr<QSettings> &) { m_runtime->stop(); }

const QString ScriptPlugin::pluginName() const { return m_name; }

const QString ScriptPlugin::pluginComment() const { return m_comment; }

QIcon ScriptPlugin::pluginIcon() const { return m_icon; }

QString ScriptPlugin::retranslate(const QString &value) {
    const auto result = m_runtime->call(
        "retranslate", {qtluau::qtToLua(m_runtime->state(), value)});
    const auto translated = decode<QString>(result);
    return translated ? *translated : value;
}

ScriptPlugin::RegisteredEvents ScriptPlugin::registeredEvents() const {
    const auto result = m_runtime->call("registered_events");
    if (result.isNil()) {
        return {};
    }
    const auto mask = result.cast<unsigned int>();
    if (!mask) {
        m_runtime->fail(QStringLiteral("Invalid registered event mask"));
        return {};
    }
    return RegisteredEvents::fromInt(int(*mask));
}

void ScriptPlugin::eventReady() { m_runtime->enqueueEvent("event_ready"); }

bool ScriptPlugin::eventClosing() {
    return boolean(*m_runtime, m_runtime->call("event_closing"), true);
}

void ScriptPlugin::eventSelectionChanged(const QByteArrayList &selections,
                                         bool preview) {
    m_runtime->enqueueEvent("event_selection_changed",
                            {qtluau::qtToLua(m_runtime->state(), selections),
                             m_runtime->value(preview)},
                            true);
}

void ScriptPlugin::eventCursorPositionChanged(
    const WingHex::HexPosition &position) {
    m_runtime->enqueueEvent("event_cursor_position_changed",
                            {qtluau::qtToLua(m_runtime->state(), position)},
                            true);
}

void ScriptPlugin::eventPluginFile(PluginFileEvent event, FileType type,
                                   const QUrl &url, int handle,
                                   const QUrl &oldUrl) {
    m_runtime->enqueueEvent(
        "event_plugin_file",
        {m_runtime->value(int(event)), m_runtime->value(int(type)),
         qtluau::qtToLua(m_runtime->state(), url), m_runtime->value(handle),
         qtluau::qtToLua(m_runtime->state(), oldUrl)});
}

QMenu *ScriptPlugin::registeredHexContextMenu() const {
    if (m_registered) {
        return m_menu;
    }
    const auto result = m_runtime->call("registered_hex_context_menu");
    if (result.isNil()) {
        return nullptr;
    }
    const auto object = result.cast<qtluau::QtWidgetRef>();
    if (object) {
        return qobject_cast<QMenu *>(object->get());
    }
    m_runtime->fail(QStringLiteral("Context menu must be a QMenu"));
    return nullptr;
}

QList<WingHex::WingRibbonToolBoxInfo>
ScriptPlugin::registeredRibbonTools() const {
    if (m_registered) {
        return m_ribbon;
    }
    QList<WingHex::WingRibbonToolBoxInfo> result;
    for (const auto &description :
         entries(m_runtime->call("registered_ribbon_tools"))) {
        WingHex::WingRibbonToolBoxInfo item;
        item.catagory = stringField(*m_runtime, description, "category");
        item.displayName = stringField(*m_runtime, description, "display_name");
        for (const auto &box :
             entries(m_runtime->field(description, "toolboxes"))) {
            WingHex::WingRibbonToolBoxInfo::Toolbox toolbox;
            toolbox.name = stringField(*m_runtime, box, "name");
            for (const auto &tool : entries(m_runtime->field(box, "tools"))) {
                auto *button =
                    qobject_cast<QToolButton *>(widget(*m_runtime, tool));
                if (!button) {
                    throw std::runtime_error(
                        "Ribbon tool must be a QToolButton");
                }
                toolbox.tools.append(button);
            }
            item.toolboxs.append(toolbox);
        }
        result.append(item);
    }
    return result;
}

QList<WingHex::WingDockWidgetInfo> ScriptPlugin::registeredDockWidgets() const {
    if (m_registered) {
        return m_docks;
    }
    QList<WingHex::WingDockWidgetInfo> result;
    for (const auto &description :
         entries(m_runtime->call("registered_dock_widgets"))) {
        WingHex::WingDockWidgetInfo item;
        item.widgetName = stringField(*m_runtime, description, "id");
        item.displayName = stringField(*m_runtime, description, "name");
        item.widget =
            widget(*m_runtime, m_runtime->field(description, "widget"));
        const auto area = m_runtime->field(description, "area").cast<int>();
        item.area = area ? Qt::DockWidgetArea(*area) : Qt::NoDockWidgetArea;
        result.append(item);
    }
    return result;
}

QList<WingHex::SettingPage *> ScriptPlugin::registeredSettingPages() const {
    if (m_registered) {
        return m_pages;
    }
    QList<WingHex::SettingPage *> result;
    for (const auto &description :
         entries(m_runtime->call("registered_setting_pages"))) {
        result.append(new ScriptSettingPage(m_runtime, description));
    }
    return result;
}

QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>>
ScriptPlugin::registeredEditorViewWidgets() const {
    if (m_registered) {
        return m_views;
    }
    QList<QSharedPointer<WingHex::WingEditorViewWidget::Creator>> result;
    for (const auto &description :
         entries(m_runtime->call("registered_editor_view_widgets"))) {
        result.append(
            QSharedPointer<ScriptViewCreator>::create(m_runtime, description));
    }
    return result;
}

void ScriptPlugin::onPaintHexEditorView(QPainter *painter, QWidget *target,
                                        WingHex::HexEditorContext *editor) {
    if (!m_runtime->active()) {
        return;
    }
    auto *context = new PluginPaintContext(painter, nullptr);
    painter->save();
    qtluau::trackContextObject(m_runtime->state(), context);
    qtluau::pushExtensionObject(m_runtime->state(), context, true);
    auto argument = luabridge::LuaRef::fromStack(m_runtime->state(), -1);
    lua_pop(m_runtime->state(), 1);
    QVariantMap snapshot;
    if (editor) {
        snapshot.insert(QStringLiteral("document_lines"),
                        qint64(editor->documentLines()));
        snapshot.insert(QStringLiteral("document_last_line"),
                        qint64(editor->documentLastLine()));
        snapshot.insert(QStringLiteral("document_last_column"),
                        qint64(editor->documentLastColumn()));
        snapshot.insert(QStringLiteral("document_width"),
                        editor->documentWidth());
        snapshot.insert(QStringLiteral("line_height"), editor->lineHeight());
        snapshot.insert(QStringLiteral("border_size"), editor->borderSize());
        snapshot.insert(QStringLiteral("hex_line_width"),
                        editor->hexLineWidth());
        snapshot.insert(QStringLiteral("area_indent"), editor->areaIndent());
        snapshot.insert(QStringLiteral("address_width"),
                        editor->addressWidth());
        snapshot.insert(QStringLiteral("header_height"),
                        editor->headerHeight());
        snapshot.insert(QStringLiteral("hex_column_x"), editor->hexColumnX());
        snapshot.insert(QStringLiteral("string_column_x"),
                        editor->stringColumnX());
        snapshot.insert(QStringLiteral("end_column_x"), editor->endColumnX());
        snapshot.insert(QStringLiteral("cell_width"), editor->cellWidth());
        snapshot.insert(QStringLiteral("current_line"),
                        qint64(editor->currentLine()));
        snapshot.insert(QStringLiteral("current_column"),
                        editor->currentColumn());
        snapshot.insert(QStringLiteral("current_nibble"),
                        editor->currentNibble());
        snapshot.insert(QStringLiteral("current_selection_length"),
                        qint64(editor->currentSelectionLength()));
        snapshot.insert(QStringLiteral("selection_count"),
                        qint64(editor->selectionCount()));
        snapshot.insert(QStringLiteral("has_selection"),
                        editor->hasSelection());
        snapshot.insert(QStringLiteral("has_internal_selection"),
                        editor->hasInternalSelection());
        snapshot.insert(QStringLiteral("insertion_mode"),
                        editor->isInInsertionMode());
        snapshot.insert(QStringLiteral("string_area_visible"),
                        editor->stringAreaVisible());
        snapshot.insert(QStringLiteral("address_area_visible"),
                        editor->addressAreaVisible());
        snapshot.insert(QStringLiteral("header_area_visible"),
                        editor->headerAreaVisible());
        snapshot.insert(QStringLiteral("header_color"), editor->headerColor());
        snapshot.insert(QStringLiteral("address_color"),
                        editor->addressColor());
        snapshot.insert(QStringLiteral("bytes_background"),
                        editor->bytesBackground());
        snapshot.insert(QStringLiteral("bytes_alter_background"),
                        editor->bytesAlterBackground());
        snapshot.insert(QStringLiteral("bytes_color"), editor->bytesColor());
        snapshot.insert(QStringLiteral("selection_color"),
                        editor->selectionColor());
        snapshot.insert(QStringLiteral("selection_background"),
                        editor->selBackgroundColor());
        snapshot.insert(QStringLiteral("border_color"), editor->borderColor());
        snapshot.insert(QStringLiteral("doc_file_name"),
                        editor->docFileName().toString());
    }
    auto state = qtluau::variantToLua(m_runtime->state(), snapshot);
    state.push();
    lua_setreadonly(m_runtime->state(), -1, true);
    lua_pop(m_runtime->state(), 1);
    m_runtime->call("on_paint_hex_editor_view",
                    {argument,
                     luabridge::LuaRef(m_runtime->state(),
                                       qtluau::QtWidgetRef(target, false)),
                     state});
    context->expire();
    painter->restore();
}

ScriptDevicePlugin::ScriptDevicePlugin(
    std::shared_ptr<ScriptPluginPackage> package)
    : m_runtime(
          std::make_shared<ScriptPluginRuntime>(std::move(package), this)) {}

ScriptPluginResult ScriptDevicePlugin::prepare() {
    QString error;
    const auto prepared = m_runtime->prepare();
    if (!prepared) {
        return prepared;
    }
    m_name = text(*m_runtime, "plugin_name");
    m_comment = text(*m_runtime, "plugin_comment");
    m_icon = icon(*m_runtime, "plugin_icon");
    if (!m_runtime->active() || m_name.trimmed().isEmpty()) {
        error = QStringLiteral("Invalid device plugin metadata");
        return {ScriptPluginError::Initialization, error, {}};
    }
    return {};
}

bool ScriptDevicePlugin::init(const std::unique_ptr<QSettings> &settings) {
    if (!m_runtime->initialize(settings.get())) {
        return false;
    }
    m_deviceName = text(*m_runtime, "supported_file_ext_display_name");
    m_deviceIcon = icon(*m_runtime, "supported_file_icon");
    if (m_deviceName.trimmed().isEmpty()) {
        m_runtime->fail(QStringLiteral("Invalid device display name"));
    }
    return m_runtime->active();
}

void ScriptDevicePlugin::unload(std::unique_ptr<QSettings> &) {
    m_runtime->stop();
}

const QString ScriptDevicePlugin::pluginName() const { return m_name; }

const QString ScriptDevicePlugin::pluginComment() const { return m_comment; }

QIcon ScriptDevicePlugin::pluginIcon() const { return m_icon; }

QString ScriptDevicePlugin::retranslate(const QString &value) {
    const auto result = m_runtime->call(
        "retranslate", {qtluau::qtToLua(m_runtime->state(), value)});
    const auto translated = decode<QString>(result);
    return translated ? *translated : value;
}

QString ScriptDevicePlugin::supportedFileExtDisplayName() const {
    return m_deviceName;
}

QIcon ScriptDevicePlugin::supportedFileIcon() const { return m_deviceIcon; }

QString ScriptDevicePlugin::onOpenFileBegin() {
    return text(*m_runtime, "on_open_file_begin");
}

WingHex::WingIODevice *ScriptDevicePlugin::onOpenFile(const QString &path) {
    const auto result = m_runtime->call(
        "on_open_file", {qtluau::qtToLua(m_runtime->state(), path)});
    if (result.isNil()) {
        return nullptr;
    }
    if (!result.isTable()) {
        m_runtime->fail(QStringLiteral("Device open must return a table"));
        return nullptr;
    }
    try {
        return new ScriptDevice(m_runtime, result);
    } catch (const std::exception &error) {
        m_runtime->fail(QString::fromUtf8(error.what()));
        return nullptr;
    }
}

bool ScriptDevicePlugin::onCloseFile(WingHex::WingIODevice *device) {
    auto *script = dynamic_cast<ScriptDevice *>(device);
    if (!script || !script->belongsTo(m_runtime)) {
        return false;
    }
    if (m_runtime->active() &&
        !boolean(*m_runtime,
                 m_runtime->call("on_close_file", {script->object()}))) {
        return false;
    }
    return script->closeDevice();
}

ScriptPluginRuntime *scriptPluginRuntime(const QObject *object) {
    if (auto *plugin = qobject_cast<const ScriptPlugin *>(object)) {
        return plugin->runtime();
    }
    if (auto *device = qobject_cast<const ScriptDevicePlugin *>(object)) {
        return device->runtime();
    }
    return nullptr;
}

void ScriptPlugin::onRegisterScriptObj(WingScriptBind::ScopeBuilder scope) {
    m_runtime->registerDefinitions(scope);
}

void ScriptDevicePlugin::onRegisterScriptObj(
    WingScriptBind::ScopeBuilder scope) {
    m_runtime->registerDefinitions(scope);
}
