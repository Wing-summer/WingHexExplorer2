local JsonRpc = require("./jsonrpc")
local Schema = require("./schema")
local Middleware = require("./middleware")

type JsonRpcId = JsonRpc.JsonRpcId

local Server = {}
Server.__index = Server

export type ServerOptions = {
	name: string,
	version: string,
	concurrent: boolean?,
	framing: string?,
	maxInFlight: number?,
	maxMessageBytes: number?,
	maxPendingWrites: number?,
	maxPendingBytes: number?,
}

export type ToolDefinition = {
	name: string,
	description: string?,
	inputSchema: any?,
}

export type ToolHandler = (args: any) -> any

type ToolEntry = {
	definition: ToolDefinition,
	handler: ToolHandler,
}

export type ResourceDefinition = {
	uri: string,
	name: string?,
	description: string?,
	mimeType: string?,
}

export type ResourceHandler = (params: any) -> any

type ResourceEntry = {
	definition: ResourceDefinition,
	handler: ResourceHandler,
}

export type PromptDefinition = {
	name: string,
	description: string?,
	arguments: any?,
}

export type PromptHandler = (params: any) -> any

type PromptEntry = {
	definition: PromptDefinition,
	handler: PromptHandler,
}

type InFlightTaskMap = { [JsonRpcId]: thread }

local JSONRPC_ERRORS = {
	PARSE_ERROR = -32700,
	INVALID_REQUEST = -32600,
	METHOD_NOT_FOUND = -32601,
	INVALID_PARAMS = -32602,
	INTERNAL_ERROR = -32603,
	SERVER_NOT_INITIALIZED = -32002,
	SERVER_OVERLOADED = -32000,
}

function Server.new(options: ServerOptions)
	assert(type(options) == "table", "options must be a table")
	assert(type(options.name) == "string" and options.name ~= "", "options.name must be a non-empty string")
	assert(type(options.version) == "string" and options.version ~= "", "options.version must be a non-empty string")

	local self = setmetatable({}, Server)
	self._name = options.name
	self._version = options.version
	self._concurrent = options.concurrent == true
	self._framing = options.framing
	self._maxInFlight = options.maxInFlight or 64
	self._maxMessageBytes = options.maxMessageBytes
	self._maxPendingWrites = options.maxPendingWrites
	self._maxPendingBytes = options.maxPendingBytes
	self._tools = {} :: { [string]: ToolEntry }
	self._resources = {} :: { [string]: ResourceEntry }
	self._prompts = {} :: { [string]: PromptEntry }
	self._running = false
	self._didInitialize = true
	self._inFlightTasks = {} :: InFlightTaskMap
	self._middleware = {} :: { Middleware.Handler }
	return self
end

function Server:tool(definition: ToolDefinition, handler: ToolHandler)
	assert(type(definition) == "table", "definition must be a table")
	assert(type(definition.name) == "string" and definition.name ~= "", "definition.name must be a non-empty string")
	assert(type(handler) == "function", "handler must be a function")

	self._tools[definition.name] = {
		definition = definition,
		handler = handler,
	}
end

function Server:resource(definition: ResourceDefinition, handler: ResourceHandler)
	assert(type(definition) == "table", "definition must be a table")
	assert(type(definition.uri) == "string" and definition.uri ~= "", "definition.uri must be a non-empty string")
	assert(type(handler) == "function", "handler must be a function")

	self._resources[definition.uri] = {
		definition = definition,
		handler = handler,
	}
end

function Server:prompt(definition: PromptDefinition, handler: PromptHandler)
	assert(type(definition) == "table", "definition must be a table")
	assert(type(definition.name) == "string" and definition.name ~= "", "definition.name must be a non-empty string")
	assert(type(handler) == "function", "handler must be a function")

	self._prompts[definition.name] = {
		definition = definition,
		handler = handler,
	}
end

function Server:use(handler: Middleware.Handler)
	assert(type(handler) == "function", "middleware must be a function")
	table.insert(self._middleware, handler)
end

function Server:_with_middleware(method: string, id: any?, params: any?, terminal: () -> any): any
	if #self._middleware == 0 then
		return terminal()
	end
	local composed = Middleware.compose(self._middleware)
	return composed({ method = method, id = id, params = params }, terminal)
end

function Server:_list_tools()
	local tools = {}
	for _, entry in pairs(self._tools) do
		table.insert(tools, {
			name = entry.definition.name,
			description = entry.definition.description,
			inputSchema = entry.definition.inputSchema,
		})
	end
	table.sort(tools, function(a, b)
		return a.name < b.name
	end)
	return { tools = tools }
end

function Server:_list_prompts()
	local prompts = {}
	for _, entry in pairs(self._prompts) do
		table.insert(prompts, {
			name = entry.definition.name,
			description = entry.definition.description,
			arguments = entry.definition.arguments,
		})
	end
	table.sort(prompts, function(a, b)
		return a.name < b.name
	end)
	return { prompts = prompts }
end

function Server:_get_prompt(params: any)
	if type(params) ~= "table" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params must be an object"
	end
	if type(params.name) ~= "string" or params.name == "" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params.name must be a non-empty string"
	end

	local entry = self._prompts[params.name]
	if entry == nil then
		return false, JSONRPC_ERRORS.METHOD_NOT_FOUND, ("prompt not found: %s"):format(params.name)
	end

	local ok, resultOrErr = pcall(function()
		return entry.handler(params)
	end)
	if not ok then
		return false, JSONRPC_ERRORS.INTERNAL_ERROR, tostring(resultOrErr)
	end

	return true, resultOrErr
end

function Server:_list_resources()
	local resources = {}
	for _, entry in pairs(self._resources) do
		table.insert(resources, {
			uri = entry.definition.uri,
			name = entry.definition.name,
			description = entry.definition.description,
			mimeType = entry.definition.mimeType,
		})
	end
	table.sort(resources, function(a, b)
		return a.uri < b.uri
	end)
	return resources
end

function Server:_list_resources_paged(params: any)
	if params ~= nil and type(params) ~= "table" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params must be an object when present"
	end

	local cursor = nil
	local limit = 100

	if params ~= nil then
		if params.cursor ~= nil and type(params.cursor) ~= "string" then
			return false, JSONRPC_ERRORS.INVALID_PARAMS, "params.cursor must be a string when present"
		end
		if params.limit ~= nil and type(params.limit) ~= "number" then
			return false, JSONRPC_ERRORS.INVALID_PARAMS, "params.limit must be a number when present"
		end
		cursor = params.cursor
		if params.limit ~= nil then
			limit = math.floor(params.limit)
		end
	end

	if limit <= 0 then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params.limit must be > 0"
	end

	local all = self:_list_resources()
	local startIndex = 1
	if cursor ~= nil then
		for i, res in ipairs(all) do
			if res.uri == cursor then
				startIndex = i + 1
				break
			end
		end
	end

	local page = {}
	for i = startIndex, math.min(#all, startIndex + limit - 1) do
		table.insert(page, all[i])
	end

	local nextCursor = nil
	local nextIndex = startIndex + limit
	if nextIndex <= #all and #page > 0 then
		nextCursor = page[#page].uri
	end

	return true, { resources = page, nextCursor = nextCursor }
end

function Server:_read_resource(params: any)
	if type(params) ~= "table" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params must be an object"
	end
	if type(params.uri) ~= "string" or params.uri == "" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params.uri must be a non-empty string"
	end

	local entry = self._resources[params.uri]
	if entry == nil then
		return false, JSONRPC_ERRORS.METHOD_NOT_FOUND, ("resource not found: %s"):format(params.uri)
	end

	local ok, resultOrErr = pcall(function()
		return entry.handler(params)
	end)
	if not ok then
		return false, JSONRPC_ERRORS.INTERNAL_ERROR, tostring(resultOrErr)
	end

	return true, resultOrErr
end

function Server:_call_tool(params: any)
	if type(params) ~= "table" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params must be an object"
	end
	if type(params.name) ~= "string" or params.name == "" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params.name must be a non-empty string"
	end

	local entry = self._tools[params.name]
	if entry == nil then
		return false, JSONRPC_ERRORS.METHOD_NOT_FOUND, ("tool not found: %s"):format(params.name)
	end

	local args = params.arguments or {}
	if entry.definition.inputSchema ~= nil then
		local okSchema, errors = Schema.validate(entry.definition.inputSchema, args)
		if not okSchema then
			return false, JSONRPC_ERRORS.INVALID_PARAMS, "arguments failed schema validation", errors
		end
	end

	local ok, resultOrErr = pcall(function()
		return entry.handler(args)
	end)
	if not ok then
		return true, {isError = true, content = {{type = "text", text = tostring(resultOrErr)}}}
	end

	return true, resultOrErr
end

function Server:_initialize(_params: any)
	self._didInitialize = true
	return {
		protocolVersion = "2024-11-05",
		serverInfo = {
			name = self._name,
			version = self._version,
		},
		capabilities = {
			tools = {},
			resources = {},
			prompts = {},
		},
	}
end

function Server:_cancel_request(params: any): (boolean, number?, string?, any?)
	if type(params) ~= "table" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params must be an object"
	end
	local cancelId = params.id
	if cancelId ~= nil and type(cancelId) ~= "string" and type(cancelId) ~= "number" then
		return false, JSONRPC_ERRORS.INVALID_PARAMS, "params.id must be a string or number"
	end

	if cancelId ~= nil then
		local running = self._inFlightTasks[cancelId]
		if running ~= nil then
			coroutine.close(running)
			self._inFlightTasks[cancelId] = nil
		end
	end

	return true, nil, nil, {}
end

function Server:_handle_request(request: JsonRpc.JsonRpcRequest): JsonRpc.JsonRpcResponse?
	local id: JsonRpcId? = request.id
	local method = request.method

	return self:_with_middleware(method, id, request.params, function()
		return self:_handle_request_core(request)
	end)
end

function Server:_handle_request_core(request: JsonRpc.JsonRpcRequest): JsonRpc.JsonRpcResponse?
	local id: JsonRpcId? = request.id
	local method = request.method

	if method ~= "initialize" and self._didInitialize == false then
		return JsonRpc.make_error(id, JSONRPC_ERRORS.SERVER_NOT_INITIALIZED, "Server not initialized")
	end

	if method == "initialize" then
		return JsonRpc.make_result(id, self:_initialize(request.params))
	end

	-- MCP uses an "initialized" notification after initialize (no response required).
	if method == "initialized" then
		return nil
	end

	-- LSP-style cancellation used by many JSON-RPC clients.
	if method == "$/cancelRequest" then
		local ok, code, message, data = self:_cancel_request(request.params)
		if ok then
			if id == nil then
				return nil
			end
			return JsonRpc.make_result(id, data)
		end
		return JsonRpc.make_error(id, code :: number, tostring(message), data)
	end

	if method == "tools/list" then
		return JsonRpc.make_result(id, self:_list_tools())
	end

	if method == "tools/call" then
		local ok, codeOrResult, message, data = self:_call_tool(request.params)
		if ok then
			return JsonRpc.make_result(id, codeOrResult)
		end
		return JsonRpc.make_error(id, codeOrResult, tostring(message), data)
	end

	if method == "resources/list" then
		local ok, resultOrCode, message = self:_list_resources_paged(request.params)
		if ok then
			return JsonRpc.make_result(id, resultOrCode)
		end
		return JsonRpc.make_error(id, resultOrCode, tostring(message))
	end

	if method == "resources/read" then
		local ok, codeOrResult, message = self:_read_resource(request.params)
		if ok then
			return JsonRpc.make_result(id, codeOrResult)
		end
		return JsonRpc.make_error(id, codeOrResult, tostring(message))
	end

	if method == "prompts/list" then
		return JsonRpc.make_result(id, self:_list_prompts())
	end

	if method == "prompts/get" then
		local ok, codeOrResult, message = self:_get_prompt(request.params)
		if ok then
			return JsonRpc.make_result(id, codeOrResult)
		end
		return JsonRpc.make_error(id, codeOrResult, tostring(message))
	end

	return JsonRpc.make_error(id, JSONRPC_ERRORS.METHOD_NOT_FOUND, ("Method not found: %s"):format(method))
end


return Server
