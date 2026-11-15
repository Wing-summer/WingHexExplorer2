#!/usr/bin/env python3
# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: BSD-3-Clause
"""Real host JSON, integer/cffi and HTTP transport tests in isolated settings."""
import argparse
import base64
from contextlib import closing
import http.client
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tests/headless'))
from test_service import Rpc, connect, service

META = {'io.modelcontextprotocol/protocolVersion': '2026-07-28',
        'io.modelcontextprotocol/clientInfo': {'name': 'stdlib-test', 'version': '1'},
        'io.modelcontextprotocol/clientCapabilities': {}}


def send_http(port, method='tools/list', params=None, overrides=None, verb='POST', body=None):
    params = dict(params or {}, _meta=META)
    value = dict(jsonrpc='2.0', id=1, method=method, params=params)
    headers = {'Content-Type': 'application/json', 'Accept': 'application/json, text/event-stream',
               'MCP-Protocol-Version': '2026-07-28', 'Mcp-Method': method}
    name = params.get('uri') if method == 'resources/read' else params.get('name')
    if name is not None:
        headers['Mcp-Name'] = name
    headers.update(overrides or {})
    connection = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
    connection.request(verb, '/mcp', body=json.dumps(value) if body is None else body, headers=headers)
    response = connection.getresponse()
    data = response.read()
    connection.close()
    return response.status, json.loads(data) if data else None


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--sdk-prefix', type=Path)
    args = parser.parse_args()
    binary = args.binary.resolve()
    assert 'HeadlessProbe' in binary.name, 'Use tests/headless/build_process_probe.py'
    with tempfile.TemporaryDirectory(prefix='wing-stdlib-native-') as temporary:
        work = Path(temporary)
        ini = work / 'config/WingCloudStudio/WingHexExplorer2.conf'
        ini.parent.mkdir(parents=True)
        ini.write_text('[General]\nscript.enable=true\nplugin.enableplugin=false\n')
        runtime = work / 'runtime'
        runtime.mkdir(mode=0o700)
        env = os.environ | {'QT_QPA_PLATFORM': 'offscreen', 'XDG_CONFIG_HOME': str(work / 'config'),
                           'XDG_DATA_HOME': str(work / 'data'), 'XDG_CACHE_HOME': str(work / 'cache'),
                           'XDG_RUNTIME_DIR': str(runtime), 'WINGHEX_HEADLESS_TEST_CONFIG': str(work / 'config')}
        for name in ('core', 'native', 'mcp'):
            subprocess.run([str(binary), '--headless', str(ROOT / f'tests/luaulib/{name}.luau')], env=env, check=True, timeout=30)
        entry = work / 'server.luau'
        entry.write_text('''local mcp = require("@s/mcp")
local server = mcp.new({name = "integration", version = "1"})
server:tool({name = "echo", inputSchema = {type = "object", properties = {text = {type = "string", ["x-mcp-header"] = "Text"}}}}, function(args)
 return {content = {{type = "text", text = args.text or "empty"}}}
end)
server:resource({uri = "test://resource", name = "test"}, function() return {contents = {{uri = "test://resource", text = "resource"}}} end)
server:prompt({name = "test"}, function() return {messages = {{role = "user", content = {type = "text", text = "prompt"}}}} end)
local listener = server:serve_http()
println("mcp-port:" .. listener.port)
''')
        with service(binary, env) as (_, port, output, diagnostics):
            with closing(connect(port).socket) as connection:
                rpc = Rpc(connection)
                rpc.call('task.execute', {'taskId': 'mcp', 'path': str(entry)})
                deadline = time.monotonic() + 10
                while not any(line.startswith(b'mcp-port:') for line in output):
                    state = rpc.call('task.get', {'taskId': 'mcp'})
                    assert state['state'] != 'failed', (state, diagnostics)
                    assert time.monotonic() < deadline, (output, diagnostics)
                    time.sleep(.02)
                mcp_port = int(next(line.split(b':')[1] for line in output if line.startswith(b'mcp-port:')))
                status, response = send_http(mcp_port, 'server/discover')
                assert status == 200 and response['result']['supportedVersions'] == ['2026-07-28'], response
                for _ in range(3):
                    status, response = send_http(mcp_port)
                    assert status == 200 and response['result']['tools'][0]['name'] == 'echo', response
                status, response = send_http(mcp_port, 'tools/call', {'name': 'echo', 'arguments': {'text': 'hello'}}, {'Mcp-Param-Text': 'hello'})
                assert status == 200 and response['result']['content'][0]['text'] == 'hello', response
                assert send_http(mcp_port, 'tools/call', {'name': 'echo', 'arguments': {'text': 'hello'}})[1]['error']['code'] == -32020
                encoded = base64.b64encode('你好'.encode()).decode()
                status, response = send_http(mcp_port, 'tools/call', {'name': 'echo', 'arguments': {'text': '你好'}}, {'Mcp-Param-Text': f'=?base64?{encoded}?='})
                assert status == 200 and response['result']['content'][0]['text'] == '你好', response
                status, response = send_http(mcp_port, 'resources/read', {'uri': 'test://resource'})
                assert status == 200 and response['result']['contents'][0]['text'] == 'resource', response
                status, response = send_http(mcp_port, 'prompts/get', {'name': 'test'})
                assert status == 200 and response['result']['messages'][0]['role'] == 'user', response
                assert send_http(mcp_port, overrides={'Origin': 'https://untrusted.example'})[0] == 403
                assert send_http(mcp_port, overrides={'Mcp-Method': 'ping'})[1]['error']['code'] == -32020
                assert send_http(mcp_port, body='{broken')[1]['error']['code'] == -32700
                assert send_http(mcp_port, verb='GET')[0] == 405
                assert send_http(mcp_port, 'missing')[0] == 404
                if args.sdk_prefix:
                    subprocess.run(['node', str(ROOT / 'tests/luaulib/sdk.mjs'), f'http://127.0.0.1:{mcp_port}/mcp',
                                    str(args.sdk_prefix.resolve())], check=True, timeout=30)
                rpc.call('task.stop', {'taskId': 'mcp'})
                rpc.state('mcp', 'stopped')
                try:
                    socket.create_connection(('127.0.0.1', mcp_port), timeout=.5).close()
                except OSError:
                    pass
                else:
                    raise AssertionError('MCP listener survived task cleanup')
                rpc.call('service.shutdown')
        print('PASS native JSON, cffi/int64, real HTTP and task cleanup')


if __name__ == '__main__':
    main()
