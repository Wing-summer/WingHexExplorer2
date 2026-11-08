# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Check that scanned APIs drive the hexdoc module and exported types."""

import json
import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from hexdoc_codegen import generate_hexdoc
from qtluau_codegen.models import JsonObject


class HexdocCodegenTests(unittest.TestCase):
    """Exercise defaults, overloads, naming, exclusion and conflicting methods."""

    def test_map_and_declaration(self) -> None:
        """Emit every selected operation once, retaining all scanned arities."""
        manifest: JsonObject = {
            "native_bindings": [
                {
                    "namespace": "reader",
                    "methods": [
                        {
                            "native_name": "documentInfo",
                            "name": "documentInfo",
                            "parameters": ["number"],
                            "result": "Info",
                        },
                        {
                            "native_name": "readString",
                            "name": "readString",
                            "parameters": ["number"],
                            "result": "string",
                        },
                        {
                            "native_name": "readString",
                            "name": "readString",
                            "parameters": ["number", "number"],
                            "result": "string",
                        },
                    ],
                },
                {
                    "namespace": "ctl",
                    "methods": [
                        {
                            "native_name": "closeFileHandle",
                            "name": "closeFileHandle",
                            "parameters": ["number"],
                            "result": "number",
                        },
                        {
                            "native_name": "select",
                            "name": "select",
                            "parameters": ["number", "number"],
                            "result": "boolean",
                        },
                    ],
                },
            ]
        }
        manifest["native_bindings"][0]["methods"].extend([
            {"native_name": "isFileOpened", "name": "isFileOpened", "parameters": ["number"], "result": "boolean"},
            {"native_name": "openedFiles", "name": "openedFiles", "parameters": [], "result": "{_qt_QUrl}"},
            {"native_name": "allOpenedFiles", "name": "allOpenedFiles", "parameters": [], "result": "{_qt_QUrl}"},
        ])
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary)
            manifest["symbol_catalog"] = "catalog.json"
            (output / "definitions").mkdir()
            (output / "definitions/catalog.json").write_text(json.dumps({
                "version": 1, "roots": [{"id": "existing"}],
                "type_display_names": {"Info": "hex.DocumentInfo"},
            }))
            generate_hexdoc(manifest, {}, output)
            runtime = (output / "generated/hexdoc_api.luau").read_text()
            declaration = runtime.split("return table.freeze", 1)[0]
            runtime_names = set(re.findall(r"^    (\w+) =", runtime, re.MULTILINE))
            type_names = set(re.findall(r"^    (\w+): \(\(self:", declaration, re.MULTILINE))
            self.assertEqual(runtime_names, {"read_string", "select"})
            self.assertEqual(runtime_names, type_names)
            self.assertIn("export type Document = {", declaration)
            self.assertIn("read_string: ((self: Document, arg0: number) -> string) &", declaration)
            self.assertIn("arg1: number) -> string", declaration)
            self.assertIn("document_info: (self: Document) -> Info", declaration)
            self.assertIn("<A..., R...>", declaration)
            self.assertIn("is_valid: (self: Document) -> boolean", declaration)
            self.assertNotIn("is_file_opened =", runtime)
            self.assertNotIn("opened_files =", runtime)
            self.assertNotIn("all_opened_files =", runtime)
            self.assertNotIn("definition_preload", manifest)
            self.assertFalse((output / "definitions/hexdoc.d.luau").exists())
            catalog = json.loads((output / "definitions/catalog.json").read_text())
            self.assertEqual(catalog["roots"][0], {"id": "existing"})
            module = catalog["roots"][1]
            self.assertEqual(module["import_path"], "@s/hexdoc")
            self.assertEqual(module["category"], "document")
            members = json.loads((output / "definitions" / module["members"]).read_text())["children"]
            document = next(item for item in members if item["label"] == "Document")
            methods = json.loads((output / "definitions" / document["members"]).read_text())["children"]
            read = next(item for item in methods if item["label"] == "read_string")
            self.assertEqual(len(read["overloads"]), 2)
            info = next(item for item in methods if item["label"] == "document_info")
            self.assertIn("hex.DocumentInfo", info["declaration"])
            self.assertNotIn("close_file_handle =", runtime)
            self.assertFalse((output / "luaulib").exists())
            generate_hexdoc(manifest, {"naming": {"functions": "native"}}, output)
            catalog = json.loads((output / "definitions/catalog.json").read_text())
            self.assertEqual(len(catalog["roots"]), 2)
            self.assertIn(
                "readString = reader.readString", (output / "generated/hexdoc_api.luau").read_text()
            )

    def test_missing_info(self) -> None:
        """Fail generation if the required underlying document API is absent."""
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(ValueError, "documentInfo"):
                generate_hexdoc({}, {}, Path(temporary))


if __name__ == "__main__":
    unittest.main()
