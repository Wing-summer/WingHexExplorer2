# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: BSD-3-Clause
"""Generation and deployment ownership regressions, without building the host."""
import importlib.util
import json
import re
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('luaulib_codegen', ROOT / 'tools/luaulib_codegen.py')
codegen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(codegen)


class CodegenTests(unittest.TestCase):
    def test_declarations_are_deterministic_and_module_local(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory)
            managed = codegen.generate(ROOT / 'luaulib', output, ROOT / 'WingPlugin/wingplugin_global.h')
            first = {name: (output / name).read_bytes() for name in managed}
            second = codegen.generate(ROOT / 'luaulib', output, ROOT / 'WingPlugin/wingplugin_global.h')
            self.assertEqual(managed, second)
            self.assertEqual(first, {name: (output / name).read_bytes() for name in second})
            api = json.loads((ROOT / 'luaulib/api.json').read_text())
            for module in api:
                source = (output / (module + '.d.luau')).read_text()
                self.assertIn('return module ::', source)
                self.assertNotIn('declare ', source)
                entry = api[module]
                if not entry.get('builtin'):
                    helper = (output / (module + '_api.luau')).read_text()
                    self.assertIn('export type Module =', helper)
                    self.assertTrue(helper.endswith('return table.freeze({})\n'))
                    runtime = (output / (module + '.luau')).read_text()
                    self.assertIn(' :: api.Module', runtime)
                    for exported in re.findall(r'^export type (\w+)', entry['types'], re.M):
                        self.assertIn(f'export type {exported} = api.{exported}', runtime)
            self.assertIn('CopyHex: number', (output / 'icons.d.luau').read_text())
            self.assertIn('StandardIcon: IconIds', (output / 'winghex.d.luau').read_text())
            self.assertIn('MIT', (output / 'mcp/LICENSE').read_text())
            self.assertNotIn('ash/init.d.luau', managed)

    def test_named_luau_functions_use_snake_case(self):
        import re
        for source in (ROOT / 'luaulib').rglob('*.luau'):
            for match in re.finditer(r'\bfunction\s+([\w.:]+)\s*\(', source.read_text()):
                name = re.split('[.:]', match[1])[-1]
                self.assertRegex(name, r'^_*[a-z][a-z0-9_]*$', str(source))

    def test_deployment_removes_only_unchanged_owned_files(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ('old.luau', 'modified.luau', 'new.luau', 'user.luau'):
                (root / name).write_text('return true\n')
            codegen.record_managed(root, {'old.luau', 'modified.luau'})
            (root / 'modified.luau').write_text('return "user edited"\n')
            codegen.record_managed(root, {'new.luau'})
            self.assertFalse((root / 'old.luau').exists())
            self.assertTrue((root / 'modified.luau').exists())
            self.assertTrue((root / 'user.luau').exists())
            self.assertTrue((root / 'new.luau').exists())


if __name__ == '__main__':
    unittest.main()
