-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local api = require("./base64_api")

local vendor = require("./vendor/base64/Base64")
local bytes = require("./bytes")
local Base64 = {}
function Base64.encode(value)
    return buffer.tostring(vendor.encode(bytes.buffer(value)))
end
function Base64.decode(value)
    local text = bytes.string(value)
    assert(#text % 4 == 0 and not string.find(text, "[^A-Za-z0-9+/=]"), "Invalid Base64 input")
    local body, padding = string.match(text, "^([A-Za-z0-9+/]*)(=*)$")
    assert(body and #padding <= 2 and (#text == 0 or #body >= 2), "Invalid Base64 padding")
    local result = vendor.decode(buffer.fromstring(text))
    assert(Base64.encode(result) == text, "Noncanonical Base64 input")
    return result
end
return table.freeze(Base64) :: api.Module
