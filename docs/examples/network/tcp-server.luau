local network = require("@network")
local server = network.tcp_server()
local clients = {}
server:connect("new_connection", function()
    while server:has_pending_connections() do
        local socket = server:next_pending_connection()
        table.insert(clients, socket)
        socket:connect("ready_read", function()
            socket:write(socket:read_all())
        end)
    end
end)
assert(server:listen(network.host_address("127.0.0.1"), 9000), server:error_string())
-- Stop this script using the scripting controls to close all its connections.
while true do
    coroutine.yield()
end
