#!/usr/bin/env python3
# Common proc_list parsing, procmap querying, and bounded command execution.
# This module is installed beside its four callers and uses only Python's
# standard library so operational installations do not need PYTHONPATH setup.
"""Shared runtime support for the Python procmap reliability commands."""

from __future__ import annotations

import dataclasses
import os
import re
import shlex
import signal
import socket
import subprocess
import sys
import time
from pathlib import Path
from typing import Iterable, List, Optional, Sequence, Tuple, Union


# Keep this interface version independent of the application version.  The
# four installed commands check it at startup so a partial installation fails
# clearly instead of calling a stale shared implementation.
API_VERSION = 1
MIN_PYTHON = (3, 8)


# ---------------------------------------------------------------------------
# Shared data model
# ---------------------------------------------------------------------------

class ProcmapControlError(RuntimeError):
    """An expected configuration, command, or procmap failure."""


@dataclasses.dataclass(frozen=True)
class ProcEntry:
    name: str
    instance: str
    start_script: str
    kill_script: str
    host: str
    priority: Optional[str] = None


@dataclasses.dataclass(frozen=True)
class ProcmapEntry:
    name: str
    instance: str
    host: str
    user: str
    pid: int
    max_interval: int
    heartbeat_age: int
    uptime: int
    registrations: int
    status: int
    status_text: str

    @property
    def is_late(self) -> bool:
        return self.heartbeat_age > self.max_interval


@dataclasses.dataclass(frozen=True)
class ProcmapSnapshot:
    host: str
    uptime: int
    entries: Tuple[ProcmapEntry, ...]


@dataclasses.dataclass(frozen=True)
class CommandResult:
    command: str
    returncode: int
    output: str
    timed_out: bool = False

    @property
    def ok(self) -> bool:
        return not self.timed_out and self.returncode == 0


_ENV_RE = re.compile(r"\$\(([A-Za-z_][A-Za-z0-9_]*)\)")
_START_ARGS_RE = re.compile(r"^([^()\s]+)\(([\w,/-]*)\)$")


# ---------------------------------------------------------------------------
# Runtime and proc_list parsing
# ---------------------------------------------------------------------------

def require_compatible_runtime(expected_api: int) -> None:
    # Check the interpreter here as well as in packaging documentation because
    # these commands are often copied directly between operational systems.
    if tuple(sys.version_info[:2]) < MIN_PYTHON:
        raise ProcmapControlError(
            "Python %d.%d or newer is required" % MIN_PYTHON
        )
    if expected_api != API_VERSION:
        raise ProcmapControlError(
            f"incompatible procmap_control.py API: expected {expected_api}, "
            f"found {API_VERSION}"
        )


def _expand_environment(value: str, source: Path, line_number: int) -> str:
    # The proc_list syntax predates shell-style ${NAME}; retain its documented
    # $(NAME) convention without invoking a shell during configuration parsing.
    def replace(match: re.Match) -> str:
        name = match.group(1)
        if name not in os.environ:
            raise ProcmapControlError(
                f"{source}:{line_number}: environment variable {name} is not set"
            )
        return os.environ[name]

    return _ENV_RE.sub(replace, value)


def _normalize_config_host(host: str) -> str:
    # Procmap registrations normally contain the real hostname rather than the
    # aliases used in project configuration files.
    if host in ("local", "localhost"):
        return socket.gethostname()
    return host


def read_proc_list(path: str, allow_includes: bool = True) -> List[ProcEntry]:
    """Read the legacy proc_list grammar, including one include level."""
    entries: List[ProcEntry] = []
    _read_proc_list(Path(path), entries, allow_includes, 0)
    return entries


def _read_proc_list(
    path: Path,
    entries: List[ProcEntry],
    allow_includes: bool,
    include_depth: int,
) -> None:
    # Includes are intentionally limited to one level, matching Procmap.pm and
    # preventing accidental recursive configuration at service startup.
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError as error:
        raise ProcmapControlError(f"cannot read process list {path}: {error}") from error

    for line_number, original in enumerate(lines, 1):
        stripped = original.strip()
        if not stripped or stripped.startswith("#"):
            continue
        fields = stripped.split()
        if fields[0] == "include":
            if not allow_includes:
                raise ProcmapControlError(
                    f"{path}:{line_number}: include directives are disabled"
                )
            if include_depth != 0:
                raise ProcmapControlError(
                    f"{path}:{line_number}: nested includes are not supported"
                )
            if len(fields) != 2:
                raise ProcmapControlError(
                    f"{path}:{line_number}: include requires exactly one filename"
                )
            include_name = _expand_environment(fields[1], path, line_number)
            _read_proc_list(Path(include_name), entries, True, include_depth + 1)
            continue

        if len(fields) == 4:
            # Historical four-column lists omitted the host and implied local.
            fields.append("localhost")
        if len(fields) not in (5, 6):
            print(
                f"WARNING: {path}:{line_number}: expected 4, 5, or 6 fields, "
                f"found {len(fields)}; skipping line",
                file=sys.stderr,
            )
            continue
        try:
            expanded = [
                _expand_environment(value, path, line_number) for value in fields
            ]
        except ProcmapControlError as error:
            # The Perl reader has always allowed one bad entry to coexist with
            # valid processes in the same operational list.
            print(f"WARNING: {error}; skipping line", file=sys.stderr)
            continue
        entries.append(
            ProcEntry(
                name=expanded[0],
                instance=expanded[1],
                start_script=expanded[2],
                kill_script=expanded[3],
                host=_normalize_config_host(expanded[4]),
                priority=expanded[5] if len(expanded) == 6 else None,
            )
        )


def start_script_has_args(start_script: str) -> bool:
    # Parentheses distinguish per-process invocations from shared scripts that
    # must be run only once for a group of proc_list entries.
    return _START_ARGS_RE.fullmatch(start_script) is not None


def build_start_command(entry: ProcEntry) -> str:
    # start_inst(mode) expands to start_inst <name> <instance> <mode>.  Do not
    # use shell word splitting for the parenthesized arguments.
    match = _START_ARGS_RE.fullmatch(entry.start_script)
    if match is None:
        return entry.start_script
    command, arg_text = match.groups()
    args = [] if not arg_text else arg_text.split(",")
    return " ".join([command, entry.name, entry.instance] + args)


def hosts_equivalent(first: str, second: str) -> bool:
    # Prefer address comparison so short and fully-qualified names match.  The
    # textual fallback keeps isolated systems useful when DNS is unavailable.
    first = first.split("@", 1)[-1]
    second = second.split("@", 1)[-1]
    if first in ("local", "localhost"):
        first = socket.gethostname()
    if second in ("local", "localhost"):
        second = socket.gethostname()
    try:
        return socket.gethostbyname(first) == socket.gethostbyname(second)
    except OSError:
        return first.split(".", 1)[0].lower() == second.split(".", 1)[0].lower()


class CommandRunner:
    """Run trusted project commands with bounded lifetime and captured output."""

    def __init__(self, remote_shell: Optional[str] = None, debug: bool = False):
        # RSH commonly contains "ssh" plus site-specific options.
        self.remote_shell = shlex.split(remote_shell or os.environ.get("RSH", "ssh"))
        self.debug = debug

    def run(self, host: str, command: str, timeout: float = 10.0) -> CommandResult:
        # Project start and kill entries are trusted shell fragments, so local
        # execution deliberately retains /bin/sh behavior.  Remote commands
        # are passed as one argument to RSH/ssh for interpretation remotely.
        if self._is_local(host):
            argv: Union[Sequence[str], str] = command
            use_shell = True
        else:
            user = None
            remote = host
            if "@" in host:
                user, remote = host.split("@", 1)
            argv_list = list(self.remote_shell) + ["-n"]
            if user:
                argv_list += ["-l", user]
            argv_list += [remote, command]
            argv = argv_list
            use_shell = False
        if self.debug:
            print(f"Running on {host}: {command}", file=sys.stderr)
        return self._run_process(argv, command, timeout, use_shell)

    def run_local_argv(
        self, argv: Sequence[str], timeout: float = 10.0
    ) -> CommandResult:
        display = shlex.join(argv)
        if self.debug:
            print(f"Running: {display}", file=sys.stderr)
        return self._run_process(argv, display, timeout, False)

    def _is_local(self, host: str) -> bool:
        # Avoid a remote-shell dependency when a configuration uses a different
        # spelling for the machine on which this command is running.
        plain_host = host.split("@", 1)[-1]
        return plain_host in ("local", "localhost") or hosts_equivalent(
            plain_host, socket.gethostname()
        )

    @staticmethod
    def _run_process(
        argv: Union[Sequence[str], str],
        display: str,
        timeout: float,
        use_shell: bool,
    ) -> CommandResult:
        try:
            # Give every command its own process group so timeout cleanup also
            # reaches shell children instead of leaving an orphaned command.
            process = subprocess.Popen(
                argv,
                shell=use_shell,
                executable="/bin/sh" if use_shell else None,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                start_new_session=True,
            )
        except OSError as error:
            return CommandResult(display, 127, str(error))
        try:
            output, _ = process.communicate(timeout=timeout)
            return CommandResult(display, process.returncode, output or "")
        except subprocess.TimeoutExpired:
            try:
                os.killpg(process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            try:
                output, _ = process.communicate(timeout=2)
            except subprocess.TimeoutExpired:
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                output, _ = process.communicate()
            return CommandResult(display, process.returncode or -1, output or "", True)


def read_procmap(host: str, timeout: float = 10.0) -> ProcmapSnapshot:
    # Using print_procmap keeps this module independent of LROSE Python
    # bindings and preserves the same interface used by the Perl scripts.
    argv = ["print_procmap", "-plain"]
    if host:
        argv += ["-host", host]
    runner = CommandRunner()
    result = runner.run_local_argv(argv, timeout)
    lines = result.output.splitlines()
    if not lines:
        raise ProcmapControlError(
            f"print_procmap returned no output for {host or 'localhost'}"
        )
    if lines[0].startswith("No procmap reachable on") or not result.ok:
        detail = result.output.strip() or f"exit status {result.returncode}"
        raise ProcmapControlError(detail)

    no_processes = lines[0].startswith("No procs registered with")
    procmap_host = host or "localhost"
    if no_processes:
        procmap_host = lines[0][len("No procs registered with"):].strip()
    else:
        procmap_host = lines[0].strip()
    if len(lines) < 2:
        raise ProcmapControlError("print_procmap output is missing the uptime")
    try:
        uptime = int(lines[1].strip())
    except ValueError as error:
        raise ProcmapControlError("invalid procmap uptime: " + lines[1]) from error

    parsed: List[ProcmapEntry] = []
    for line_number, line in enumerate(lines[2:], 3):
        # The status text may contain spaces, hence the bounded split after the
        # ten fixed fields emitted by print_procmap -plain.
        fields = line.split(maxsplit=10)
        if len(fields) < 10:
            raise ProcmapControlError(
                f"invalid print_procmap line {line_number}: {line}"
            )
        try:
            parsed.append(
                ProcmapEntry(
                    name=fields[0],
                    instance=fields[1],
                    host=fields[2],
                    user=fields[3],
                    pid=int(fields[4]),
                    max_interval=abs(int(fields[5])),
                    heartbeat_age=abs(int(fields[6])),
                    uptime=abs(int(fields[7])),
                    registrations=int(fields[8]),
                    status=int(fields[9]),
                    status_text=fields[10].strip('"') if len(fields) > 10 else "",
                )
            )
        except ValueError as error:
            raise ProcmapControlError(
                f"invalid numeric value on print_procmap line {line_number}: {line}"
            ) from error
    return ProcmapSnapshot(procmap_host, uptime, tuple(parsed))


def find_registration(
    expected: ProcEntry, registrations: Iterable[ProcmapEntry]
) -> Optional[ProcmapEntry]:
    # Process identity includes host for restart and kill operations; the check
    # command intentionally uses only name/instance for legacy compatibility.
    for registration in registrations:
        if (
            registration.name == expected.name
            and registration.instance == expected.instance
            and hosts_equivalent(registration.host, expected.host)
        ):
            return registration
    return None


def run_start(
    entry: ProcEntry, runner: CommandRunner, timeout: float = 10.0
) -> CommandResult:
    # Keep this message on stderr because operational wrappers often redirect
    # stdout to machine-readable status output.
    command = build_start_command(entry)
    print(
        f">> Starting process with command '{command}' on host {entry.host}",
        file=sys.stderr,
    )
    return runner.run(entry.host, command, timeout)


def run_kill_script(
    entry: ProcEntry, runner: CommandRunner, timeout: float = 10.0
) -> CommandResult:
    # snuff_inst is a proc_list keyword, not a literal project kill script.
    if entry.kill_script == "snuff_inst":
        command = " ".join(
            ["snuff_inst", shlex.quote(entry.name), shlex.quote(entry.instance)]
        )
    else:
        command = entry.kill_script
    return runner.run(entry.host, command, timeout)


def format_timestamp(use_local: bool = False) -> str:
    # auto_restart_stats parses the [utime:N] token and the restart phrases, so
    # retain the legacy timestamp layout exactly.
    now = time.time()
    stamp = time.localtime(now) if use_local else time.gmtime(now)
    zone = (time.strftime("%Z", stamp) or "LOCAL") if use_local else "UTC"
    return f"{time.strftime('%Y%m%d %H:%M:%S', stamp)} {zone} [utime:{int(now)}]"
