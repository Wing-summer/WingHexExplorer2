-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local nativeRequire = require
local Binary = nativeRequire("./stdlib/binaryreader")
local Structures = nativeRequire("./stdlib/binary")
local BaseI18n = nativeRequire("./lib/i18n")
local localeName = "en_US"
local I18n = { new = function(locale) return BaseI18n.new(locale or localeName) end }
local active, macro, released, ends, copied = false, false, 0, 0, nil
local selection = { "\0\15\255", "", "\127" }
local failMetadata = false
local failClipboard = false
local integerCount = false
local hasDocument = true
local notifications = {}
local require
local reader = {
    is_current_doc_editing = function() return hasDocument end,
    document_bytes = function() assert(active); return 16 end,
    read_bytes = function(_, n) assert(active); return string.rep("\0", n) end,
    selection_count = function()
        if integerCount then return { to_number = function() return #selection end } end
        return #selection
    end,
    selected_bytes = function(index) assert(active); return selection[index + 1] end,
}
local hex = { ErrFile = { Success = 0 } }
local ctl = {
    open_current = function() assert(not active); active = true; return 1 end,
    close_current = function() assert(active and not macro); active = false; released += 1; return 0 end,
    begin_marco = function() assert(active); macro = true; return true end,
    end_marco = function() assert(macro); macro = false; ends += 1; return true end,
    reset_marco = function() assert(macro); macro = false; ends += 1; return true end,
    metadata = function() assert(active and macro); return not failMetadata end,
}
local function color(...) return { ... } end
local toast = { show = function(icon, message)
    assert(not active and not macro, "Toast shown before cleanup")
    table.insert(notifications, { icon = icon, message = message })
end }
local function expect_toast(icon, text)
    local notification = notifications[#notifications]
    assert(notification.icon == ":/com.wingsummer.winghex/images/" .. icon .. ".svg")
    assert(string.find(notification.message, text, 1, true), notification.message)
end
local function load_document()
-- INSERT_DOCUMENT
end
local function load_std_i18n()
    local function require(name)
        if name == "./i18n_api" then return nativeRequire("./stdlib/i18n_api") end
        assert(name == "@s/env")
        return setmetatable({}, {__index = function(_, key) if key == "locale" then return localeName end end})
    end
-- INSERT_STD_I18N
end
local StandardI18n = load_std_i18n()
local function load_analysis()
    local function require(name)
        if name == "./analysis_api" then return {} end
        if name == "./binary" then return Structures end
        assert(name == "@s/hexdoc")
        return { new = function()
            local document = {}
            function document:open_current() ctl.open_current(); return 0 end
            function document:close_handle() return ctl.close_current() end
            function document:use(callback, ...) return callback(self, ...) end
            return setmetatable(document, {__index = function(_, key)
                local target = reader[key] or ctl[key]
                assert(target, key)
                return function(_, ...) return target(...) end
            end})
        end }
    end
-- INSERT_ANALYSIS
end
local Analysis = load_analysis()
local Icons = { Id = {Warning = "warning", CopyHex = "copy-hex", Error = "error", ColorPalette = "color-palette"}, pixmap = function(id) return ":/com.wingsummer.winghex/images/" .. id .. ".svg" end }
local Document
require = function(name)
    if name == "@s/binary" then return Structures end
    if name == "@s/i18n" then return StandardI18n end
    if name == "@s/icons" then return Icons end
    if name == "@s/analysis" then return Analysis end
    if name == "@qt" then return { QClipboard = { set_text = function(text) assert(not failClipboard, "Clipboard unavailable"); copied = text end } } end
    if string.find(name, "i18n", 1, true) then return I18n end
    if string.find(name, "binary", 1, true) then return Binary end
    if string.find(name, "document", 1, true) then return Document end
    error(name)
end
Document = load_document()
local hostRequire = require
local function copy()
    local function require(name)
        assert(name == "@qt" or string.sub(name, 1, 3) == "@s/", name)
        return hostRequire(name)
    end
-- INSERT_COPY
end
copy()
assert(copied == "unsigned char array[] = {0x00, 0x0F, 0xFF, 0x7F};")
assert(not active and released == 1)
expect_toast("copy-hex", "Copied to clipboard")
selection = {}
copied = nil
copy()
assert(copied == nil and released == 2)
expect_toast("warning", "Nothing to copy")
selection = { "" }
copy()
assert(copied == nil and released == 3)
expect_toast("warning", "Nothing to copy")
local function parser(p) p:mark(0, 4, "header"); p:mark(4, 4, "data") end
Document.colorize(parser, "fixture")
assert(not active and not macro and ends == 1)
expect_toast("color-palette", "2 annotations applied")
local ok = pcall(function() Document.colorize(function(p) p:mark(16, 1, "invalid") end, "bad") end)
assert(not ok and not active and not macro and ends == 1)
expect_toast("error", "out of bounds")
failMetadata = true
ok = pcall(Document.colorize, parser, "failed metadata")
assert(not ok and not active and not macro and ends == 2)
expect_toast("error", "Cannot annotate")
hasDocument = false
ok = pcall(copy)
assert(not ok and not active)
expect_toast("error", "No document is being edited")
assert(#notifications == 7)
hasDocument = true
selection = { "\1" }
integerCount = true
copy()
assert(copied == "unsigned char array[] = {0x01};")
expect_toast("copy-hex", "Copied to clipboard")
failClipboard = true
local beforeRelease = released
ok = pcall(copy)
assert(not ok and not active and released == beforeRelease + 1)
expect_toast("error", "Clipboard unavailable")
assert(#notifications == 9)
print("Passed clipboard, preflight, cleanup and toast checks")

-- Both tools choose the same language while the copy tool remains standalone.
local localized = {
    { "en_US", "Copied to clipboard", "Nothing to copy", "No document is being edited", "2 annotations applied", "out of bounds", "Copy as C array", "PE structure", "Execution failed" },
    { "zh_CN", "已复制到剪贴板", "没有可复制的内容", "当前没有正在编辑的文档", "已添加 2 条标注", "文件范围越界", "复制为 C 数组", "PE 结构", "执行失败" },
    { "zh_TW", "已複製到剪貼簿", "沒有可複製的內容", "目前沒有正在編輯的文件", "已新增 2 條標註", "檔案範圍越界", "複製為 C 陣列", "PE 結構", "執行失敗" },
    { "zh-Hant-HK", "已複製到剪貼簿", "沒有可複製的內容", "目前沒有正在編輯的文件", "已新增 2 條標註", "檔案範圍越界", "複製為 C 陣列", "PE 結構", "執行失敗" },
    { "zh_HK", "已複製到剪貼簿", "沒有可複製的內容", "目前沒有正在編輯的文件", "已新增 2 條標註", "檔案範圍越界", "複製為 C 陣列", "PE 結構", "執行失敗" },
    { "zh-MO", "已複製到剪貼簿", "沒有可複製的內容", "目前沒有正在編輯的文件", "已新增 2 條標註", "檔案範圍越界", "複製為 C 陣列", "PE 結構", "執行失敗" },
    { "zh-Hans-TW", "已复制到剪贴板", "没有可复制的内容", "当前没有正在编辑的文档", "已添加 2 条标注", "文件范围越界", "复制为 C 数组", "PE 结构", "执行失败" },
    { "fr_FR", "Copied to clipboard", "Nothing to copy", "No document is being edited", "2 annotations applied", "out of bounds", "Copy as C array", "PE structure", "Execution failed" },
}
for _, text in localized do
    localeName = text[1]
    failClipboard, failMetadata, hasDocument = false, false, true
    selection = { "\15\255" }
    copy()
    assert(copied == "unsigned char array[] = {0x0F, 0xFF};", "Localization changed generated C code")
    expect_toast("copy-hex", text[2])
    expect_toast("copy-hex", text[7])
    selection = {}
    copy()
    expect_toast("warning", text[3])
    hasDocument = false
    assert(not pcall(copy))
    expect_toast("error", text[4])
    expect_toast("error", text[9])
    hasDocument = true
    Document.colorize(parser, "PE structure")
    expect_toast("color-palette", text[5])
    expect_toast("color-palette", text[8])
    assert(not pcall(function()
        Document.colorize(function(p) p:mark(16, 1, "bad") end, "PE structure")
    end))
    expect_toast("error", text[6])
end
-- Explicit entry-script choices override the host UI language.
localeName = "zh_CN"
Document.colorize(parser, "ELF structure", "en_US")
expect_toast("color-palette", "ELF structure: 2 annotations applied")
Document.colorize(parser, "ELF structure", "zh_TW")
expect_toast("color-palette", "ELF 結構：已新增 2 條標註")
print("Passed localized copy/analysis toasts and language fallback checks")

-- Exercise the shipped structure-edit entry with the real binary/analysis modules.
local structure_bytes = "HEAD" .. string.rep("\0", 4)
local backup
reader.document_bytes = function() assert(active); return #structure_bytes end
reader.read_bytes = function(offset, size) assert(active); return string.sub(structure_bytes, offset + 1, offset + size) end
ctl.write_bytes = function(offset, value)
    assert(active and macro)
    structure_bytes = string.sub(structure_bytes, 1, offset) .. value .. string.sub(structure_bytes, offset + #value + 1)
    return true
end
local begin_macro, reset_macro = ctl.begin_marco, ctl.reset_marco
ctl.begin_marco = function(...) backup = structure_bytes; return begin_macro(...) end
ctl.reset_marco = function(...) structure_bytes = backup; return reset_macro(...) end
local function structure_edit()
    local require = hostRequire
-- INSERT_STRUCTURE_EDIT
end
structure_edit()
assert(structure_bytes == "HEAD\1\0\0\0" and not active and not macro)
print("Passed structure-edit entry, write, annotation and rollback checks")
