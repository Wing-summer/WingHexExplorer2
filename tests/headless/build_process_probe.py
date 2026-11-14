# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: AGPL-3.0-only
"""Relink a completed Ninja host build with an isolated-settings test entry."""
import argparse
from pathlib import Path
import shlex
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--build", type=Path, required=True)
args = parser.parse_args()
build = args.build.resolve()
root = Path(__file__).resolve().parents[2]


def command(target: str) -> list[str]:
    """Read the build's actual compiler or linker arguments."""
    output = subprocess.check_output(["ninja", "-C", str(build), "-t", "commands", target], text=True)
    result = shlex.split(output.splitlines()[-1])
    if result[:2] == [":", "&&"]:
        result = result[2:]
    if "&&" in result:
        result = result[:result.index("&&")]
    return result


compile_args = command("CMakeFiles/WingHexExplorer2.dir/main.cpp.o")
for flag in ("-MT", "-MF"):
    if flag in compile_args:
        index = compile_args.index(flag)
        del compile_args[index:index + 2]
compile_args = [value for value in compile_args if value != "-MD"]
entry = build / "headless_process_main.o"
compile_args[compile_args.index("-o") + 1] = str(entry)
subprocess.run([*compile_args[:1], "-Dmain=wingApplicationMain", *compile_args[1:]], cwd=build, check=True)
wrapper = build / "headless_process_wrapper.o"
compile_args[compile_args.index("-o") + 1] = str(wrapper)
compile_args[-1] = str(root / "tests/headless/process_main.cpp")
subprocess.run(compile_args, cwd=build, check=True)
link = command("WingHexExplorer2")
link = [str(entry) if value.endswith("/WingHexExplorer2.dir/main.cpp.o") else value for value in link]
link = [value for value in link if not value.startswith("-Wl,--dependency-file=")]
link.insert(1, str(wrapper))
link[link.index("-o") + 1] = str(build / "WingHexHeadlessProbe")
subprocess.run(link, cwd=build, check=True)
print(build / "WingHexHeadlessProbe")
