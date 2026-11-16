#!/usr/bin/env python3
# Start proc_list entries in file order, including the historical convention
# that a shared no-argument script runs only once per invocation.
"""Start the processes named in a proc_list file."""

import argparse
import sys
import time

try:
    from procmap_control import (
        CommandRunner,
        ProcmapControlError,
        read_proc_list,
        require_compatible_runtime,
        run_start,
        start_script_has_args,
    )
except ImportError as error:
    raise SystemExit(
        "Cannot import procmap_control.py; install it beside ProcmapListStart.py"
    ) from error


EXPECTED_API_VERSION = 1


# ---------------------------------------------------------------------------
# Command-line interface
# ---------------------------------------------------------------------------

def parse_args(argv):
    parser = argparse.ArgumentParser(
        add_help=False, description="Start processes from a proc_list file."
    )
    parser.add_argument("-h", "-help", "--help", action="store_true", help="show this help")
    parser.add_argument("-proc_list", "--proc_list", help="process-list file (required)")
    parser.add_argument("-sleep", "--sleep", type=float, default=0.0, help="seconds between starts")
    parser.add_argument("-timeout", "--timeout", type=float, default=10.0, help="command timeout seconds")
    parser.add_argument("-debug", "--debug", action="store_true", help="print command diagnostics")
    args = parser.parse_args(argv)
    if args.help or not args.proc_list:
        parser.print_help()
        raise SystemExit(0)
    if args.sleep < 0 or args.timeout <= 0:
        parser.error("sleep must be nonnegative and timeout must be positive")
    return args


# ---------------------------------------------------------------------------
# Ordered process startup
# ---------------------------------------------------------------------------

def main(argv=None):
    require_compatible_runtime(EXPECTED_API_VERSION)
    args = parse_args(argv)
    try:
        entries = read_proc_list(args.proc_list)
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        return 1
    runner = CommandRunner(debug=args.debug)
    used_scripts = set()
    failed = False
    for entry in entries:
        # A shared no-argument script may start several registered processes.
        # Parenthesized scripts are per-entry and therefore always run.
        if not start_script_has_args(entry.start_script):
            if entry.start_script in used_scripts:
                continue
            used_scripts.add(entry.start_script)
        result = run_start(entry, runner, args.timeout)
        if not result.ok:
            # Unlike the Perl implementation, propagate command failure to
            # automation instead of silently returning success.
            failed = True
            reason = "timed out" if result.timed_out else f"exit {result.returncode}"
            print(f"ERROR - start command {reason}: {result.command}", file=sys.stderr)
            if result.output:
                print(result.output, end="", file=sys.stderr)
        if args.sleep:
            time.sleep(args.sleep)
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        raise SystemExit(1)
