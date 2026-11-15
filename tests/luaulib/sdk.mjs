// Copyright (C) 2026 WingSummer
// SPDX-License-Identifier: BSD-3-Clause
import assert from 'node:assert/strict';
import { pathToFileURL } from 'node:url';
import path from 'node:path';
const location = path.join(process.argv[3], 'node_modules/@modelcontextprotocol/client/dist/index.mjs');
const { Client, StreamableHTTPClientTransport } = await import(pathToFileURL(location).href);
for (let index = 0; index < 2; index++) {
    const client = new Client({ name: 'wing-stdlib-sdk-test', version: '1' }, { versionNegotiation: { mode: { pin: '2026-07-28' } } });
    try {
        await client.connect(new StreamableHTTPClientTransport(new URL(process.argv[2]), { fetch: async (url, options) => { const response = await fetch(url, options); if (!response.ok) console.error("SDK request", Object.fromEntries(new Headers(options.headers)), options.body); return response; } }));
        const discovery = await client.discover();
        assert(discovery.supportedVersions.includes('2026-07-28'));
        assert.equal((await client.listTools()).tools[0].name, 'echo');
        assert.equal((await client.callTool({ name: 'echo', arguments: { text: 'official SDK' } })).content[0].text, 'official SDK');
        assert.equal((await client.readResource({ uri: 'test://resource' })).contents[0].text, 'resource');
        assert.equal((await client.getPrompt({ name: 'test' })).messages[0].role, 'user');
    } finally {
        await client.close();
    }
}
console.log('PASS official MCP TypeScript client 2026-07-28');
