-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local api = require("./bytes_api")
export type Bytes = api.Bytes

local vendor = require("./vendor/bytes/lib")
local Bytes = {}
function Bytes.buffer(value)
    assert(type(value) == "string" or typeof(value) == "buffer", "Expected string or buffer")
    return if type(value) == "string" then buffer.fromstring(value) else value
end
function Bytes.string(value)
    return if type(value) == "string" then value else buffer.tostring(Bytes.buffer(value))
end
function Bytes.from_hex(value)
    local text = Bytes.string(value)
    assert(#text % 2 == 0, "Hex input must contain pairs of digits")
    return vendor.fromhex(buffer.fromstring(text))
end
function Bytes.to_hex(value)
    return buffer.tostring(vendor.tohex(Bytes.buffer(value)))
end
return table.freeze(Bytes) :: api.Module
