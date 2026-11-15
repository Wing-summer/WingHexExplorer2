-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
-- Run on an editable eight-byte fixture: "HEAD" followed by a little-endian u32.

local binary = require("@s/binary")
local analysis = require("@s/analysis")
local language = nil -- nil follows the host UI language.
local simplified = { ["Edit header flags"] = "编辑头标志", ["Header flags"] = "头标志" }
local traditional = { ["Edit header flags"] = "編輯標頭旗標", ["Header flags"] = "標頭旗標" }
local tr = require("@s/i18n").create({
    zh = simplified, zh_Hant = traditional,
    zh_TW = traditional, zh_HK = traditional, zh_MO = traditional,
}, language)
local schema = binary.register("example.header", { endian = "little", fields = {
    {name = "magic", type = "bytes", size = 4},
    {name = "flags", type = "u32"},
} })
analysis.current(function(document)
    local parsed = analysis.read(document, schema)
    assert(parsed.values.magic == "HEAD", "Expected the HEAD example fixture")
    local updated = analysis.write(document, parsed, { flags = bit32.bor(parsed.values.flags, 1) }, tr("Edit header flags"))
    analysis.annotate(document, updated.fields, { offset = updated.offset, title = tr("Header flags") })
    -- A failed independent transaction rolls back its changes automatically.
    local ok = pcall(analysis.transaction, document, "Rollback example", function()
        assert(document:write_bytes(0, "FAIL"))
        error("Demonstrate rollback")
    end)
    assert(not ok and document:read_bytes(0, 8) == updated.original)
end)
