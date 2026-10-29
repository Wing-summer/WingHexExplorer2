// Copyright (c) 2026, wingsummer
// SPDX-License-Identifier: BSD-3-Clause
// See 3rdparty/WingLuauQt/LICENSE for license information.

#include "Luau/Common.h"
#include "Luau/Compiler.h"
#include "generated_bindings.hpp"
#include "lualib.h"
#include "qtluau/callback.hpp"
#include "qtluau/execution.hpp"
#include "src/luau/runtime/scriptfilesystem.h"
#include "src/luau/wingluaurequire.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QScopeGuard>
#include <QSslCertificate>
#include <QSslCipher>
#include <QSslServer>
#include <QSslSocket>
#include <QTcpSocket>
#include <QThread>
#include <QUuid>
#include <iostream>
#include <stdexcept>
#include <thread>

LUAU_FASTFLAG(LuauIntegerType2)

namespace {
void run(lua_State *L, const QString &code) {
    QString error;
    if (!qtluau::executeLuau(L, code, &error)) {
        throw std::runtime_error(error.toStdString() + "\n" +
                                 code.toStdString());
    }
}
void pump(int duration = 100) {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < duration) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        QThread::msleep(1);
    }
}
void waitFor(lua_State *L, const QString &expression, int duration = 3000) {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < duration) {
        run(L, QStringLiteral("_network_done = (%1)").arg(expression));
        lua_getglobal(L, "_network_done");
        const bool done = lua_toboolean(L, -1);
        lua_pop(L, 1);
        if (done) {
            return;
        }
        pump(5);
    }
    const auto client = luabridge::LuaRef::getGlobal(L, "T")["tls_client"];
    if (!client.isNil()) {
        auto *socket = qtluau::qtFromLua<QSslSocket *>(client);
        std::cerr << "TLS state: " << socket->state() << "; "
                  << socket->errorString().toStdString() << '\n';
        std::cerr << "TLS client mode: " << socket->mode() << "; encrypted "
                  << socket->isEncrypted() << "; pending "
                  << socket->bytesToWrite() << "; ciphers "
                  << socket->sslConfiguration().ciphers().size() << '\n';
        auto *server = qtluau::qtFromLua<QSslServer *>(
            luabridge::LuaRef::getGlobal(L, "T")["tls_server"]);
        std::cerr << "TLS server: " << server->errorString().toStdString()
                  << "; pending " << server->hasPendingConnections() << '\n';
        for (auto *peer : server->findChildren<QSslSocket *>()) {
            std::cerr << "TLS peer: " << peer->state() << "; mode "
                      << peer->mode() << "; encrypted " << peer->isEncrypted()
                      << "; " << peer->errorString().toStdString() << '\n';
        }

        for (const auto &error : socket->sslHandshakeErrors()) {
            std::cerr << error.errorString().toStdString() << '\n';
        }
    }
    run(L, QStringLiteral("assert(%1, 'network fixture timed out')")
               .arg(expression));
}
void check(bool qtFirst) {
    auto *L = luaL_newstate();
    luaL_openlibs(L);
    lua_pushstring(L, qPrintable(QStringLiteral("winghex-") +
                                 QUuid::createUuid().toString(QUuid::Id128)));
    lua_setglobal(L, "_network_local_name");
    for (const auto &entry :
         {std::pair{"certificate.pem", "_network_certificate"},
          std::pair{"key.pem", "_network_key"}}) {
        QFile file(QStringLiteral(WING_NETWORK_FIXTURES) + QLatin1Char('/') +
                   QString::fromLatin1(entry.first));
        if (!file.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("Cannot read TLS fixture");
        }
        const auto data = file.readAll();
        lua_pushlstring(L, data.constData(), static_cast<size_t>(data.size()));
        lua_setglobal(L, entry.second);
    }
    auto cleanup = qScopeGuard([&] {
        qtluau::clearLuaCallbacks(L);
        lua_close(L);
    });
    int firstContext = 1;
    int secondContext = 2;
    lua_setthreaddata(L, &firstContext);
    qtluau::setLuaContextResolver(
        L, +[](lua_State *state) -> const void * {
            const auto data = lua_getthreaddata(state);
            return data ? data : state;
        });
    ScriptFileSystem::setFileSystemAccessPolicy(
        {[] { return false; }, [] { return false; }});
    bindings::registerEagerModules(L);
    lua_getfield(L, LUA_REGISTRYINDEX, "qtluau.registration.states");
    lua_getfield(L, -1, "sdk.QTcpSocket");
    if (!lua_isnil(L, -1)) {
        throw std::runtime_error("Network types registered eagerly");
    }
    lua_pop(L, 2);
    WingLuauRequire require;
    if (!require.initRequire(L, {})) {
        throw std::runtime_error("Cannot initialize require");
    }
    require.installRequire();
    int loads = 0;
    require.registerBuiltinLoader("network", [&](lua_State *caller) {
        ++loads;
        return qtluau::loadRegisteredModule(caller, "network",
                                            bindings::registerModule);
    });
    require.registerBuiltinLoader("qt", [&](lua_State *caller) {
        return qtluau::loadRegisteredModule(caller, "qt",
                                            bindings::registerModule);
    });
    if (qtFirst) {
        run(L, QStringLiteral("assert(require('@qt').QTcpSocket ~= nil)"));
    }
    run(L, QStringLiteral(R"LUA(
assert(network == nil and qt == nil)
N = require('@network')
assert(N == require('@b/network'))
assert(network == nil and qt == nil)
assert(not pcall(N.abstract_socket))
assert(N.ssl.SslProtocol ~= nil)
assert(N.host_address(N.host_address.SpecialAddress.LocalHost):is_loopback())
assert(N.host_address(2130706433):to_string() == '127.0.0.1')
T = { data = 'A' .. string.char(0) .. string.char(255) .. 'B', states = {}, reads = 0 }
T.server = N.tcp_server()
assert(T.server:listen(N.host_address(N.host_address.SpecialAddress.LocalHost), 0), T.server:error_string())
T.server:connect('new_connection', function()
    T.peer = T.server:next_pending_connection()
    T.peer:connect('ready_read', function()
        T.reads += 1
        T.received = (T.received or '') .. T.peer:read_all()
    end)
end)
T.client = N.tcp_socket()
T.client:connect('state_changed', function(state) table.insert(T.states, state) end)
T.client:connect('bytesWritten(qint64)', function(count) T.bytes = tostring(count) end)
T.client:connect_to_host('127.0.0.1', T.server:server_port())
assert(T.client:wait_for_connected(1000))
assert(#T.states == 0, 'signals must be queued')
)LUA"));
    lua_getfield(L, LUA_REGISTRYINDEX, "qtluau.registration.states");
    lua_getfield(L, -1, "base.widgets");
    const bool widgetsLoaded = !lua_isnil(L, -1);
    lua_pop(L, 2);
    if (widgetsLoaded != qtFirst) {
        throw std::runtime_error("Network import eagerly registered widgets");
    }
    waitFor(L, QStringLiteral("T.peer ~= nil"));
    run(L, QStringLiteral(R"LUA(
assert(tostring(T.client:write(T.data)) == '4')
T.client:flush()
)LUA"));
    waitFor(L, QStringLiteral("T.received == T.data and T.bytes == '4'"));
    auto *nativeSocket = qtluau::qtFromLua<QTcpSocket *>(
        luabridge::LuaRef::getGlobal(L, "T")["client"]);
    run(L, QStringLiteral("T.foreign_count = #T.states"));
    bool rejectedOtherThread = false;
    std::thread emitter([&] {
        try {
            qtluau::QObjectRef(nativeSocket).get();
        } catch (const std::exception &) {
            rejectedOtherThread = true;
        }
        Q_EMIT nativeSocket->stateChanged(QAbstractSocket::ConnectedState);
    });
    emitter.join();
    if (!rejectedOtherThread) {
        throw std::runtime_error("Socket wrapper accepted a foreign thread");
    }
    run(L, QStringLiteral("assert(#T.states == T.foreign_count)"));
    pump();
    run(L, QStringLiteral("assert(#T.states == T.foreign_count + 1)"));
    run(L, QStringLiteral(R"LUA(
assert(#T.states >= 1 and type(T.states[1]) == 'number')
assert(T.reads >= 1)
T.udp_receiver = N.udp_socket()
assert(T.udp_receiver:bind(N.host_address(N.host_address.SpecialAddress.LocalHost), 0))
T.udp_receiver:connect('ready_read', function() T.error_seen = true; error('expected network callback error') end)
T.udp_receiver:connect('ready_read', function() T.yield_seen = true; coroutine.yield() end)
T.udp_receiver:connect('ready_read', function() T.after_error = true end)
T.udp_sender = N.udp_socket()
assert(tostring(T.udp_sender:write_datagram(T.data, N.host_address('127.0.0.1'), T.udp_receiver:local_port())) == '4')
)LUA"));
    waitFor(L, QStringLiteral("T.after_error == true and T.error_seen == true "
                              "and T.yield_seen == true and "
                              "T.udp_receiver:has_pending_datagrams()"));
    run(L, QStringLiteral(R"LUA(
local packet = T.udp_receiver:receive_datagram()
assert(packet:is_valid() and packet:data() == T.data)
assert(packet:sender_port() == T.udp_sender:local_port())
assert(packet:sender_address():to_string() ~= '')
T.local_server = N.local_server()
T.local_client = N.local_socket()
assert(T.local_server:listen(_network_local_name))
local duplicate = N.local_server()
assert(not duplicate:listen(_network_local_name))
T.local_client:connect_to_server(_network_local_name)
assert(T.local_client:wait_for_connected(1000))
)LUA"));
    waitFor(L, QStringLiteral("T.local_server:has_pending_connections()"));
    run(L, QStringLiteral(R"LUA(
T.local_peer = T.local_server:next_pending_connection()
assert(tostring(T.local_client:write(bytearray(T.data))) == '4')
T.local_client:flush()
)LUA"));
    waitFor(L,
            QStringLiteral("tostring(T.local_peer:bytes_available()) == '4'"));
    run(L, QStringLiteral(R"LUA(
assert(T.local_peer:read_all() == T.data)
assert(not T.server:wait_for_new_connection(0))
local qt = require('@qt')
assert(qt.QTcpSocket == N.tcp_socket and qt.QUdpSocket == N.udp_socket)
assert(qt.QSslSocket == N.ssl_socket and qt.QSsl == N.ssl)
assert(qt.QHostAddress == N.host_address)
)LUA"));
    if (loads != 1) {
        throw std::runtime_error(
            "Repeated require registered network more than once");
    }
    if (QSslSocket::supportsSsl()) {
        run(L, QStringLiteral(R"LUA(
local certificate = N.ssl_certificate(_network_certificate)
local key = N.ssl_key(_network_key, N.ssl.KeyAlgorithm.Rsa)
assert(not certificate:is_null() and not key:is_null())
local server_config = N.ssl_configuration.default_configuration()
server_config:set_local_certificate(certificate)
server_config:set_private_key(key)
T.tls_server = N.ssl_server()
T.tls_server:set_ssl_configuration(server_config)
assert(T.tls_server:listen(N.host_address('127.0.0.1'), 0))
T.tls_server:connect('pending_connection_available', function()
    T.tls_peer = T.tls_server:next_pending_connection()
    assert(T.tls_peer:is_encrypted(), 'TLS accept lost its native type')
    T.tls_peer:connect('ready_read', function() T.tls_data = T.tls_peer:read_all() end)
end)
T.tls_client = N.ssl_socket()
local client_config = N.ssl_configuration.default_configuration()
client_config:set_ca_certificates({certificate})
assert(not pcall(function() client_config:set_ca_certificates({[2] = certificate}) end))
assert(not pcall(function() client_config:set_ca_certificates({bad = certificate}) end))
assert(not pcall(function() client_config:set_ca_certificates({false}) end))
local cycle = {}; cycle[1] = cycle
assert(not pcall(function() client_config:set_ca_certificates(cycle) end))
T.tls_client:set_ssl_configuration(client_config)
T.tls_client:connect_to_host_encrypted('127.0.0.1', T.tls_server:server_port())
)LUA"));
        waitFor(L, QStringLiteral(
                       "T.tls_peer ~= nil and T.tls_client:is_encrypted()"));
        run(L, QStringLiteral("assert(tostring(T.tls_client:write(T.data)) == "
                              "'4'); T.tls_client:flush()"));
        waitFor(L, QStringLiteral("T.tls_data == T.data"));
        run(L, QStringLiteral(R"LUA(
T.bad_tls = N.ssl_socket()
T.bad_tls:connect('ssl_errors', function(errors)
    T.tls_errors = errors
end)
T.bad_tls:connect_to_host_encrypted('127.0.0.1', T.tls_server:server_port())
)LUA"));
        waitFor(L, QStringLiteral("T.tls_errors ~= nil"));
        run(L,
            QStringLiteral(
                "assert(#T.tls_errors > 0 and not T.bad_tls:is_encrypted())"));
        QFile certificateFile(
            QStringLiteral(WING_NETWORK_FIXTURES "/certificate.pem"));
        if (!certificateFile.open(QIODevice::ReadOnly)) {
            throw std::runtime_error("Cannot read certificate fixture");
        }
        const QSslCertificate certificate(certificateFile.readAll());
        const QMap<QString, QList<QSslCertificate>> certificates = {
            {QStringLiteral("证书"), {certificate}}};
        const auto encoded = qtluau::qtToLua(L, certificates);
        if (qtluau::qtFromLua<QMap<QString, QList<QSslCertificate>>>(encoded) !=
            certificates) {
            throw std::runtime_error(
                "Typed certificate container roundtrip failed");
        }
        std::cout << "TLS success, typed accept, binary data and certificate "
                     "rejection passed\n";
    } else {
        std::cout << "SKIP TLS execution: Qt has no runtime SSL backend\n";
    }
    auto *waiting = lua_newthread(L);
    const int waitingRef = lua_ref(L, -1);
    lua_pop(L, 1);
    lua_setthreaddata(waiting, &firstContext);
    const auto bytecode = Luau::compile(R"LUA(
local network = require('@network')
T.yield_socket = network.udp_socket()
assert(T.yield_socket:bind(network.host_address('127.0.0.1'), 0))
T.yield_socket:connect('ready_read', function()
    while T.yield_socket:has_pending_datagrams() do
        T.yield_received = T.yield_socket:receive_datagram():data()
    end
    T.from_callback = network.tcp_socket()
end)
assert(tostring(T.yield_socket:write_datagram(T.data, network.host_address('127.0.0.1'), T.yield_socket:local_port())) == '4')
coroutine.yield('paused')
assert(T.yield_received == T.data)
return 'done'
)LUA");
    if (luau_load(waiting, "=network-coroutine-probe", bytecode.data(),
                  bytecode.size(), 0) != LUA_OK ||
        lua_resume(waiting, nullptr, 0) != LUA_YIELD) {
        throw std::runtime_error(
            std::string("Cannot suspend the socket-owning coroutine: ") +
            (lua_tostring(waiting, -1) ? lua_tostring(waiting, -1)
                                       : "no error"));
    }
    waitFor(L, QStringLiteral("T.yield_received == T.data"));
    if (lua_status(waiting) != LUA_YIELD || lua_gettop(waiting) != 1 ||
        std::string(lua_tostring(waiting, -1)) != "paused") {
        throw std::runtime_error(
            "Signal callback corrupted its suspended owner");
    }
    lua_settop(waiting, 0);
    if (lua_resume(waiting, nullptr, 0) != LUA_OK ||
        std::string(lua_tostring(waiting, -1)) != "done") {
        throw std::runtime_error(
            "Socket-owning coroutine did not resume correctly");
    }
    lua_settop(waiting, 0);
    lua_unref(L, waitingRef);
    auto *other = lua_newthread(L);
    const auto otherRef = lua_ref(L, -1);
    lua_pop(L, 1);
    lua_setthreaddata(other, &secondContext);
    run(other, QStringLiteral("OtherSocket = N.tcp_socket()"));
    run(L, QStringLiteral(
               "T.states_before_close = #T.states; "
               "T.client:disconnect_from_host(); T.local_server:close()"));
    qtluau::clearLuaContext(L);
    pump();
    run(L, QStringLiteral(R"LUA(
assert(#T.states == T.states_before_close, 'queued callback survived context cleanup')
assert(not pcall(function() return T.from_callback:state() end))
assert(not pcall(function() return T.client:state() end))
assert(not pcall(function() return T.peer:state() end))
assert(not pcall(function() return T.local_peer:state() end))
assert(OtherSocket:state() == N.abstract_socket.SocketState.UnconnectedState)
assert(not pcall(N.local_server.remove_server, _network_local_name))
)LUA"));
    ScriptFileSystem::setFileSystemAccessPolicy(
        {[] { return false; }, [] { return true; }});
    run(L, QStringLiteral("assert(pcall(N.local_server.remove_server, "
                          "_network_local_name))"));
    ScriptFileSystem::setFileSystemAccessPolicy(
        {[] { return false; }, [] { return false; }});
    qtluau::clearLuaContext(other);
    run(L, QStringLiteral(
               "assert(not pcall(function() return OtherSocket:state() end))"));
    lua_unref(L, otherRef);
    luaL_sandbox(L);
    auto *sandbox = lua_newthread(L);
    luaL_sandboxthread(sandbox);
    lua_setthreaddata(sandbox, &firstContext);
    run(sandbox,
        QStringLiteral("local module = require('@network'); assert(module == "
                       "require('@b/network')); assert(network == nil)"));
    lua_pop(L, 1);
    if (lua_gettop(L) != 0) {
        throw std::runtime_error("Queued callbacks leaked Lua stack values");
    }
    std::cout << "TCP, UDP, local socket, queued parameters and context "
                 "isolation passed\n";
}
} // namespace

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    FFlag::LuauIntegerType2.value = true;
    try {
        for (const bool qtFirst : {false, true}) {
            // Install fixture strings before the generated module registration.
            // The context test itself creates a fresh VM for each load order.
            check(qtFirst);
        }
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
