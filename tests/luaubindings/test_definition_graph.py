# Copyright (c) 2026, wingsummer
# SPDX-License-Identifier: BSD-3-Clause
# See 3rdparty/WingLuauQt/LICENSE for license information.

"""Exercise recursive shards and native overload preservation without scanning Qt."""

import json

from qtluau_codegen.declaration_tables import callable_tables
from qtluau_codegen.definition_graph import DefinitionFiles, partition
from qtluau_codegen.symbol_catalog import emit_symbol_catalog


def main() -> None:
    """Check identity, dependency ordering, callable classes and lazy symbol roots."""
    source = {
        "_qt_Base": "type _qt_Base = { child: (self: _qt_Base) -> _qt_Child }",
        "_qt_Child": "type _qt_Child = _qt_Base & { value: (self: _qt_Child) -> number }",
        "ustring": "type ustring = { to_std_string: (self: ustring) -> string }",
        "Factory": "type Factory = { __call: ((self: any) -> _qt_Base) & ((self: any, value: number) -> _qt_Base), count: number }",
    }
    types: dict[str, str] = {}
    for name, text in source.items():
        types[name] = callable_tables(text, types)
    files = DefinitionFiles()
    entries = partition(types, ["declare text: ustring"], {"qt": "return (nil :: any) :: Factory"}, files, "fixture")
    assert len(files.preload) == 1
    private = "\n".join(text for name, text in files.sources.items() if name not in files.preload)
    assert "declare extern type _qt_Child extends _qt_Base" in private
    assert private.index("declare extern type _qt_Base") < private.index("declare extern type _qt_Child")
    assert private.count("__call:") == 2
    assert "require(" in entries["qt"]
    duplicate = DefinitionFiles()
    assert entries == partition(types, ["declare text: ustring"], {"qt": "return (nil :: any) :: Factory"}, duplicate, "fixture")
    assert files == duplicate
    catalog = DefinitionFiles()
    emit_symbol_catalog(source, {"qt": "{ QChild: () -> _qt_Child, count: number, run: ((text: string) -> ()) & ((value: number) -> ()) }"}, set(), [], catalog, "fixture", {})
    root = json.loads(catalog.sources["fixture_symbols.json"])["roots"][0]
    assert root["module_root"]
    children = json.loads(catalog.sources[root["members"]])["children"]
    child_type = next(item for item in children if item["label"] == "QChild")
    assert not child_type["module_root"]
    assert child_type.get("members")
    assert {item["label"] for item in children} == {"QChild", "count", "run"}
    assert len(next(item for item in children if item["label"] == "run")["overloads"]) == 2
    assert next(item for item in children if item["label"] == "count")["kind"] == "constant"


if __name__ == "__main__":
    main()
