# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: AGPL-3.0-only
"""Exercise the real warm script service over framed JSON-RPC.
通过带长度帧的 JSON-RPC 验证真实常驻脚本服务。
"""
import argparse
from contextlib import contextmanager
import json
import os
from pathlib import Path
from queue import Queue, Empty
import re
import shutil
import signal
import sys
import socket
import ssl
import subprocess
import tempfile
from threading import Thread
import time


class Rpc:
    def __init__(self, connection):
        self.socket = connection
        self.buffer = b""
        self.next_id = 0
        self.notifications = []

    @staticmethod
    def frame(value):
        body = json.dumps(value, ensure_ascii=False).encode()
        return f"Content-Length: {len(body)}\r\n\r\n".encode() + body

    def receive(self):
        while b"\r\n\r\n" not in self.buffer:
            data = self.socket.recv(65536)
            assert data, "RPC connection closed"
            self.buffer += data
        header, body = self.buffer.split(b"\r\n\r\n", 1)
        length = int(header.split(b":", 1)[1])
        while len(body) < length:
            data = self.socket.recv(65536)
            assert data, "Truncated RPC body"
            body += data
        self.buffer = body[length:]
        return json.loads(body[:length])

    def call(self, method, params=None, error=None, fragmented=False):
        self.next_id += 1
        message = dict(jsonrpc="2.0", id=self.next_id, method=method,
                       params={} if params is None else params)
        frame = self.frame(message)
        if fragmented:
            for i in range(0, len(frame), 3):
                self.socket.sendall(frame[i:i + 3])
        else:
            self.socket.sendall(frame)
        while True:
            response = self.receive()
            if "method" in response:
                self.notifications.append(response)
                continue
            assert response.get("id") == self.next_id, response
            if error is not None:
                assert response["error"]["code"] == error, response
                return response
            assert "error" not in response, response
            return response["result"]

    def state(self, task_id, wanted, timeout=8):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            record = self.call("task.get", {"taskId": task_id})
            if record["state"] == wanted:
                return record
            assert record["state"] != "failed" or wanted == "failed", record
            time.sleep(.01)
        raise AssertionError((task_id, wanted, record))


def drain(stream, destination, ready=None):
    for line in iter(stream.readline, b""):
        destination.append(line)
        if ready and b"Script service listening on " in line:
            ready.put(line.decode().strip().split(" on ", 1)[1])


@contextmanager
def service(binary, env, options=(), code=0):
    ready = Queue()
    output, diagnostics = [], []
    process = subprocess.Popen([str(binary), "--headless", "--live", "--port", "0", *options],
                               env=env, stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    threads = [Thread(target=drain, args=(process.stdout, output), daemon=True),
               Thread(target=drain, args=(process.stderr, diagnostics, ready), daemon=True)]
    for thread in threads:
        thread.start()
    try:
        try:
            endpoint = ready.get(timeout=20)
        except Empty:
            raise AssertionError((process.poll(), b"".join(diagnostics)))
        port = int(endpoint.rsplit(":", 1)[1])
        yield process, port, output, diagnostics
        assert process.wait(timeout=15) == code, b"".join(diagnostics)
    except Exception:
        print(b"".join(diagnostics).decode(errors="replace"), file=sys.stderr)
        raise
    finally:
        if process.poll() is None:
            process.kill()
            process.wait()
        for thread in threads:
            thread.join(timeout=2)


def connect(port, context=None):
    connection = socket.create_connection(("127.0.0.1", port), timeout=3)
    if context:
        connection = context.wrap_socket(connection, server_hostname="localhost")
    return Rpc(connection)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--plugins", action="store_true")
    parser.add_argument("--slow", action="store_true")
    args = parser.parse_args()
    binary = args.binary.resolve()
    assert "HeadlessProbe" in binary.name, "Use the isolated-settings probe"
    with tempfile.TemporaryDirectory(prefix="wing-service-") as directory:
        temporary = Path(directory)
        config = temporary / "config"
        ini = config / "WingCloudStudio/WingHexExplorer2.conf"
        ini.parent.mkdir(parents=True)
        ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=false\nscript.timeout=1\n")
        runtime = temporary / "runtime"
        runtime.mkdir(mode=0o700)
        env = os.environ.copy()
        env.update(QT_QPA_PLATFORM="offscreen", XDG_RUNTIME_DIR=str(runtime),
                   XDG_CONFIG_HOME=str(config), XDG_DATA_HOME=str(temporary / "data"),
                   XDG_CACHE_HOME=str(temporary / "cache"), WINGHEX_HEADLESS_TEST_CONFIG=str(config), WINGHEX_HEADLESS_TEST_NO_UI="1")
        env.pop("DISPLAY", None)
        env.pop("WAYLAND_DISPLAY", None)
        def script(name, source):
            path = temporary / (name + ".luau")
            path.write_text(source)
            return str(path)
        ok = script("入口", 'println("entry-output"); return')
        failure = script("failure", 'error("entry failure")')
        fake_timeout = script("fake-timeout", 'error("timed-out is just user text")')
        syntax = script("syntax", ') invalid')
        forever = script("forever", 'while true do end')
        input_file = script("input", 'input()')
        no_host = script("no-host", 'require("@b/host")')
        timer = ('local c=require("@b/core"); local t=c.timer(); '
                 't:set_single_shot(true); t:connect("timeout", function() %s end); t:start(10)')
        async_failure = script("async-error", timer % 'error("async failure")')
        busy_callback = script("busy-callback", timer % 'while true do end')
        script("state", 'return {count=0}')
        isolated = script("isolated", 'assert(marker==nil); marker=true; '
                          'local m=require("./state"); assert(m.count==0); m.count=1; '
                          + timer % 'assert(marker and m.count==1); println("isolated")')
        script("module-loop", 'while true do end')
        module_loop = script("requiring-loop", 'require("./module-loop")')
        child_loop = script("child-loop", 'local co=coroutine.create(function() while true do end end); '
                            'assert(coroutine.resume(co))')
        wrapped_loop = script("wrapped-loop", 'coroutine.wrap(function() while true do end end)()')
        coroutine_checks = script("coroutines", r'''local function work()
    local sum=0
    for i=1,1000000 do sum+=i end
    assert(sum==500000500000)
    return require("./state")
end
local child=coroutine.create(function()
    local m=work()
    coroutine.yield(11,22)
    assert(work()==m)
    return 33
end)
local ok,a,b=coroutine.resume(child)
assert(ok and a==11 and b==22)
local ok,result=coroutine.resume(child)
assert(ok and result==33)
local wrapped=coroutine.wrap(function()
    assert(work().count==0)
    coroutine.yield(44)
    assert(work().count==0)
    return 55
end)
assert(wrapped()==44)
assert(wrapped()==55)
assert(not pcall(wrapped))
local outer=coroutine.create(function()
    return coroutine.wrap(function() return work() end)()
end)
local ok,m=coroutine.resume(outer)
assert(ok and m.count==0)
''')
        repeated = script("repeated", r'''local core=require("@b/core")
local taskState=require("./state")
local count=0
local timer=core.timer()
timer:connect("timeout", function()
    count+=1
    assert(require("./state")==taskState)
    assert(taskState.count==count-1)
    taskState.count=count
    local child=coroutine.create(function() return count end)
    local ok,value=coroutine.resume(child)
    assert(ok and value==count)
    if count==5 then
        timer:stop()
        println("repeated-callbacks-passed")
    end
end)
timer:start(5)
''')
        script("nested-state", 'return require("./state")')
        nested = script("nested", 'local m=require("./nested-state"); assert(m.count==0); m.count=2')
        invalid_display = dict(env, DISPLAY="invalid-display", QT_QPA_PLATFORM="invalid-platform")
        for flag in ["--help", "--version"]:
            result = subprocess.run([str(binary), "--headless", "--live", flag],
                                    env=invalid_display, capture_output=True, timeout=10)
            assert result.returncode == 0 and result.stdout, result
        for params in (["--headless", "--live", ok], ["--port", "42"],
                       ["--headless", "--live", "--port", "65536"],
                       ["--headless", "--live", "--address", "0.0.0.0"],
                       ["--headless", "--live", "--cert", "missing"]):
            result = subprocess.run([str(binary), *params], env=env, capture_output=True, timeout=10)
            assert result.returncode == 6, (params, result)
        display_free = env.copy()
        display_free.pop("QT_QPA_PLATFORM", None)
        with service(binary, display_free) as (process, port, output, diagnostics):
            rpc = connect(port)
            assert rpc.call("service.status", fragmented=True)["activeTasks"] == 0
            rpc.call("service.subscribe", {"enabled": True})
            occupied = subprocess.run([str(binary), "--headless", "--live", "--port", str(port)],
                                      env=env, capture_output=True, timeout=10)
            assert occupied.returncode == 9, occupied
            rpc.call("absent", error=-32601)
            rpc.call("task.get", {}, error=-32602)
            rpc.call("task.get", {"taskId": "absent"}, error=-32001)
            rpc.call("task.execute", {"taskId": "absent", "path": ok + ".missing"}, error=-32005)
            for task_id, path in [("a", isolated), ("b", isolated), ("nested", nested), ("nested2", nested),
                                  ("coroutines", coroutine_checks), ("repeated", repeated), ("normal", ok), ("bad", failure), ("syntax", syntax), ("fake-timeout", fake_timeout),
                                  ("stdin", input_file), ("old-host", no_host), ("async", async_failure)]:
                rpc.call("task.execute", {"taskId": task_id, "path": path})
            for task_id in ["a", "b", "nested", "nested2", "normal", "coroutines", "repeated"]:
                rpc.state(task_id, "resident")
            deadline = time.monotonic() + 3
            while b"repeated-callbacks-passed\n" not in b"".join(output):
                assert time.monotonic() < deadline, rpc.call("task.get", {"taskId": "repeated"})
                time.sleep(.01)
            for task_id in ["bad", "syntax", "stdin", "old-host", "async"]:
                assert rpc.state(task_id, "failed")["error"]["message"]
            assert rpc.state("fake-timeout", "failed")["error"]["code"] == "execution-error"
            rpc.call("task.execute", {"taskId": "a", "path": ok}, error=-32002)
            rpc.call("task.remove", {"taskId": "a"}, error=-32003)
            # Entry, module and callback CPU loops must all yield to management I/O.
            for task_id, path in [("loop", forever), ("module", module_loop), ("callback", busy_callback), ("child", child_loop), ("wrapped", wrapped_loop)]:
                rpc.call("task.execute", {"taskId": task_id, "path": path})
                time.sleep(.08)
                if task_id in ("child", "wrapped", "module"):
                    assert rpc.call("task.get", {"taskId": task_id})["state"] == "running"
                started = time.monotonic()
                rpc.call("service.status")
                assert time.monotonic() - started < 1
                rpc.call("task.stop", {"taskId": task_id})
                rpc.state(task_id, "stopped")
                rpc.call("task.remove", {"taskId": task_id})
            # Stopped IDs can be reused without retaining a callback/context.
            rpc.call("task.stop", {"taskId": "a"})
            rpc.state("a", "stopped")
            rpc.call("task.remove", {"taskId": "a"})
            rpc.call("task.execute", {"taskId": "a", "path": isolated})
            rpc.state("a", "resident")
            assert any(x["method"] == "task.changed" for x in rpc.notifications)
            rpc.call("service.subscribe", {"enabled": False})
            # Batch and notifications, including a following request in the same write.
            batch = [dict(jsonrpc="2.0", id="batch", method="task.list"),
                     dict(jsonrpc="2.0", method="service.status")]
            rpc.socket.sendall(Rpc.frame(batch) + Rpc.frame(dict(jsonrpc="2.0", id="next", method="service.status")))
            while True:
                response = rpc.receive()
                if isinstance(response, list):
                    assert len(response) == 1 and response[0]["id"] == "batch"
                    break
            assert rpc.receive()["id"] == "next"
            rpc.socket.sendall(b"Content-Length: 1\r\n\r\n{")
            assert rpc.receive()["error"]["code"] == -32700
            rpc.socket.sendall(Rpc.frame(17))
            assert rpc.receive()["error"]["code"] == -32600
            for malformed in [b"Content-Length: -1\r\n\r\n",
                              b"Content-Length: 1048577\r\n\r\n",
                              b"Content-Length: 1\r\nContent-Length: 1\r\n\r\n{",
                              b"X: " + b"x" * 8192]:
                with socket.create_connection(("127.0.0.1", port), timeout=3) as bad:
                    bad.sendall(malformed)
                    try:
                        assert bad.recv(1) == b""
                    except ConnectionResetError:
                        pass
            rpc.socket.close()
            rpc = connect(port)
            assert rpc.call("task.get", {"taskId": "a"})["state"] == "resident"
            # Other clients cannot consume the final connection slot or evict this session.
            held = [socket.create_connection(("127.0.0.1", port), timeout=3) for _ in range(63)]
            rpc.call("service.status")
            with socket.create_connection(("127.0.0.1", port), timeout=3) as overflow:
                try:
                    assert overflow.recv(1) == b""
                except ConnectionResetError:
                    pass
            for client in held:
                client.close()
            active = rpc.call("service.status")["activeTasks"]
            for i in range(64 - active):
                rpc.call("task.execute", {"taskId": f"capacity-{i}", "path": ok})
            rpc.call("task.execute", {"taskId": "overflow", "path": ok}, error=-32004)
            # Script socket data and management frames use different listening ports.
            echo = script("echo", Path(__file__).parents[2].joinpath("examples/headless/socket-service.luau").read_text())
            rpc.call("task.stop", {"taskId": "normal"})
            rpc.state("normal", "stopped")
            rpc.call("task.execute", {"taskId": "echo", "path": echo})
            rpc.state("echo", "resident")
            deadline = time.monotonic() + 3
            echo_port = None
            while time.monotonic() < deadline:
                for line in list(output):
                    if line.strip().isdigit(): echo_port = int(line)
                if echo_port: break
                time.sleep(.01)
            assert echo_port, output
            with socket.create_connection(("127.0.0.1", echo_port), timeout=3) as peer:
                peer.sendall(b"script socket payload\n")
                try:
                    assert peer.recv(64) == b"script socket payload\n"
                except (OSError, AssertionError) as error:
                    raise AssertionError((rpc.call("task.get", {"taskId": "echo"}),
                                          b"".join(output), b"".join(diagnostics))) from error
            rpc.call("task.stop", {"taskId": "echo"})
            rpc.state("echo", "stopped")
            try:
                socket.create_connection(("127.0.0.1", echo_port), timeout=.3).close()
                raise AssertionError("Task socket survived stop")
            except OSError:
                pass
            if args.slow:
                rpc.call("task.stop", {"taskId": "b"})
                rpc.state("b", "stopped")
                delayed = script("delayed", timer.replace("t:start(10)", "t:start(61000)") % 'println("late-timer")')
                rpc.call("task.execute", {"taskId": "late", "path": delayed})
                rpc.state("late", "resident")
                rpc.call("task.execute", {"taskId": "timeout", "path": forever})
                assert rpc.state("timeout", "failed", timeout=70)["error"]["code"] == "timeout"
                assert rpc.state("a", "resident")["state"] == "resident"
                deadline = time.monotonic() + 5
                while b"late-timer\n" not in b"".join(output):
                    assert time.monotonic() < deadline, (output, diagnostics)
                    time.sleep(.01)
                assert rpc.state("late", "resident")["state"] == "resident"
            for record in rpc.call("task.list"):
                if record["state"] not in ("failed", "stopped"):
                    rpc.call("task.stop", {"taskId": record["taskId"]})
            deadline = time.monotonic() + 5
            while rpc.call("service.status")["activeTasks"]:
                assert time.monotonic() < deadline
                time.sleep(.01)
            records = rpc.call("service.status")["taskRecords"]
            while records < 1024:
                size = min(32, 1024 - records)
                batch = [dict(jsonrpc="2.0", id=f"record-{records + i}", method="task.execute",
                              params=dict(taskId=f"record-{records + i}", path=syntax)) for i in range(size)]
                rpc.socket.sendall(Rpc.frame(batch))
                replies = rpc.receive()
                assert len(replies) == size and all("result" in item for item in replies), replies
                deadline = time.monotonic() + 5
                while rpc.call("service.status")["activeTasks"]:
                    assert time.monotonic() < deadline
                    time.sleep(.01)
                records += size
            rpc.call("task.execute", {"taskId": "records-overflow", "path": ok}, error=-32004)
            # A deliberately slow reader must not make the service buffer unlimited replies.
            with socket.create_connection(("127.0.0.1", port), timeout=3) as slow:
                slow.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 4096)
                slow.sendall(b"".join(Rpc.frame(dict(jsonrpc="2.0", id=i, method="task.list")) for i in range(32)))
                time.sleep(.3)
                try:
                    while slow.recv(65536):
                        pass
                except ConnectionResetError:
                    pass
            assert rpc.call("service.status")["taskRecords"] == 1024
            rpc.call("task.remove", {"taskId": "bad"})
            rpc.call("task.execute", {"taskId": "bad", "path": ok})
            rpc.state("bad", "resident")
            rpc.call("service.shutdown")
        assert b"entry-output\n" in b"".join(output)
        assert b"Content-Length:" not in b"".join(output)
        for termination, code in [(signal.SIGINT, 130)] + ([(signal.SIGTERM, 143)] if os.name != "nt" else []):
            with service(binary, env, code=code) as (process, port, _, _):
                rpc = connect(port)
                rpc.call("task.execute", {"taskId": "busy", "path": forever})
                time.sleep(.05)
                process.send_signal(termination)
        check_tls(binary, temporary, env)
        if args.plugins:
            check_plugins(binary, temporary, env, ini, ok)
        print("Script service framing, isolation, lifecycle, I/O and TLS checks passed")


def check_tls(binary, temporary, env):
    openssl = shutil.which("openssl")
    assert openssl, "OpenSSL CLI required for certificate integration tests"
    app = temporary / "application"
    app.mkdir()
    executable = app / binary.name
    try: os.link(binary, executable)
    except OSError: shutil.copy2(binary, executable)
    for name in ["lang", "luaulib", "luaudef", "plugin", "devdrv"]:
        if (binary.parent / name).exists():
            (app / name).symlink_to(binary.parent / name, target_is_directory=True)
    certs = app / "certs"
    certs.mkdir()
    def certificate(name, algorithm):
        key, cert = temporary / (name + ".key"), temporary / (name + ".crt")
        subprocess.run([openssl, "req", "-x509", "-newkey", algorithm, "-nodes", "-days", "1",
                        "-subj", f"/CN={name}", "-keyout", str(key), "-out", str(cert)],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return key, cert
    server_key, server_cert = certificate("localhost", "rsa:2048")
    client_key, client_cert = certificate("client", "ed25519")
    unknown_key, unknown_cert = certificate("unknown", "ed25519")
    def signed_certificate(name, key, signer_cert=None, signer_key=None, days=1):
        request = temporary / (name + ".csr")
        cert = temporary / (name + ".crt")
        subprocess.run([openssl, "req", "-new", "-key", str(key), "-subj", f"/CN={name}", "-out", str(request)],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        signing = ["-CA", str(signer_cert), "-CAkey", str(signer_key), "-set_serial", "2"] if signer_cert else ["-signkey", str(key)]
        if days < 0:
            database = temporary / "ca-index"
            database.write_text("")
            serial = temporary / "ca-serial"
            serial.write_text("02\n")
            configuration = temporary / "ca.cnf"
            configuration.write_text(
                f"[ca]\ndefault_ca=signer\n[signer]\ndatabase={database}\n"
                f"serial={serial}\nnew_certs_dir={temporary}\ndefault_md=default\n"
                "policy=names\n[names]\ncommonName=supplied\n")
            command = [openssl, "ca", "-batch", "-selfsign", "-config", str(configuration),
                       "-keyfile", str(key), "-in", str(request), "-out", str(cert),
                       "-startdate", "20000101000000Z", "-enddate", "20000102000000Z"]
        else:
            command = [openssl, "x509", "-req", "-in", str(request), *signing,
                       "-days", str(days), "-out", str(cert)]
        subprocess.run(command, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return cert
    signed = signed_certificate("signed-unregistered", unknown_key, client_cert, client_key)
    expired = signed_certificate("expired", unknown_key, days=-1)
    shutil.copy2(expired, certs / "expired.crt")
    (certs / "expired.crt").chmod(0o444)
    registered = certs / "client.crt"
    shutil.copy2(client_cert, registered)
    registered.chmod(0o444)
    options = ["--address", "0.0.0.0", "--cert", str(server_cert), "--key", str(server_key)]
    with service(executable, env, options) as (process, port, output, diagnostics):
        context = ssl.create_default_context(cafile=str(server_cert))
        context.load_cert_chain(str(client_cert), str(client_key))
        rpc = connect(port, context)
        assert rpc.call("service.status")["endpoint"].startswith("tls://")
        unknown = ssl.create_default_context(cafile=str(server_cert))
        unknown.load_cert_chain(str(unknown_cert), str(unknown_key))
        # Keep failed handshakes on another client thread: OpenSSL errors are
        # thread-local, and should not contaminate the legitimate client session.
        signed_context = ssl.create_default_context(cafile=str(server_cert))
        signed_context.load_cert_chain(str(signed), str(unknown_key))
        expired_context = ssl.create_default_context(cafile=str(server_cert))
        expired_context.load_cert_chain(str(expired), str(unknown_key))
        failures = []
        def reject_clients():
            for rejected in [unknown, signed_context, expired_context,
                             ssl.create_default_context(cafile=str(server_cert))]:
                other = None
                try:
                    other = connect(port, rejected)
                    other.call("service.status")
                    failures.append("Unregistered or missing client certificate was accepted")
                except socket.timeout:
                    failures.append("Certificate rejection timed out")
                except (ssl.SSLError, OSError, AssertionError):
                    pass
                finally:
                    if other:
                        other.socket.close()
        rejection = Thread(target=reject_clients)
        rejection.start()
        rejection.join(timeout=10)
        assert not rejection.is_alive() and not failures, failures
        try:
            assert rpc.call("service.status")["state"] == "running"
            rpc.call("service.shutdown")
        except Exception as error:
            raise AssertionError((process.poll(), b"".join(diagnostics))) from error
    mismatch = subprocess.run([str(executable), "--headless", "--live", "--cert", str(server_cert),
                               "--key", str(unknown_key)], env=env, capture_output=True, timeout=10)
    assert mismatch.returncode == 9, mismatch
    expired_server = signed_certificate("expired-server", server_key, days=-1)
    expired_start = subprocess.run([str(executable), "--headless", "--live", "--cert", str(expired_server),
                                   "--key", str(server_key)], env=env, capture_output=True, timeout=10)
    assert expired_start.returncode == 9, expired_start


def check_plugins(binary, temporary, env, ini, script):
    import zlib
    def compressed(name):
        raw = name.encode()
        return "@ByteArray(" + "".join(f"\\x{x:02x}" for x in len(raw).to_bytes(4, "big") + zlib.compress(raw)) + ")"
    ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=true\nplugin.ext=" + compressed("HeadlessProbe") +
                   "\nplugin.dev=" + compressed("HeadlessDeviceProbe") + "\n")
    trace = temporary / "plugin-trace"
    plugin_env = dict(env, WINGHEX_HEADLESS_TEST_TRACE=str(trace))
    verification = temporary / "plugin-service.luau"
    verification.write_text('assert(HeadlessProbe.answer()==42 and HeadlessProbe.ready()); assert(HeadlessDeviceProbe.answer()==42)')
    with service(binary, plugin_env) as (_, port, _, _):
        rpc = connect(port)
        for i in range(3):
            rpc.call("task.execute", {"taskId": str(i), "path": str(verification)})
            rpc.state(str(i), "resident")
        rpc.call("service.shutdown")
    assert trace.read_text().splitlines() == ["device:init", "plugin:init", "plugin:register", "device:register",
        "plugin:ready", "plugin:closing", "plugin:unload", "device:unload"]


if __name__ == "__main__":
    main()
