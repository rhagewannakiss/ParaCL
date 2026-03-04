#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


VALID_DIR = "valid_progs"
INVALID_DIR = "invalid_progs"


class Ansi:
    RESET = "\033[0m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"


@dataclass(frozen=True)
class Case:
    kind: str
    name: str
    pcl_path: Path
    ans_path: Path
    in_path: Path | None


@dataclass(frozen=True)
class CaseResult:
    case: Case
    ok: bool
    expected_exit_desc: str
    actual_exit: int
    expected_stdout: str
    actual_stdout: str
    expected_stderr: str
    actual_stderr: str
    failure_reason: str = ""


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[2]
    default_binary = repo_root / "build" / "bin" / "paracl-cli"

    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", default=str(default_binary))
    parser.add_argument("--root", default=str(repo_root / "test" / "e2e"))
    parser.add_argument(
        "--color",
        choices=("auto", "always", "never"),
        default="auto",
        help="colorize output: auto (default), always, never",
    )
    return parser.parse_args()


def use_color(mode: str) -> bool:
    if mode == "always":
        return True
    if mode == "never":
        return False
    return sys.stdout.isatty() and "NO_COLOR" not in os.environ


def paint(text: str, color: str, enabled: bool) -> str:
    if not enabled:
        return text
    return f"{color}{text}{Ansi.RESET}"


def discover_cases(root: Path) -> list[Case]:
    cases: list[Case] = []
    for kind_dir_name in (VALID_DIR, INVALID_DIR):
        kind_dir = root / kind_dir_name
        if not kind_dir.exists():
            continue

        for pcl_path in sorted(kind_dir.rglob("*.pcl")):
            stem = pcl_path.with_suffix("")
            ans_path = stem.with_suffix(".ans")
            in_path = stem.with_suffix(".in")
            if not ans_path.exists():
                raise FileNotFoundError(f"Missing .ans for {pcl_path}")

            relative_name = pcl_path.relative_to(kind_dir).with_suffix("").as_posix()
            cases.append(
                Case(
                    kind=kind_dir_name,
                    name=relative_name,
                    pcl_path=pcl_path,
                    ans_path=ans_path,
                    in_path=in_path if in_path.exists() else None,
                )
            )
    return cases


def expected_exit_description(kind: str) -> str:
    if kind == VALID_DIR:
        return "0"
    return "!= 0"


def expected_stdout(kind: str, ans_text: str) -> str:
    return ans_text if kind == VALID_DIR else ""


def expected_stderr(kind: str, ans_text: str) -> str:
    return ans_text if kind == INVALID_DIR else ""


def exit_code_matches(kind: str, returncode: int) -> bool:
    if kind == VALID_DIR:
        return returncode == 0
    return returncode != 0


def run_case(case: Case, binary: Path, repo_root: Path) -> CaseResult:
    ans_text = case.ans_path.read_text(encoding="utf-8")
    stdin_text = ""
    if case.in_path is not None:
        stdin_text = case.in_path.read_text(encoding="utf-8")

    rel_program_path = case.pcl_path.relative_to(repo_root)
    proc = subprocess.run(
        [str(binary), str(rel_program_path)],
        cwd=repo_root,
        input=stdin_text,
        capture_output=True,
        text=True,
        check=False,
    )

    exp_stdout = expected_stdout(case.kind, ans_text)
    exp_stderr = expected_stderr(case.kind, ans_text)
    exit_ok = exit_code_matches(case.kind, proc.returncode)
    stdout_ok = proc.stdout == exp_stdout
    stderr_ok = proc.stderr == exp_stderr
    ok = exit_ok and stdout_ok and stderr_ok

    reason_parts = []
    if not exit_ok:
        reason_parts.append("exit code mismatch")
    if not stdout_ok:
        reason_parts.append("stdout mismatch")
    if not stderr_ok:
        reason_parts.append("stderr mismatch")

    return CaseResult(
        case=case,
        ok=ok,
        expected_exit_desc=expected_exit_description(case.kind),
        actual_exit=proc.returncode,
        expected_stdout=exp_stdout,
        actual_stdout=proc.stdout,
        expected_stderr=exp_stderr,
        actual_stderr=proc.stderr,
        failure_reason=", ".join(reason_parts),
    )


def print_failure(result: CaseResult) -> None:
    print(f"  reason: {result.failure_reason}")
    print(f"  expected exit: {result.expected_exit_desc}")
    print(f"  actual exit:   {result.actual_exit}")
    print(f"  expected stdout: {result.expected_stdout!r}")
    print(f"  actual stdout:   {result.actual_stdout!r}")
    print(f"  expected stderr: {result.expected_stderr!r}")
    print(f"  actual stderr:   {result.actual_stderr!r}")


def main() -> int:
    args = parse_args()
    binary = Path(args.binary).resolve()
    root = Path(args.root).resolve()
    repo_root = root.parents[1]
    color_enabled = use_color(args.color)

    run_tag = paint("[ RUN      ]", Ansi.BLUE, color_enabled)
    ok_tag = paint("[       OK ]", Ansi.GREEN, color_enabled)
    failed_tag = paint("[  FAILED  ]", Ansi.RED, color_enabled)
    passed_tag = paint("[  PASSED  ]", Ansi.GREEN, color_enabled)
    error_tag = paint("[  ERROR   ]", Ansi.YELLOW, color_enabled)

    if not binary.exists():
        print(f"{error_tag} missing binary: {binary}")
        return 1

    cases = discover_cases(root)
    if not cases:
        print(f"{error_tag} no E2E cases found")
        return 1

    failures = 0
    failed_names: list[str] = []
    for case in cases:
        test_name = f"{case.kind}/{case.name}"
        print(f"{run_tag} {test_name}")
        result = run_case(case, binary, repo_root)
        if result.ok:
            print(f"{ok_tag} {test_name}")
            continue

        failures += 1
        failed_names.append(test_name)
        print(f"{failed_tag} {test_name}")
        print_failure(result)

    total = len(cases)
    passed = total - failures
    print("")
    if failures == 0:
        print(f"{passed_tag} {passed} tests.")
        return 0

    print(f"{failed_tag} {failures} tests, listed below:")
    for failed_name in failed_names:
        print(f"{failed_tag} {failed_name}")
    print(f"{passed_tag} {passed} tests.")
    print(f"{failed_tag} {failures} tests.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
