local network = require("@network")
assert(network.ssl_socket.supports_ssl(), "No TLS backend")
local socket = network.ssl_socket()
local done = false
socket:connect("encrypted", function()
    socket:write("hello\0tls")
end)
socket:connect("ready_read", function()
    print(socket:read_all())
    done = true
end)
socket:connect("ssl_errors", function(errors)
    for _, failure in errors do
        print(failure:error_string())
    end
    done = true
end)
socket:connect("error_occurred", function()
    print(socket:error_string())
    done = true
end)
-- Uses the default trusted CAs. Configure a private CA per socket when needed.
socket:connect_to_host_encrypted("localhost", 9443)
while not done do
    coroutine.yield()
end
