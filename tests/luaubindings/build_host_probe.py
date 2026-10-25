# Copyright (c) 2026, wingsummer
# SPDX-License-Identifier: BSD-3-Clause
# See 3rdparty/WingLuauQt/LICENSE for license information.

"""Reuse a completed Ninja application build for a real-host input regression probe."""

import argparse
import json
import pathlib
import shlex
import subprocess


def command(build: pathlib.Path, target: str) -> list[str]:
    """Read the existing compiler/linker invocation without executing a shell."""
    output = subprocess.check_output(["ninja", "-C", str(build), "-t", "commands", target], text=True)
    args = shlex.split(output.splitlines()[-1])
    if args[:2] == [":", "&&"]:
        args = args[2:]
    if "&&" in args:
        args = args[:args.index("&&")]
    return args


def main() -> None:
    """Compile one test entry point and link the already-built application objects."""
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", type=pathlib.Path, required=True)
    args = parser.parse_args()
    build = args.build.resolve()
    root = pathlib.Path(__file__).resolve().parents[2]
    test_object = build / "host_input_probe.o"
    compile_args = command(build, "CMakeFiles/WingHexExplorer2.dir/main.cpp.o")
    for flag in ("-MT", "-MF"):
        if flag in compile_args:
            index = compile_args.index(flag)
            del compile_args[index:index + 2]
    compile_args = [value for value in compile_args if value != "-MD"]
    includes = [root / "3rdparty/WingLuauQt/include", root / "3rdparty/LuaBridge3/Source", *(root / "3rdparty/luau").glob("*/include")]
    compile_args[1:1] = ["-I" + str(path) for path in includes]
    compile_args[compile_args.index("-o") + 1] = str(test_object)
    compile_args[-1] = str(root / "tests/luaubindings/host_input.cpp")
    subprocess.run(compile_args, cwd=build, check=True)
    support_object = build / "host_input_execution.o"
    compile_args[compile_args.index("-o") + 1] = str(support_object)
    compile_args[-1] = str(root / "3rdparty/WingLuauQt/support/execute_luau.cpp")
    subprocess.run(compile_args, cwd=build, check=True)
    resource_source = build / "host_input_resources.cpp"
    autogen = next((build / "CMakeFiles/WingHexExplorer2_autogen.dir").glob("AutoRcc*_Info.json"))
    rcc = json.loads(autogen.read_text())["RCC_EXECUTABLE"]
    subprocess.run([rcc, "-name", "wing_luau_host_probe", "-o", str(resource_source),
                    str(root / "tests/luaubindings/plugin_definitions.qrc")], cwd=build, check=True)
    resource_object = build / "host_input_resources.o"
    compile_args[compile_args.index("-o") + 1] = str(resource_object)
    compile_args[-1] = str(resource_source)
    subprocess.run(compile_args, cwd=build, check=True)
    link_args = command(build, "WingHexExplorer2")
    link_args[1:1] = [str(support_object), str(resource_object)]
    link_args[link_args.index("-o") + 1] = str(build / "WingLuauHostInputProbe")
    link_args = [str(test_object) if value.endswith("/WingHexExplorer2.dir/main.cpp.o") else value for value in link_args]
    # Keep the application's linker dependency file separate.
    link_args = [value for value in link_args if not value.startswith("-Wl,--dependency-file=")]
    subprocess.run(link_args, cwd=build, check=True)


if __name__ == "__main__":
    main()
