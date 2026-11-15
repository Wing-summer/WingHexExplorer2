"""Generate plugin callback lookup and Luau protocols from one contract."""
from __future__ import annotations

import json
import pathlib
import re


def generate(root: pathlib.Path, output: pathlib.Path) -> None:
    """Emit a validated callback contract, helpers and deployable definitions."""
    protocol = json.loads((root / "codegen/plugin_protocol.json").read_text())
    callbacks = protocol["callbacks"]
    names = [entry["name"] for entry in callbacks]
    if len(names) != len(set(names)):
        raise ValueError("Duplicate plugin callback")
    header = ["#pragma once", "struct PluginCallbackDescriptor { const char *name; bool required; bool deviceOnly; bool ordinaryOnly; const char *result; };", "inline constexpr PluginCallbackDescriptor pluginCallbackDescriptors[] = {"]
    for entry in callbacks:
        flags = [str(entry.get(key, False)).lower() for key in ("required", "device_only", "ordinary_only")]
        header.append('{"' + entry["name"] + '", ' + ", ".join(flags) + ', "' + entry["returns"] + '"},')
    header.append("};")
    base = ["local Base = {}", "Base.__index = Base", "function Base:extend() local child = {}; child.__index = child; return setmetatable(child, self) end", "function Base:new() return setmetatable({}, self) end"]
    base.extend(["local Ordinary = Base:extend()", "local Device = Base:extend()"] )
    for entry in callbacks:
        if "default" in entry:
            arguments = [argument.split(":", 1)[0] for argument in entry.get("arguments", [])]
            target = "Device" if entry.get("device_only") else "Ordinary" if entry.get("ordinary_only") else "Base"
            base.append("function " + target + ":" + entry["name"] + "(" + ", ".join(arguments) + ") " + entry["default"] + " end")
    base.extend(["plugin.Plugin = Ordinary", "plugin.DevicePlugin = Device"])
    header.append('inline constexpr char pluginBaseSource[] = R"WING(' + "\n".join(base) + ')WING";')
    (output / "generated/plugin_protocol.hpp").write_text("\n".join(header) + "\n")
    registration = ["// Generated plugin helper registration."]
    helper_types: list[str] = []
    for group in protocol["helpers"]:
        scope = group["scope"]
        if not re.fullmatch(r"[a-z_][a-z_0-9]*", scope):
            raise ValueError("Invalid helper namespace")
        if group.get("merge"):
            registration.extend(['lua_getfield(m_state, -1, "' + scope + '");', 'if (!lua_istable(m_state, -1)) { lua_pop(m_state, 1); lua_newtable(m_state); }'])
        else:
            registration.append("lua_newtable(m_state);")
        fields: list[str] = []
        for function in group["functions"]:
            name, native = function["name"], function["native"]
            if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", native):
                raise ValueError("Invalid helper registration symbol")
            registration.extend(['lua_pushcfunction(m_state, &' + native + ', "plugin.' + scope + '.' + name + '");', 'lua_rawsetfield(m_state, -2, "' + name + '");'])
            fields.append(name + ": (" + ", ".join(function["arguments"]) + ") -> " + function["returns"])
        registration.append('lua_rawsetfield(m_state, -2, "' + scope + '");')
        helper_types.append(scope + ": " + ("PluginService & " if group.get("merge") else "") + "{" + ", ".join(fields) + "}")
    (output / "generated/plugin_helpers.inc").write_text("\n".join(registration) + "\n")
    definitions = ["-- Generated from codegen/plugin_protocol.json; do not edit.",
        "type WingDeviceObject = { writable: boolean?, sequential: boolean?, keep_size: boolean?, locked: boolean?, overwrite: boolean?, read: (self: WingDeviceObject, count: number) -> (string | bytearray), write: ((self: WingDeviceObject, bytes: string | bytearray) -> number)?, size: (self: WingDeviceObject) -> number, seek: ((self: WingDeviceObject, position: number) -> boolean)?, close: ((self: WingDeviceObject) -> boolean)? }"]
    definitions.extend([
        "type WingRibbonDescription = {category: string, display_name: string, toolboxes: {{name: string, tools: {_qt_QToolButton}}}}",
        "type WingDockDescription = {id: string, name: string, widget: _qt_QWidget, area: number?}",
        "type WingSettingDescription = {id: string, name: string, widget: _qt_QWidget, restore: ((self: WingSettingDescription) -> ())?, discard: ((self: WingSettingDescription) -> ())?, contain_unsaved_changes: ((self: WingSettingDescription) -> boolean)?, highlight_unsaved_change: ((self: WingSettingDescription) -> ())?}",
        "type WingViewState = {widget: _qt_QWidget, load_state: ((self: WingViewState, state: string | bytearray) -> ())?, save_state: ((self: WingViewState) -> (string | bytearray))?, has_unsaved_state: ((self: WingViewState) -> boolean)?, set_saved: ((self: WingViewState) -> ())?, toggled: ((self: WingViewState, visible: boolean) -> ())?, on_closing: ((self: WingViewState) -> boolean)?, on_work_space_notify: ((self: WingViewState, workspace: boolean) -> ())?}",
        "type WingViewFactory = {id: string, name: string, create: (self: WingViewFactory) -> WingViewState}",
    ])
    for name in ("Plugin", "DevicePlugin"):
        definitions.append("type Wing" + name + " = {")
        for entry in callbacks:
            if (entry.get("device_only") and name == "Plugin") or (entry.get("ordinary_only") and name == "DevicePlugin"):
                continue
            arguments = ["self: Wing" + name, *entry.get("arguments", [])]
            definitions.append("    " + entry["name"] + ": (" + ", ".join(arguments) + ") -> " + entry["returns"] + ",")
        definitions.append("}")
        definitions.append("type Wing" + name + "Class = Wing" + name + " & { extend: <T>(self: T) -> T, new: <T>(self: T) -> T }")
    common = "info: {Id: string, SDK: number | Integer64, Version: string, [string]: unknown}, log: typeof(log), toast: PluginToast, " + ", ".join(helper_types)
    definitions.append("export type PluginAPI = { Plugin: WingPluginClass, reader: typeof(reader), ctl: typeof(ctl), " + common + " }")
    definitions.append("export type DevicePluginAPI = { DevicePlugin: WingDevicePluginClass, " + common + ", io: {open_file: (path: string | ustring, mode: number) -> _ext__plugin_device_types_IODevice, wrap: (handle: string | ustring) -> _ext__plugin_device_types_IODevice}, IODevice: {} }")
    manifest = json.loads((output / "manifest.json").read_text()) if (output / "manifest.json").exists() else {}
    preload = set(manifest.get("definition_preload", []))
    required = set(re.findall(r"\b_(?:qt|ext)_[A-Za-z0-9_]+", "\n".join(definitions)))
    providers: dict[str, str] = {}
    for component in (output / "definitions").glob("WingLuauBind_types_*.d.luau"):
        for symbol in re.findall(r"(?:declare extern type|export type)\s+([A-Za-z_][A-Za-z0-9_]*)", component.read_text()):
            providers[symbol] = component.name
    imports: list[str] = []
    for name in ("service", "toast"):
        declaration = (output / "definitions" / ("WingLuauBind_" + name + ".d.luau"))
        if declaration.exists():
            match = re.search(r"declare\s+" + name + r":\s*([A-Za-z_][A-Za-z0-9_]*)", declaration.read_text())
            if not match:
                raise ValueError("Missing generated service value type: " + name)
            imports.append("type Plugin" + name.title() + " = " + match.group(1))
        else:
            imports.append("type Plugin" + name.title() + " = {}")
    modules: dict[str, str] = {}
    for symbol in sorted(required):
        provider = providers.get(symbol)
        if provider in preload or provider is None:
            continue
        if provider not in modules:
            modules[provider] = "pluginTypes" + str(len(modules))
            imports.append('local ' + modules[provider] + ' = require("./' + provider + '")')
        imports.append("type " + symbol + " = " + modules[provider] + "." + symbol)
    definitions = imports + definitions + ["return {}"]
    filename = "WingLuauBind_plugin_protocol.d.luau"
    (output / "definitions" / filename).write_text("\n".join(definitions) + "\n")
    copy = output / "copy_definitions.cmake"
    source = copy.read_text()
    addition = 'list(APPEND _expected "' + filename + '")\n'
    if addition not in source:
        source = source.replace("file(GLOB", addition + "file(GLOB", 1)
        if addition not in source:
            source = source.replace("foreach(_file", addition + "foreach(_file", 1)
    copy.write_text(source)
