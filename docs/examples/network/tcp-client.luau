local network = require("@network")
local socket = network.tcp_socket()
local done = false
socket:connect("connected", function()
    socket:write("hello\0tool")
end)
socket:connect("ready_read", function()
    print(socket:read_all())
    done = true
end)
socket:connect("error_occurred", function(code)
    print(code, socket:error_string())
    done = true
end)
socket:connect_to_host("127.0.0.1", 9000)
while not done do
    coroutine.yield()
end
socket:disconnect_from_host()
