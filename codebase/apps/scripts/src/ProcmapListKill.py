#!/usr/bin/env python3
# Stop proc_list entries in two phases: signal matching procmap PIDs, then run
# each configured kill script after a grace interval.
"""Stop the processes named in a proc_list file."""

import argparse
import os
import sys
import time

try:
    from procmap_control import (
        CommandRunner,
        ProcmapControlError,
        hosts_equivalent,
        read_proc_list,
        read_procmap,
        require_compatible_runtime,
        run_kill_script,
    )
except ImportError as error:
    raise SystemExit(
        "Cannot import procmap_control.py; install it beside ProcmapListKill.py"
    ) from error


EXPECTED_API_VERSION = 1


# ---------------------------------------------------------------------------
# Command-line interface
# ---------------------------------------------------------------------------

def parse_args(argv):
    parser = argparse.ArgumentParser(
        add_help=False, description="Stop processes from a proc_list file."
    )
    parser.add_argument("-h", "-help", "--help", action="store_true", help="show this help")
    parser.add_argument("-fast", "--fast", action="store_true", help="use local killall by process name")
    parser.add_argument("-proc_list", "--proc_list", help="process-list file (required)")
    parser.add_argument("-procmap_host", "--procmap_host", default=None, help="procmap host")
    parser.add_argument("-sleep", "--sleep", type=float, default=15.0, help="seconds before kill scripts")
    parser.add_argument("-timeout", "--timeout", type=float, default=10.0, help="command timeout seconds")
    parser.add_argument("-debug", "--debug", action="store_true", help="print command diagnostics")
    args = parser.parse_args(argv)
    if args.help or not args.proc_list:
        parser.print_help()
        raise SystemExit(0)
    if args.sleep < 0 or args.timeout <= 0:
        parser.error("sleep must be nonnegative and timeout must be positive")
    if args.procmap_host is None:
        # Retain the operational convention that PROCMAP_HOST supplies the
        # default server when no command-line host is provided.
        args.procmap_host = os.environ.get("PROCMAP_HOST", "local")
    return args


def report_failure(result):
    # Keep command output with its failure so remote authentication and timeout
    # problems can be diagnosed from unattended shutdown logs.
    reason = "timed out" if result.timed_out else f"exit {result.returncode}"
    print(f"ERROR - command {reason}: {result.command}", file=sys.stderr)
    if result.output:
        print(result.output, end="", file=sys.stderr)


def main(argv=None):
    require_compatible_runtime(EXPECTED_API_VERSION)
    args = parse_args(argv)
    try:
        entries = read_proc_list(args.proc_list)
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        return 1
    runner = CommandRunner(debug=args.debug)
    failed = False

    if args.fast:
        # Fast mode intentionally mirrors the broad legacy behavior: kill by
        # executable name on this host without consulting procmap instances.
        for entry in entries:
            result = runner.run_local_argv(["killall", entry.name], args.timeout)
            if not result.ok:
                failed = True
                report_failure(result)
        return 1 if failed else 0

    try:
        snapshot = read_procmap(args.procmap_host, args.timeout)
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        return 1

    for registration in snapshot.entries:
        # Phase one signals only registrations represented in the proc_list.
        # Sending the same graceful signal sequence preserves Perl behavior.
        entry = next(
            (
                current
                for current in entries
                if current.name == registration.name
                and current.instance == registration.instance
                and hosts_equivalent(current.host, registration.host)
            ),
            None,
        )
        if entry is None:
            continue
        print(f"Killing {registration.host} {registration.pid}", file=sys.stderr)
        for signal_name in ("TERM", "QUIT", "INT"):
            command = f"kill -{signal_name} {registration.pid}"
            result = runner.run(registration.host, command, args.timeout)
            if not result.ok:
                failed = True
                report_failure(result)
    if args.sleep:
        # Allow applications time to handle signals before invoking the more
        # forceful project-specific kill scripts in phase two.
        time.sleep(args.sleep)

    for entry in entries:
        if args.debug:
            print(
                f"Killing {entry.name} {entry.instance} with {entry.kill_script}"
            )
        result = run_kill_script(entry, runner, args.timeout)
        if not result.ok:
            failed = True
            report_failure(result)
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        raise SystemExit(1)
