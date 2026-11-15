"""Package a Luau plugin without executing its source / 打包 Luau 插件，不执行源码。"""
from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import tempfile
import zipfile

SOURCE = {".lua", ".luau"}
BINARY = {".luac", ".luauc"}
MAX_FILE = 16 * 1024 * 1024
MAX_TOTAL = 64 * 1024 * 1024


def package(source: pathlib.Path, destination: pathlib.Path, compiler: pathlib.Path | None = None) -> None:
    source = source.resolve(strict=True)
    destination = destination.absolute()
    if destination.suffix.lower() not in {".wingplg", ".wingdrv"}:
        raise ValueError("Output must end in .wingplg or .wingdrv")
    if destination.is_relative_to(source):
        raise ValueError("Output must be outside the source directory")
    manifest = json.loads((source / "manifest.json").read_text(encoding="utf-8"))
    if manifest.get("FormatVersion") != 1:
        raise ValueError("Unsupported FormatVersion")
    entries = [path for path in source.iterdir() if path.stem == "main" and path.suffix.lower() in SOURCE | BINARY]
    if len(entries) != 1:
        raise ValueError("Exactly one main.lua/main.luau/main.luac/main.luauc is required")
    files: dict[str, bytes] = {}
    folded: set[str] = set()
    total = 0
    for path in sorted(source.rglob("*")):
        if path.is_symlink():
            raise ValueError(f"Symlinks are not supported: {path}")
        if not path.is_file():
            continue
        name = path.relative_to(source).as_posix()
        if ":" in name or "\\" in name:
            raise ValueError(f"Invalid archive path: {name}")
        if path.stat().st_size > MAX_FILE:
            raise ValueError(f"File is too large: {name}")
        data = path.read_bytes()
        if compiler and path.suffix.lower() in SOURCE and not name.endswith(".d.luau"):
            result = subprocess.run([str(compiler), "--binary", "-O2", "-g1", str(path)], capture_output=True, check=True)
            data = result.stdout
            if not data or data[0] == 0:
                raise ValueError(f"Compiler did not produce bytecode: {name}")
            name = str(pathlib.PurePosixPath(name).with_suffix(".luauc"))
        if pathlib.PurePosixPath(name).suffix.lower() in SOURCE | BINARY and not name.endswith(".d.luau") and len(data) > 1024 * 1024:
            raise ValueError(f"Executable entry exceeds 1 MiB: {name}")
        key = name.casefold()
        if key in folded:
            raise ValueError(f"Duplicate output path: {name}")
        folded.add(key)
        total += len(data)
        if len(data) > MAX_FILE or total > MAX_TOTAL or len(files) >= 4096:
            raise ValueError("Package exceeds resource limits")
        files[name] = data
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Publish only a complete archive; leave an existing output intact on error.
    with tempfile.NamedTemporaryFile(dir=destination.parent, delete=False) as temporary:
        temporary_path = pathlib.Path(temporary.name)
    try:
        with zipfile.ZipFile(temporary_path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
            for name, data in files.items():
                info = zipfile.ZipInfo(name, (2026, 1, 1, 0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                archive.writestr(info, data)
        temporary_path.replace(destination)
    finally:
        temporary_path.unlink(missing_ok=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=pathlib.Path)
    parser.add_argument("output", type=pathlib.Path)
    parser.add_argument("--compiler", type=pathlib.Path, help="Matching luau-compile; compile source to .luauc")
    args = parser.parse_args()
    package(args.source, args.output, args.compiler)


if __name__ == "__main__":
    main()
