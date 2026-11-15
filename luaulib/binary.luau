-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
-- Offsets are zero based; arrays and table results are one based.
local api = require("./binary_api")
export type Field = api.Field
export type Definition = api.Definition
export type Schema = api.Schema
export type Source = api.Source
export type Options = api.Options
export type Mark = api.Mark
export type Result = api.Result

local bytes = require("./bytes")
local Binary = {}
local registry = {}
local primitives = {
    u8 = {1, false}, i8 = {1, true}, u16 = {2, false}, i16 = {2, true},
    u32 = {4, false}, i32 = {4, true}, u64 = {8, false}, i64 = {8, true},
    f32 = {4, false, true}, f64 = {8, false, true},
}
for _, primitive in primitives do table.freeze(primitive) end
local function integer(value, name)
    assert(type(value) == "number" and value >= 0 and value <= 9007199254740991 and value % 1 == 0, "Invalid " .. name)
    return value
end
local function range(offset, size, length)
    integer(offset, "offset"); integer(size, "size")
    assert(offset <= length and size <= length - offset, "Binary range out of bounds")
end
local function schema_of(value)
    local schema = if type(value) == "string" then registry[value] else value
    assert(type(schema) == "table" and schema._binarySchema == true, "Unknown binary structure")
    return schema
end
function Binary.register(name, definition)
    assert(type(name) == "string" and name ~= "" and registry[name] == nil, "Duplicate or invalid structure name")
    local fields, ending, names = {}, 0, {}
    local endian = definition.endian or "little"
    assert(endian == "little" or endian == "big", "Invalid byte order")
    for _, source in definition.fields do
        assert(type(source.name) == "string" and source.name ~= "" and not names[source.name], "Duplicate or invalid field name")
        names[source.name] = true
        local kind = source.type
        local primitive = primitives[kind]
        local nested = if not primitive and kind ~= "bytes" and kind ~= "string" then schema_of(kind) else nil
        local width = if primitive then primitive[1] elseif nested then nested.size else integer(source.size, "field size")
        local count = integer(source.count or 1, "array count")
        local offset = integer(source.offset or ending, "field offset")
        local order = source.endian or endian
        assert(order == "little" or order == "big", "Invalid byte order")
        local size = integer(width * count, "field extent")
        -- Overlap is permitted only for a union with an explicitly selected member.
        assert(definition.union == true or offset >= ending, "Overlapping structure fields")
        if definition.union ~= true or source.name == definition.member then
            table.insert(fields, table.freeze({ name = source.name, type = kind, primitive = primitive,
                schema = nested, width = width, count = count, array = source.count ~= nil,
                offset = offset, size = size, endian = order, group = source.group or 1 }))
        end
        ending = math.max(ending, offset + size)
    end
    assert(definition.union ~= true or #fields == 1, "Union requires an explicit member")
    local size = integer(definition.size or ending, "structure size")
    assert(size >= ending, "Structure size is smaller than its fields")
    local schema = table.freeze({ _binarySchema = true, name = name, fields = table.freeze(fields), size = size, endian = endian })
    registry[name] = schema
    return schema
end
function Binary.register_cffi(name, ffi, ctype, definition)
    assert(type(ffi) == "table" and type(ffi.sizeof) == "function", "Expected an isolated cffi instance")
    local layout = table.clone(definition)
    layout.fields = {}
    layout.size = ffi.sizeof(ctype)
    local alignment = ffi.alignof(ctype)
    assert(layout.size % alignment == 0, "Invalid cffi alignment")
    for _, field in definition.fields do
        local copy = table.clone(field)
        local offset, bit, width = ffi.offsetof(ctype, field.name)
        assert(type(offset) == "number", "Unknown cffi field: " .. field.name)
        assert(bit == nil and width == nil, "C bitfields require explicit disk layouts")
        assert(copy.offset == nil or copy.offset == offset, "cffi field offset mismatch")
        copy.offset = offset
        table.insert(layout.fields, copy)
    end
    assert(definition.size == nil or definition.size == layout.size, "cffi structure size mismatch")
    return Binary.register(name, layout)
end
local function ordered(raw, endian)
    return if endian == "little" then raw else string.reverse(raw)
end
local function decode_scalar(field, raw)
    local primitive = field.primitive
    if not primitive then return raw end
    local little = ordered(raw, field.endian)
    if primitive[3] then
        local data = buffer.fromstring(little)
        return if field.width == 4 then buffer.readf32(data, 0) else buffer.readf64(data, 0)
    end
    if field.width == 8 then
        local data = buffer.fromstring(little)
        local low, high = buffer.readu32(data, 0), buffer.readu32(data, 4)
        if primitive[2] and high >= 2147483648 then
            local negativeLow = bit32.bnot(low) + 1
            local negativeHigh = (bit32.bnot(high) + (if negativeLow > 4294967295 then 1 else 0)) % 4294967296
            return int64(string.format("-0x%08x%08x", negativeHigh, negativeLow % 4294967296))
        end
        local value = string.format("0x%08x%08x", high, low)
        return if primitive[2] then int64(value) else uint64(value)
    end
    local value = 0
    for index = #little, 1, -1 do value = value * 256 + string.byte(little, index) end
    if primitive[2] and value >= 2 ^ (field.width * 8 - 1) then value -= 2 ^ (field.width * 8) end
    return value
end
local function encode_scalar(field, value)
    local primitive = field.primitive
    if not primitive then
        local raw = bytes.string(value)
        assert(#raw == field.width, "Fixed field length mismatch: " .. field.name)
        return raw
    end
    local data = buffer.create(field.width)
    if primitive[3] then
        assert(type(value) == "number", "Expected floating point value")
        if field.width == 4 then buffer.writef32(data, 0, value) else buffer.writef64(data, 0, value) end
    elseif field.width == 8 then
        local wide = if primitive[2] then int64(value) else uint64(value)
        buffer.writeu32(data, 0, wide:low()); buffer.writeu32(data, 4, wide:high())
    else
        local bits = field.width * 8
        local minimum = if primitive[2] then -2 ^ (bits - 1) else 0
        local maximum = if primitive[2] then 2 ^ (bits - 1) - 1 else 2 ^ bits - 1
        assert(type(value) == "number" and value % 1 == 0 and value >= minimum and value <= maximum, "Integer field out of range: " .. field.name)
        value %= 2 ^ bits
        for index = 0, field.width - 1 do buffer.writeu8(data, index, value % 256); value = math.floor(value / 256) end
    end
    return ordered(buffer.tostring(data), field.endian)
end
local function walk_decode(schema, raw, base, path, marks, limits, depth)
    assert(depth <= limits.maxDepth, "Binary nesting limit exceeded")
    local values = {}
    for _, field in schema.fields do
        local value = {}
        local fieldPath = if path == "" then field.name else path .. "." .. field.name
        for index = 1, field.count do
            limits.work += 1
            assert(limits.work <= limits.maxWork, "Binary work limit exceeded")
            local offset = base + field.offset + (index - 1) * field.width
            local itemPath = fieldPath .. (if field.array then "[" .. index .. "]" else "")
            local part = string.sub(raw, offset + 1, offset + field.width)
            table.insert(marks, table.freeze({ path = itemPath, name = itemPath, offset = offset,
                size = field.width, group = field.group, raw = part }))
            value[index] = if field.schema then walk_decode(field.schema, raw, offset, itemPath, marks, limits, depth + 1)
                else decode_scalar(field, part)
        end
        values[field.name] = if field.array then value else value[1]
    end
    return values
end
function Binary.decode(schema, source, offset, options)
    schema = schema_of(schema)
    offset = integer(offset or 0, "offset")
    options = options or {}
    assert(schema.size <= (options.maxBytes or 64 * 1024 * 1024), "Binary byte limit exceeded")
    local raw
    if type(source) == "table" then
        range(offset, schema.size, integer(source.size, "source size"))
        raw = source.read(offset, schema.size)
    else
        local text = bytes.string(source)
        range(offset, schema.size, #text)
        raw = string.sub(text, offset + 1, offset + schema.size)
    end
    assert(type(raw) == "string" and #raw == schema.size, "Short binary read")
    local marks = {}
    local limits = { work = 0, maxWork = integer(options.maxWork or 200000, "work limit"), maxDepth = integer(options.maxDepth or 64, "depth limit") }
    local values = walk_decode(schema, raw, 0, "", marks, limits, 0)
    return table.freeze({ schema = schema, values = values, fields = table.freeze(marks), original = raw, offset = offset, size = schema.size })
end
local function walk_encode(schema, values, output, base, depth, limits)
    assert(depth <= limits.maxDepth, "Binary nesting limit exceeded")
    assert(type(values) == "table", "Expected structure table")
    for _, field in schema.fields do
        local fieldValue = values[field.name]
        if fieldValue ~= nil then
            if field.array then assert(type(fieldValue) == "table" and #fieldValue == field.count, "Array length mismatch") end
            for index = 1, field.count do
                limits.work += 1
                assert(limits.work <= limits.maxWork, "Binary work limit exceeded")
                local value = if field.array then fieldValue[index] else fieldValue
                local offset = base + field.offset + (index - 1) * field.width
                if field.schema then walk_encode(field.schema, value, output, offset, depth + 1, limits)
                else buffer.writestring(output, offset, encode_scalar(field, value)) end
            end
        end
    end
end
function Binary.encode(schema, values, original, options)
    schema = schema_of(schema)
    options = options or {}
    assert(schema.size <= (options.maxBytes or 64 * 1024 * 1024), "Binary byte limit exceeded")
    local output
    if original ~= nil then
        local text = bytes.string(original)
        assert(#text == schema.size, "Original structure length mismatch")
        output = buffer.fromstring(text)
    else output = buffer.create(schema.size) end
    walk_encode(schema, values, output, 0, 0, { work = 0, maxWork = integer(options.maxWork or 200000, "work limit"), maxDepth = integer(options.maxDepth or 64, "depth limit") })
    return buffer.tostring(output)
end
return table.freeze(Binary) :: api.Module
