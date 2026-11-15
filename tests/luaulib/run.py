#!/usr/bin/env python3
# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: BSD-3-Clause
"""Exercise the real pure-script modules with Luau, using only a host-env stub."""
import argparse
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
from luaulib_codegen import generate


def bundle(test, modules):
    lines = ['local factories, cache = {}, {}', 'local host = {env = table.freeze({locale = "en-US"})}']
    for path in sorted(modules.rglob('*.luau')):
        if path.name.endswith('.d.luau'):
            continue
        name = path.relative_to(modules).with_suffix('').as_posix()
        lines.append(f'factories["{name}"] = function(require)\n{path.read_text()}\nend')
    lines.append('''local loadModule
loadModule = function(name, parent)
    if name == "@b/winghex" then return host end
    if string.sub(name, 1, 3) == "@s/" then name = string.sub(name, 4)
    elseif string.sub(name, 1, 1) == "." then name = (string.match(parent, "^(.*)/") or "") .. "/" .. name end
    local parts = {}
    for part in string.gmatch(name, "[^/]+") do
        if part == ".." then table.remove(parts) elseif part ~= "." then table.insert(parts, part) end
    end
    name = table.concat(parts, "/")
    if not factories[name] and factories[name .. "/init"] then name ..= "/init" end
    if cache[name] then return cache[name] end
    assert(factories[name], "Missing module: " .. name)
    local value = factories[name](function(required) return loadModule(required, name) end)
    cache[name] = value
    return value
end
local function require(name) return loadModule(name, "test") end
''')
    lines.append(test.read_text())
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--luau', default='luau')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='wing-stdlib-') as directory:
        script = Path(directory) / 'test.luau'
        modules = Path(directory) / 'modules'
        generate(ROOT / 'luaulib', modules, ROOT / 'WingPlugin/wingplugin_global.h')
        script.write_text(bundle(ROOT / 'tests/luaulib/core.luau', modules))
        subprocess.run([args.luau, str(script)], check=True)


if __name__ == '__main__':
    main()
