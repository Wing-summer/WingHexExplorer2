#!/usr/bin/env python3
"""Run actual (non-sparse) file workloads; cache state is uncontrolled.
运行实际写入文件基准（非稀疏文件）；不控制操作系统缓存。
"""

import argparse
import json
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--executable", type=Path, required=True)
parser.add_argument("--directory", type=Path, required=True)
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--resume", action="store_true")
parser.add_argument("--sizes", type=int, nargs="+", default=[32768, 65536])
args = parser.parse_args()
args.directory.mkdir(parents=True, exist_ok=True)
reports = (
    json.loads(args.output.read_text()) if args.resume and args.output.exists() else []
)
for size in args.sizes:
    for pattern in ("same", "sparse", "insert", "delete", "dense", "unrelated"):
        if any(r["sizeMiB"] == size and r["pattern"] == pattern for r in reports):
            continue
        print(f"Running {size} MiB: {pattern}", flush=True)
        run = subprocess.run(
            [
                str(args.executable.resolve()),
                str(size),
                pattern,
                str(args.directory.resolve()),
            ],
            check=True,
            capture_output=True,
            text=True,
        )
        report = json.loads(run.stdout)
        reports.append(report)
        args.output.write_text(json.dumps(reports, indent=2) + "\n")
        print(json.dumps(report), flush=True)
