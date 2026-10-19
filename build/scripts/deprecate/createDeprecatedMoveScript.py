#!/usr/bin/env python3

"""Create a reviewable script to move unused code directories.

The generated script keeps every directory selected by any retained build
profile and moves only complete, Makefile-bearing directories omitted by all
of them.  It intentionally does not classify individual files within an
active target.
"""

from pathlib import Path
import argparse
import shlex
import subprocess
import sys


RETAINED_PROFILES = (
    "lrose-core",
    "lrose-radx",
    "lrose-cidd",
    "samurai",
    "hcr",
)

# These directories contain build metadata or assets that cannot be judged by
# SUB_DIRS membership alone.
RESERVED_NAMES = {
    "CVS",
    "_makefiles",
    "images",
    "include",
    "perl5",
    "resources",
    "scripts",
}

BUILD_MARKERS = ("Makefile", "makefile", "__makefile.template")


def values_for_key(makefile_path, key):
    """Parse a multiline Make variable like createCMakeLists.py does."""
    try:
        lines = makefile_path.read_text(errors="replace").splitlines(True)
    except OSError:
        return []

    found = False
    value = ""
    for line in lines:
        if not found and line.startswith("#"):
            continue
        if key in line:
            found = True
            value += line
            if "\\" not in line:
                break
        elif found:
            if line.startswith("#") or len(line) < 2:
                break
            value += line
            if "\\" not in line:
                break

    if not found:
        return []

    for token in (key, "=", "\t", "\\", "\r", "\n"):
        value = value.replace(token, " ")
    return value.split()


def makefile_for_profile(directory, profile):
    if profile is None:
        return directory / "Makefile"
    variant = directory / "_makefiles" / ("makefile." + profile)
    if variant.is_file():
        return variant
    return directory / "Makefile"


def collect_decisions(codebase, profiles):
    """Return each traversed directory and the union of selected children."""
    decisions = {}

    def walk(directory, profile):
        makefile = makefile_for_profile(directory, profile)
        if not makefile.is_file():
            return

        parts = directory.relative_to(codebase).parts

        # At a library src directory, SUB_DIRS names the source components.
        if len(parts) >= 3 and parts[-3] == "libs" and parts[-1] == "src":
            decisions.setdefault(directory, set()).update(
                values_for_key(makefile, "SUB_DIRS")
            )
            return

        # An application directory is a target, not another recursion level.
        if len(parts) >= 4 and parts[-4] == "apps" and parts[-2] == "src":
            return

        # apps/scripts/src is handled specially by createCMakeLists.py.
        if (
            len(parts) >= 3
            and parts[-3] == "apps"
            and parts[-2] == "scripts"
            and parts[-1] == "src"
        ):
            return

        subdirs = values_for_key(makefile, "SUB_DIRS")
        decisions.setdefault(directory, set()).update(subdirs)
        for name in subdirs:
            child = directory / name
            if child.is_dir():
                walk(child, profile)

    # Preserve the active checked-in Makefiles as well as the named profile
    # variants. Some profile snapshots lag changes made to the active files.
    for profile in (None,) + tuple(profiles):
        walk(codebase / "libs", profile)
        walk(codebase / "apps", profile)

    return decisions


def collect_candidates(decisions):
    candidates = []
    for directory, selected_names in decisions.items():
        for child in directory.iterdir():
            if not child.is_dir():
                continue
            if child.name in selected_names or child.name in RESERVED_NAMES:
                continue
            if any((child / marker).exists() for marker in BUILD_MARKERS):
                candidates.append(child)

    # If an entire parent is a candidate, do not also list its descendants.
    candidates.sort(key=lambda path: (len(path.parts), str(path)))
    maximal = []
    for candidate in candidates:
        if not any(parent in candidate.parents for parent in maximal):
            maximal.append(candidate)
    return sorted(maximal)


def find_explicit_references(repo_root, candidates, excluded_paths=()):
    """Find literal codebase/path references originating outside a candidate."""
    candidate_set = set(candidates)
    excluded_paths = {path.resolve() for path in excluded_paths}
    needles = {candidate: candidate.relative_to(repo_root).as_posix() for candidate in candidates}
    references = {candidate: [] for candidate in candidates}

    # Let git grep narrow this to the small set of tracked text files that can
    # contain an explicit codebase path. Scanning every source against every
    # candidate is needlessly expensive in this repository.
    result = subprocess.run(
        [
            "git",
            "grep",
            "-Ilz",
            "-e",
            "codebase/apps/",
            "-e",
            "codebase/libs/",
            "--",
        ],
        cwd=repo_root,
        stdout=subprocess.PIPE,
        check=False,
    )
    if result.returncode not in (0, 1):
        raise subprocess.CalledProcessError(result.returncode, result.args)

    paths = [repo_root / Path(item.decode()) for item in result.stdout.split(b"\0") if item]
    for path in paths:
        if path.resolve() in excluded_paths:
            continue
        if any(candidate == path.parent or candidate in path.parents for candidate in candidate_set):
            continue
        try:
            if path.stat().st_size > 5 * 1024 * 1024:
                continue
            text = path.read_text(errors="ignore")
        except OSError:
            continue
        for candidate, needle in needles.items():
            if needle in text:
                references[candidate].append(path.relative_to(repo_root).as_posix())

    return {path: refs for path, refs in references.items() if refs}


def shell_quote(value):
    return shlex.quote(str(value))


def write_move_script(output_path, repo_root, candidates, references):
    relative_candidates = [path.relative_to(repo_root) for path in candidates]
    lines = [
        "#!/usr/bin/env bash",
        "",
        "# Generated by build/scripts/deprecate/createDeprecatedMoveScript.py.",
        "#",
        "# This moves complete code directories that are not selected by any of:",
        "#   " + ", ".join(RETAINED_PROFILES),
        "# The currently active checked-in Makefiles are preserved as well.",
        "#",
        "# Review this file before running it. It performs a full preflight before",
        "# making changes and refuses to move directories containing untracked or",
        "# ignored files.",
        "",
        "set -euo pipefail",
        "",
        'script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"',
        'repo_root="$(cd -- "${script_dir}/../../.." && pwd)"',
        'cd "${repo_root}"',
        "",
        'if [[ ! -d .git || ! -d codebase ]]; then',
        '  echo "ERROR: expected the lrose-core repository root: ${repo_root}" >&2',
        "  exit 1",
        "fi",
        "",
        'if [[ $# -ne 1 || "$1" != "--apply" ]]; then',
        '  echo "Usage: $0 --apply" >&2',
        '  echo "Review this script and its .references.txt report first." >&2',
        "  exit 2",
        "fi",
        "",
        "moves=(",
    ]

    for source in relative_candidates:
        destination = Path("deprecated") / source.relative_to("codebase")
        lines.append("  " + shell_quote(source.as_posix()))
        lines.append("  " + shell_quote(destination.as_posix()))
    lines.extend([
        ")",
        "",
        "# Verify every move before changing the worktree.",
        "source_paths=()",
        "for ((i = 0; i < ${#moves[@]}; i += 2)); do",
        '  source_path="${moves[i]}"',
        '  destination_path="${moves[i + 1]}"',
        '  source_paths+=("${source_path}")',
        '  if [[ ! -d "${source_path}" ]]; then',
        '    echo "ERROR: source directory is missing: ${source_path}" >&2',
        "    exit 1",
        "  fi",
        '  if [[ -e "${destination_path}" ]]; then',
        '    echo "ERROR: destination already exists: ${destination_path}" >&2',
        "    exit 1",
        "  fi",
        "done",
        "",
        "extra_files=()",
        'while IFS= read -r -d "" path; do extra_files+=("${path}"); done < <(',
        '  git ls-files -z --others --exclude-standard -- "${source_paths[@]}"',
        ")",
        'while IFS= read -r -d "" path; do extra_files+=("${path}"); done < <(',
        '  git ls-files -z --others --ignored --exclude-standard -- "${source_paths[@]}"',
        ")",
        'if ((${#extra_files[@]})); then',
        '  echo "ERROR: candidate directories contain untracked or ignored files." >&2',
        '  printf "  %s\\n" "${extra_files[@]}" >&2',
        '  echo "Clean or relocate those files, then run this script again." >&2',
        "  exit 1",
        "fi",
        "",
        "# All candidates passed preflight; preserve the codebase-relative tree.",
        "for ((i = 0; i < ${#moves[@]}; i += 2)); do",
        '  source_path="${moves[i]}"',
        '  destination_path="${moves[i + 1]}"',
        '  mkdir -p -- "$(dirname -- "${destination_path}")"',
        '  git mv -- "${source_path}" "${destination_path}"',
        "done",
        "",
        'echo "Moved $((${#moves[@]} / 2)) directories into deprecated/."',
        "",
    ])
    output_path.write_text("\n".join(lines))
    output_path.chmod(0o755)

    reference_path = output_path.with_suffix(".references.txt")
    report = [
        "Explicit references to candidate paths",
        "======================================",
        "",
        "These literal path references do not affect build-graph selection, but",
        "documentation or auxiliary scripts may need updating after the move.",
        "",
    ]
    if references:
        for candidate in sorted(references):
            report.append(candidate.relative_to(repo_root).as_posix())
            for referring_path in sorted(set(references[candidate])):
                report.append("  " + referring_path)
            report.append("")
    else:
        report.append("No literal references found.")
    reference_path.write_text("\n".join(report) + "\n")
    return reference_path


def main():
    script_path = Path(__file__).resolve()
    repo_root = script_path.parents[3]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=Path,
        default=script_path.with_name("moveUnusedCodeToDeprecated.sh"),
        help="generated shell script path",
    )
    args = parser.parse_args()
    output_path = args.output
    if not output_path.is_absolute():
        output_path = repo_root / output_path

    codebase = repo_root / "codebase"
    decisions = collect_decisions(codebase, RETAINED_PROFILES)
    candidates = collect_candidates(decisions)
    reference_path = output_path.with_suffix(".references.txt")
    references = find_explicit_references(
        repo_root,
        candidates,
        excluded_paths=(script_path, output_path, reference_path),
    )
    reference_path = write_move_script(
        output_path, repo_root, candidates, references
    )

    print("Wrote %s" % output_path)
    print("Wrote %s" % reference_path)
    print("Candidate directories: %d" % len(candidates))
    print("Candidates with literal external references: %d" % len(references))


if __name__ == "__main__":
    try:
        main()
    except (OSError, subprocess.CalledProcessError) as error:
        print("ERROR: %s" % error, file=sys.stderr)
        sys.exit(1)
