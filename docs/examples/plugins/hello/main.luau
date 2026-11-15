local Inspector = plugin.Plugin:extend()
function Inspector:plugin_name() return "Luau Byte Inspector" end
function Inspector:plugin_comment() return "Pure-value byte statistics service" end

-- No UI registrations or shared-memory resources used by native test plugins.
plugin.service.export({
    byte_summary = function(self, bytes)
        assert(type(bytes) == "string", "Expected a byte string")
        local zeros, printable = 0, 0
        for index = 1, #bytes do
            local byte = string.byte(bytes, index)
            if byte == 0 then zeros += 1 end
            if byte >= 32 and byte <= 126 then printable += 1 end
        end
        return { length = #bytes, zeros = zeros, printable = printable }
    end,
})
return Inspector:new()
