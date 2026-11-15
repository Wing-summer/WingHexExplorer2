-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local api = require("./analysis_api")
export type Document = api.Document
export type AnnotationOptions = api.AnnotationOptions

local binary = require("./binary")
local hexdoc = require("@s/hexdoc")
local Analysis = {}
function Analysis.current(callback, ...)
    local document = hexdoc.new()
    local result = document:open_current()
    assert(result == hex.ErrFile.Success, "Cannot acquire the current document")
    local values = table.pack(pcall(document.use, document, callback, ...))
    local released = document:close_handle()
    assert(released == hex.ErrFile.Success, "Cannot release the document handle")
    if not values[1] then error(values[2], 0) end
    return table.unpack(values, 2, values.n)
end
function Analysis.transaction(document, title, callback)
    assert(document:begin_marco(title), "Cannot start annotation undo group")
    local values = table.pack(pcall(callback))
    if not values[1] then
        assert(document:reset_marco(), "Cannot roll back annotation undo group")
        error(values[2], 0)
    end
    if not document:end_marco() then
        assert(document:reset_marco(), "Cannot roll back annotation undo group")
        error("Cannot finish annotation undo group", 0)
    end
    return table.unpack(values, 2, values.n)
end
function Analysis.read(document, schema, offset, options)
    local size = document:document_bytes()
    if type(size) ~= "number" then size = size:to_number() end
    return binary.decode(schema, { size = size, read = function(start, length) return document:read_bytes(start, length) end }, offset, options)
end
function Analysis.write(document, result, values, title)
    local encoded = binary.encode(result.schema, values or result.values, result.original)
    assert(document:read_bytes(result.offset, result.size) == result.original, "Document changed since structure was read")
    Analysis.transaction(document, title or "Write structure", function()
        assert(document:write_bytes(result.offset, encoded), "Cannot write structure")
    end)
    return binary.decode(result.schema, { size = result.offset + #encoded, read = function() return encoded end }, result.offset)
end
local palette = { {229, 192, 123}, {97, 175, 239}, {152, 195, 121}, {198, 120, 221}, {86, 182, 194}, {224, 108, 117} }
function Analysis.annotate(document, marks, options)
    options = options or {}
    local size = document:document_bytes()
    if type(size) ~= "number" then size = size:to_number() end
    local plan = {}
    for order, mark in marks do
        local offset = mark.offset + (options.offset or 0)
        assert(offset >= 0 and mark.size >= 0 and offset % 1 == 0 and mark.size % 1 == 0 and offset <= size and mark.size <= size - offset, "Annotation range out of bounds")
        local copy = table.clone(mark); copy.offset = offset; copy.order = order
        table.insert(plan, copy)
    end
    table.sort(plan, function(a, b) return if a.size == b.size then a.order < b.order else a.size > b.size end)
    Analysis.transaction(document, options.title or "Annotate structure", function()
        for _, mark in plan do
            if mark.size == 0 then continue end
            local rgb = palette[((mark.group or 1) - 1) % #palette + 1]
            local label = (options.tr or tostring)(mark.name or mark.path)
            assert(document:metadata(mark.offset, mark.size, mark.foreground or color(24, 24, 24), mark.background or color(rgb[1], rgb[2], rgb[3]), label), "Cannot annotate " .. label)
            if mark.bookmark then assert(document:add_book_mark(mark.offset, label), "Cannot add bookmark") end
        end
    end)
    return #plan
end
return table.freeze(Analysis) :: api.Module
