local network = require("@network")
local name = "winghex-example-" .. tostring(math.random(1, 1000000000))
local server = network.local_server()
local client = network.local_socket()
local done = false
server:connect("new_connection", function()
    local peer = server:next_pending_connection()
    peer:connect("ready_read", function()
        print(peer:read_all())
        done = true
    end)
end)
assert(server:listen(name), server:error_string())
client:connect("connected", function()
    client:write("hello\0local")
end)
client:connect_to_server(name)
while not done do
    coroutine.yield()
end
server:close()
