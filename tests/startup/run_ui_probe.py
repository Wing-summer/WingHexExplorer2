#!/usr/bin/env python3
"""Link the real-host lazy settings probe against an existing Ninja application build."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shlex
import subprocess


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().with_name("ui_probe.cpp"))
    args = parser.parse_args()
    build = args.build.resolve()
    source = args.source.resolve()
    commands = json.loads(subprocess.check_output(
        ["ninja", "-t", "compdb"], cwd=build, text=True))
    entry = next(item for item in commands if Path(item["file"]).name == "main.cpp"
                 and "WingHexExplorer2.dir" in item["command"])
    compile_args = shlex.split(entry["command"])
    compile_args.extend(["-I", str(build)])
    original_object = compile_args[compile_args.index("-o") + 1]
    probe_object = build / "startup_ui_probe.o"
    compile_args[compile_args.index("-o") + 1] = str(probe_object)
    compile_args[compile_args.index("-c") + 1] = str(source)
    subprocess.run(compile_args, cwd=entry["directory"], check=True)
    commands_output = subprocess.check_output(
        ["ninja", "-t", "commands", "WingHexExplorer2"], cwd=build, text=True
    )
    link_line = next(line for line in reversed(commands_output.splitlines())
                     if original_object in line and " -o " in line and " -c " not in line)
    # CMake emits ': && compiler ... && :' for this link step. Never execute shell text.
    link_line = link_line.removeprefix(": && ").removesuffix(" && :")
    link_args = shlex.split(link_line)
    if "&&" in link_args:
        raise RuntimeError("Unsupported compound link command")
    executable = build / "startup_ui_probe"
    link_args[link_args.index(original_object)] = str(probe_object)
    link_args[link_args.index("-o") + 1] = str(executable)
    subprocess.run(link_args, cwd=build, check=True)
    environment = os.environ.copy()
    environment["QT_QPA_PLATFORM"] = "offscreen"
    for name in ("XDG_CONFIG_HOME", "XDG_DATA_HOME", "XDG_CACHE_HOME"):
        location = build / "startup-ui-probe-state" / name.lower()
        location.mkdir(parents=True, exist_ok=True)
        environment[name] = str(location)
    subprocess.run([str(executable)], cwd=build, env=environment, check=True, timeout=120)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
