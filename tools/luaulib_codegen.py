#!/usr/bin/env python3
# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: BSD-3-Clause
"""Generate module-local declarations and stage/deploy the offline script library.
生成模块级声明，暂存及部署离线脚本库。
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text() != text:
        path.write_text(text)


def copy(source, destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    if not destination.exists() or source.read_bytes() != destination.read_bytes():
        shutil.copyfile(source, destination)


def generate(source, output, icons):
    api = json.loads((source / 'api.json').read_text())
    enum = re.search(r'enum class StandardIcon\s*\{(.*?)\};', icons.read_text(), re.S)
    if enum is None:
        raise ValueError('Cannot find StandardIcon')
    ids = re.findall(r'^\s*(\w+)\s*(?:=\s*\d+)?\s*,?\s*$', enum[1], re.M)
    icon_type = 'export type IconIds = { ' + ', '.join(f'{name}: number' for name in ids) + ' }\n'
    managed = set()
    for name, module in api.items():
        path = Path(name)
        if path.is_absolute() or '..' in path.parts or (not module.get('builtin') and not source.joinpath(name + '.luau').is_file()):
            raise ValueError('Invalid module: ' + name)
        exports = module['exports']
        for key, signature in exports.items():
            if '->' in signature and not re.fullmatch(r'[a-z][a-z0-9_]*', key):
                raise ValueError('Public functions must use snake_case: ' + key)
        result = exports.get('$value')
        if not result:
            result = '{\n' + ''.join(f'    {key}: {value},\n' for key, value in exports.items()) + '}'
        content = '--!strict\n-- Generated from luaulib/api.json; do not edit.\n'
        content += '-- ' + module['doc'] + '\n' + module['types'] + '\n'
        if name == 'icons':
            content += icon_type
        content += 'export type Module = ' + result + '\n'
        declaration = content + 'local module = nil :: any\nreturn module :: Module\n'
        write(output / (name + '.d.luau'), declaration)
        managed.add(name + '.d.luau')
        if not module.get('builtin'):
            helper = name + '_api.luau'
            write(output / helper, content + 'return table.freeze({})\n')
            managed.add(helper)
    for path in sorted(source.rglob('*')):
        if path.is_file() and (path.suffix == '.luau' or path.name in {'LICENSE', 'manifest.json'}):
            relative = path.relative_to(source)
            copy(path, output / relative)
            managed.add(relative.as_posix())
    return managed


def record_managed(root, managed):
    inventory = root / '.wing-luaulib.json'
    previous = json.loads(inventory.read_text()) if inventory.exists() else {}
    for name, digest in previous.items():
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Invalid deployment inventory path')
        path = root / relative
        if name not in managed and path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest() == digest:
            path.unlink()
    write(inventory, json.dumps({name: hashlib.sha256((root / name).read_bytes()).hexdigest()
                                for name in sorted(managed)}, indent=2) + '\n')


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--icons', type=Path, required=True)
    parser.add_argument('--deploy', type=Path)
    args = parser.parse_args()
    managed = generate(args.source, args.output, args.icons)
    record_managed(args.output, managed)
    if args.deploy:
        for name in sorted(managed):
            copy(args.output / name, args.deploy / name)
        record_managed(args.deploy, managed)


if __name__ == '__main__':
    main()
