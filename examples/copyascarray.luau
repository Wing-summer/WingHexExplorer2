-- Copyright (C) WingSummer (wingsummer)
-- SPDX-License-Identifier: BSD-3-Clause
-- Uses the deployed system library and host clipboard/toast APIs.
-- Copies all selections, in selection order, as an unsigned C/C++ byte array.
local qt = require("@qt")

local I18n = require("@s/i18n")
local Icons = require("@s/icons")
local Analysis = require("@s/analysis")
-- nil follows the host UI language. / nil 跟随宿主界面语言。
local language = nil -- e.g. "en-US", "zh-CN", "zh-TW"
local simplified = {
    ["Copy as C array: %s"] = "复制为 C 数组：%s",
    ["Nothing to copy"] = "没有可复制的内容",
    ["Copied to clipboard"] = "已复制到剪贴板",
    ["Invalid selection count"] = "选区数量无效",
    ["No document is being edited"] = "当前没有正在编辑的文档",
    ["Cannot acquire the current document"] = "无法获取当前文档的访问权限",
    ["Cannot release the document handle"] = "无法释放文档句柄",
    ["Execution failed: %s"] = "执行失败：%s",
}
local traditional = {
    ["Copy as C array: %s"] = "複製為 C 陣列：%s",
    ["Nothing to copy"] = "沒有可複製的內容",
    ["Copied to clipboard"] = "已複製到剪貼簿",
    ["Invalid selection count"] = "選取範圍數量無效",
    ["No document is being edited"] = "目前沒有正在編輯的文件",
    ["Cannot acquire the current document"] = "無法取得目前文件的存取權限",
    ["Cannot release the document handle"] = "無法釋放文件控制代碼",
    ["Execution failed: %s"] = "執行失敗：%s",
}
-- Explicit catalog aliases; the standard library does not infer regions.
local catalog = {
    zh = simplified,
    zh_Hant = traditional,
    zh_TW = traditional,
    zh_HK = traditional,
    zh_MO = traditional,
}

local tr = I18n.create(catalog, language)

local function copy_selections(document)
    local count = document:selection_count()
    if type(count) ~= "number" then count = count:to_number() end
    assert(count >= 0 and count <= 9007199254740991 and count % 1 == 0, tr("Invalid selection count"))
    if count == 0 then
        return tr("Nothing to copy"), Icons.Id.Warning
    end
    local parts = {}
    for index = 0, count - 1 do
        local bytes = document:selected_bytes(index)
        for i = 1, #bytes do
            table.insert(parts, string.format("0x%02X", string.byte(bytes, i)))
        end
    end
    if #parts == 0 then
        return tr("Nothing to copy"), Icons.Id.Warning
    end
    qt.QClipboard.set_text("unsigned char array[] = {" .. table.concat(parts, ", ") .. "};")
    return tr("Copied to clipboard"), Icons.Id.CopyHex
end

local function copy_current()
    assert(reader.is_current_doc_editing(), tr("No document is being edited"))
    return Analysis.current(copy_selections)
end

local ok, message, icon = pcall(copy_current)
local pixmap = Icons.pixmap(if ok then icon else Icons.Id.Error)
if not ok then
    local detail = string.match(tostring(message), "^.-:%d+: (.*)$") or tostring(message)
    message = tr("Execution failed: %s", tr(detail))
end
toast.show(pixmap, tr("Copy as C array: %s", message))
if not ok then error(message, 0) end
