-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local api = require("./lz4_api")

local vendor = require("./vendor/lz4/llz4")
local bytes = require("./bytes")
local Lz4 = {}
function Lz4.compress(value)
    return vendor.compress(bytes.string(value))
end
function Lz4.decompress(value, maxBytes)
    local limit = maxBytes or 64 * 1024 * 1024
    assert(type(limit) == "number" and limit > 0 and limit % 1 == 0 and limit <= 2147483647, "Invalid LZ4 output limit")
    local input = bytes.buffer(value)
    local output, length = vendor.decompress_buffer(input, 0, buffer.len(input), -limit)
    return buffer.readstring(output, 0, length)
end
return table.freeze(Lz4) :: api.Module
