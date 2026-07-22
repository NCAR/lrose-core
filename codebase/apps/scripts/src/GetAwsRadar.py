#!/usr/bin/env python3

#=====================================================================
#
# Download NEXRAD Level-II radar files from the public AWS archive.
#
#=====================================================================

import argparse
import datetime
import os
import re
import shutil
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
from datetime import timedelta

BUCKET_NAME = "unidata-nexrad-level2"


class HelpFormatter(
    argparse.ArgumentDefaultsHelpFormatter,
    argparse.RawDescriptionHelpFormatter,
):
    """Show defaults while preserving line breaks in descriptions and examples."""

BUCKET_URL = f"https://{BUCKET_NAME}.s3.amazonaws.com"

# Examples:
#   KFTG20260720_123456_V06
#   KFTG20260720_123456_V06.gz
# MDM metadata files and other non-volume objects do not match.
VOLUME_RE = re.compile(
    r"^(?P<site>[A-Z0-9]{4})"
    r"(?P<date>\d{8})_"
    r"(?P<time>\d{6})_"
    r"V\d{2}(?:\.gz)?$"
)


def main():
    global options, startTime, endTime, archiveMode, fileCount, thisScriptName
    global tmpDir
    
    fileCount = 0
    thisScriptName = os.path.basename(__file__)
    parseArgs()

    print("=============================================", file=sys.stderr)
    print(f"BEGIN: {thisScriptName} at {datetime.datetime.now()}", file=sys.stderr)
    print("=============================================", file=sys.stderr)

    os.makedirs(options.outputDir, exist_ok=True)
    tmpDir = os.path.join(options.outputDir, "tmp")
    os.makedirs(tmpDir, exist_ok=True)

    if options.realtimeMode:
        lookback = timedelta(seconds=options.lookbackSecs)
        while True:
            fileCount = 0
            # NEXRAD object times and S3 keys are UTC.
            endTime = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
            startTime = endTime - lookback
            manageRetrieval(startTime, endTime)
            time.sleep(options.sleepSecs)
    else:
        manageRetrieval(startTime, endTime)

    print("==============================================", file=sys.stderr)
    print(f"END: {thisScriptName} at {datetime.datetime.now()}", file=sys.stderr)
    print("==============================================", file=sys.stderr)


def manageRetrieval(start_time, end_time):
    if start_time > end_time:
        raise ValueError("start time is later than end time")

    if options.debug:
        print(f"Retrieving for times: {start_time} to {end_time} UTC", file=sys.stderr)

    start_day = start_time.date()
    end_day = end_time.date()
    this_day = start_day

    while this_day <= end_day:
        day_start = datetime.datetime.combine(this_day, datetime.time.min)
        day_end = datetime.datetime.combine(this_day, datetime.time.max)
        period_start = max(start_time, day_start)
        period_end = min(end_time, day_end)
        getForInterval(options.radarName, this_day, period_start, period_end)
        this_day += timedelta(days=1)

    print(f"---->> Num files downloaded: {fileCount}", file=sys.stderr)


def list_s3_keys(prefix):
    """Yield all keys for prefix using S3 ListObjectsV2 pagination."""
    continuation_token = None

    while True:
        params = {
            "list-type": "2",
            "prefix": prefix,
            "max-keys": "1000",
        }
        if continuation_token:
            params["continuation-token"] = continuation_token

        list_url = f"{BUCKET_URL}/?{urllib.parse.urlencode(params)}"
        if options.debug:
            print(f"list URL: {list_url}", file=sys.stderr)

        request = urllib.request.Request(
            list_url,
            headers={"User-Agent": f"{thisScriptName}/2.0"},
        )
        try:
            with urllib.request.urlopen(request, timeout=options.timeoutSecs) as response:
                root = ET.parse(response).getroot()
        except (urllib.error.URLError, TimeoutError, ET.ParseError) as exc:
            raise RuntimeError(f"cannot list S3 prefix {prefix}: {exc}") from exc

        # S3 XML uses a default namespace. Stripping it keeps the code simple.
        for elem in root.iter():
            if "}" in elem.tag:
                elem.tag = elem.tag.split("}", 1)[1]

        for contents in root.findall("Contents"):
            key = contents.findtext("Key")
            if key:
                yield key

        is_truncated = root.findtext("IsTruncated", "false").lower() == "true"
        if not is_truncated:
            break

        continuation_token = root.findtext("NextContinuationToken")
        if not continuation_token:
            raise RuntimeError("S3 response was truncated but had no continuation token")


def getForInterval(radarName, thisDay, start_time, end_time):
    local_files = set(getLocalFileList(thisDay, radarName))
    prefix = f"{thisDay:%Y/%m/%d}/{radarName}/"

    keys_found = 0
    for key in list_s3_keys(prefix):
        file_name = key.rsplit("/", 1)[-1]
        match = VOLUME_RE.match(file_name)
        if not match or match.group("site") != radarName:
            if options.verbose:
                print(f"skipping non-volume entry: {key}", file=sys.stderr)
            continue

        file_time = datetime.datetime.strptime(
            match.group("date") + match.group("time"), "%Y%m%d%H%M%S"
        )
        keys_found += 1

        if options.verbose:
            print(f"checking file: {key}, time: {file_time}", file=sys.stderr)

        if start_time <= file_time <= end_time:
            if options.force or file_name not in local_files:
                doDownload(radarName, file_time, key, file_name)
            elif options.debug:
                print(f"file previously downloaded: {file_name}", file=sys.stderr)

    if options.debug:
        print(f"valid volume keys found for {prefix}: {keys_found}", file=sys.stderr)


def doDownload(radarName, fileTime, key, fileName):
    global fileCount

    url = f"{BUCKET_URL}/{urllib.parse.quote(key, safe='/')}"
    date_str = fileTime.strftime("%Y%m%d")
    radar_dir = os.path.join(options.outputDir, radarName)
    out_day_dir = os.path.join(radar_dir, date_str)
    final_path = os.path.join(out_day_dir, fileName)
    tmp_path = os.path.join(tmpDir, fileName + ".part")

    if options.debug or options.dryRun:
        print(f"Downloading: {url} -> {final_path}", file=sys.stderr)
    if options.dryRun:
        return

    os.makedirs(out_day_dir, exist_ok=True)
    os.makedirs(tmpDir, exist_ok=True)

    request = urllib.request.Request(
        url,
        headers={"User-Agent": f"{thisScriptName}/2.0"},
    )

    try:
        with urllib.request.urlopen(request, timeout=options.timeoutSecs) as response, \
                open(tmp_path, "wb") as out:
            shutil.copyfileobj(response, out, length=1024 * 1024)
        os.replace(tmp_path, final_path)
    except (urllib.error.URLError, OSError, TimeoutError) as exc:
        try:
            os.remove(tmp_path)
        except FileNotFoundError:
            pass
        print(f"ERROR downloading {url}: {exc}", file=sys.stderr)
        return

    fileCount += 1

    time_str = fileTime.strftime("%Y%m%d%H%M%S")
    rel_path = os.path.join(date_str, fileName)
    runCommand([
        "LdataWriter",
        "-dir", radar_dir,
        "-rpath", rel_path,
        "-ltime", time_str,
        "-writer", thisScriptName,
        "-dtype", "level2",
    ])


def getLocalFileList(date, radarName):
    date_str = date.strftime("%Y%m%d")
    day_dir = os.path.join(options.outputDir, radarName, date_str)
    os.makedirs(day_dir, exist_ok=True)
    return os.listdir(day_dir)


def parse_utc_time(value):
    try:
        return datetime.datetime.strptime(value, "%Y %m %d %H %M %S")
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            'time must use format "YYYY MM DD HH MM SS" (UTC)'
        ) from exc


def parseArgs():
    global options, startTime, endTime, archiveMode

    parser = argparse.ArgumentParser(
        description="Download NEXRAD Level-II radar files from AWS.",
        formatter_class=HelpFormatter,
        epilog=(
            'Archive example:\n'
            '  %(prog)s --radarName KFTG '
            '--start "2026 07 20 12 00 00" '
            '--end "2026 07 20 13 00 00"\n\n'
            'Realtime example:\n'
            '  %(prog)s --radarName KFTG --realtime'
        ),
    )

    parser.add_argument(
        "--debug",
        dest="debug",
        default=False,
        action="store_true",
        help="Set debugging on",
    )
    parser.add_argument(
        "--verbose",
        dest="verbose",
        default=False,
        action="store_true",
        help="Set verbose debugging on",
    )
    parser.add_argument(
        "--radarName",
        dest="radarName",
        default="KFTG",
        metavar="RADAR",
        help="4-character name for radar",
    )
    parser.add_argument(
        "--outputDir",
        dest="outputDir",
        default="/tmp/aws",
        metavar="DIR",
        help="Path of output dir to which the files are written",
    )
    parser.add_argument(
        "--force",
        dest="force",
        default=False,
        action="store_true",
        help="Force transfer even if file previously downloaded",
    )
    parser.add_argument(
        "--dryRun",
        dest="dryRun",
        default=False,
        action="store_true",
        help="Dry run: do not download data, list what would be downloaded",
    )
    parser.add_argument(
        "--realtime",
        dest="realtimeMode",
        default=False,
        action="store_true",
        help="Realtime mode - check every sleepSecs, look back lookbackSecs",
    )
    parser.add_argument(
        "--lookbackSecs",
        dest="lookbackSecs",
        type=int,
        default=1800,
        metavar="SECS",
        help="Lookback secs in realtime mode",
    )
    parser.add_argument(
        "--sleepSecs",
        dest="sleepSecs",
        type=int,
        default=10,
        metavar="SECS",
        help="Sleep secs in realtime mode",
    )
    parser.add_argument(
        "--timeoutSecs",
        dest="timeoutSecs",
        type=int,
        default=60,
        metavar="SECS",
        help="Timeout secs for AWS list and download requests",
    )
    parser.add_argument(
        "--start",
        dest="startTime",
        type=parse_utc_time,
        metavar='"YYYY MM DD HH MM SS"',
        help=(
            'Start time for retrieval - archival mode. '
            'Format is "yyyy mm dd hh mm ss", including double quotes. '
            'Time is interpreted as UTC.'
        ),
    )
    parser.add_argument(
        "--end",
        dest="endTime",
        type=parse_utc_time,
        metavar='"YYYY MM DD HH MM SS"',
        help=(
            'End time for retrieval - archival mode. '
            'Format is "yyyy mm dd hh mm ss", including double quotes. '
            'Time is interpreted as UTC.'
        ),
    )

    options = parser.parse_args()
    options.radarName = options.radarName.upper()

    if not re.fullmatch(r"[A-Z0-9]{4}", options.radarName):
        parser.error("--radarName must be exactly four letters/digits")

    if options.lookbackSecs < 0:
        parser.error("--lookbackSecs must be nonnegative")
    if options.sleepSecs <= 0:
        parser.error("--sleepSecs must be greater than zero")
    if options.timeoutSecs <= 0:
        parser.error("--timeoutSecs must be greater than zero")

    if options.verbose:
        options.debug = True

    if options.realtimeMode:
        if options.startTime is not None or options.endTime is not None:
            parser.error("do not specify --start or --end with --realtime")
        startTime = endTime = None
        archiveMode = False
    else:
        if options.startTime is None or options.endTime is None:
            parser.error("archive mode requires both --start and --end")
        if options.startTime > options.endTime:
            parser.error("--start must not be later than --end")
        startTime = options.startTime
        endTime = options.endTime
        archiveMode = True

    if options.debug:
        print("Options:", file=sys.stderr)
        print("  debug? ", options.debug, file=sys.stderr)
        print("  force? ", options.force, file=sys.stderr)
        print("  dryRun? ", options.dryRun, file=sys.stderr)
        print("  radarName: ", options.radarName, file=sys.stderr)
        print("  outputDir: ", options.outputDir, file=sys.stderr)
        print("  archiveMode? ", archiveMode, file=sys.stderr)
        print("  realtimeMode? ", options.realtimeMode, file=sys.stderr)
        print("  lookbackSecs: ", options.lookbackSecs, file=sys.stderr)
        print("  sleepSecs: ", options.sleepSecs, file=sys.stderr)
        print("  timeoutSecs: ", options.timeoutSecs, file=sys.stderr)
        print("  startTime: ", startTime, file=sys.stderr)
        print("  endTime: ", endTime, file=sys.stderr)


def runCommand(args):
    if options.debug:
        print("running command:", " ".join(args), file=sys.stderr)
    try:
        result = subprocess.run(args, check=False)
        if result.returncode != 0:
            print(
                f"WARNING: command returned status {result.returncode}: {' '.join(args)}",
                file=sys.stderr,
            )
    except OSError as exc:
        print(f"ERROR executing {' '.join(args)}: {exc}", file=sys.stderr)


if __name__ == "__main__":
    main()
