local network = require("@network")
assert(network.ssl_socket.supports_ssl(), "No TLS backend")
local function read_file(path: string): string
    local file = filesystem.file(path)
    assert(file:open(filesystem.file.OpenModeFlag.ReadOnly), file:error_string())
    local bytes = file:read_all()
    file:close()
    return bytes
end
local config = network.ssl_configuration.default_configuration()
config:set_local_certificate(network.ssl_certificate(read_file("certificate.pem")))
config:set_private_key(network.ssl_key(read_file("key.pem"), network.ssl.KeyAlgorithm.Rsa))
local server = network.ssl_server()
server:set_ssl_configuration(config)
local clients = {}
server:connect("pending_connection_available", function()
    while server:has_pending_connections() do
        local socket = server:next_pending_connection()
        assert(socket:is_encrypted())
        table.insert(clients, socket)
        socket:connect("ready_read", function()
            socket:write(socket:read_all())
        end)
    end
end)
assert(server:listen(network.host_address("127.0.0.1"), 9443), server:error_string())
while true do
    coroutine.yield()
end
