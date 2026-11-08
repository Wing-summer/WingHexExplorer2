-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: AGPL-3.0-or-later
-- The transport fixture tests the real script module and protected-call helper.
local context = 1
local nextHandle = 0
local handles = {}
local active = {}
local files = {}
local switchCount = 0
hex = { ErrFile = { Success = 0, NotExist = -1, Error = -2 }, DocumentType = { InValid = 0, File = 1 } }

local function owned(handle)
    local entry = handles[handle]
    return entry and entry.owner == context and entry or nil
end
local function target()
    local entry = owned(active[context])
    assert(entry and entry.file.open, "Invalid selected handle")
    return entry.file
end
ctl = setmetatable({
    _document_context = function() return context end,
    _file_url = function(path) return "file:" .. path end,
    _workspace_url = function(path) return "wingpro:" .. path end,
    _device_url = function(driver, path) return "wdrv:" .. driver .. "/" .. path end,
    open_file = function(path)
        if path == "file:missing" then return -1 end
        if not files[path] or not files[path].open then files[path] = { open = true, modified = false, bytes = "abcdefgh" } end
        for handle, entry in handles do
            if entry.owner == context and entry.file == files[path] then return handle end
        end
        nextHandle += 1
        handles[nextHandle] = { owner = context, file = files[path], path = path }
        return nextHandle
    end,
    close_file_handle = function(handle)
        if not owned(handle) then return -1 end
        handles[handle] = nil
        return 0
    end,
    close_file = function(handle, force)
        local entry = owned(handle)
        if not entry or not entry.file.open then return -1 end
        if entry.file.modified and not force then return -2 end
        entry.file.open = false
        for id, other in handles do if other.file == entry.file then handles[id] = nil end end
        return 0
    end,
    switch_document = function(handle)
        if not owned(handle) or not owned(handle).file.open then return false end
        active[context] = handle
        switchCount += 1
        return true
    end,
    raise_document = function(handle) return owned(handle) ~= nil end,
    write_bytes = function(offset, value)
        local file = target()
        file.bytes = string.sub(file.bytes, 1, offset) .. value .. string.sub(file.bytes, offset + #value + 1)
        file.modified = true
        return true
    end,
    select = function(offset, length) target().selection = { offset, length }; return true end,
    add_book_mark = function(offset, comment) target().bookmark = { offset, comment }; return true end,
    metadata = function(...) target().annotation = { ... }; return true end,
}, { __index = function(_, _) return function() return false end end })
reader = setmetatable({
    is_file_opened = function(handle)
        local entry = owned(handle)
        return entry ~= nil and entry.file.open
    end,
    document_info = function(handle)
        local entry = owned(handle)
        return { documentType = entry and entry.file.open and 1 or 0, isSaved = entry and not entry.file.modified or false }
    end,
    read_bytes = function(offset, length) return string.sub(target().bytes, offset + 1, offset + length) end,
    selected_bytes = function() local file = target(); return string.sub(file.bytes, file.selection[1] + 1, file.selection[1] + file.selection[2]) end,
    book_mark_comment = function() return target().bookmark[2] end,
}, { __index = function(_, _) return function() return false end end })

local function expectError(callback, pattern)
    local ok, err = pcall(callback)
    assert(not ok, "Expected an error")
    assert(tostring(err):find(pattern, 1, true), tostring(err))
end
local function releaseContext(id)
    for handle, entry in handles do if entry.owner == id then handles[handle] = nil end end
end

function run_hexdoc_tests()
    local hexdoc = require("@s/hexdoc")
    assert(type(hexdoc.new) == "function")
    assert(table.isfrozen(hexdoc))
    local a, b = hexdoc.new(), hexdoc.new()
    assert(table.isfrozen(a))
    assert(not a:is_valid())
    assert(a:open_file("a") == 0 and b:open_file("b") == 0)
    assert(a:is_valid())
    local alias = hexdoc.new()
    assert(alias:open_file("a") == 0)
    assert(alias:is_valid())
    assert(alias:close_handle() == 0)
    assert(not a:is_valid())
    assert(a:open_file("a") == 0)
    expectError(function() a:read_bytes(0, 1) end, "use()")
    expectError(function() a:write_bytes(0, "X") end, "use()")
    local before = switchCount
    local values = table.pack(a:use(function(doc, arg)
        assert(doc == a and arg == "argument")
        assert(doc:read_bytes(0, 3) == "abc")
        assert(doc:write_bytes(0, "A"))
        assert(doc:select(0, 2))
        assert(doc:selected_bytes(0) == "Ab")
        assert(doc:add_book_mark(1, "mark"))
        assert(doc:book_mark_comment(1) == "mark")
        assert(doc:metadata(0, 1, "red", "blue", "note"))
        doc:use(function(inner) assert(inner:read_bytes(0, 1) == "A") end)
        assert(switchCount == before + 1)
        expectError(function() b:use(function() end) end, "Another document")
        return "first", nil, 3, nil
    end, "argument"))
    assert(values.n == 4 and values[4] == nil and values[1] == "first" and values[2] == nil and values[3] == 3)
    expectError(function() a:read_bytes(0, 1) end, "use()")
    local marker = {}
    local ok, err = pcall(function() a:use(function() error(marker) end) end)
    assert(not ok and err == marker, "Error identity was lost")
    b:use(function() end)
    expectError(function() a:use(function() coroutine.yield("forbidden") end) end, "yield")
    b:use(function() end)
    expectError(function() a:use("not a callback") end, "callback")
    assert(a:close() == -2)
    a:use(function() assert(a:read_bytes(0, 1) == "A") end)
    assert(a:close_handle() == 0 and files["file:a"].open)
    assert(not a:is_valid())
    expectError(function() a:use(function() end) end, "invalid document")
    assert(a:open_file("a") == 0)
    a:use(function()
        assert(a:close(true) == 0)
        expectError(function() a:read_bytes(0, 1) end, "use()")
    end)
    assert(not files["file:a"].open)
    assert(a:open_file("reopen") == 0)
    a:use(function()
        assert(a:open_file("replacement") == 0)
        expectError(function() a:read_bytes(0, 1) end, "use()")
    end)
    assert(files["file:reopen"].open)
    a:use(function() assert(a:read_bytes(0, 1) == "a") end)
    assert(a:open_ext("driver", "/中文 # %?path") == 0)
    assert(a:open_workspace("work space.wingpro") == 0)
    assert(a:activate())
    assert(a:open_file("missing") == -1)
    expectError(function() a:use(function() end) end, "invalid document")
    assert(a:open_file("lifetime") == 0)
    releaseContext(context)
    context = 2
    assert(not a:is_valid())
    expectError(function() a:document_info() end, "finished or different")
    local other = hexdoc.new()
    assert(other:open_file("other-context") == 0)
    assert(files["file:lifetime"].open, "Context cleanup closed an editor")
    assert(ctl.close_file_handle(1) == -1, "Stale handle crossed contexts")
    other:use(function() assert(other:read_bytes(0, 1) == "a") end)
    releaseContext(context)
    print("hexdoc script tests passed")
end
