#!/usr/bin/env python3
"""Check maintained catalogs for incomplete text, placeholders and identifier labels."""
from collections import Counter
from pathlib import Path
import re
import json
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
# Brand names are intentionally camel case. Technical acronyms have no lowercase
# to uppercase transition and do not need blanket exceptions.
BRANDS = {"WingHexExplorer", "WingHexExplorer2", "WingCloudStudio", "wingsummer"}
PLACEHOLDER = re.compile(r"%(?:L?\d+|L?n)")
IDENTIFIER = re.compile(r"^[A-Za-z0-9_:]*[a-z][A-Z][A-Za-z0-9_:]*$")


def check():
    errors = []
    catalogs = sorted((ROOT / "lang").glob("*/winghex_*.ts"))
    if {p.parent.name for p in catalogs} != {"zh_CN", "zh_TW"}:
        errors.append("Only zh_CN and zh_TW catalogs should be maintained")
    keys_by_language = {}
    for path in catalogs:
        keys = set()
        for context in ET.parse(path).getroot().findall("context"):
            name = context.findtext("name")
            for message in context.findall("message"):
                source = message.findtext("source", "")
                translation = message.find("translation")
                key = (name, source, message.findtext("comment", ""))
                label = f"{path.relative_to(ROOT)}: {name}: {source[:85]!r}"
                if key in keys:
                    errors.append(f"{label}: duplicate translation key")
                keys.add(key)
                if translation is None:
                    errors.append(f"{label}: missing translation")
                    continue
                if translation.get("type") in {"unfinished", "vanished", "obsolete"}:
                    errors.append(f"{label}: {translation.get('type')} translation")
                forms = translation.findall("numerusform") or [translation]
                for form in forms:
                    text = "".join(form.itertext()).strip()
                    if not text:
                        errors.append(f"{label}: empty translation")
                    if Counter(PLACEHOLDER.findall(source)) != Counter(PLACEHOLDER.findall(text)):
                        errors.append(f"{label}: inconsistent placeholders")
                    if "<" not in source and source.count("&") != text.count("&"):
                        errors.append(f"{label}: inconsistent mnemonic markers")
                    filters = lambda value: Counter(re.findall(r"\*\.[A-Za-z0-9*?]+", value))
                    if filters(source) != filters(text):
                        errors.append(f"{label}: inconsistent file filter patterns")
                owned = any(loc.get("filename", "").startswith("../../src/")
                            for loc in message.findall("location"))
                if owned and source not in BRANDS and IDENTIFIER.fullmatch(source):
                    errors.append(f"{label}: identifier used as UI text")
        keys_by_language[path.parent.name] = keys
    if len(keys_by_language) == 2 and len(set(map(frozenset, keys_by_language.values()))) != 1:
        errors.append("Chinese catalogs have different message keys")
    all_sources = {key[1] for keys in keys_by_language.values() for key in keys}
    literal = r'"(?:[^"\\]|\\.)*"'
    calls = re.compile(r'\b(?:tr|QT_TR_NOOP)\s*\(\s*((?:' + literal + r'\s*)+)')
    for path in (ROOT / "src").rglob("*"):
        sources = []
        if path.suffix == ".ui":
            for string in ET.parse(path).getroot().iter("string"):
                if string.get("notr") != "true" and string.text:
                    sources.append(string.text)
        elif path.suffix in {".cpp", ".h"}:
            for match in calls.finditer(path.read_text()):
                try:
                    sources.append("".join(json.loads(text) for text in re.findall(literal, match[1])))
                except ValueError:
                    continue  # C++-specific escape forms are checked by lupdate.
        for source in sources:
            label = f"{path.relative_to(ROOT)}: {source[:85]!r}"
            if source and source not in all_sources:
                errors.append(f"{label}: source text is missing from the catalogs")
            if source not in BRANDS and IDENTIFIER.fullmatch(source):
                errors.append(f"{label}: identifier used as UI text")
    for error in errors:
        print(error, file=sys.stderr)
    if not errors:
        print(f"Translation checks passed ({len(catalogs)} catalogs)")
    return bool(errors)


if __name__ == "__main__":
    sys.exit(check())
