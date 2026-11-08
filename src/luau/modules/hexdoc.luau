--!strict
-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: AGPL-3.0-or-later

local api = require("./hexdoc_api")
export type Document = api.Document

local operations = api :: { [string]: (...any) -> ...any }
local native = ctl :: any
local states = setmetatable({}, { __mode = "k" }) :: { [any]: any }
local scopes = setmetatable({}, { __mode = "v" }) :: { [number]: any }
local methods = {} :: { [string]: any }
local metatable = { __index = methods }

local function stateOf(document: any): any
    local state = states[document]
    if state == nil then
        error("Invalid hexdoc object", 3)
    end
    if state.owner ~= native._document_context() then
        error("Document belongs to a finished or different script context", 3)
    end
    return state
end

local function invalidate(document: any, state: any)
    local scope = scopes[state.owner]
    if scope ~= nil and scope.document == document then
        scope.active = false
        scopes[state.owner] = nil
    end
    state.handle = -1
end

function methods.close_handle(self: any): number
    local state = stateOf(self)
    if state.handle < 0 then
        return hex.ErrFile.NotExist
    end
    local result = ctl.close_file_handle(state.handle)
    if result == hex.ErrFile.Success or result == hex.ErrFile.NotExist then
        invalidate(self, state)
    end
    return result
end

local function open(document: any, url: any): number
    local state = stateOf(document)
    if state.handle >= 0 then
        local result = methods.close_handle(document)
        if result ~= hex.ErrFile.Success and result ~= hex.ErrFile.NotExist then
            return result
        end
    end
    local handle = ctl.open_file(url)
    if handle < 0 then
        return handle
    end
    state.handle = handle
    return hex.ErrFile.Success
end

function methods.open_file(self: any, path: any): number
    return open(self, native._file_url(path))
end

function methods.open_ext(self: any, driver: any, path: any): number
    return open(self, native._device_url(driver, path))
end

function methods.open_workspace(self: any, path: any): number
    return open(self, native._workspace_url(path))
end

function methods.close(self: any, force: boolean?): number
    local state = stateOf(self)
    if state.handle < 0 then
        return hex.ErrFile.NotExist
    end
    local result = ctl.close_file(state.handle, force == true)
    if result == hex.ErrFile.Success or result == hex.ErrFile.NotExist then
        invalidate(self, state)
    end
    return result
end

function methods.is_valid(self: any): boolean
    local state = states[self]
    if state == nil then
        error("Invalid hexdoc object", 2)
    end
    return state.handle >= 0
        and state.owner == native._document_context()
        and reader.is_file_opened(state.handle)
end

function methods.document_info(self: any): any
    return reader.document_info(stateOf(self).handle)
end

function methods.activate(self: any): boolean
    local state = stateOf(self)
    return state.handle >= 0 and ctl.raise_document(state.handle)
end

function methods.use(self: any, callback: any, ...: any): ...any
    if type(callback) ~= "function" then
        error("hexdoc use expects a callback", 2)
    end
    local state = stateOf(self)
    if state.handle < 0 then
        error("Cannot use an invalid document handle", 2)
    end
    local scope = scopes[state.owner]
    local outermost = scope == nil
    if scope ~= nil and scope.document ~= self then
        error("Another document is already in use in this script context", 2)
    end
    if outermost then
        if not ctl.switch_document(state.handle) then
            error("Cannot switch the script document", 2)
        end
        scope = { document = self, handle = state.handle, active = true }
        scopes[state.owner] = scope
    end
    local results = table.pack(native._call_sync(callback, self, ...))
    if outermost then
        scope.active = false
        if scopes[state.owner] == scope then
            scopes[state.owner] = nil
        end
    end
    if not results[1] then
        error(results[2], 0)
    end
    return table.unpack(results, 2, results.n)
end

for name, operation in operations do
    methods[name] = function(self: any, ...: any): ...any
        local state = stateOf(self)
        local scope = scopes[state.owner]
        if scope == nil or not scope.active or scope.document ~= self or scope.handle ~= state.handle then
            error("Document operations require a live use() callback", 2)
        end
        return operation(...)
    end
end

table.freeze(methods)
table.freeze(metatable)

local function new(): any
    local document = setmetatable({}, metatable)
    states[document] = { handle = -1, owner = native._document_context() }
    return table.freeze(document)
end

return table.freeze({ new = new }) :: { new: () -> Document }
