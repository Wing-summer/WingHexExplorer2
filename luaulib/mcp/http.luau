-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: MIT
local json = require("@s/mcp/json")
local rpc = require("@s/vendor/mcp/jsonrpc")
local base64 = require("@s/base64")
local Http = {}
local function header_value(value)
    if string.sub(value, 1, 9) == "=?base64?" and string.sub(value, -2) == "?=" then
        return buffer.tostring(base64.decode(string.sub(value, 10, -3)))
    end
    return value
end
local function validate_headers(request, headers, server)
    local params = request.params
    local meta = if type(params) == "table" then params._meta else nil
    if not meta or not headers["mcp-protocol-version"] or headers["mcp-protocol-version"] ~= meta["io.modelcontextprotocol/protocolVersion"] then return false end
    if headers["mcp-method"] ~= request.method then return false end
    local name = if request.method == "resources/read" then params.uri
        elseif request.method == "tools/call" or request.method == "prompts/get" then params.name else nil
    if name ~= nil then
        if not headers["mcp-name"] or header_value(headers["mcp-name"]) ~= name then return false end
    end
    if request.method == "tools/call" then
        local entry = server.core._tools[params.name]
        for header, descriptor in if entry then entry.headerParams else {} do
            local value = params.arguments
            for _, key in descriptor.path do value = if type(value) == "table" then value[key] else nil end
            if value ~= nil and value ~= json.null then
                if not headers[header] then return false end
                local mirrored = header_value(headers[header])
                if descriptor.type == "integer" then
                    if type(value) ~= "number" or value % 1 ~= 0 or math.abs(value) > 9007199254740991 or tonumber(mirrored) ~= value then return false end
                elseif mirrored ~= tostring(value) then return false end
            end
        end
    end
    return true
end
function Http.serve(server, options)
    local network = require("@b/network")
    local core = require("@b/core")
    local host = require("@b/winghex")
    local address, port = options.host or "127.0.0.1", options.port or 0
    local path = options.path or "/mcp"
    local limit = options.maxMessageBytes or 1024 * 1024
    local maximum = options.maxConnections or 32
    assert(address == "127.0.0.1" or address == "::1", "MCP transport currently supports loopback listening only")
    assert(type(port) == "number" and port >= 0 and port <= 65535 and port % 1 == 0, "Invalid MCP port")
    assert(limit > 0 and limit % 1 == 0 and maximum > 0 and maximum % 1 == 0, "Invalid MCP limits")
    local listener = network.tcp_server()
    assert(listener:listen(network.host_address(address), port), tostring(listener:error_string()))
    local sockets, count, running, ticks = {}, 0, true, 0
    local timer = core.timer()
    local transport = { port = listener:server_port(), host = address, path = path }
    local function finish(socket)
        if sockets[socket] then sockets[socket] = nil; count -= 1 end
        socket:disconnect_from_host()
    end
    local function respond(socket, status, value)
        local payload = if value == nil then "" else json.encode(value)
        local names = { [200] = "OK", [202] = "Accepted", [400] = "Bad Request", [403] = "Forbidden", [404] = "Not Found", [405] = "Method Not Allowed", [413] = "Payload Too Large", [415] = "Unsupported Media Type", [503] = "Service Unavailable" }
        socket:write(string.format("HTTP/1.1 %d %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\nAllow: POST\r\n\r\n", status, names[status], #payload) .. payload)
        finish(socket)
    end
    local function process(socket)
        local state = sockets[socket]
        if not state then return end
        local pending = socket:bytes_available()
        if type(pending) ~= "number" then pending = pending:to_number() end
        if pending + #state.input > limit + 16384 then respond(socket, 413); return end
        state.input ..= socket:read_all()
        local ending = string.find(state.input, "\r\n\r\n", 1, true)
        if not ending then if #state.input > 16384 then respond(socket, 413) end; return end
        if ending > 16384 then respond(socket, 413); return end
        local head = string.sub(state.input, 1, ending - 1)
        local line = string.match(head, "^[^\r\n]+")
        local method, target = string.match(line, "^([A-Z]+) ([^ ]+) HTTP/1%.[01]$")
        if not method then respond(socket, 400); return end
        local headers = {}
        for line in string.gmatch(head .. "\r\n", "([^\r\n]+)\r\n") do
            if string.sub(line, 1, #method + 1) == method .. " " then continue end
            local key, value = string.match(line, "^([!#$%%&'*+.^_`|~%w-]+):[ \t]*(.-)[ \t]*$")
            if not key then respond(socket, 400); return end
            key = string.lower(key)
            if headers[key] ~= nil then respond(socket, 400); return end
            headers[key] = value
        end
        if target ~= path then respond(socket, 404); return end
        local origin = headers.origin
        if origin then
            local allowed = false
            for _, item in options.allowedOrigins or {} do if origin == item then allowed = true end end
            if not allowed then respond(socket, 403); return end
        end
        if method ~= "POST" then respond(socket, 405); return end
        if string.match(string.lower(headers["content-type"] or ""), "^([^; ]+)") ~= "application/json" then respond(socket, 415); return end
        if headers["transfer-encoding"] or not string.match(headers["content-length"] or "", "^%d+$") then respond(socket, 400); return end
        local length = tonumber(headers["content-length"])
        if length > limit then respond(socket, 413); return end
        local body = string.sub(state.input, ending + 4)
        if #body < length then return end
        if #body ~= length then respond(socket, 400); return end
        local ok, request = pcall(json.decode, body)
        if not ok then respond(socket, 400, rpc.make_error(nil, -32700, "Parse error")); return end
        local valid = rpc.is_valid_request(request)
        if not valid then respond(socket, 400, rpc.make_error(nil, -32600, "Invalid Request")); return end
        local headerOk, matches = pcall(validate_headers, request, headers, server)
        if not headerOk or not matches then respond(socket, 400, rpc.make_error(request.id, -32020, "Header mismatch")); return end
        local response = server:dispatch(request)
        local status = if response == nil then 202 elseif response.error and response.error.code == -32022 then 400
            elseif response.error and response.error.code == -32601 then 404 else 200
        respond(socket, status, response)
    end
    listener:connect("new_connection", function()
        while listener:has_pending_connections() do
            local socket = listener:next_pending_connection()
            host.release_socket_on_disconnect(socket)
            if count >= maximum or not running then respond(socket, 503); continue end
            sockets[socket] = { input = "", started = ticks }; count += 1
            socket:connect("disconnected", function()
                if sockets[socket] then sockets[socket] = nil; count -= 1 end
            end)
            socket:connect("ready_read", function() process(socket) end)
            process(socket)
        end
    end)
    timer:connect("timeout", function()
        ticks += 1
        for socket, state in sockets do if ticks - state.started >= 15 then socket:abort(); sockets[socket] = nil; count -= 1 end end
    end)
    timer:start(1000)
    function transport:stop()
        if not running then return end
        running = false; timer:stop(); listener:close()
        for socket in sockets do socket:abort() end
        table.clear(sockets); count = 0
    end
    return transport
end
return Http
