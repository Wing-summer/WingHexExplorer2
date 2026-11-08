#!/usr/bin/env python3
"""Measure an instrumented host in an isolated, persistent settings directory."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import statistics
import subprocess
import sys
import tempfile
import threading
import time


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--config", type=Path, help="Copy an existing Qt .conf; never edit the original")
    parser.add_argument("--file", type=Path, action="append", default=[], help="Open a fixture file at startup")
    parser.add_argument("--runs", type=int, default=5)
    parser.add_argument("--warmup", type=int, default=1)
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--platform", default="offscreen", help="Qt platform, or 'native' to inherit")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if sys.platform != "linux":
        parser.error("This runner isolates Qt settings through XDG paths and supports Linux only")
    if args.runs < 1 or args.warmup < 0:
        parser.error("runs must be positive and warmup non-negative")
    executable = args.executable.resolve(strict=True)
    if args.config:
        args.config = args.config.resolve(strict=True)
    samples = []
    launch_times = []
    feedback_times = []
    with tempfile.TemporaryDirectory(prefix="wing-startup-") as directory:
        root = Path(directory)
        if args.config:
            settings = root / "config/WingCloudStudio/WingHexExplorer2.conf"
            settings.parent.mkdir(parents=True)
            shutil.copyfile(args.config, settings)
        runtime = root / "runtime"
        runtime.mkdir(mode=0o700)
        env = os.environ | {
            "XDG_CONFIG_HOME": str(root / "config"),
            "XDG_CACHE_HOME": str(root / "cache"),
            "XDG_DATA_HOME": str(root / "data"),
            # QtSingleApplication identifies this app by organization/user, not
            # executable path. Isolate its lock and local socket as well.
            "TMPDIR": str(runtime),
            "XDG_RUNTIME_DIR": str(runtime),
            "WINGHEX_STARTUP_TRACE": "1",
            "WINGHEX_STARTUP_EXIT_AFTER_READY": "1",
        }
        if args.platform == "native":
            display = os.environ.get("WAYLAND_DISPLAY", "")
            original_runtime = os.environ.get("XDG_RUNTIME_DIR", "")
            if display and original_runtime and not Path(display).is_absolute():
                env["WAYLAND_DISPLAY"] = str(Path(original_runtime) / display)
        if args.platform != "native":
            env["QT_QPA_PLATFORM"] = args.platform
        for index in range(args.warmup + args.runs):
            records = []
            diagnostics = []
            ready = []
            feedback = []
            launched = time.perf_counter()
            command = [str(executable), *(str(path.resolve(strict=True)) for path in args.file)]
            with subprocess.Popen(command, cwd=executable.parent, env=env,
                                  text=True, stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT) as process:
                def collect() -> None:
                    assert process.stdout is not None
                    for line in process.stdout:
                        diagnostics.append(line)
                        if line.startswith("[startup] "):
                            record = json.loads(line.removeprefix("[startup] "))
                            records.append(record)
                            observed_ms = (time.perf_counter() - launched) * 1000
                            if record["phase"] == "process.first_paint_ready":
                                ready.append(observed_ms)
                            if not feedback and record["phase"] in (
                                    "process.splash_first_paint", "process.first_paint_ready"):
                                feedback.append(observed_ms)
                reader = threading.Thread(target=collect, daemon=True)
                reader.start()
                try:
                    process.wait(timeout=args.timeout)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                    reader.join()
                    raise RuntimeError("Host timed out:\n" + "".join(diagnostics)[-6000:]) from None
                reader.join()
                if process.returncode or not ready:
                    raise RuntimeError(f"Host failed or profiling is disabled (exit={process.returncode}):\n" + "".join(diagnostics)[-6000:])
            if index >= args.warmup:
                samples.append(records)
                launch_times.append(ready[0])
                feedback_times.append(feedback[0])
    phases = {}
    for records in samples:
        for record in records:
            values = phases.setdefault(record["phase"], {"at_ms": [], "duration_ms": []})
            for key in values:
                values[key].append(record[key])
    summary = {phase: {key: statistics.median(values) for key, values in measurements.items()}
               for phase, measurements in phases.items()}
    report = {"executable": str(executable), "platform": args.platform,
              "runs": args.runs, "warmup": args.warmup, "summary": summary,
              "launch_to_feedback_ms": feedback_times,
              "launch_to_feedback_median_ms": statistics.median(feedback_times),
              "launch_to_ready_ms": launch_times,
              "launch_to_ready_median_ms": statistics.median(launch_times),
              "samples": samples}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({"launch_to_feedback_median_ms": report["launch_to_feedback_median_ms"],
                      "launch_to_ready_median_ms": report["launch_to_ready_median_ms"],
                      "phases": summary}, indent=2))


if __name__ == "__main__":
    main()
