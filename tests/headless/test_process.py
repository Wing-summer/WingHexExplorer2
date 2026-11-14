# Copyright (C) 2026 WingSummer
# SPDX-License-Identifier: AGPL-3.0-only
"""One-shot headless regression / 一次性无头脚本回归。"""
import argparse
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", type=Path, required=True)
parser.add_argument("--plugins", action="store_true")
parser.add_argument("--slow", action="store_true")
args = parser.parse_args()
binary = args.binary.resolve()
assert "HeadlessProbe" in binary.name
count = 0
with tempfile.TemporaryDirectory(prefix="wing-oneshot-") as directory:
    temporary = Path(directory)
    config = temporary / "config"
    ini = config / "WingCloudStudio/WingHexExplorer2.conf"
    ini.parent.mkdir(parents=True)
    ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=false\nscript.timeout=1\n")
    env = os.environ.copy()
    env.update(QT_QPA_PLATFORM="offscreen", XDG_CONFIG_HOME=str(config),
               XDG_DATA_HOME=str(temporary / "data"), XDG_CACHE_HOME=str(temporary / "cache"),
               WINGHEX_HEADLESS_TEST_CONFIG=str(config), WINGHEX_HEADLESS_TEST_NO_UI="1")
    env.pop("DISPLAY", None)
    env.pop("WAYLAND_DISPLAY", None)
    def run(source, code=0, output=None):
        global count
        path = temporary / f"入口-{count}.luau"
        path.write_text(source)
        result = subprocess.run([str(binary), "--headless", str(path)], env=env,
                                input=b"must not be read\n", capture_output=True,
                                timeout=75 if args.slow else 10)
        assert result.returncode == code, (source, result)
        if output is not None:
            assert result.stdout == output, result
        count += 1
        return result
    run('println("hello"); print("tail")', output=b"hello\ntail")
    run('return', output=b"")
    run('error("failure")', code=8)
    run('local c=require("@b/core"); local t=c.timer(); t:set_single_shot(true); '
        't:connect("timeout", function() error("one-shot callback failed") end); '
        't:start(0); while true do coroutine.yield() end', code=8)
    run(') invalid syntax', code=8)
    assert b"Standard input is unavailable" in run('input()', code=8).stderr
    run('require("@b/host")', code=8)
    for module in ["qt", "dialogs", "toast", "clipboard"]:
        run(f'require("@b/{module}")', code=8)
    run('assert(filesystem.dir("."):exists()); require("@b/network"); require("@b/cffi")')
    missing = subprocess.run([str(binary), "--headless", str(temporary / "missing.luau")],
                             env=env, capture_output=True, timeout=10)
    assert missing.returncode == 7
    startup = dict(env, WINGHEX_HEADLESS_TEST_CHECK_ENV="1", WINGHEX_DISALLOW_ROOT_PLUGIN="0",
                   WINGHEX_CERT_SKIP="1", WINGHEX_CERT_FORCE_ALL="1")
    assert subprocess.run([str(binary), "--unknown"], env=startup, capture_output=True).returncode == 6
    ini.write_text("[General]\nscript.enable=false\nplugin.enableplugin=true\n")
    run('error("must not execute")', code=5, output=b"")
    assert subprocess.run([str(binary), "--headless", "--live"], env=env,
                          capture_output=True, timeout=10).returncode == 5
    ini.write_text("[General]\nscript.enable=true\nplugin.enableplugin=false\nscript.timeout=1\n")
    if args.slow:
        assert b"timed-out" in run('while true do end', code=8).stderr
    entry = temporary / "loop.luau"
    entry.write_text('while true do end')
    for number, code in [(signal.SIGINT, 130)] + ([(signal.SIGTERM, 143)] if os.name != "nt" else []):
        process = subprocess.Popen([str(binary), "--headless", str(entry)], env=env,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        try:
            time.sleep(.2)
            process.send_signal(number)
            out, error = process.communicate(timeout=10)
            assert process.returncode == code, (process.returncode, error)
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
    print(f"{count} one-shot headless scenarios plus arguments and signals passed")
