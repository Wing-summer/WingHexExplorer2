#!/usr/bin/env python3
"""Exercise the production loader in isolated application directories."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--lrelease", help="Override the lrelease discovered by CMake")
    args = parser.parse_args()
    args.build_dir = args.build_dir.resolve()
    executable_suffix = ".exe" if os.name == "nt" else ""
    subprocess.run(["cmake", "-S", str(Path(__file__).parent), "-B", str(args.build_dir)], check=True)
    subprocess.run(["cmake", "--build", str(args.build_dir), "-j", "4"], check=True)
    qt_tools = json.loads((args.build_dir / "qt-tools.json").read_text())
    lrelease = args.lrelease or qt_tools["lrelease"]
    with tempfile.TemporaryDirectory(prefix="wing-language-") as temporary:
        base = Path(temporary)
        packages = {}
        for language in ("zh_CN", "zh_TW"):
            qm = base / f"winghex_{language}.qm"
            subprocess.run([lrelease, str(ROOT / f"lang/{language}/winghex_{language}.ts"),
                            "-qm", str(qm)], check=True)
            packages[language] = qm

        # A real plugin resource must remain loadable even in built-in English mode.
        entries = []
        for language in ("en_US", "zh_CN", "zh_TW"):
            ts = base / f"plugin_{language}.ts"
            ts.write_text(f'<TS version="2.1" language="{language}"><context><name>Fixture</name>'
                          '<message><source>Plugin action</source>'
                          f'<translation>Plugin translation ({language})</translation>'
                          '</message></context></TS>')
            qm = base / f"i18n_fixture_{language}.qm"
            subprocess.run([lrelease, str(ts), "-qm", str(qm)], check=True)
            entries.append(f'<file>{qm.name}</file>')
        qrc = base / "plugin.qrc"
        qrc.write_text('<RCC><qresource prefix="/PLGLANG/i18n_fixture">' +
                       ''.join(entries) + '</qresource></RCC>')
        plugin_resource = base / "plugin.rcc"
        subprocess.run([qt_tools["rcc"], "--binary", str(qrc), "-o", str(plugin_resource)], check=True)

        def run(name, configured, system, expected, pack=None, damage=None, stale=False):
            directory = base / name
            directory.mkdir()
            executable = directory / ("language_probe" + executable_suffix)
            shutil.copy2(args.build_dir / ("language_probe" + executable_suffix), executable)
            if pack:
                target = directory / "lang" / pack / "winghex_tr.pak"
                target.parent.mkdir(parents=True)
                if damage == "zip":
                    target.write_bytes(b"broken package")
                else:
                    with zipfile.ZipFile(target, "w") as archive:
                        archive.writestr(f"winghex_{pack}.qm",
                                         b"invalid qm" if damage == "qm" else packages[pack].read_bytes())
                        archive.writestr("about.md", "Localized about")
                        if damage != "missing-document":
                            archive.writestr("devs.md", "Localized developers")
            if stale:
                target = directory / "lang/en_US/winghex_tr.pak"
                target.parent.mkdir(parents=True)
                with zipfile.ZipFile(target, "w") as archive:
                    archive.writestr("winghex_en_US.qm", packages["zh_CN"].read_bytes())
                    archive.writestr("about.md", "Stale about")
                    archive.writestr("devs.md", "Stale developers")
            environment = os.environ | {"WING_TEST_LANGUAGE": configured, "LC_ALL": system,
                                        "LANG": system, "QT_QPA_PLATFORM": "offscreen",
                                        "WING_TEST_PLUGIN_RESOURCE": str(plugin_resource)}
            result = subprocess.run([str(executable)], env=environment, text=True,
                                    capture_output=True, check=True)
            actual = json.loads(result.stdout)
            assert actual["locale"] == expected, (name, actual, result.stderr)
            assert actual["pluginLoaded"], (name, actual)
            assert actual["pluginText"] == f"Plugin translation ({expected})", (name, actual)
            assert actual["englishAvailable"] and actual["about"] and actual["developers"], (name, actual)
            assert actual["translated"] == (expected != "en_US"), (name, actual)
            assert actual["missing"] == "A missing translation", (name, actual)
            if expected == "en_US":
                assert actual["open"] == "Open", (name, actual)
            else:
                assert actual["open"] != "Open", (name, actual)
            print(f"PASS {name}")

        run("explicit-English-no-directory", "en_US", "zh_CN.UTF-8", "en_US")
        run("system-English", "", "en_GB.UTF-8", "en_US")
        run("system-unsupported", "", "de_DE.UTF-8", "en_US", pack="zh_CN")
        run("explicit-unsupported", "fr_FR", "zh_CN.UTF-8", "en_US", pack="zh_CN")
        run("invalid-setting", "invalid-language", "zh_CN.UTF-8", "en_US", pack="zh_CN")
        run("Chinese-without-pack", "zh_CN", "zh_CN.UTF-8", "en_US")
        run("system-simplified", "", "zh_CN.UTF-8", "zh_CN", pack="zh_CN")
        run("explicit-traditional", "zh_TW", "en_US.UTF-8", "zh_TW", pack="zh_TW")
        for damage in ("zip", "qm", "missing-document"):
            run(f"corrupt-{damage}", "zh_CN", "zh_CN.UTF-8", "en_US", pack="zh_CN", damage=damage)
        run("stale-English-pack", "en_US", "en_US.UTF-8", "en_US", stale=True)
        run("English-with-Chinese-pack", "en_US", "zh_CN.UTF-8", "en_US", pack="zh_CN")

        for scale in ("1", "1.5", "2"):
            for language in ("en_US", "zh_CN", "zh_TW"):
                translation = "en_US" if language == "en_US" else str(packages[language])
                subprocess.run([str(args.build_dir / ("ui_probe" + executable_suffix)), str(ROOT), translation],
                               env=os.environ | {"QT_QPA_PLATFORM": "offscreen", "QT_SCALE_FACTOR": scale},
                               check=True)
                print(f"PASS UI forms {language} at scale {scale}")


if __name__ == "__main__":
    main()
