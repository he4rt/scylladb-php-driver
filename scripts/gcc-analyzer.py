#!/usr/bin/env python3
import argparse
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

ANALYZER_FLAGS = [
    "-fanalyzer",
    "-Wno-analyzer-too-complex",
    "-fdiagnostics-plain-output",
    "-Wno-format",
    "-Wno-deprecated-declarations",
    "-Wno-unused-parameter",
    "-Wno-unused-result",
]

FINDING = re.compile(r"warning:.*\[-Wanalyzer-")


def build_command(entry, compiler):
    args = shlex.split(entry["command"])
    kept = []
    skip = False
    for arg in args:
        if skip:
            skip = False
            continue
        if arg == "-o":
            skip = True
            continue
        if arg == "-c":
            continue
        kept.append(arg)
    kept[0] = compiler
    handle, obj = tempfile.mkstemp(suffix=".o")
    os.close(handle)
    return kept[:1] + ANALYZER_FLAGS + kept[1:] + ["-c", "-o", obj], obj


def analyze(entry, compiler):
    command, obj = build_command(entry, compiler)
    try:
        result = subprocess.run(
            command, cwd=entry["directory"], capture_output=True, text=True, check=False
        )
    finally:
        os.unlink(obj)
    return entry["file"], result.stderr


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-db", required=True)
    parser.add_argument("--compiler", default="gcc")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    with open(args.compile_db) as db:
        entries = json.load(db)
    wanted = {os.path.realpath(f) for f in args.files}
    if wanted:
        entries = [e for e in entries if os.path.realpath(e["file"]) in wanted]
    else:
        root = os.path.realpath(os.path.join(os.path.dirname(args.compile_db), "..", ".."))
        entries = [e for e in entries if os.path.realpath(e["file"]).startswith(os.path.join(root, "src") + os.sep)]

    if not entries:
        print("gcc-analyzer: nothing to analyze")
        return 0

    findings = 0
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for path, stderr in pool.map(lambda e: analyze(e, args.compiler), entries):
            reported = [line for line in stderr.splitlines() if FINDING.search(line)]
            if not reported:
                continue
            findings += len(reported)
            print(f"--- {path}")
            print(stderr.rstrip())

    print(f"gcc-analyzer: {len(entries)} translation units, {findings} findings")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
