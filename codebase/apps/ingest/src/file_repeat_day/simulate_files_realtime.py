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

Features:
  - Reads files in time order based on YYYYMMDD_HHMMSS found in filenames
  - Copies files to destination, preserving YYYYMMDD day structure
  - Calls LdataWriter after each copied file
  - Optional sleep interval between files
  - Optional time filtering
  - Optional overwrite
  - Optional dry-run
  - Optional renaming of output file timestamp to current time
  - Optional infinite looping over the same file set

Example:

  python simulate_radar_archive.py \
      --src-root /data/archive \
      --dst-root /data/sim/input \
      --sleep-secs 2.0 \
      --ldir-scope root

Rename files to current time as they are replayed:

  python simulate_radar_archive.py \
      --src-root /data/archive \
      --dst-root /data/sim/input \
      --sleep-secs 2.0 \
      --rename-to-now \
      --ldir-scope root

Loop forever:

  python simulate_radar_archive.py \
      --src-root /data/archive \
      --dst-root /data/sim/input \
      --sleep-secs 2.0 \
      --rename-to-now \
      --loop-forever
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
from datetime import datetime, timezone
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
    parser.add_argument(
        "--rename-to-now",
        action="store_true",
        help=(
            "Rename the output file so that the YYYYMMDD_HHMMSS portion of the "
            "filename is replaced with the current UTC time at write time."
        ),
    )
    parser.add_argument(
        "--loop-forever",
        action="store_true",
        help="Continuously replay the same input file list forever.",
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


def current_utc_times() -> tuple[str, str]:
    """
    Returns:
      filename_ts: YYYYMMDD_HHMMSS
      ldata_ts:    YYYYMMDDHHMMSS
    """
    now = datetime.now(timezone.utc)
    return now.strftime("%Y%m%d_%H%M%S"), now.strftime("%Y%m%d%H%M%S")


def make_output_name(original_name: str, rename_to_now: bool) -> tuple[str, str]:
    """
    Returns:
      output_filename
      ltime_string_for_LdataWriter (YYYYMMDDHHMMSS)

    If rename_to_now is False:
      - preserve original filename
      - ltime is based on filename timestamp already present in original_name

    If rename_to_now is True:
      - replace timestamp in filename with current UTC timestamp
      - ltime is set to the same current UTC time
    """
    if not rename_to_now:
        dt = extract_data_time(original_name)
        if dt is None:
            raise ValueError(f"Cannot extract timestamp from filename: {original_name}")
        return original_name, format_ltime(dt)

    now_fname_ts, now_ltime = current_utc_times()

    if TS_RE.search(original_name):
        new_name = TS_RE.sub(now_fname_ts, original_name, count=1)
    else:
        new_name = f"{now_fname_ts}_{original_name}"

    return new_name, now_ltime


def compute_dest_paths(
    dst_root: Path,
    day_dir: str,
    output_filename: str,
) -> tuple[Path, Path]:
    dst_day_dir = dst_root / day_dir
    dst_path = dst_day_dir / output_filename
    return dst_day_dir, dst_path


def run_ldatawriter(
    ldatawriter: str,
    dst_root: Path,
    day_dir: str,
    output_filename: str,
    ltime_str: str,
    ldir_scope: str,
    dry_run: bool,
    verbose: bool,
) -> None:
    if ldir_scope == "root":
        dir_arg = str(dst_root)
        rpath_arg = f"{day_dir}/{output_filename}"
    elif ldir_scope == "day":
        dir_arg = str(dst_root / day_dir)
        rpath_arg = output_filename
    else:
        raise ValueError(f"Unsupported ldir_scope: {ldir_scope}")

    cmd = [
        ldatawriter,
        "-dir", dir_arg,
        "-ltime", ltime_str,
        "-rpath", rpath_arg,
    ]

    if dry_run or verbose:
        print("Running:", " ".join(cmd))

    if not dry_run:
        subprocess.run(cmd, check=True)


def copy_one_file(
    src_path: Path,
    dst_day_dir: Path,
    dst_path: Path,
    overwrite: bool,
    dry_run: bool,
    verbose: bool,
) -> None:
    if dry_run or verbose:
        print(f"Copy: {src_path} -> {dst_path}")

    if dry_run:
        return

    dst_day_dir.mkdir(parents=True, exist_ok=True)

    if dst_path.exists():
        if overwrite:
            pass
        else:
            raise FileExistsError(
                f"Destination file already exists (use --overwrite to replace): {dst_path}"
            )

    shutil.copy2(src_path, dst_path)


def replay_once(
    entries: List[FileEntry],
    args: argparse.Namespace,
    pass_num: int,
) -> int:
    dst_root = Path(args.dst_root).expanduser().resolve()

    if args.loop_forever:
        print(f"Starting replay pass {pass_num}")

    total = len(entries)

    for i, entry in enumerate(entries, start=1):
        try:
            output_filename, ltime_str = make_output_name(
                entry.filename,
                rename_to_now=args.rename_to_now,
            )

            dst_day_dir, dst_path = compute_dest_paths(
                dst_root=dst_root,
                day_dir=entry.day_dir,
                output_filename=output_filename,
            )

            print(
                f"[pass {pass_num} file {i}/{total}] "
                f"src_time={entry.data_time.strftime('%Y-%m-%d %H:%M:%S')} "
                f"day={entry.day_dir} out={output_filename}"
            )

            copy_one_file(
                src_path=entry.src_path,
                dst_day_dir=dst_day_dir,
                dst_path=dst_path,
                overwrite=args.overwrite,
                dry_run=args.dry_run,
                verbose=args.verbose,
            )

            run_ldatawriter(
                ldatawriter=args.ldatawriter,
                dst_root=dst_root,
                day_dir=entry.day_dir,
                output_filename=output_filename,
                ltime_str=ltime_str,
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

        if args.sleep_secs > 0 and (i < total or args.loop_forever):
            if args.verbose:
                print(f"Sleeping {args.sleep_secs:.3f} seconds")
            if not args.dry_run:
                time.sleep(args.sleep_secs)

    return 0


def main() -> int:
    args = parse_args()

    src_root = Path(args.src_root).expanduser().resolve()

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

    pass_num = 1
    while True:
        ret = replay_once(entries, args, pass_num)
        if ret != 0:
            return ret

        if not args.loop_forever:
            break

        pass_num += 1

    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

