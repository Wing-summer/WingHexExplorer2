# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: AGPL-3.0-or-later
"""Check private helper conversion and raw Lua callback registration."""

import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from generate_luau_bindings import emit_private_helpers
from qtluau_codegen.models import JsonObject


class PrivateHelperTests(unittest.TestCase):
    """Keep host helpers declarative while preserving their Lua contracts."""

    def test_host_helpers(self) -> None:
        """Use Qt codecs for values and preserve the raw multi-result callback."""
        root = Path(__file__).resolve().parents[2]
        policy = json.loads((root / "codegen/forwarding.json").read_text())
        generated = "\n".join(emit_private_helpers(policy))
        self.assertIn('[scriptContexts](lua_State *L)', generated)
        self.assertIn('scriptContexts->forCaller(L)->token()', generated)
        self.assertIn('QUrl result = WingHex::fileUrl(arg0.value);', generated)
        self.assertIn('WingHex::deviceUrl(arg0.value, arg1.value)', generated)
        self.assertIn('return qtToLua(L, result);', generated)
        self.assertIn('scope.addFunction("_call_sync", &LuauUtil::callSync);', generated)
        self.assertEqual(generated.count('scope.addFunction('), 5)

    def test_invalid_helpers(self) -> None:
        """Reject ambiguous declarations and alterations to raw signatures."""
        cases: list[JsonObject] = [
            {"name": "helper", "native": "fn", "context_method": "token", "result": "int"},
            {"name": "helper", "native": "fn()", "result": "int"},
            {"name": "helper", "native": "fn", "raw": True, "result": "int"},
            {"name": "helper", "native": "fn", "parameters": ["UnknownType"], "result": "int"},
            {"name": "helper", "native": "fn", "result": "bad;code"},
        ]
        for function in cases:
            with self.subTest(function=function), self.assertRaises(ValueError):
                emit_private_helpers({"private_helpers": [{"namespace": "ctl", "functions": [function]}]})
        helper: JsonObject = {"name": "helper", "native": "fn", "result": "int"}
        with self.assertRaisesRegex(ValueError, "Duplicate private helper"):
            emit_private_helpers({"private_helpers": [{"namespace": "ctl", "functions": [helper, helper]}]})


if __name__ == "__main__":
    unittest.main()
