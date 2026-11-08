# Copyright (c) 2026, wingsummer
# SPDX-License-Identifier: BSD-3-Clause
"""Generate the hexdoc operation map and declaration from scanned script APIs."""

from __future__ import annotations

import json
import re
from pathlib import Path

from qtluau_codegen.definition_graph import DefinitionFiles
from qtluau_codegen.extensions import write
from qtluau_codegen.function_naming import function_style, snake_case
from qtluau_codegen.models import (
    JsonObject,
    object_field,
    object_list_field,
    string_list_field,
)
from qtluau_codegen.symbol_catalog import emit_symbol_catalog

_MANAGEMENT = {
    "openFile",
    "closeFile",
    "closeFileHandle",
    "openCurrent",
    "closeCurrent",
    "switchDocument",
    "raiseDocument",
    "documentInfo",
    "isFileOpened",
    "openedFiles",
    "allOpenedFiles",
}


def generate_hexdoc(manifest: JsonObject, config: JsonObject, output: Path) -> None:
    """Write runtime operations and their matching public Luau declaration.

    The scanner remains the single source of operation signatures and defaults.
    Lifecycle methods are a small, script-owned contract rather than a C++ class.
    """
    style = function_style(config)
    operations: dict[str, str] = {}
    signatures: dict[str, list[str]] = {}
    info_type = "unknown"
    for group in object_list_field(manifest, "native_bindings"):
        namespace = group.get("namespace")
        if namespace not in {"reader", "ctl"}:
            continue
        for method in object_list_field(group, "methods"):
            native = str(method["native_name"])
            if native == "documentInfo":
                info_type = str(method["result"])
            if native in _MANAGEMENT:
                continue
            name = str(method["name"])
            public = snake_case(name) if style == "snake_case" else name
            target = f"{namespace}.{public}"
            if public in operations and operations[public] != target:
                raise ValueError(f"Conflicting hexdoc operation: {public}")
            operations[public] = target
            parameters = [
                f"arg{i}: {typ}" for i, typ in enumerate(string_list_field(method, "parameters"))
            ]
            signature = (
                "((self: Document"
                + (", " + ", ".join(parameters) if parameters else "")
                + f") -> {method['result']})"
            )
            overloads = signatures.setdefault(public, [])
            if signature not in overloads:
                overloads.append(signature)
    if not operations or info_type == "unknown":
        raise ValueError("hexdoc requires reader/ctl operations and documentInfo")
    (output / "generated").mkdir(parents=True, exist_ok=True)
    prefix = "--!strict\n-- Generated from the host script API manifest. Do not edit.\n"
    declaration = (
        "export type Document = {\n"
        + "\n".join(
            [
                "    open_file: (self: Document, path: string | ustring) -> number,",
                "    open_ext: (self: Document, driver: string | ustring, path: string | ustring) -> number,",
                "    open_workspace: (self: Document, path: string | ustring) -> number,",
                "    close: (self: Document, force: boolean?) -> number,",
                "    close_handle: (self: Document) -> number,",
                "    is_valid: (self: Document) -> boolean,",
                f"    document_info: (self: Document) -> {info_type},",
                "    activate: (self: Document) -> boolean,",
                "    use: <A..., R...>(self: Document, callback: (Document, A...) -> R..., A...) -> R...,",
            ]
        )
        + "\n"
        + "".join(
            f"    {name}: " + " & ".join(overloads) + ",\n"
            for name, overloads in sorted(signatures.items())
        )
        + "}\n\n"
    )
    runtime = (
        prefix
        + declaration
        + "return table.freeze({\n"
        + "".join(f"    {name} = {target},\n" for name, target in sorted(operations.items()))
        + "})\n"
    )
    write(output / "generated/hexdoc_api.luau", runtime)
    generate_hexdoc_catalog(manifest, config, output, declaration)


def generate_hexdoc_catalog(
    manifest: JsonObject, config: JsonObject, output: Path, declaration: str
) -> None:
    """Add the host module and its exported type to the existing lazy catalog.

    Catalog entries describe the browser API only; module resolution continues
    to use the host require resolver and its deployed Luau files.
    """
    catalog_name = manifest.get("symbol_catalog")
    if not isinstance(catalog_name, str) or not catalog_name:
        raise ValueError("hexdoc requires the generated symbol catalog")
    directory = output / "definitions"
    catalog_path = directory / catalog_name
    catalog: JsonObject = json.loads(catalog_path.read_text())
    files = DefinitionFiles()
    emit_symbol_catalog(
        {"Document": declaration}, {"hexdoc": "{ new: () -> Document }"},
        set(), [], files, "hexdoc", {"hexdoc": "document"},
        module_types={"hexdoc": ["Document"]}, naming_config=config,
    )
    display_names = {
        key: value for key, value in object_field(catalog, "type_display_names").items()
        if isinstance(value, str)
    }

    def display(source: str) -> str:
        """Apply existing public type names without rewriting string literals."""
        return re.sub(
            r'''"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|\b[A-Za-z_]\w*\b''',
            lambda match: display_names.get(match[0], match[0]), source,
        )

    for filename, source in files.sources.items():
        page: JsonObject = json.loads(source)
        for item in [*object_list_field(page, "roots"), *object_list_field(page, "children")]:
            for field in ("detail", "declaration"):
                value = item.get(field)
                if isinstance(value, str):
                    item[field] = display(value)
            if "overloads" in item:
                item["overloads"] = [display(value) for value in string_list_field(item, "overloads")]
        if filename == "hexdoc_symbols.json":
            roots = object_list_field(catalog, "roots")
            extra = object_list_field(page, "roots")
            for item in extra:
                item["import_path"] = "@s/hexdoc"
            catalog["roots"] = [item for item in roots if item.get("id") != "module:hexdoc"] + extra
        else:
            write(directory / filename, json.dumps(page, ensure_ascii=False, indent=2) + "\n")
    write(catalog_path, json.dumps(catalog, ensure_ascii=False, indent=2) + "\n")
    write(output / "module_api_report.json", json.dumps(catalog, ensure_ascii=False, indent=2) + "\n")
