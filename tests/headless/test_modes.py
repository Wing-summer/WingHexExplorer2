# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: AGPL-3.0-only
"""Exercise standalone windows using the isolated-settings host process probe."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import zlib
from queue import Queue
from threading import Thread

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", type=Path, required=True)
parser.add_argument("--plugins", action="store_true")
args = parser.parse_args()
binary = args.binary.resolve()
assert "HeadlessProbe" in binary.name, "Use the isolated-settings process probe"
count = 0
with tempfile.TemporaryDirectory(prefix="wing-modes-") as directory:
    temporary = Path(directory)
    config = temporary / "config"
    ini = config / "WingCloudStudio/WingHexExplorer2.conf"
    ini.parent.mkdir(parents=True)
    env = os.environ.copy()
    env.update(QT_QPA_PLATFORM="offscreen", XDG_CONFIG_HOME=str(config),
               XDG_DATA_HOME=str(temporary / "data"), XDG_CACHE_HOME=str(temporary / "cache"),
               WINGHEX_HEADLESS_TEST_CONFIG=str(config))
    runtime = temporary / "runtime"
    runtime.mkdir(mode=0o700)
    env["XDG_RUNTIME_DIR"] = str(runtime)
    env.pop("DISPLAY", None)
    env.pop("WAYLAND_DISPLAY", None)
    left = temporary / "左文件.bin"
    right = temporary / "right.bin"
    left.write_bytes(b"left contents")
    right.write_bytes(b"right contents")
    scripts = [temporary / "一.luau", temporary / "two.luau"]
    for script in scripts:
        script.write_text('error("Editor startup must not execute this file")\n')

    def run(arguments, code=0, mode="", files=0, display_free=False):
        global count
        launch = env.copy()
        if display_free:
            launch.pop("QT_QPA_PLATFORM", None)
        if mode:
            launch.update(WINGHEX_MODE_TEST=mode, WINGHEX_MODE_FILES=str(files))
        try:
            result = subprocess.run([str(binary), *map(str, arguments)], env=launch,
                                    capture_output=True, timeout=30, cwd=temporary)
        except subprocess.TimeoutExpired as error:
            raise AssertionError((arguments, error.stdout, error.stderr)) from error
        assert result.returncode == code, (arguments, result)
        count += 1
        return result

    ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=false\n")
    for arguments in (["--diff", "--script"], ["--headless", "--diff", left],
                      ["--live"], ["--script", "--live"], ["--diff", left, right, left],
                      ["--headless"], ["--headless", left, right], ["--unknown"]):
        run(arguments, code=6)
    run(["--unknown"], code=6, display_free=True)
    run(["--diff", "--script"], code=6, display_free=True)
    assert b"--diff" in run(["--help"]).stdout
    assert b"WingHexExplorer2" in run(["--version"]).stdout
    for language, description in (("zh_CN", "二进制编辑器与 Luau 工具"), ("zh_TW", "二進位編輯器與 Luau 工具")):
        ini.write_text(f"[General]\napp.lang={language}\nscript.enable=true\nplugin.enableplugin=false\n")
        assert description in run(["--help"]).stdout.decode()
        message = run(["--headless"], code=6).stderr.decode()
        assert "--headless" in message and ("必须" in message or "必須" in message), message
    ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=false\n")
    for paths in ([], [left], [left, right]):
        env["WINGHEX_MODE_REOPEN"] = str(left) if paths else ""
        run(["--diff", *paths], mode="diff", files=len(paths))
    run(["--diff", "-style", "Fusion", left], mode="diff", files=1)
    dash_script = temporary / "-leading.luau"
    dash_script.write_text('error("must not execute")\n')
    run(["--script", "--", dash_script.name], mode="script", files=1)
    for paths in ([], scripts):
        run(["--script", *paths], mode="script", files=len(paths))
    run(["--script", *scripts, temporary / "missing-one.luau", temporary / "missing-two.luau"], mode="script-errors", files=2)
    toast_script = temporary / "toast.luau"
    toast_script.write_text('assert(toast.show, "missing toast.show before require"); local qt = require("@b/qt"); assert(qt.QPixmap, "missing qt.QPixmap"); assert(toast.show, "missing toast.show after require"); toast.show(qt.QPixmap(), "script toast")')
    env["WINGHEX_MODE_TOAST"] = str(toast_script)
    run(["--script"], mode="script", files=0)
    env["WINGHEX_MODE_TOAST_PARENTLESS"] = "1"
    run(["--script"], mode="script", files=0)
    env.pop("WINGHEX_MODE_TOAST_PARENTLESS")
    env.pop("WINGHEX_MODE_TOAST")
    env["WINGHEX_MODE_UNSAVED"] = "1"
    run(["--script", *scripts], mode="script", files=2)
    env.pop("WINGHEX_MODE_UNSAVED")
    ini.write_text("[General]\nscript.enable=false\nplugin.enableplugin=false\n")
    run(["--script"], code=5, mode="disabled")
    run(["--diff", left, right], mode="diff", files=2)
    if args.plugins:
        def compressed(name):
            raw = name.encode()
            data = len(raw).to_bytes(4, "big") + zlib.compress(raw)
            return "@ByteArray(" + "".join(f"\\x{byte:02x}" for byte in data) + ")"
        ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=true\nplugin.ext=" + compressed("HeadlessProbe") +
                       "\nplugin.dev=" + compressed("HeadlessDeviceProbe") + "\n")
        trace = temporary / "plugins.txt"
        verification = temporary / "verify-plugins.luau"
        verification.write_text('assert(HeadlessProbe.answer() == 42); assert(HeadlessProbe.ready()); assert(HeadlessDeviceProbe.answer() == 42)')
        env.update(WINGHEX_HEADLESS_TEST_TRACE=str(trace), WINGHEX_MODE_PLUGIN=str(verification))
        run(["--script", *scripts], mode="script", files=2)
        assert trace.read_text().splitlines() == ["device:init", "plugin:init", "plugin:register", "device:register",
                                                  "plugin:ready", "plugin:closing", "plugin:unload", "device:unload"]
        env.pop("WINGHEX_MODE_PLUGIN")
        env.pop("WINGHEX_HEADLESS_TEST_TRACE")
    # The full-mode process remains alive while standalone windows run.
    ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=false\nsys.dontUseSplash=true\n")
    full_env = env.copy()
    full_env["WINGHEX_MODE_TEST"] = "full"
    first = subprocess.Popen([str(binary), str(left)], env=full_env,
                             stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    try:
        lines = Queue()
        def await_ready():
            while line := first.stdout.readline():
                if line == b"full-ready\n":
                    lines.put(line)
                    return
        Thread(target=await_ready, daemon=True).start()
        assert lines.get(timeout=30) == b"full-ready\n"
        run(["--diff", left, right], mode="diff", files=2)
        assert first.poll() is None, "standalone diff affected the full-mode process"
        ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=false\nsys.dontUseSplash=true\n")
        run(["--script", *scripts], mode="script", files=2)
        entry = temporary / "entry.luau"
        entry.write_text("return\n")
        run(["--headless", entry])
        assert first.poll() is None, "a standalone script mode affected the full-mode process"
        run([right], code=0)
        assert first.wait(timeout=30) == 0, "full-mode file forwarding or close failed"
        count += 1
    finally:
        if first.poll() is None:
            first.kill()
            first.wait(timeout=5)
        first.stdout.close()
print(f"{count} startup mode scenarios passed")
