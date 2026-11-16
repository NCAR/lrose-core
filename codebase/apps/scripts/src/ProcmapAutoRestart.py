#!/usr/bin/env python3
# Maintain the procmap reliability layer by recovering procmap itself and then
# restarting expected applications whose registrations are missing or late.
"""Continuously restart missing or late processes registered with procmap."""

import argparse
import fcntl
import os
import shlex
import signal
import socket
import subprocess
import sys
import time

try:
    from procmap_control import (
        CommandRunner,
        ProcmapControlError,
        find_registration,
        format_timestamp,
        read_proc_list,
        read_procmap,
        require_compatible_runtime,
        run_kill_script,
        run_start,
        start_script_has_args,
    )
except ImportError as error:
    raise SystemExit(
        "Cannot import procmap_control.py; install it beside ProcmapAutoRestart.py"
    ) from error


EXPECTED_API_VERSION = 1
STOP_REQUESTED = False


# ---------------------------------------------------------------------------
# Signal handling and bounded waits
# ---------------------------------------------------------------------------

def request_stop(_signum, _frame):
    global STOP_REQUESTED
    STOP_REQUESTED = True


def interruptible_sleep(seconds):
    # Short slices keep termination responsive without using signal-unsafe
    # cleanup inside the handler.
    deadline = time.monotonic() + max(0.0, seconds)
    while not STOP_REQUESTED:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return
        time.sleep(min(remaining, 0.5))


class LoopLog:
    # Reopen the logfile for each write so operators can rotate or remove it
    # while the long-running restarter remains active.
    def __init__(self, path, use_stdout, debug, debug_log):
        self.path = path
        self.use_stdout = use_stdout
        self.debug = debug
        self.debug_log = debug_log

    def write(self, message, always=True, debug=False):
        if debug and self.debug:
            print(message, end="", file=sys.stderr, flush=True)
        if not always and not self.debug_log:
            return
        if self.use_stdout:
            print(message, end="", flush=True)
        else:
            try:
                with open(self.path, "a", encoding="utf-8") as stream:
                    stream.write(message)
                    stream.flush()
            except OSError as error:
                raise ProcmapControlError(
                    f"cannot append auto-restart log {self.path}: {error}"
                ) from error


# ---------------------------------------------------------------------------
# Command-line interface and notifications
# ---------------------------------------------------------------------------

def parse_args(argv):
    home = os.environ.get("HOME", ".")
    parser = argparse.ArgumentParser(
        add_help=False,
        description="Restart missing or late processes registered with procmap.",
    )
    parser.add_argument("-h", "-help", "--help", action="store_true", help="show this help")
    parser.add_argument("-d", "-debug", "--debug", action="store_true", help="print diagnostics to stderr")
    parser.add_argument("-l", "-logdebug", "--logdebug", action="store_true", help="include diagnostics in the log")
    parser.add_argument("-v", "-verbosedebug", "--verbosedebug", action="store_true", help="trace external commands")
    parser.add_argument("-a", "-alsorun", "--alsorun", help="additional command to run each pass")
    parser.add_argument("-e", "-email", "--email", action="append", default=[], help="notification address; repeatable")
    parser.add_argument("-f", "-filelock", "--filelock", default=f"{home}/.ProcmapAutoRestart.py.LOCK", help="lock-file path")
    parser.add_argument("-i", "-ignoreoutlog", "--ignoreoutlog", action="store_true", help="write log messages to stdout")
    parser.add_argument("-m", "-map_host", "--map_host", default="localhost", help="procmap host")
    parser.add_argument("-n", "-nolock", "--nolock", action="store_true", help="disable singleton lock")
    parser.add_argument("-o", "-outlog", "--outlog", default=f"{home}/.ProcmapAutoRestart.py.log", help="append log path")
    parser.add_argument("-p", "-proc_list", "--proc_list", help="process-list file (required)")
    parser.add_argument("-q", "-quit", "--quit", action="store_true", help="run one check and exit")
    parser.add_argument("-r", "-restart_script", "--restart_script", default="start_procmap", help="procmap start command")
    parser.add_argument("-s", "-sleep", "--sleep", type=float, default=60.0, help="seconds between checks")
    parser.add_argument("-t", "-timezone", "--timezone", choices=("local", "utc"), default="utc", help="log timezone")
    parser.add_argument("-u", "-uptime_min", "--uptime_min", type=int, default=60, help="minimum procmap uptime")
    parser.add_argument("-timeout", "--timeout", type=float, default=10.0, help="external-command timeout")
    args = parser.parse_args(argv)
    if args.help:
        parser.print_help()
        raise SystemExit(0)
    if not args.proc_list:
        parser.error("-proc_list is required")
    if args.sleep < 0 or args.uptime_min < 0 or args.timeout <= 0:
        parser.error("sleep and uptime_min must be nonnegative; timeout must be positive")
    return args


def send_email(addresses, message):
    # Use the system MTA just as the Perl utility did.  No email dependency is
    # required when the repeatable -email option is absent.
    if not addresses:
        return
    mail = (
        f"To: {' '.join(addresses)}\n"
        f"From: auto_restart@{socket.gethostname()}\n"
        "Subject: auto_restart errors\n\n"
        f"{message}"
    )
    try:
        subprocess.run(
            ["/usr/sbin/sendmail", "-t"],
            input=mail,
            text=True,
            check=False,
            timeout=10,
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        print(f"ERROR - cannot send auto-restart email: {error}", file=sys.stderr)


def command_problem(result):
    # Convert command results into text suitable for both the logfile and the
    # aggregate notification sent after a monitoring pass.
    if result.ok:
        return ""
    reason = "timed out" if result.timed_out else f"exit {result.returncode}"
    detail = f"Command {reason}: {result.command}\n"
    if result.output:
        detail += result.output
        if not detail.endswith("\n"):
            detail += "\n"
    return detail


def restart_entry(entry, registration, kill_processes, runner, timeout, logger):
    # A stale registration is killed by PID first and then by its configured
    # script.  When procmap has no registrations, skip killing and start cleanly.
    problems = ""
    if kill_processes:
        if registration is not None and registration.pid > 0:
            result = runner.run(entry.host, f"kill -9 {registration.pid}", timeout)
            problems += command_problem(result)
        if STOP_REQUESTED:
            return problems
        interruptible_sleep(5)
        result = run_kill_script(entry, runner, timeout)
        problems += command_problem(result)
        if STOP_REQUESTED:
            return problems
        interruptible_sleep(5)
    result = run_start(entry, runner, timeout)
    problems += command_problem(result)
    if problems:
        logger.write(problems, always=True, debug=True)
    return problems


def acquire_lock(path):
    # Keep the descriptor open for the process lifetime; closing it would
    # release flock and permit a second restarter to enter.
    try:
        stream = open(path, "w", encoding="utf-8")
    except OSError as error:
        raise ProcmapControlError(f"cannot open lock file {path}: {error}") from error
    try:
        fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        stream.close()
        raise ProcmapControlError(f"cannot lock {path}; another instance may be running")
    return stream


def main(argv=None):
    require_compatible_runtime(EXPECTED_API_VERSION)
    args = parse_args(argv)
    lock_stream = None if args.nolock else acquire_lock(args.filelock)
    logger = LoopLog(args.outlog, args.ignoreoutlog, args.debug, args.logdebug)
    runner = CommandRunner(debug=args.verbosedebug)
    for sig in (signal.SIGINT, signal.SIGQUIT, signal.SIGTERM):
        signal.signal(sig, request_stop)

    local_time = args.timezone == "local"
    stamp = format_timestamp(local_time)
    logger.write("\n---------------------------------------------------\n")
    logger.write(f"{stamp}: STARTING ProcmapAutoRestart.py\n")
    logger.write(f"{stamp}: Checking procmap on {args.map_host}\n")
    send_email(args.email, "Starting auto_restart\n")

    # Re-read proc_list and procmap every pass so configuration changes and
    # procmap recovery are observed without restarting this controller.
    while not STOP_REQUESTED:
        email_message = f"Time for messages: {format_timestamp(local_time)}\n"
        send_this_loop = False
        stamp = format_timestamp(local_time)
        logger.write(f"\n{stamp}: Checking processes\n", always=False, debug=True)

        if args.alsorun:
            # The auxiliary hook is deliberately isolated from the monitoring
            # result; its failure is logged but does not suppress restarts.
            result = runner.run("localhost", args.alsorun, 5)
            problem = command_problem(result)
            if problem:
                logger.write(problem, always=True, debug=True)

        try:
            entries = read_proc_list(args.proc_list)
        except ProcmapControlError as error:
            text = f"{stamp}: Error reading process list: {error}\n"
            logger.write(text, always=True, debug=True)
            email_message += text
            send_email(args.email, email_message)
            if args.quit or STOP_REQUESTED:
                break
            interruptible_sleep(args.sleep)
            continue

        try:
            snapshot = read_procmap(args.map_host, args.timeout)
        except ProcmapControlError as error:
            text = f"{stamp}: Error getting procmap state: {error}\n"
            logger.write(text, always=True, debug=True)
            email_message += text
            logger.write("   Killing procmap with snuff command\n")
            # Procmap itself cannot report through procmap when it fails.  The
            # external snuff/start pair is therefore the bootstrap recovery path.
            runner.run(args.map_host, "snuff procmap", args.timeout)
            logger.write("   Restarting procmap with start_procmap cmd\n")
            command = (
                f"{args.restart_script} > {shlex.quote(args.outlog + '.1')} "
                "2>&1 < /dev/null &"
            )
            runner.run(args.map_host, command, args.timeout)
            if not args.quit:
                interruptible_sleep(max(0.0, args.uptime_min - args.sleep))
            send_email(args.email, email_message)
            if args.quit or STOP_REQUESTED:
                break
            interruptible_sleep(args.sleep)
            continue

        kill_processes = bool(snapshot.entries)
        if not kill_processes:
            text = f"{stamp}: No procs registered with {snapshot.host}\n"
            logger.write(text, always=True, debug=True)
            email_message += text
            send_this_loop = True

        if snapshot.uptime < args.uptime_min:
            # Give applications started with procmap time to register before
            # declaring the entire expected set missing.
            text = (
                f"{stamp}: Procmap just restarted {snapshot.uptime} secs ago -- skipping\n"
            )
            logger.write(text, always=False, debug=True)
            email_message += text
            send_this_loop = True
        else:
            used_scripts = set()
            # Preserve the legacy two-phase policy: restart all registered but
            # late processes first, then handle missing registrations.  This
            # matters when one no-argument start script owns several entries.
            for entry in entries:
                registration = find_registration(entry, snapshot.entries)
                if registration is None or not registration.is_late:
                    continue
                text = (
                    f"{stamp}: Restarting process {entry.name} {entry.instance} "
                    f"on {registration.host}\n"
                )
                logger.write(text, always=True, debug=True)
                email_message += text
                send_this_loop = True
                email_message += restart_entry(
                    entry,
                    registration,
                    kill_processes,
                    runner,
                    args.timeout,
                    logger,
                )
                if not start_script_has_args(entry.start_script):
                    used_scripts.add(entry.start_script)

            for entry in entries:
                if find_registration(entry, snapshot.entries) is not None:
                    continue
                text = (
                    f"{stamp}: Missing process {entry.name} {entry.instance} "
                    f"on {entry.host} in procmap -- restarting.\n"
                )
                logger.write(text, always=True, debug=True)
                email_message += text
                send_this_loop = True
                if (
                    not start_script_has_args(entry.start_script)
                    and entry.start_script in used_scripts
                ):
                    logger.write(
                        f"   Already starting script {entry.start_script}, skipping.\n",
                        always=False,
                        debug=True,
                    )
                    continue
                email_message += restart_entry(
                    entry,
                    None,
                    kill_processes,
                    runner,
                    args.timeout,
                    logger,
                )
                if not start_script_has_args(entry.start_script):
                    used_scripts.add(entry.start_script)

        if send_this_loop:
            send_email(args.email, email_message)
        if args.quit or STOP_REQUESTED:
            break
        interruptible_sleep(args.sleep)

    if lock_stream is not None:
        fcntl.flock(lock_stream, fcntl.LOCK_UN)
        lock_stream.close()
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProcmapControlError as error:
        print(f"ERROR - {error}", file=sys.stderr)
        raise SystemExit(1)
