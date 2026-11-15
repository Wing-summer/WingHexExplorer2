-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local I18n = require("./lib/i18n")
for _, case in {
    { "zh", 1 }, { "zh_CN", 1 }, { "zh-SG", 1 }, { "zh-Hans-TW", 1 },
    { "zh_TW", 2 }, { "zh_HK", 2 }, { "zh-MO", 2 }, { "zh-Hant-CN", 2 },
} do
    local expected = if case[2] == 1 then "已添加 123 条标注" else "已新增 123 條標註"
    assert(I18n.new(case[1]):text("%d annotations applied", 123) == expected, case[1])
end
for _, locale in { "en_US", "de_DE", "C", "", "invalid" } do
    assert(I18n.new(locale):text("%d annotations applied", 123) == "123 annotations applied")
end
local cn, tw = I18n.new("zh_CN"), I18n.new("zh_TW")
assert(cn:text("%d annotations applied", 123) == "已添加 123 条标注")
assert(tw:text("%d annotations applied", 123) == "已新增 123 條標註")
assert(cn:text("COFF.Symbol[7].Value") == "COFF.Symbol[7].Value")
assert(tw:text("File overlay / unclassified data") == "檔案尾端附加資料 / 未分類資料")
for _, case in {
    { "File range out of bounds: 0x10 + 0x2", "文件范围越界：0x10 + 0x2" },
    { "Unmapped file-backed RVA 0x20 + 0x4", "RVA 没有对应的文件数据：0x20 + 0x4" },
    { "Misaligned Exception table", "Exception 表未正确对齐" },
    { "Unterminated DelayImport descriptors", "DelayImport 描述符缺少终止项" },
    { "Invalid string table reference: /tmp/100%.bin", "字符串表引用无效：/tmp/100%.bin" },
    { "Invalid section entry size: .dynamic", "节表项大小无效：.dynamic" },
    { "Unterminated string: Resource.Name", "字符串缺少终止符：Resource.Name" },
    { "Cannot annotate DOS.e_magic", "无法标注 DOS.e_magic" },
} do
    assert(cn:diagnostic("/a/path:42: " .. case[1]) == case[2], case[1])
end
assert(cn:diagnostic("/a/path:5: Truncated certificate") == "证书被截断")
assert(tw:diagnostic("/a/path:5: Truncated ELF note") == "ELF 註記遭截斷")
assert(cn:diagnostic("/a/path:5: Unknown native error: 50%") == "Unknown native error: 50%")
print("Passed locale selection, placeholders and diagnostic translations")
