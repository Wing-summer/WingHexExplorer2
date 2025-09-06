// MIT Licensed
// see https://github.com/Paril/angelscript-debugger

#pragma once

#include <dap/io.h>
#include <dap/network.h>
#include <dap/protocol.h>

#include <memory>

class asIDBDebugger;
class asIDBDAPClient;

// implementation of DAP server that connects
// back to an asIDBDebugger.
class asIDBDAPServer {
public:
    // port to connect to
    const int port;

    // ptr back to debugger
    asIDBDebugger *dbg;

    asIDBDAPServer(int port, asIDBDebugger *debugger);

    virtual ~asIDBDAPServer();

    // queries
    bool ServerActive() { return server.get() != nullptr; }
    bool ClientConnected() { return ServerActive() && client.get() != nullptr; }

    // server management
    void StartServer();
    void StopServer();
    void Tick();

    // client management

    template <typename T>
    void SendEventToClient(const T &event) {
        const dap::TypeInfo *typeinfo = dap::TypeOf<T>::type();
        SendEventToClient(typeinfo, &event);
    }

protected:
    // server ptr
    std::unique_ptr<dap::net::Server> server;

    // connected client
    // TODO: does this have to be mutexed? I think it's
    // atomic and we only care if it's set or not, we never
    // call into the client from the main thread
    std::unique_ptr<asIDBDAPClient> client;

    void ClientConnected(const std::shared_ptr<dap::ReaderWriter> &socket);
    void ClientError(const char *msg);
    void SendEventToClient(const dap::TypeInfo *typeinfo, const void *event);
};
