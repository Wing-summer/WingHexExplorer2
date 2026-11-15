#!/usr/bin/env python3
# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: BSD-3-Clause
"""Run the actual example parsers with the Luau CLI; no third-party Python deps."""
import argparse
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
from luaulib_codegen import generate


def pe(wide=False):
    b = bytearray(4096)
    def put(o, fmt, *v): struct.pack_into('<' + fmt, b, o, *v)
    b[:2] = b'MZ'
    put(60, 'I', 128)
    b[128:132] = b'PE\0\0'
    optsize = 240 if wide else 224
    put(132, 'HHIIIHH', 0x8664 if wide else 0x14c, 1, 0, 0, 0, optsize, 2)
    o = 152
    put(o, 'H', 0x20b if wide else 0x10b)
    put(o + (24 if wide else 28), 'Q' if wide else 'I', 0x140000000 if wide else 0x400000)
    put(o + 60, 'I', 512)
    base = 112 if wide else 96
    put(o + base - 4, 'I', 16)
    s = o + optsize
    b[s:s+8] = b'.data\0\0\0'
    put(s + 8, 'IIII', 0x1000, 0x1000, 3584, 512)
    def directory(index, offset, size, file_offset=False):
        put(o + base + index * 8, 'II', offset if file_offset else offset - 512 + 0x1000, size)
    # Normal and delayed imports, including ordinal, name, terminators.
    for index, offset, delayed in [(1, 512, False), (13, 600, True)]:
        directory(index, offset, 64 if delayed else 40)
        if delayed: put(offset, 'IIIIIIII', 1, 0x1200, 0, 0x1260, 0x1240, 0, 0, 0)
        else: put(offset, 'IIIII', 0x1240, 0, 0, 0x1200, 0x1260)
    b[1024:1030] = b'x.dll\0'
    width, fmt = (8, 'Q') if wide else (4, 'I')
    for a in (1088, 1120): put(a, fmt * 3, 0x1280, 1 << (width * 8 - 1) | 3, 0)
    put(1152, 'H', 7)
    b[1154:1158] = b'foo\0'
    directory(0, 1200, 80)
    put(1200, 'IIHHIIIIIII', 0, 0, 0, 0, 0x1200, 1, 1, 1, 0x1320, 0x1324, 0x1328)
    put(1312, 'IIH', 0x1500, 0x1282, 0)
    directory(5, 1400, 12)
    put(1400, 'IIHH', 0x1000, 12, 0x3004, 0)
    directory(2, 1500, 40)
    put(1500 + 14, 'H', 1)
    put(1516, 'II', 10, 24)
    put(1524, 'IIII', 0x1500, 4, 0, 0)
    directory(4, 2000, 16, True)
    put(2000, 'IHH', 12, 0x200, 2)
    directory(6, 2100, 28)
    put(2100 + 12, 'IIII', 2, 8, 0, 2200)
    directory(9, 2300, width * 4 + 8)
    directory(10, 2400, 4)
    put(2400, 'I', 4)
    directory(11, 2500, 32)
    put(2500, 'IHH', 1, 24, 0)
    b[2524:2528] = b'dll\0'
    directory(14, 2600, 72)
    put(2600, 'I', 72)
    return b


def elf(wide, little, extended=False):
    b = bytearray(2048)
    endian = '<' if little else '>'
    def put(o, fmt, *v): struct.pack_into(endian + fmt, b, o, *v)
    b[:7] = b'\x7fELF' + bytes([2 if wide else 1, 1 if little else 2, 1])
    word = 'Q' if wide else 'I'
    hs, ps, ss = (64, 56, 64) if wide else (52, 32, 40)
    shoff = 256
    put(16, 'HHI' + word * 3 + 'IHHHHHH', 3, 62 if wide else 3, 1, 0, hs, shoff, 0,
        hs, ps, 65535 if extended else 1, ss, 0 if extended else 8, 65535 if extended else 1)
    if wide: put(hs, 'IIQQQQQQ', 1, 5, 0, 0xffffffffffffffff, 0, 2048, 4096, 4096)
    else: put(hs, 'IIIIIIII', 1, 0, 0xffffffff, 0, 2048, 4096, 5, 4096)
    strings = b'\0.shstrtab\0.strtab\0.symtab\0.rela\0.dynamic\0.note\0.bss\0'
    b[800:800+len(strings)] = strings
    b[900:905] = b'\0foo\0'
    symSize = 24 if wide else 16
    relSize = (8 if wide else 4) * 3
    dynSize = (8 if wide else 4) * 2
    def section(i, name, kind, offset, size, link=0, info=0, stride=0):
        ni = strings.find(name.encode()) if name else 0
        put(shoff + i * ss, 'II' + word * 4 + 'II' + word * 2,
            ni, kind, 0, 0, offset, size, link, info, 4, stride)
    section(0, '', 0, 0, 8 if extended else 0, 1 if extended else 0, 1 if extended else 0)
    section(1, '.shstrtab', 3, 800, len(strings))
    section(2, '.strtab', 3, 900, 5)
    section(3, '.symtab', 2, 960, symSize, 2, stride=symSize)
    put(960, 'I', 1)
    section(4, '.rela', 4, 1024, relSize, 3, stride=relSize)
    section(5, '.dynamic', 6, 1100, dynSize * 2, 2, stride=dynSize)
    put(1100, word * 4, 1, 1, 0, 0)
    section(6, '.note', 7, 1200, 20)
    put(1200, 'III', 4, 4, 1)
    b[1212:1220] = b'GNU\0abcd'
    section(7, '.bss', 8, 999999, 1000000000)
    return b


def literal(data):
    # Fixed decimal escapes avoid locale, UTF-8 and embedded-NUL ambiguities.
    return '"' + ''.join('\\%03d' % v for v in data) + '"'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--luau', required=True, help='Luau CLI executable')
    ap.add_argument('--host', type=Path, help='Optional WingHexHeadlessProbe or host executable')
    ap.add_argument('--pe', type=Path, action='append', default=[], help='Additional real PE fixture')
    ap.add_argument('--elf', type=Path, action='append', default=[], help='Additional real ELF fixture')
    args = ap.parse_args()
    cases = []
    for wide in (False, True):
        data = pe(wide)
        cases.append(('pe', data, True, 'PE64' if wide else 'PE32'))
        for offset, fmt, value, label in [(60, 'I', 0xffffffff, 'bad PE offset'),
                (152 + (112 if wide else 96) - 4, 'I', 1000, 'directory overflow'),
                (1404, 'I', 0, 'zero relocation block'),
                (2000, 'I', 0, 'zero certificate'),
                (1516 + 4, 'I', 0x80000000, 'resource cycle')]:
            bad = data.copy(); struct.pack_into('<' + fmt, bad, offset, value)
            cases.append(('pe', bad, False, label))
        cases.append(('pe', data[:200], False, 'truncated PE'))
        if wide:
            high = data.copy()
            base = 0xfffff80000000000
            struct.pack_into('<Q', high, 152 + 24, base)
            struct.pack_into('<QQQQ', high, 2300, base + 0x1c00, base + 0x1c04,
                             base + 0x1a00, base + 0x1b00)
            struct.pack_into('<QQ', high, 3328, base + 0x1d00, 0)
            cases.append(('pe', high, True, 'high-half TLS virtual addresses'))

    for wide in (False, True):
        for little in (False, True):
            for extended in (False, True):
                data = elf(wide, little, extended)
                cases.append(('elf', data, True, f'ELF wide={wide} little={little} extended={extended}'))
                cases.append(('elf', data[:350], False, 'truncated ELF'))
                if wide:
                    bad = data.copy()
                    struct.pack_into(('<' if little else '>') + 'Q', bad, 40, 0xffffffffffffffff)
                    cases.append(('elf', bad, False, 'unsafe 64-bit file offset'))
                bad = data.copy(); bad[5] = 0
                cases.append(('elf', bad, False, 'invalid endian'))
                bad = data.copy()
                struct.pack_into(('<' if little else '>') + ('Q' if wide else 'I'), bad, 1100 + (8 if wide else 4), 99)
                cases.append(('elf', bad, False, 'bad dynamic string index'))
    synthetic_count = len(cases)
    for kind in ('pe', 'elf'):
        for path in getattr(args, kind): cases.append((kind, path.read_bytes(), True, str(path)))
    with tempfile.TemporaryDirectory(prefix='wing-examples-') as tmp:
        work = Path(tmp)
        shutil.copytree(ROOT / 'examples' / 'file-structure' / 'lib', work / 'lib')
        generate(ROOT / 'luaulib', work / 'stdlib', ROOT / 'WingPlugin/wingplugin_global.h')
        (work / 'stdlib/env.luau').write_text('return { locale = "en_US" }\n')
        (work / '.luaurc').write_text(json.dumps({'aliases': {'s': './stdlib'}}))
        chunks = ['local Binary = require("@s/binaryreader")',
                  'local parsers = { pe = require("./lib/pe"), elf = require("./lib/elf") }']
        for kind, data, expected, label in cases:
            chunks.append('do\nlocal data = ' + literal(data) + '\n'
                'local p = Binary.new(#data, function(o, n) return string.sub(data, o + 1, o + n) end)\n'
                f'local ok, err = pcall(parsers.{kind}, p)\n'
                f'assert(ok == {str(expected).lower()}, {literal(label.encode())} .. ": " .. tostring(err))\n'
                'if ok then\nassert(#p.marks > 0)\nlocal spans = table.clone(p.marks)\n'
                'table.sort(spans, function(a,b) return a.offset < b.offset end)\n'
                'local ending = 0\nfor _, m in spans do assert(m.offset <= ending); ending = math.max(ending, m.offset + m.size) end\n'
                'assert(ending == #data)\nfor _, m in p.marks do '
                'assert(m.offset >= 0 and m.size > 0 and m.offset + m.size <= #data) end\nend\nend')
        host_chunks = chunks[:2 + synthetic_count] + [f'print("Passed {synthetic_count} binary parser cases in host VM")']
        (work / 'host-parsers.luau').write_text('\n'.join(host_chunks))
        chunks.append(f'print("Passed {len(cases)} binary parser cases")')
        (work / 'run.luau').write_text('\n'.join(chunks))
        subprocess.run([args.luau, str(work / 'run.luau')], check=True)
        # Embed the production wrappers with a controlled host boundary, preserving
        # the real scripts while exercising cleanup, clipboard and undo behavior.
        harness = (ROOT / 'tests/examples/host.luau').read_text()
        for name in ('document',):
            source = (ROOT / f'examples/file-structure/lib/{name}.luau').read_text()
            harness = harness.replace('-- INSERT_' + name.upper(), source)
        harness = harness.replace('-- INSERT_COPY', (ROOT / 'examples/copyascarray.luau').read_text())
        harness = harness.replace('-- INSERT_STRUCTURE_EDIT', (ROOT / 'examples/structure-edit.luau').read_text())
        harness = harness.replace('-- INSERT_ANALYSIS', (ROOT / 'luaulib/analysis.luau').read_text())
        harness = harness.replace('-- INSERT_STD_I18N', (ROOT / 'luaulib/i18n.luau').read_text())
        (work / 'host.luau').write_text(harness)
        subprocess.run([args.luau, str(work / 'host.luau')], check=True)
        shutil.copyfile(ROOT / 'tests/examples/i18n.luau', work / 'i18n.luau')
        subprocess.run([args.luau, str(work / 'i18n.luau')], check=True)
        if args.host:
            config = work / 'config'
            ini = config / 'WingCloudStudio/WingHexExplorer2.conf'
            ini.parent.mkdir(parents=True)
            ini.write_text('[General]\nscript.enable=true\nplugin.enableplugin=false\n')
            runtime = work / 'runtime'
            runtime.mkdir(mode=0o700)
            env = os.environ.copy()
            env.update(QT_QPA_PLATFORM='offscreen', XDG_CONFIG_HOME=str(config),
                       XDG_DATA_HOME=str(work / 'data'), XDG_CACHE_HOME=str(work / 'cache'),
                       XDG_RUNTIME_DIR=str(runtime), WINGHEX_HEADLESS_TEST_CONFIG=str(config))
            for entry in ('host-parsers.luau', 'host.luau', 'i18n.luau'):
                subprocess.run([str(args.host.resolve()), '--headless', str(work / entry)],
                               env=env, check=True, timeout=60)



if __name__ == '__main__':
    main()
