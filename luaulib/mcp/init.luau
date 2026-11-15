-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: MIT
local api = require("@s/mcp/init_api")
export type ServerOptions = api.ServerOptions
export type ToolDefinition = api.ToolDefinition
export type ResourceDefinition = api.ResourceDefinition
export type PromptDefinition = api.PromptDefinition
export type HttpOptions = api.HttpOptions
export type Transport = api.Transport
export type Server = api.Server

local Vendor = require("@s/vendor/mcp/server")
local rpc = require("@s/vendor/mcp/jsonrpc")
local json = require("@s/mcp/json")
local Http = require("@s/mcp/http")
local Mcp = { ProtocolVersion = "2026-07-28" }
local Methods = {}
Methods.__index = Methods
for _, method in { "resource", "prompt", "use" } do
    Methods[method] = function(self, ...) return self.core[method](self.core, ...) end
end
local function collect_headers(schema, path, headers, seen, depth)
    assert(type(schema) == "table" and depth <= 32 and not seen[schema], "Invalid or cyclic tool schema")
    seen[schema] = true
    local annotation = schema["x-mcp-header"]
    if annotation then
        assert(path ~= nil and #path > 0 and type(annotation) == "string" and string.match(annotation, "^[!#$%%&'*+.^_`|~%w-]+$"), "Invalid x-mcp-header")
        assert(schema.type == "string" or schema.type == "integer" or schema.type == "boolean", "Header parameter must be a primitive")
        local key = "mcp-param-" .. string.lower(annotation)
        assert(headers[key] == nil, "Duplicate x-mcp-header")
        headers[key] = {path = path, type = schema.type}
    end
    for key, child in schema do
        if type(child) == "table" then
            if key == "properties" then
                for name, property in child do
                    local nextPath = if path then table.clone(path) else nil
                    if nextPath then table.insert(nextPath, name) end
                    collect_headers(property, nextPath, headers, seen, depth + 1)
                end
            else
                -- Annotations below composition, arrays or references cannot mirror headers.
                collect_headers(child, nil, headers, seen, depth + 1)
            end
        end
    end
    seen[schema] = nil
end
function Methods:tool(definition, handler)
    local copy = table.clone(definition)
    copy.inputSchema = copy.inputSchema or {type = "object"}
    assert(type(copy.inputSchema) == "table", "Expected tool input schema")
    local headers = {}
    collect_headers(copy.inputSchema, {}, headers, {}, 0)
    self.core:tool(copy, handler)
    self.core._tools[copy.name].headerParams = headers
end
function Mcp.new(options)
    return setmetatable({ core = Vendor.new(options), transport = nil }, Methods)
end
function Methods:dispatch(request)
    local valid, reason = rpc.is_valid_request(request)
    if not valid then return rpc.make_error(nil, -32600, "Invalid Request", reason) end
    local notification = request.id == nil
    local params = request.params
    local meta = if type(params) == "table" then params._meta else nil
    local response
    if type(meta) ~= "table" or meta["io.modelcontextprotocol/protocolVersion"] ~= Mcp.ProtocolVersion then
        response = rpc.make_error(request.id, -32022, "Unsupported protocol version", { supported = {Mcp.ProtocolVersion}, requested = if meta then meta["io.modelcontextprotocol/protocolVersion"] else json.null })
    elseif type(meta["io.modelcontextprotocol/clientInfo"]) ~= "table" or type(meta["io.modelcontextprotocol/clientCapabilities"]) ~= "table" then
        response = rpc.make_error(request.id, -32602, "Missing client metadata")
    elseif request.method == "server/discover" then
        response = rpc.make_result(request.id, { supportedVersions = {Mcp.ProtocolVersion}, _meta = { ["io.modelcontextprotocol/serverInfo"] = { name = self.core._name, version = self.core._version } }, capabilities = { tools = {}, resources = {}, prompts = {} } })
    elseif request.method == "initialize" or request.method == "initialized" or request.method == "$/cancelRequest" then
        response = rpc.make_error(request.id, -32601, "This server supports MCP " .. Mcp.ProtocolVersion)
    elseif request.method == "ping" then response = rpc.make_result(request.id, {})
    else
        local ok, result = pcall(self.core._handle_request, self.core, request)
        response = if ok then result else rpc.make_error(request.id, -32603, "Internal error")
    end
    if notification then return nil end
    if response and response.result then
        response.result = table.clone(response.result)
        response.result.resultType = "complete"
        response.result.ttlMs = 0
        response.result.cacheScope = "private"
        for _, key in { "tools", "resources", "prompts", "contents", "content", "messages" } do
            if response.result[key] then response.result[key] = json.array(table.clone(response.result[key])) end
        end
    end
    return response
end
function Methods:serve_http(options)
    assert(not self.transport, "MCP server is already running")
    self.transport = Http.serve(self, options or {})
    return self.transport
end
function Methods:stop()
    if self.transport then self.transport:stop(); self.transport = nil end
end
return table.freeze(Mcp) :: api.Module
