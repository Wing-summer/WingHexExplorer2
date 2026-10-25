# Copyright (c) 2026, wingsummer
# SPDX-License-Identifier: BSD-3-Clause
# See 3rdparty/WingLuauQt/LICENSE for license information.

"""Generate one SDK class override for fast runtime probes against a built library."""

import argparse
import json
import pathlib

from qtluau_codegen.assets import support_root
from qtluau_codegen.framework_bindings import SdkBindings
from qtluau_codegen.function_naming import rename_registration_source


def main() -> None:
    """Reuse framework class policy and emit one independently compiled probe."""
    parser = argparse.ArgumentParser()
    parser.add_argument("--class-name", required=True)
    parser.add_argument("--output", required=True, type=pathlib.Path)
    args = parser.parse_args()
    policy = json.loads((support_root() / "config/framework-bindings.json").read_text())
    selected = next(
        group["classes"][args.class_name]
        for group in policy["groups"].values()
        if args.class_name in group.get("classes", {})
    )
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    generator = SdkBindings({})
    generator.classes = {args.class_name}
    generator.emit_class(
        args.class_name,
        output,
        bool(selected.get("static_only", False)),
        str(selected.get("provider", "")),
        bool(selected.get("implicit_parent", False)),
    )
    source = output / f"sdk_{args.class_name}.cpp"
    source.write_text(rename_registration_source({}, source.read_text()))


if __name__ == "__main__":
    main()
