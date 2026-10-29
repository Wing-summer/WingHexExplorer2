local network = require("@network")
local socket = network.udp_socket()
local done = false
assert(socket:bind(network.host_address("127.0.0.1"), 0))
socket:connect("ready_read", function()
    while socket:has_pending_datagrams() do
        local packet = socket:receive_datagram()
        print(packet:sender_address():to_string(), packet:sender_port(), packet:data())
        done = true
    end
end)
socket:write_datagram("hello\0udp", network.host_address("127.0.0.1"), socket:local_port())
while not done do
    coroutine.yield()
end
