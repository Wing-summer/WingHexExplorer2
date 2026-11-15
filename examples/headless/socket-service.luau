-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
-- Submit this entry with task.execute; task.stop owns its lifetime.
local network = require("@b/network")
local server = network.tcp_server()
assert(server:listen(network.host_address("127.0.0.1"), 0), tostring(server:error_string()))
println(server:server_port())
server:connect("new_connection", function()
    while server:has_pending_connections() do
        local socket = server:next_pending_connection()
        local function echo_pending()
            socket:write(socket:read_all())
        end
        socket:connect("ready_read", echo_pending)
        -- A queued accept callback can run after data has already arrived.
        echo_pending()
    end
end)
