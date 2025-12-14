#!/usr/bin/env python3
"""
Download GFS GRIB2 files from NOAA's public AWS Open Data bucket.

Primary bucket (operational GFS): s3://noaa-gfs-bdp-pds/  (region us-east-1)

Typical object key pattern (atmospheric files):
  gfs.YYYYMMDD/HH/atmos/gfs.tHHz.pgrb2.0p25.fFFF

Examples:
  gfs.20210711/00/atmos/gfs.t00z.pgrb2.0p25.f000

Notes:
- Uses unsigned (anonymous) S3 requests; no AWS credentials required.
- By default downloads 0.25-degree "pgrb2" files.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import datetime as dt
import os
import sys
from typing import Iterable, List, Optional, Sequence, Tuple

import boto3
from botocore import UNSIGNED
from botocore.config import Config


BUCKET = "noaa-gfs-bdp-pds"
REGION = "us-east-1"


@dataclasses.dataclass(frozen=True)
class Run:
    yyyymmdd: str  # YYYYMMDD
    cycle: str     # "00","06","12","18"

    @property
    def prefix(self) -> str:
        # e.g. gfs.20251213/12/
        return f"gfs.{self.yyyymmdd}/{self.cycle}/"

    def key_for(self, subdir: str, filename: str) -> str:
        return f"{self.prefix}{subdir.rstrip('/')}/{filename}"


def make_s3_client():
    # Anonymous / unsigned requests (like aws s3 ... --no-sign-request)
    return boto3.client("s3", region_name=REGION, config=Config(signature_version=UNSIGNED))


def s3_prefix_exists(s3, bucket: str, prefix: str) -> bool:
    # Cheap existence check: list a single key under prefix.
    resp = s3.list_objects_v2(Bucket=bucket, Prefix=prefix, MaxKeys=1)
    return "Contents" in resp and len(resp["Contents"]) > 0


def find_latest_run(
    s3,
    search_days_back: int,
    cycles: Sequence[str],
    require_subdir: str = "atmos/",
) -> Run:
    """
    Search backwards from 'today' UTC for the latest run that has content.
    We check for existence of prefix: gfs.YYYYMMDD/CC/atmos/
    """
    today = dt.datetime.utcnow().date()
    for day_back in range(search_days_back + 1):
        d = today - dt.timedelta(days=day_back)
        yyyymmdd = d.strftime("%Y%m%d")
        for cyc in cycles:
            prefix = f"gfs.{yyyymmdd}/{cyc}/{require_subdir}"
            if s3_prefix_exists(s3, BUCKET, prefix):
                return Run(yyyymmdd=yyyymmdd, cycle=cyc)

    raise RuntimeError(
        f"Could not find any run within last {search_days_back} days "
        f"for cycles {list(cycles)} under subdir '{require_subdir}'."
    )


def iter_runs_in_date_range(
    start_date: dt.date,
    end_date: dt.date,
    cycles: Sequence[str],
) -> Iterable[Run]:
    if end_date < start_date:
        raise ValueError("end_date must be >= start_date")
    d = start_date
    while d <= end_date:
        yyyymmdd = d.strftime("%Y%m%d")
        for cyc in cycles:
            yield Run(yyyymmdd=yyyymmdd, cycle=cyc)
        d += dt.timedelta(days=1)


def parse_fhours(fhours: str) -> List[int]:
    """
    Accept:
      - comma list: "0,3,6,12"
      - range step: "0:384:3"  (start:stop:step; inclusive of stop if divisible)
    """
    fhours = fhours.strip()
    if ":" in fhours:
        parts = fhours.split(":")
        if len(parts) != 3:
            raise ValueError("Range format must be start:stop:step, e.g. 0:384:3")
        start, stop, step = map(int, parts)
        if step <= 0:
            raise ValueError("step must be > 0")
        out = []
        f = start
        while f <= stop:
            out.append(f)
            f += step
        return out
    else:
        return [int(x) for x in fhours.split(",") if x.strip()]


def build_filename(cycle: str, res: str, fhr: int, variant: str) -> str:
    """
    variant:
      - "pgrb2"  -> gfs.tCCz.pgrb2.0p25.fFFF
      - "pgrb2b" -> gfs.tCCz.pgrb2b.0p25.fFFF (alternate parameter set)
    """
    return f"gfs.t{cycle}z.{variant}.{res}.f{fhr:03d}"


def download_one(s3, key: str, out_path: str, dry_run: bool = False) -> Tuple[str, bool, Optional[str]]:
    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    if dry_run:
        return (key, True, None)

    # Skip if already present (simple guard)
    if os.path.exists(out_path) and os.path.getsize(out_path) > 0:
        return (key, True, "exists")

    try:
        s3.download_file(BUCKET, key, out_path)
        return (key, True, None)
    except Exception as e:
        return (key, False, str(e))


def download_run_files(
    s3,
    run: Run,
    out_dir: str,
    subdir: str,
    res: str,
    variant: str,
    fhours: Sequence[int],
    max_workers: int,
    dry_run: bool,
    only_if_run_exists: bool = True,
) -> int:
    """
    Download a set of forecast hours for one run.
    Returns number of failed downloads.
    """
    if only_if_run_exists:
        prefix_check = f"{run.prefix}{subdir.rstrip('/')}/"
        if not s3_prefix_exists(s3, BUCKET, prefix_check):
            print(f"[skip] No data for {prefix_check}", file=sys.stderr)
            return 0

    jobs: List[Tuple[str, str]] = []
    for fhr in fhours:
        fname = build_filename(run.cycle, res, fhr, variant)
        key = run.key_for(subdir=subdir, filename=fname)
        out_path = os.path.join(out_dir, run.yyyymmdd, run.cycle, subdir, fname)
        jobs.append((key, out_path))

    failures = 0
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as ex:
        futs = [ex.submit(download_one, s3, key, out_path, dry_run) for key, out_path in jobs]
        for fut in concurrent.futures.as_completed(futs):
            key, ok, note = fut.result()
            if ok:
                if note == "exists":
                    print(f"[ok]   {key} (already downloaded)")
                else:
                    print(f"[ok]   {key}" if not dry_run else f"[dry]  {key}")
            else:
                failures += 1
                print(f"[FAIL] {key} :: {note}", file=sys.stderr)

    return failures


def main():
    p = argparse.ArgumentParser(
        description="Download GFS from NOAA's public AWS bucket (noaa-gfs-bdp-pds).",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    sub = p.add_subparsers(dest="cmd", required=True)

    # latest
    p_latest = sub.add_parser("latest", help="Find latest available run and download it.")
    p_latest.add_argument("--out", default="./gfs_downloads", help="Output directory root")
    p_latest.add_argument("--search-days-back", type=int, default=3, help="How far back to look for latest run")
    p_latest.add_argument("--cycles", default="18,12,06,00", help="Cycle search order, comma-separated")
    p_latest.add_argument("--subdir", default="atmos", help="Subdirectory under run prefix (e.g., atmos)")
    p_latest.add_argument("--res", default="0p25", help="Resolution token used in filename (e.g., 0p25, 0p50)")
    p_latest.add_argument("--variant", default="pgrb2", choices=["pgrb2", "pgrb2b"], help="Filename variant")
    p_latest.add_argument("--fhours", default="0:48:3", help="Forecast hours list (e.g. 0,3,6 or 0:384:3)")
    p_latest.add_argument("--workers", type=int, default=8, help="Parallel download workers")
    p_latest.add_argument("--dry-run", action="store_true", help="List what would be downloaded, without downloading")

    # archive
    p_arch = sub.add_parser("archive", help="Download runs across a date range.")
    p_arch.add_argument("--start", required=True, help="Start date YYYY-MM-DD")
    p_arch.add_argument("--end", required=True, help="End date YYYY-MM-DD")
    p_arch.add_argument("--out", default="./gfs_downloads", help="Output directory root")
    p_arch.add_argument("--cycles", default="00,06,12,18", help="Cycles to attempt each day, comma-separated")
    p_arch.add_argument("--subdir", default="atmos", help="Subdirectory under run prefix (e.g., atmos)")
    p_arch.add_argument("--res", default="0p25", help="Resolution token used in filename (e.g., 0p25, 0p50)")
    p_arch.add_argument("--variant", default="pgrb2", choices=["pgrb2", "pgrb2b"], help="Filename variant")
    p_arch.add_argument("--fhours", default="0:48:3", help="Forecast hours list (e.g. 0,3,6 or 0:384:3)")
    p_arch.add_argument("--workers", type=int, default=8, help="Parallel download workers")
    p_arch.add_argument("--dry-run", action="store_true", help="List what would be downloaded, without downloading")
    p_arch.add_argument("--skip-missing-runs", action="store_true",
                        help="Skip cycles that don't exist (recommended).")

    args = p.parse_args()
    s3 = make_s3_client()

    if args.cmd == "latest":
        cycles = [c.strip() for c in args.cycles.split(",") if c.strip()]
        fhours = parse_fhours(args.fhours)
        run = find_latest_run(
            s3,
            search_days_back=args.search_days_back,
            cycles=cycles,
            require_subdir=f"{args.subdir.strip('/')}/",
        )
        print(f"Latest run found: {run.yyyymmdd} {run.cycle}Z  (prefix {run.prefix}{args.subdir}/)")
        failures = download_run_files(
            s3=s3,
            run=run,
            out_dir=args.out,
            subdir=args.subdir,
            res=args.res,
            variant=args.variant,
            fhours=fhours,
            max_workers=args.workers,
            dry_run=args.dry_run,
            only_if_run_exists=True,
        )
        sys.exit(1 if failures else 0)

    elif args.cmd == "archive":
        start = dt.datetime.strptime(args.start, "%Y-%m-%d").date()
        end = dt.datetime.strptime(args.end, "%Y-%m-%d").date()
        cycles = [c.strip() for c in args.cycles.split(",") if c.strip()]
        fhours = parse_fhours(args.fhours)

        total_failures = 0
        for run in iter_runs_in_date_range(start, end, cycles):
            print(f"\n=== {run.yyyymmdd} {run.cycle}Z ===")
            total_failures += download_run_files(
                s3=s3,
                run=run,
                out_dir=args.out,
                subdir=args.subdir,
                res=args.res,
                variant=args.variant,
                fhours=fhours,
                max_workers=args.workers,
                dry_run=args.dry_run,
                only_if_run_exists=args.skip_missing_runs,
            )

        sys.exit(1 if total_failures else 0)


if __name__ == "__main__":
    main()
