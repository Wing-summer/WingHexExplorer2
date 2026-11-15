-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: MIT
-- Thin adapter over the registered JSON module; no script JSON parser.
local native = json -- Eager host registration, also available without QtWidgets.
local Json = {}
local objectMeta = table.freeze({ __qtluau_object = true })
local arrayMeta = table.freeze({ __jsonArray = true })
local nullDocument = native.from_json("[null]")
Json.null = nullDocument:to_variant()[1]
function Json.array(value) return setmetatable(value or {}, arrayMeta) end
local function normalize(value, seen, depth)
    assert(depth <= 64, "JSON nesting limit exceeded")
    if value == Json.null then return value end
    if type(value) == "string" then assert(utf8.len(value), "Invalid JSON UTF-8"); return value end
    if type(value) ~= "table" then return value end
    assert(not seen[value], "Cyclic JSON value")
    seen[value] = true
    local result = {}
    for key, item in value do result[key] = normalize(item, seen, depth + 1) end
    local meta = getmetatable(value)
    if #result == 0 and meta ~= arrayMeta then setmetatable(result, objectMeta) end
    seen[value] = nil
    return result
end
function Json.encode(value)
    -- A document wrapper also permits scalar JSON roots without QtGui.
    local document = native.from_variant({ normalize(value, {}, 0) })
    local text = document:to_json(native.JsonFormat.Compact)
    return string.sub(text, 2, -2)
end
local function restore(value, depth, work)
    work.count += 1
    assert(depth <= 64 and work.count <= 100000, "JSON conversion limit exceeded")
    if value:is_array() then
        local array, result = value:to_array(), Json.array()
        for index = 0, array:size() - 1 do result[index + 1] = restore(array:at(index), depth + 1, work) end
        return result
    elseif value:is_object() then
        local object, result = value:to_object(), setmetatable({}, objectMeta)
        for _, key in object:keys() do result[key] = restore(object:value(key), depth + 1, work) end
        return result
    end
    return value:to_variant()
end
function Json.decode(text)
    assert(type(text) == "string" and utf8.len(text), "Invalid JSON UTF-8")
    local document, failure = native.from_json("[" .. text .. "]")
    assert(failure.error == 0, tostring(failure:error_string()))
    local array = document:array()
    assert(array:size() == 1, "Expected one JSON value")
    return restore(array:at(0), 0, {count = 0})
end
return table.freeze(Json)
