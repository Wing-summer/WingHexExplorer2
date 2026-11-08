#!/usr/bin/env python3
"""Check the frozen pre-extension public-slot signatures and return types."""
import json
from pathlib import Path
import re


def public_slots(header: str) -> dict[str, str]:
    header = re.sub(r"/\*.*?\*/|//[^\n]*", "", header, flags=re.S)
    sections = re.split(
        r"(public\s+slots:|private\s+slots:|protected\s+slots:|public:|private:|protected:)",
        header,
    )
    mode = ""
    result = {}
    for section in sections:
        if re.fullmatch(r"(?:public|private|protected)(?:\s+slots)?:", section):
            mode = section
            continue
        if mode != "public slots:":
            continue
        for method in re.finditer(
            r"([\w:\s<>,*&]+?)\b(\w+)\s*\(([^;{}]*?)\)\s*;", section
        ):
            parameters = []
            for parameter in method[3].split(","):
                parameter = re.sub(r"\b\w+\s*$", "", parameter.strip())
                parameters.append(re.sub(r"\s*([*&])\s*", r"\1", parameter).strip())
            signature = method[2] + "(" + ",".join(parameters) + ")"
            result[signature] = " ".join(method[1].split())
    return result


def main() -> None:
    directory = Path(__file__).resolve().parent
    expected = json.loads((directory / "legacy_slots.json").read_text())
    header = directory.parents[1] / "src/class/plugin/pluginsystem.h"
    actual = public_slots(header.read_text())
    missing = {signature: result for signature, result in expected.items()
               if actual.get(signature) != result}
    if missing:
        raise SystemExit(f"Existing plugin API changed: {missing}")
    print(f"All {len(expected)} existing public slot contracts are unchanged")


if __name__ == "__main__":
    main()
