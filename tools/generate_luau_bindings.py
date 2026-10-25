# Copyright (c) 2026, wingsummer
# SPDX-License-Identifier: BSD-3-Clause
# See 3rdparty/WingLuauQt/LICENSE for license information.

"""Generate host bindings using declarative interface and forwarding policies."""

from __future__ import annotations

import argparse
import json
import pathlib
from string import Template

from qtluau_codegen.extensions import write
from qtluau_codegen.models import (
    JsonObject,
    object_field,
    object_list_field,
    string_list_field,
)
from qtluau_codegen.project import export_project


def text(parent: JsonObject, key: str, default: str = "") -> str:
    """Return a string configuration field, rejecting other JSON values."""
    value = parent.get(key, default)
    if not isinstance(value, str):
        raise ValueError(f"{key} must be a string")
    return value


def emit_host_forwarders(manifest: JsonObject, policy: JsonObject, output: pathlib.Path) -> None:
    """Emit callback forwarding from scanned signatures and configured rules.

    Args:
        manifest: Generated native callback contracts.
        policy: Preamble, group bodies, method overrides and native assignments.
        output: Generated project root.

    Raises:
        ValueError: A required rule, group or output path is invalid.
    """
    lines = string_list_field(policy, "preamble")
    captures = ", ".join(string_list_field(policy, "captures"))
    binding_namespace = text(manifest, "namespace")
    rules = object_field(policy, "method_rules")
    groups = object_field(policy, "groups")
    fallbacks = object_field(policy, "fallbacks")
    for group in object_list_field(manifest, "native_bindings"):
        namespace = text(group, "namespace")
        lines.append("{")
        lines.append(f"{text(group, 'callback_type')} {namespace}Callbacks;")
        group_policy = object_field(groups, namespace)
        seen: set[str] = set()
        for method in object_list_field(group, "methods"):
            callback = text(method, "callback")
            if callback in seen:
                continue
            seen.add(callback)
            types = string_list_field(method, "native_arguments")
            params = ", ".join(f"{typ} arg{i}" for i, typ in enumerate(types))
            native = text(method, "native_name")
            override = object_field(rules, native)
            args = string_list_field(override, "arguments", [f"arg{i}" for i in range(len(types))])
            arguments = ", ".join(args)
            result = text(method, "native_result")
            substitutions = {
                "native": text(override, "native", native),
                "arguments": arguments,
                "comma_arguments": ", " + arguments if args else "",
                "return_prefix": "return " if result != "void" else "",
                "fallback": text(fallbacks, native, result + "{}"),
            }
            body = Template(text(override, "body", text(group_policy, "body", text(policy, "default_body")))).substitute(substitutions)
            lines.append(f"{namespace}Callbacks.{callback} = [{captures}]({params}) -> {result} {{ {body} }};")
        module = text(group, "module")
        lines.append(f"{binding_namespace}::bind_{module or 'public'}_{namespace}(L, {namespace}Callbacks);")
        lines.append("}")
    lines.extend(string_list_field(policy, "assignments"))
    destination = pathlib.Path(text(policy, "output"))
    if destination.is_absolute() or ".." in destination.parts:
        raise ValueError("forwarding output must remain inside the generated project")
    write(output / destination, "\n".join(lines) + "\n")


def main() -> int:
    """Generate from codegen configuration; command-line paths override defaults."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=pathlib.Path, default=pathlib.Path(__file__).resolve().parent.parent / "codegen/generation.json")
    parser.add_argument("--host", type=pathlib.Path)
    parser.add_argument("--config", type=pathlib.Path)
    parser.add_argument("--out", type=pathlib.Path)
    parser.add_argument("--build-dir", type=pathlib.Path, help="Build directory for configuration-relative output (required without --out)")
    args = parser.parse_args()
    project_path = args.project.resolve()
    project: JsonObject = json.loads(project_path.read_text())
    host = (args.host or project_path.parent.parent).resolve()
    config_path = args.config or project_path.parent / text(project, "bindings")
    if args.out is None and args.build_dir is None:
        parser.error("pass --build-dir or --out to select build output")
    output = args.out if args.out is not None else args.build_dir.resolve() / text(project, "output")
    output = output.resolve()
    configuration_root = host / "codegen"
    if output == host or output == configuration_root or configuration_root in output.parents:
        parser.error("generated output must not overwrite the host or codegen configuration directory")
    config: JsonObject = json.loads(config_path.read_text())
    forwarding: JsonObject = json.loads((project_path.parent / text(project, "forwarding")).read_text())
    manifest = export_project(config, output, host,
                              postprocess=lambda manifest, staging: emit_host_forwarders(manifest, forwarding, staging))
    print(f"Generated {manifest['name']} in {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
