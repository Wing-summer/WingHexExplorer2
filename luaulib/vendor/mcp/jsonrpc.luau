local serde = require("@s/mcp/json")

local JsonRpc = {}

export type JsonRpcId = string | number

export type JsonRpcRequest = {
	jsonrpc: "2.0",
	method: string,
	id: JsonRpcId?,
	params: any?,
}

export type JsonRpcErrorObject = {
	code: number,
	message: string,
	data: any?,
}

export type JsonRpcResponse = {
	jsonrpc: "2.0",
	id: JsonRpcId?,
	result: any?,
	error: JsonRpcErrorObject?,
}

local function is_object(value: any): boolean
	return type(value) == "table"
end

local function is_array(value: any): boolean
	if type(value) ~= "table" then
		return false
	end
	local n = 0
	for k, _ in pairs(value) do
		if type(k) ~= "number" then
			return false
		end
		n += 1
	end
	return n == #value
end

function JsonRpc.decode(line: string): (boolean, any)
	local ok, value = pcall(function()
		return serde.decode(line)
	end)
	return ok, value
end

function JsonRpc.encode(value: any): string
	return serde.encode(value)
end

function JsonRpc.is_valid_request(value: any): (boolean, string?)
	if not is_object(value) then
		return false, "request is not an object"
	end
	if is_array(value) then
		return false, "batch requests are not supported"
	end
	if value.jsonrpc ~= "2.0" then
		return false, "jsonrpc must be '2.0'"
	end
	if type(value.method) ~= "string" then
		return false, "method must be a string"
	end
	if value.id ~= nil and type(value.id) ~= "string" and type(value.id) ~= "number" then
		return false, "id must be string|number when present"
	end
	return true
end

function JsonRpc.make_result(id: JsonRpcId?, result: any): JsonRpcResponse
	return { jsonrpc = "2.0", id = id, result = result }
end

function JsonRpc.make_error(id: JsonRpcId?, code: number, message: string, data: any?): JsonRpcResponse
	return { jsonrpc = "2.0", id = id or serde.null, error = { code = code, message = message, data = data } }
end

return JsonRpc
