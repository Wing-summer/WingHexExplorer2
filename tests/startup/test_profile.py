"""Validate compile-time exclusion and opt-in first-paint profiling."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def probe(executable: str, trace: bool | None) -> tuple[str, list[dict]]:
    env = os.environ | {
        "WINGHEX_STARTUP_TRACE": str(int(bool(trace))),
        "WINGHEX_STARTUP_EXIT_AFTER_READY": "1",
    }
    if trace is None:
        env.pop("WINGHEX_STARTUP_TRACE", None)
    result = subprocess.run([executable], env=env, capture_output=True, text=True,
                            timeout=10, check=True)
    records = [json.loads(line.removeprefix("[startup] "))
               for line in result.stderr.splitlines() if line.startswith("[startup] ")]
    return result.stdout, records


enabled, disabled = sys.argv[1:]
stdout, records = probe(enabled, None)
assert "closes=1" in stdout and records
stdout, records = probe(disabled, None)
assert "closes=0" in stdout and not records
stdout, records = probe(enabled, True)
assert "closes=1" in stdout
assert [record["phase"] for record in records] == [
    "probe.application", "process.first_paint_ready"]
assert all(record["duration_ms"] >= 0 for record in records)
assert records[-1]["at_ms"] >= records[0]["at_ms"]
for executable in (enabled, disabled):
    stdout, records = probe(executable, False)
    assert "closes=0" in stdout and not records
stdout, records = probe(disabled, True)
assert "closes=0" in stdout and not records
print("Startup profile compile/runtime gates and first-paint exit passed")

if sys.platform == "linux":
    runner = Path(__file__).resolve().parents[2] / "tools/benchmark_startup.py"
    with tempfile.TemporaryDirectory(prefix="startup-profile-test-") as directory:
        output = Path(directory) / "enabled.json"
        result = subprocess.run([sys.executable, str(runner), enabled,
                                 "--runs", "2", "--warmup", "0",
                                 "--output", str(output)],
                                capture_output=True, text=True, timeout=15)
        assert result.returncode == 0, result.stderr
        report = json.loads(output.read_text())
        assert len(report["samples"]) == len(report["launch_to_ready_ms"]) == 2
        assert report["launch_to_ready_median_ms"] > 0
        assert report["launch_to_feedback_ms"] == report["launch_to_ready_ms"]
        disabled_output = Path(directory) / "disabled.json"
        result = subprocess.run([sys.executable, str(runner), disabled,
                                 "--runs", "1", "--warmup", "0",
                                 "--output", str(disabled_output)],
                                capture_output=True, text=True, timeout=15)
        assert result.returncode != 0 and not disabled_output.exists()
    print("Runner records launch-to-ready and rejects disabled profiling")
