"""Exercise package publication, naming conflicts and generated protocol contracts."""
import importlib.util
import json
import pathlib
import tempfile
import unittest
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("plugin_packager", ROOT / "tools/package_luau_plugin.py")
assert SPEC and SPEC.loader
PACKAGER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACKAGER)


class PackagingTest(unittest.TestCase):
    """Validate archives without executing package code."""
    def test_roundtrip_and_publication(self):
        """Keep declarations intact and publish deterministic archives."""
        with tempfile.TemporaryDirectory() as temporary:
            directory = pathlib.Path(temporary)
            source = directory / "source"
            source.mkdir()
            (source / "manifest.json").write_text(json.dumps({"FormatVersion": 1, "Id": "Test"}))
            (source / "main.luau").write_text("return {}")
            (source / "api.d.luau").write_text("declare Example: {answer: () -> number}")
            output = directory / "test.wingplg"
            PACKAGER.package(source, output)
            original = output.read_bytes()
            self.assertEqual(original[:2], b"PK")
            PACKAGER.package(source, output)
            self.assertEqual(output.read_bytes(), original)
            with zipfile.ZipFile(output) as archive:
                self.assertEqual(archive.read("main.luau"), b"return {}")
                self.assertEqual(archive.read("api.d.luau"), b"declare Example: {answer: () -> number}")
            (source / "main.luauc").write_bytes(b"binary")
            with self.assertRaises(ValueError):
                PACKAGER.package(source, output)
            self.assertEqual(output.read_bytes(), original)

    def test_example_identities(self):
        """Ship examples that can coexist with the native test plugins."""
        manifests = [ROOT / "TestPlugin/TestPlugin.json",
                     ROOT / "TestDepPlugin/TestDepPlugin.json",
                     ROOT / "ShareMemoryDrv/ShareMemoryDrv.json",
                     ROOT / "docs/examples/plugins/hello/manifest.json",
                     ROOT / "docs/examples/plugins/device/manifest.json"]
        identities = [json.loads(path.read_text())["Id"].casefold() for path in manifests]
        self.assertEqual(len(identities), len(set(identities)))
        with tempfile.TemporaryDirectory() as temporary:
            for folder, suffix in [("hello", ".wingplg"), ("device", ".wingdrv")]:
                source = ROOT / "docs/examples/plugins" / folder
                output = pathlib.Path(temporary) / (folder + suffix)
                PACKAGER.package(source, output)
                with zipfile.ZipFile(output) as archive:
                    self.assertEqual(json.loads(archive.read("manifest.json")),
                                     json.loads((source / "manifest.json").read_text()))

    def test_protocol_generation(self):
        """Generate every helper and keep definition deployment idempotent."""
        spec = importlib.util.spec_from_file_location("plugin_protocol", ROOT / "tools/plugin_protocol.py")
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as temporary:
            output = pathlib.Path(temporary)
            (output / "generated").mkdir()
            (output / "definitions").mkdir()
            copy = output / "copy_definitions.cmake"
            copy.write_text("set(_expected)\nfile(GLOB existing *)\n")
            module.generate(ROOT, output)
            module.generate(ROOT, output)
            self.assertEqual(copy.read_text().count("list(APPEND"), 1)
            helpers = (output / "generated/plugin_helpers.inc").read_text()
            self.assertIn('"plugin.i18n.tr"', helpers)
            self.assertIn('lua_rawsetfield', helpers)
            self.assertIn('pluginBaseSource', (output / "generated/plugin_protocol.hpp").read_text())
            definitions = (output / "definitions/WingLuauBind_plugin_protocol.d.luau").read_text()
            device = definitions.split("type WingDevicePlugin = {", 1)[1].split("}\n", 1)[0]
            self.assertNotIn("registered_events", device)
            self.assertIn("on_open_file", device)

    def test_symlinks(self):
        """Do not follow resources outside the package root."""
        with tempfile.TemporaryDirectory() as temporary:
            directory = pathlib.Path(temporary)
            source = directory / "source"
            source.mkdir()
            (source / "manifest.json").write_text('{"FormatVersion":1}')
            (source / "main.lua").write_text("return {}")
            (source / "outside").symlink_to(directory)
            with self.assertRaises(ValueError):
                PACKAGER.package(source, directory / "test.wingdrv")


if __name__ == "__main__":
    unittest.main()
