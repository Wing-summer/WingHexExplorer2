local json = require("@s/mcp/json")
local mcp = require("@s/mcp")
local function rejects(callback) assert(not pcall(callback), "Expected rejection") end
for _, value in { 'null', '{}', '[]', '[null,true,false,1,-2.5e3,"你好"]', '{"x":"\\uD83D\\uDE00"}' } do
    assert(json.encode(json.decode(json.encode(json.decode(value)))) == json.encode(json.decode(value)))
end
for _, value in { '01', '[1,]', 'NaN', 'null garbage' } do assert(not pcall(json.decode, value), "Expected rejection: " .. value) end
local server = mcp.new({name = "test", version = "1"})
server:tool({name = "echo", inputSchema = {type = "object"}}, function(args) return {content = {{type = "text", text = args.text or "empty"}}} end)
local meta = { ["io.modelcontextprotocol/protocolVersion"] = mcp.ProtocolVersion, ["io.modelcontextprotocol/clientInfo"] = {name = "test", version = "1"}, ["io.modelcontextprotocol/clientCapabilities"] = {} }
local request = {jsonrpc = "2.0", id = 1, method = "tools/call", params = {name = "echo", arguments = {text = "hi"}, _meta = meta}}
assert(server:dispatch(request).result.content[1].text == "hi")
request.params.arguments = {}; assert(server:dispatch(request).result.content[1].text == "empty")
request.id = nil; assert(server:dispatch(request) == nil)
request.id = 2; request.method = "tools/list"; assert(#server:dispatch(request).result.tools == 1)
request.method = "server/discover"; assert(server:dispatch(request).result.supportedVersions[1] == mcp.ProtocolVersion)
request.params._meta = {}; assert(server:dispatch(request).error.code == -32022)
assert(server:dispatch({}).error.code == -32600)
print("PASS standard library MCP and registered JSON")
