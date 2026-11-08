#!/usr/bin/env python3
"""Link the probe to real host objects (no UI/document stubs).
链接真实宿主对象验证 UI 与文档集成，不使用替身。
"""

import argparse
import json
import os
import shlex
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--build-dir", type=Path, required=True)
args = parser.parse_args()
build = args.build_dir.resolve()
root = Path(__file__).resolve().parents[2]
commands = json.loads((build / "compile_commands.json").read_text())
source = next(x for x in commands if x["file"] == str(root / "main.cpp"))
compile_args = shlex.split(source["command"])
# A probe must not overwrite the application's compiler dependency files.
for flag in ("-MF", "-MT", "-MQ"):
    while flag in compile_args:
        index = compile_args.index(flag)
        del compile_args[index : index + 2]
compile_args = [arg for arg in compile_args if arg not in ("-MD", "-MMD", "-MP")]

compile_args[compile_args.index(source["file"])] = str(
    root / "tests/diff/host_probe.cpp"
)
probe_object = build / "diff_host_probe.o"
compile_args[compile_args.index("-o") + 1] = str(probe_object)
subprocess.run(compile_args, cwd=build, check=True)
lines = subprocess.check_output(
    ["ninja", "-t", "commands", "WingHexExplorer2"], cwd=build, text=True
).splitlines()
link = next(line for line in reversed(lines) if " -o WingHexExplorer2 " in line)
link_args = shlex.split(link)
if link_args[:2] == [":", "&&"]:
    link_args = link_args[2:]
if "&&" in link_args:
    link_args = link_args[: link_args.index("&&")]
link_args[link_args.index("CMakeFiles/WingHexExplorer2.dir/main.cpp.o")] = str(
    probe_object
)
executable = build / "diff_host_probe"
link_args[link_args.index("-o") + 1] = str(executable)
subprocess.run(link_args, cwd=build, check=True)
env = dict(os.environ, QT_QPA_PLATFORM="offscreen")
subprocess.run([str(executable)], cwd=build, env=env, check=True, timeout=90)
