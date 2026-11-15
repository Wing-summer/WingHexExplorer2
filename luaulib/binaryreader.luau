-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
-- Bounded, random-access parser. Parsing builds a plan and never edits a document.
local api = require("./binaryreader_api")
export type Mark = api.Mark
export type Reader = api.Reader

local Binary = {}
local Methods = {}
Methods.__index = Methods

function Binary.number(value)
    if type(value) ~= "number" then value = value:to_number() end
    assert(value >= 0 and value <= 9007199254740991 and value % 1 == 0, "Unsafe integer")
    return value
end

function Binary.new(size, read)
    return setmetatable({ size = Binary.number(size), read = read, marks = {}, little = true, work = 0 }, Methods)
end

function Methods:range(offset, size)
    assert(offset >= 0 and size >= 0 and offset % 1 == 0 and size % 1 == 0
        and offset <= self.size and size <= self.size - offset,
        string.format("File range out of bounds: 0x%X + 0x%X", offset, size))
end

function Methods:bytes(offset, size)
    self:range(offset, size)
    self.work += 1
    assert(self.work <= 2000000, "Parser work limit exceeded")
    local bytes = self.read(offset, size)
    assert(type(bytes) == "string" and #bytes == size, "Short document read")
    return bytes
end

function Methods:uint(offset, size)
    assert(size <= 8, "Integer too wide")
    local bytes = self:bytes(offset, size)
    local value = 0
    for i = 1, size do
        local index = if self.little then size - i + 1 else i
        value = value * 256 + string.byte(bytes, index)
        assert(value <= 9007199254740991, "File offset/count exceeds exact Luau integer range")
    end
    return value
end

-- Subtract unsigned address words byte by byte; only the resulting file RVA is
-- converted to a number. This also supports high-half kernel image addresses.
function Methods:delta(addressOffset, baseOffset, width)
    local address, base = self:bytes(addressOffset, width), self:bytes(baseOffset, width)
    local result, borrow = {}, 0
    for i = 1, width do
        local index = if self.little then i else width - i + 1
        local byte = string.byte(address, index) - string.byte(base, index) - borrow
        borrow = if byte < 0 then 1 else 0
        result[i] = byte % 256
    end
    assert(borrow == 0, "Virtual address precedes image base")
    local value = 0
    for i = width, 1, -1 do
        value = value * 256 + result[i]
        assert(value <= 9007199254740991, "Address difference exceeds exact integer range")
    end
    return value
end

function Methods:mark(offset, size, name, group)
    self:range(offset, size)
    if size == 0 then return end
    assert(#self.marks < 200000, "Annotation limit exceeded")
    table.insert(self.marks, { offset = offset, size = size, name = name, group = group or 1, order = #self.marks + 1 })
end

-- Layout syntax: field:size, separated by whitespace. Values stay as raw bytes;
-- 64-bit virtual addresses are never rounded through a floating-point number.
function Methods:struct(offset, layout, name, group)
    local start = offset
    local index = 0
    for field, width in string.gmatch(layout, "([%w_]+):(%d+)") do
        local size = tonumber(width)
        self:mark(offset, size, name .. "." .. field, (group or 1) + index % 2)
        index += 1
        offset += size
    end
    return offset - start
end

function Methods:array(offset, count, stride, minimum, callback)
    assert(count <= 200000 and stride >= minimum, "Invalid table count or entry size")
    self:range(offset, count * stride)
    for i = 0, count - 1 do callback(offset + i * stride, i) end
end

function Methods:cstring(offset, limit, name, group)
    self:range(offset, 0)
    local maximum = math.min(limit or 65536, self.size - offset)
    local parts = {}
    local length = 0
    while length < maximum do
        local bytes = self:bytes(offset + length, math.min(256, maximum - length))
        local zero = string.find(bytes, "\0", 1, true)
        if zero then
            table.insert(parts, string.sub(bytes, 1, zero - 1))
            self:mark(offset, length + zero, name, group)
            return table.concat(parts)
        end
        table.insert(parts, bytes)
        length += #bytes
    end
    error("Unterminated string: " .. name)
end

-- Cover alignment gaps, overlays and vendor data without guessing their format.
function Methods:finish()
    local spans = table.clone(self.marks)
    table.sort(spans, function(a, b) return a.offset < b.offset end)
    local ending = 0
    for _, span in spans do
        if span.offset > ending then
            self:mark(ending, span.offset - ending, "File padding / unclassified data", 6)
        end
        ending = math.max(ending, span.offset + span.size)
    end
    if ending < self.size then self:mark(ending, self.size - ending, "File overlay / unclassified data", 6) end
end

return Binary :: api.Module
