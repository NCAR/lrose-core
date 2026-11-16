#!/usr/bin/env python3
# Compare a proc_list with one procmap snapshot and retain the legacy text
# output consumed by monitoring scripts such as write_host_stats.
"""Report missing or late processes from a proc_list file."""

import argparse
import sys

try:
    from procmap_control import (
        ProcmapControlError,
        read_proc_list,
        read_procmap,
        require_compatible_runtime,
    )
except ImportError as error:
    raise SystemExit(
        "Cannot import procmap_control.py; install it beside ProcmapListCheck.py"
    ) from error


EXPECTED_API_VERSION = 1


# ---------------------------------------------------------------------------
# Command-line interface
# ---------------------------------------------------------------------------

def parse_args(argv):
    parser = argparse.ArgumentParser(
        add_help=False,
        description="Report missing or late processes from a proc_list file.",
    )
    parser.add_argument("-h", "-help", "--help", action="store_true", help="show this help")
    parser.add_argument("-debug", "--debug", action="store_true", help="print diagnostics")
    parser.add_argument("-verbose", "--verbose", action="store_true", help="print verbose diagnostics")
    parser.add_argument("-ok", "--ok", action="store_true", help="also list healthy processes")
    parser.add_argument("-silent", "--silent", action="store_true", help="print only summary lines")
    parser.add_argument("-percent", "--percent", action="store_true", help="print percentage healthy")
    parser.add_argument("-proc_list", "--proc_list", help="process-list file (required)")
    parser.add_argument("-host", "--host", default="localhost", help="procmap host")
    args = parser.parse_args(argv)
    if args.help:
        parser.print_help()
        raise SystemExit(1)
    if not args.proc_list:
        parser.error("-proc_list is required")
    return args


# ---------------------------------------------------------------------------
# Status comparison and reporting
# ---------------------------------------------------------------------------

def main(argv=None):
    require_compatible_runtime(EXPECTED_API_VERSION)
    args = parse_args(argv)
    try:
        expected = read_proc_list(args.proc_list)
        snapshot = read_procmap(args.host)
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        return 1

    debug = args.debug or args.verbose
    if debug:
        print(f"Expected number of processes: {len(expected)}", file=sys.stderr)
        print(f"Number of running processes: {len(snapshot.entries)}", file=sys.stderr)

    healthy = []
    problems = []
    for item in expected:
        # The Perl check utility historically ignores the configured process
        # host here; -host selects the procmap server for the whole request.
        registration = next(
            (
                current
                for current in snapshot.entries
                if current.name == item.name and current.instance == item.instance
            ),
            None,
        )
        if registration is None:
            problems.append((item, "missing"))
        elif registration.is_late:
            problems.append((item, "late"))
        else:
            healthy.append(item)

    print(f"  {len(problems)} process(es) down")
    # Preserve stdout wording because write_host_stats consumes this output.
    if args.ok:
        print(f"  {len(healthy)} process(es) OK")
    if not args.silent:
        if args.ok:
            for item in healthy:
                print(f"    {item.name} {item.instance} OK")
        for item, problem in problems:
            print(f"    {item.name} {item.instance} {problem}")
    if args.percent:
        percent = 100.0 if not expected else 100.0 * len(healthy) / len(expected)
        print(f"  Percentage up: {percent:.2f}%")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        raise SystemExit(1)
