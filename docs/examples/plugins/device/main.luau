local Tail = plugin.DevicePlugin:extend()
function Tail:plugin_name() return "Luau Tail Window" end
function Tail:plugin_comment() return "Read-only view of the final 4096 file bytes" end
function Tail:supported_file_ext_display_name() return "File tail (Luau, read-only)" end
function Tail:on_open_file_begin() return "" end -- Use the host file dialog.
function Tail:on_open_file(path)
    local backend = plugin.io.open_file(path, 1) -- QIODevice::ReadOnly
    local length = math.min(4096, backend:size())
    local start = backend:size() - length
    if not backend:seek(start) then backend:close(); return nil end
    local object = { writable = false, sequential = false, keep_size = true }
    function object:read(count)
        return backend:read(math.min(count, length - (backend:pos() - start)))
    end
    function object:size() return length end
    function object:seek(position)
        return position >= 0 and position <= length and backend:seek(start + position)
    end
    function object:close() backend:close(); return true end
    return object
end
function Tail:on_close_file() return true end
return Tail:new()
