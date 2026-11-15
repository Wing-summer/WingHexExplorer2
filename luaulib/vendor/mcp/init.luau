local Server = require("./server")

local Mcp = {}

export type ServerOptions = Server.ServerOptions
export type ToolDefinition = Server.ToolDefinition
export type ToolHandler = Server.ToolHandler
export type ResourceDefinition = Server.ResourceDefinition
export type ResourceHandler = Server.ResourceHandler
export type PromptDefinition = Server.PromptDefinition
export type PromptHandler = Server.PromptHandler

function Mcp.new(options: ServerOptions)
	return Server.new(options)
end

return Mcp
