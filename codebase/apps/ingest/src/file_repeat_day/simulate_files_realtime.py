#!/usr/bin/env python3
"""
simulate_radar_archive.py

Replay archived radar files in time order by copying them into a target
directory tree and updating latest_data_info via LdataWriter.

Expected source layout:

  SRC_ROOT/
    20260101/
      some_prefix_20260101_120000_suffix.ext
      ...
    20260102/
      ...

Destination layout will mirror the day-directory structure:

  DST_ROOT/
    20260101/
      same_filename.ext
    20260102/
      ...

Example:

  python simulate_radar_archive.py \
      --src-root /data/archive \
      --dst-root /data/sim/input \
      --sleep-secs 2.0 \
      --ldir-scope root

If you want latest_data_info written separately for each YYYYMMDD directory:

  python simulate_radar_archive.py \
      --src-root /data/archive \
      --dst-root /data/sim/input \
      --sleep-secs 2.0 \
      --ldir-scope day
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import List, Optional


DAY_DIR_RE = re.compile(r"^\d{8}$")
TS_RE = re.compile(r"(\d{8}_\d{6})")


@dataclass(order=True)
class FileEntry:
    data_time: datetime
    src_path: Path
    day_dir: str
    filename: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Replay archived radar files by copying them in time order "
            "to a destination tree and calling LdataWriter after each file."
        )
    )

    parser.add_argument(
        "--src-root",
        required=True,
        help="Top-level source directory containing YYYYMMDD day directories.",
    )
    parser.add_argument(
        "--dst-root",
        required=True,
        help="Top-level destination directory to receive copied files.",
    )
    parser.add_argument(
        "--sleep-secs",
        type=float,
        default=0.0,
        help="Seconds to sleep after handling each file. Default: 0.0",
    )
    parser.add_argument(
        "--ldatawriter",
        default="LdataWriter",
        help="Path to LdataWriter executable. Default: LdataWriter",
    )
    parser.add_argument(
        "--ldir-scope",
        choices=["root", "day"],
        default="root",
        help=(
            "Where latest_data_info should be written: "
            "'root' => use --dst-root as -dir and YYYYMMDD/file as -rpath; "
            "'day'  => use --dst-root/YYYYMMDD as -dir and file as -rpath. "
            "Default: root"
        ),
    )
    parser.add_argument(
        "--start-time",
        help="Optional lower time bound, format YYYYMMDD_HHMMSS.",
    )
    parser.add_argument(
        "--end-time",
        help="Optional upper time bound, format YYYYMMDD_HHMMSS.",
    )
    parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Overwrite destination files if they already exist.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print what would be done, but do not copy files or run LdataWriter.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print extra progress messages.",
    )

    return parser.parse_args()


def parse_filter_time(value: Optional[str], arg_name: str) -> Optional[datetime]:
    if value is None:
        return None
    try:
        return datetime.strptime(value, "%Y%m%d_%H%M%S")
    except ValueError as exc:
        raise SystemExit(f"ERROR: bad {arg_name} '{value}', expected YYYYMMDD_HHMMSS") from exc


def extract_data_time(filename: str) -> Optional[datetime]:
    match = TS_RE.search(filename)
    if not match:
        return None
    try:
        return datetime.strptime(match.group(1), "%Y%m%d_%H%M%S")
    except ValueError:
        return None


def scan_source_tree(
    src_root: Path,
    start_time: Optional[datetime],
    end_time: Optional[datetime],
    verbose: bool = False,
) -> List[FileEntry]:
    entries: List[FileEntry] = []

    if not src_root.is_dir():
        raise SystemExit(f"ERROR: source root does not exist or is not a directory: {src_root}")

    for child in sorted(src_root.iterdir()):
        if not child.is_dir():
            continue
        if not DAY_DIR_RE.match(child.name):
            continue

        day_dir = child.name

        if verbose:
            print(f"Scanning day directory: {child}")

        for item in sorted(child.iterdir()):
            if not item.is_file():
                continue

            dt = extract_data_time(item.name)
            if dt is None:
                if verbose:
                    print(f"  Skipping (no valid timestamp): {item.name}")
                continue

            if start_time is not None and dt < start_time:
                continue
            if end_time is not None and dt > end_time:
                continue

            entries.append(
                FileEntry(
                    data_time=dt,
                    src_path=item,
                    day_dir=day_dir,
                    filename=item.name,
                )
            )

    entries.sort()
    return entries


def format_ltime(dt: datetime) -> str:
    return dt.strftime("%Y%m%d%H%M%S")


def run_ldatawriter(
    ldatawriter: str,
    dst_root: Path,
    entry: FileEntry,
    ldir_scope: str,
    dry_run: bool,
    verbose: bool,
) -> None:
    if ldir_scope == "root":
        dir_arg = str(dst_root)
        rpath_arg = f"{entry.day_dir}/{entry.filename}"
    elif ldir_scope == "day":
        dir_arg = str(dst_root / entry.day_dir)
        rpath_arg = entry.filename
    else:
        raise ValueError(f"Unsupported ldir_scope: {ldir_scope}")

    cmd = [
        ldatawriter,
        "-dir", dir_arg,
        "-ltime", format_ltime(entry.data_time),
        "-rpath", rpath_arg,
    ]

    if dry_run or verbose:
        print("Running:", " ".join(cmd))

    if not dry_run:
        subprocess.run(cmd, check=True)


def copy_one_file(
    entry: FileEntry,
    dst_root: Path,
    overwrite: bool,
    dry_run: bool,
    verbose: bool,
) -> Path:
    dst_day_dir = dst_root / entry.day_dir
    dst_path = dst_day_dir / entry.filename

    if dry_run or verbose:
        print(f"Copy: {entry.src_path} -> {dst_path}")

    if dry_run:
        return dst_path

    dst_day_dir.mkdir(parents=True, exist_ok=True)

    if dst_path.exists() and not overwrite:
        raise FileExistsError(
            f"Destination file already exists (use --overwrite to replace): {dst_path}"
        )

    shutil.copy2(entry.src_path, dst_path)
    return dst_path


def main() -> int:
    args = parse_args()

    src_root = Path(args.src_root).expanduser().resolve()
    dst_root = Path(args.dst_root).expanduser().resolve()

    start_time = parse_filter_time(args.start_time, "--start-time")
    end_time = parse_filter_time(args.end_time, "--end-time")

    if start_time and end_time and start_time > end_time:
        raise SystemExit("ERROR: --start-time is later than --end-time")

    entries = scan_source_tree(
        src_root=src_root,
        start_time=start_time,
        end_time=end_time,
        verbose=args.verbose,
    )

    if not entries:
        print("No matching files found.")
        return 0

    print(f"Found {len(entries)} files to replay.")

    for i, entry in enumerate(entries, start=1):
        print(
            f"[{i}/{len(entries)}] "
            f"{entry.data_time.strftime('%Y-%m-%d %H:%M:%S')}  "
            f"{entry.day_dir}/{entry.filename}"
        )

        try:
            copy_one_file(
                entry=entry,
                dst_root=dst_root,
                overwrite=args.overwrite,
                dry_run=args.dry_run,
                verbose=args.verbose,
            )

            run_ldatawriter(
                ldatawriter=args.ldatawriter,
                dst_root=dst_root,
                entry=entry,
                ldir_scope=args.ldir_scope,
                dry_run=args.dry_run,
                verbose=args.verbose,
            )

        except subprocess.CalledProcessError as exc:
            print(f"ERROR: LdataWriter failed with exit code {exc.returncode}", file=sys.stderr)
            return exc.returncode
        except Exception as exc:
            print(f"ERROR: {exc}", file=sys.stderr)
            return 1

        if args.sleep_secs > 0 and i < len(entries):
            if args.verbose:
                print(f"Sleeping {args.sleep_secs:.3f} seconds")
            if not args.dry_run:
                time.sleep(args.sleep_secs)

    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

    
