#!/usr/bin/env python3
"""Prepare, run, and summarise the controlled BitVec-predicate A/B.

Preparation is the default-safe operation: it records exact commands and
metadata but runs nothing.  ``run`` is deliberately explicit and executes
cells serially (off, then unsigned) so the two modes do not contend for the
same machine.
"""

from __future__ import annotations

import argparse
import collections
import csv
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shlex
import shutil
import statistics
import subprocess
import sys
import time


REPO = Path(__file__).resolve().parents[2]
STAGE_A = ("s27", "s953", "plena_data_flow_control_flat")
STAGE_B = (
    "c17", "c432", "c499", "c880", "c1355", "s27", "s298", "s344",
    "s349", "s382", "s386", "s400", "s420", "s444", "s510", "s641",
    "s713", "s820", "s832", "s838", "s953", "s1488",
)
MODES = ("off", "unsigned")
MANIFEST_VERSION = 2


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def tree_identity(path: Path) -> dict[str, object]:
    """Content identity for a fixed benchmark directory."""
    files = sorted(p for p in path.rglob("*") if p.is_file())
    digest = hashlib.sha256()
    for item in files:
        rel = item.relative_to(path).as_posix()
        digest.update(rel.encode())
        digest.update(b"\0")
        digest.update(sha256_file(item).encode())
        digest.update(b"\n")
    mutants = [
        p for p in files
        if p.name.startswith("mutant_") and p.suffix == ".sv"
        and not p.name.endswith("_assertion.sv")
    ]
    generated = [p for p in files if p.name.endswith("_assertion.sv")]
    if not mutants:
        raise ValueError(f"NO-FIXED-MUTANTS: {path}")
    if generated:
        raise ValueError(f"fixed mutant directory contains generated outputs: {generated[0]}")
    return {"files": len(files), "mutants": len(mutants), "sha256": digest.hexdigest()}


def design_identity(main: Path) -> dict[str, object]:
    """Identity of the main RTL and every sibling source SMART auto-discovers."""
    files = sorted(
        path for path in main.parent.iterdir()
        if path.is_file() and path.suffix in (".sv", ".v")
    )
    if main not in files:
        raise ValueError(f"design source disappeared: {main}")
    digest = hashlib.sha256()
    for path in files:
        digest.update(path.name.encode())
        digest.update(b"\0")
        digest.update(sha256_file(path).encode())
        digest.update(b"\n")
    return {"files": len(files), "sha256": digest.hexdigest()}


def run_capture(command: list[str], cwd: Path, env: dict[str, str] | None = None) -> str:
    try:
        result = subprocess.run(
            command, cwd=cwd, env=env, text=True, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, timeout=20, check=False,
        )
        return result.stdout.strip()[:4000]
    except (OSError, subprocess.TimeoutExpired) as error:
        return f"unavailable: {error}"


def tool_environment(repo: Path) -> dict[str, str]:
    env = os.environ.copy()
    prefixes = (
        repo / ".deps/hw-cbmc/src/ebmc",
        repo / "otherTools/cvc5/bin",
        repo / "otherTools/oss-cad-suite/bin",
    )
    env["PATH"] = os.pathsep.join(map(str, prefixes)) + os.pathsep + env.get("PATH", "")
    return env


def find_design(root: Path, design: str) -> Path:
    matches = sorted(root.rglob(f"{design}/{design}.sv"))
    if len(matches) != 1:
        raise ValueError(
            f"expected exactly one {design}/{design}.sv under {root}, found {len(matches)}"
        )
    return matches[0].resolve()


def git_metadata(repo: Path) -> dict[str, object]:
    revision = run_capture(["git", "rev-parse", "HEAD"], repo)
    status = run_capture(["git", "status", "--short"], repo)
    return {"revision": revision, "dirty": bool(status), "status": status}


def prepare(args: argparse.Namespace) -> int:
    repo = args.repo.resolve()
    destination = args.output.resolve()
    if destination.exists() and any(destination.iterdir()):
        raise ValueError(f"refusing to overwrite non-empty experiment directory: {destination}")
    destination.mkdir(parents=True, exist_ok=True)

    config = args.config.resolve()
    smart = args.smart.resolve()
    mutants_root = args.mutants.resolve()
    benchmark_root = args.benchmarks.resolve()
    for path, label in ((config, "config"), (smart, "smart binary")):
        if not path.is_file():
            raise ValueError(f"missing {label}: {path}")

    designs = tuple(args.design or (STAGE_A if args.stage == "A" else STAGE_B))
    sources = {name: find_design(benchmark_root, name) for name in designs}
    design_sources = {
        name: {"main": str(source), **design_identity(source)}
        for name, source in sources.items()
    }
    mutant_sets: dict[str, dict[str, object]] = {}
    for name in designs:
        fixed = mutants_root / name / "benchmarks"
        if not fixed.is_dir():
            raise ValueError(f"NO-FIXED-MUTANTS: {fixed}")
        mutant_sets[name] = {"path": str(fixed.resolve()), **tree_identity(fixed)}

    frozen_config = destination / "config.json"
    shutil.copy2(config, frozen_config)
    env = tool_environment(repo)
    versions = {
        "smart": run_capture([str(smart), "--version"], repo, env),
        "ebmc": run_capture(["ebmc", "--version"], repo, env),
        "cvc5": run_capture(["cvc5", "--version"], repo, env),
        "verilator": run_capture(["verilator", "--version"], repo, env),
        "iverilog": run_capture(["iverilog", "-V"], repo, env),
    }

    cells = []
    for design in designs:
        for mode in MODES:
            cell = destination / "cells" / design / mode
            work = cell / "work"
            output = cell / f"{design}_assertion.sv"
            command = [
                str(smart), str(sources[design]), "--top", design,
                "--config", str(frozen_config), "--seed", str(args.seed),
                "--bv-predicates", mode, "--jobs", str(args.jobs),
                "--core-timeout", str(args.core_timeout), "--workdir", str(work),
                "--output", str(output), "--keep-work", "-q",
            ]
            cells.append({
                "design": design, "mode": mode, "directory": str(cell),
                "source": str(sources[design]), "seed": args.seed,
                "fixed_mutants": mutant_sets[design]["mutants"],
                "mutant_sha256": mutant_sets[design]["sha256"],
                "mine_command": command,
                "eval_command": [
                    sys.executable, "evaluater.py", design, str(args.eval_bound),
                ],
            })

    manifest = {
        "manifest_version": MANIFEST_VERSION,
        "created_utc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "stage": args.stage,
        "repo": str(repo),
        "git": git_metadata(repo),
        "host": {
            "node": platform.node(), "platform": platform.platform(),
            "cpu_count": os.cpu_count(),
        },
        "tools": versions,
        "smart_binary": {"path": str(smart), "sha256": sha256_file(smart)},
        "config": {
            "source": str(config), "frozen": str(frozen_config),
            "sha256": sha256_file(frozen_config),
        },
        "design_sources": design_sources,
        "seed": args.seed,
        "jobs": args.jobs,
        "core_timeout": args.core_timeout,
        "mine_timeout": args.mine_timeout,
        "eval_timeout": args.eval_timeout,
        "eval_bound": args.eval_bound,
        "mutant_sets": mutant_sets,
        "cells": cells,
    }
    manifest_path = destination / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    commands = ["# Generated only; cells must run serially.", "set -eu"]
    for cell in cells:
        commands.append(" ".join(shlex.quote(part) for part in cell["mine_command"]))
    commands.append("")
    (destination / "mine-commands.sh").write_text("\n".join(commands))
    print(f"prepared {len(cells)} cells in {destination}; no experiment was run")
    print(f"review {manifest_path}, then run: {Path(__file__).name} run {manifest_path}")
    return 0


def load_manifest(path: Path) -> dict[str, object]:
    data = json.loads(path.read_text())
    if data.get("manifest_version") != MANIFEST_VERSION:
        raise ValueError(f"unsupported manifest version in {path}")
    return data


def verify_frozen_inputs(manifest: dict[str, object]) -> None:
    config = manifest["config"]
    if sha256_file(Path(config["frozen"])) != config["sha256"]:
        raise ValueError("frozen config changed after preparation")
    smart = manifest["smart_binary"]
    if sha256_file(Path(smart["path"])) != smart["sha256"]:
        raise ValueError("SMART binary changed after preparation")
    for design, expected in manifest["design_sources"].items():
        actual = design_identity(Path(expected["main"]))
        if actual != {key: expected[key] for key in ("files", "sha256")}:
            raise ValueError(f"design source changed after preparation: {design}")
    for design, expected in manifest["mutant_sets"].items():
        actual = tree_identity(Path(expected["path"]))
        if actual != {key: expected[key] for key in ("files", "mutants", "sha256")}:
            raise ValueError(f"fixed mutant set changed after preparation: {design}")


def execute(command: list[str], cwd: Path, output: Path, timeout: int,
            env: dict[str, str]) -> tuple[int, float, bool]:
    started = time.monotonic()
    with output.open("w") as log:
        try:
            result = subprocess.run(
                command, cwd=cwd, env=env, stdout=log, stderr=subprocess.STDOUT,
                timeout=timeout, check=False,
            )
            return result.returncode, time.monotonic() - started, False
        except subprocess.TimeoutExpired:
            return 124, time.monotonic() - started, True


def parse_evaluation(log: Path) -> dict[str, object]:
    text = log.read_text(errors="replace")
    def last(pattern: str, cast):
        found = re.findall(pattern, text)
        return cast(found[-1]) if found else None
    return {
        "detected": last(r"Found mutations:\s*(\d+)", int),
        "mutants_without_timeout": last(
            r"Total mutations\(Without timeout file\):\s*(\d+)", int),
        "timeouts": last(r"Timeout mutations:\s*(\d+)", int),
        # evaluater.py excludes timed-out mutants from its denominator. Keep
        # that historical number as a diagnostic only; the experiment's MD
        # is always detected / fixed_mutants below.
        "reported_non_timeout_md_percent": last(
            r"Mutation Detection\(MD\) rate:\s*([0-9.]+)", float),
        "tool_errors": last(r"ebmc failed to run on\s+(\d+)", int) or 0,
    }


def evaluation_metrics(evaluation: dict[str, object], fixed_mutants: int) -> dict[str, object]:
    """Return the fixed-denominator MD and reasons this cell is invalid."""
    if not evaluation:
        return {
            "valid": False,
            "invalid_reasons": ["evaluation-missing"],
            "md_fixed_percent": None,
        }
    reasons = []
    if evaluation.get("exit") != 0:
        reasons.append(f"evaluation-exit-{evaluation.get('exit')}")
    if evaluation.get("timed_out"):
        reasons.append("evaluation-timed-out")
    tool_errors = evaluation.get("tool_errors")
    if not isinstance(tool_errors, int) or tool_errors > 0:
        reasons.append("evaluation-tool-errors")

    detected = evaluation.get("detected")
    without_timeout = evaluation.get("mutants_without_timeout")
    timeouts = evaluation.get("timeouts")
    if not isinstance(detected, int) or not 0 <= detected <= fixed_mutants:
        reasons.append("invalid-detected-count")
        main_md = None
    else:
        main_md = 100.0 * detected / fixed_mutants if fixed_mutants else None
    if (not isinstance(without_timeout, int) or not isinstance(timeouts, int)
            or without_timeout + timeouts != fixed_mutants):
        reasons.append("mutation-denominator-mismatch")
    return {
        "valid": not reasons,
        "invalid_reasons": reasons,
        "md_fixed_percent": main_md,
    }


def validate_start_record(records: list[dict[str, object]], mode: str) -> dict[str, object]:
    starts = [record for record in records if record.get("stage") == "start"]
    if len(starts) != 1:
        raise ValueError(f"expected exactly one start record, found {len(starts)}")
    if starts[0].get("bv_predicates") != mode:
        raise ValueError(
            f"start bv_predicates mismatch: {starts[0].get('bv_predicates')!r} != {mode!r}"
        )
    return starts[0]


def run_cell(manifest: dict[str, object], cell: dict[str, object], env: dict[str, str]) -> None:
    repo = Path(manifest["repo"])
    cell_dir = Path(cell["directory"])
    result_file = cell_dir / "result.json"
    if result_file.exists():
        print(f"skip completed cell {cell['design']} {cell['mode']}")
        return
    if cell_dir.exists():
        raise ValueError(f"refusing to overwrite partial cell: {cell_dir}")
    cell_dir.mkdir(parents=True)

    result: dict[str, object] = {
        "design": cell["design"], "mode": cell["mode"],
        "started_utc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "load_average": list(os.getloadavg()) if hasattr(os, "getloadavg") else None,
        "mine_command": cell["mine_command"],
    }
    mine_exit, mine_seconds, mine_timed_out = execute(
        cell["mine_command"], repo, cell_dir / "mine.log",
        int(manifest["mine_timeout"]), env,
    )
    result["mine"] = {
        "exit": mine_exit, "seconds": mine_seconds, "timed_out": mine_timed_out,
    }

    work = cell_dir / "work"
    invariants = work / "invariants.txt"
    if mine_exit == 0 and invariants.is_file():
        fixed = Path(manifest["mutant_sets"][cell["design"]]["path"])
        evaluation = cell_dir / "evaluation"
        (evaluation / "runtime").mkdir(parents=True)
        # evaluater.py needs no Python helpers unless benchmarks/ is empty.
        # Deliberately omit mutation.py, in addition to pre-populating and
        # checking benchmarks/, so this experiment cannot generate mutants.
        (evaluation / "src/python").mkdir(parents=True)
        shutil.copy2(repo / "smart/evaluater.py", evaluation)
        shutil.copy2(invariants, evaluation)
        if (work / "log.txt").is_file():
            shutil.copy2(work / "log.txt", evaluation)
        shutil.copytree(work / "runtime/verilog", evaluation / "runtime/verilog")
        shutil.copytree(fixed, evaluation / "benchmarks")
        copied = tree_identity(evaluation / "benchmarks")
        expected = manifest["mutant_sets"][cell["design"]]
        if copied != {key: expected[key] for key in ("files", "mutants", "sha256")}:
            raise ValueError(f"copied mutant set does not match manifest: {cell['design']}")
        eval_command = cell["eval_command"]
        result["eval_command"] = eval_command
        eval_exit, eval_seconds, eval_timed_out = execute(
            eval_command, evaluation, cell_dir / "evaluate.log",
            int(manifest["eval_timeout"]), env,
        )
        parsed = parse_evaluation(cell_dir / "evaluate.log")
        if not invariants.read_text().strip():
            parsed.update({
                "detected": 0, "mutants_without_timeout": cell["fixed_mutants"],
                "timeouts": 0, "reported_non_timeout_md_percent": 0.0,
            })
        result["evaluation"] = {
            "exit": eval_exit, "seconds": eval_seconds,
            "timed_out": eval_timed_out,
            **parsed,
        }
    result["finished_utc"] = dt.datetime.now(dt.timezone.utc).isoformat()
    result_file.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")


def run_experiment(args: argparse.Namespace) -> int:
    manifest_path = args.manifest.resolve()
    manifest = load_manifest(manifest_path)
    verify_frozen_inputs(manifest)
    env = tool_environment(Path(manifest["repo"]))
    print("running cells serially; do not start another A/B on this machine")
    for cell in manifest["cells"]:
        smart = manifest["smart_binary"]
        if sha256_file(Path(smart["path"])) != smart["sha256"]:
            raise ValueError("SMART binary changed between A/B cells")
        print(f"[{cell['design']} {cell['mode']}]")
        run_cell(manifest, cell, env)
    return summarise_manifest(manifest_path, quiet=False)


def selected_run(path: Path) -> tuple[object | None, list[dict[str, object]]]:
    groups: dict[object, list[dict[str, object]]] = collections.defaultdict(list)
    if not path.is_file():
        return None, []
    for line in path.read_text(errors="replace").splitlines():
        try:
            record = json.loads(line)
        except json.JSONDecodeError:
            continue
        groups[record.get("run")].append(record)
    complete = [(run, rows) for run, rows in groups.items()
                if any(row.get("stage") == "done" for row in rows)]
    if len(complete) > 1:
        raise ValueError(f"multiple completed run ids in {path}; refusing ambiguous timing")
    choices = complete or list(groups.items())
    if not choices:
        return None, []
    # A reused log can contain several timelines. Prefer a completed group;
    # within that class, use the group whose final record appeared latest.
    run, rows = max(choices, key=lambda item: max(float(r.get("t", 0)) for r in item[1]))
    return run, rows


def cvc5_timer(path: Path) -> tuple[int, float, float | None]:
    if not path.is_file():
        return 0, 0.0, None
    text = path.read_text(errors="replace")
    calls = sum(map(int, re.findall(r"CVC5 are called (\d+) times", text)))
    per_block = list(map(float, re.findall(r"CVC5 Timer: ([0-9.]+) seconds", text)))
    return calls, sum(per_block), statistics.median(per_block) if per_block else None


def comparison_count(work: Path) -> int:
    invariants_file = work / "invariants.txt"
    compare_file = work / "runtime/CompareResult.txt"
    if not invariants_file.is_file() or not compare_file.is_file():
        return 0
    invariants = set(invariants_file.read_text(errors="replace").splitlines())
    chunks: dict[str, list[str]] = collections.defaultdict(list)
    current = None
    for line in compare_file.read_text(errors="replace").splitlines():
        if line.endswith(":") and "define-fun" not in line:
            current = line[:-1]
        elif current is not None:
            chunks[current].append(line)
    token = r"(?:\|[^|]+\||[^()\s]+)"
    params_re = re.compile(rf"\(\s*({token})\s+\(_\s+BitVec\s+(\d+)\)\s*\)")
    equals_re = re.compile(rf"\(=\s+({token})\s+({token})\s*\)")
    count = 0
    for assertion in invariants:
        smt = " ".join(chunks.get(assertion, ()))
        is_compare = "(bvult " in smt or "(bvule " in smt
        widths = dict(params_re.findall(smt))
        if not is_compare:
            is_compare = any(a in widths and widths.get(a) == widths.get(b)
                             for a, b in equals_re.findall(smt))
        count += int(is_compare)
    return count


def cell_metrics(cell: dict[str, object]) -> dict[str, object]:
    directory = Path(cell["directory"])
    result_path = directory / "result.json"
    result = json.loads(result_path.read_text()) if result_path.is_file() else {}
    run, records = selected_run(directory / "work/run-log.jsonl")
    mine_succeeded = result.get("mine", {}).get("exit") == 0
    if mine_succeeded and (run is None or not records):
        raise ValueError(f"successful cell has no grouped run log: {cell['design']} {cell['mode']}")
    if records:
        try:
            validate_start_record(records, cell["mode"])
        except ValueError as error:
            raise ValueError(f"{cell['design']} {cell['mode']}: {error}") from error
    effective_path = directory / "work/effective-config.json"
    if mine_succeeded and not effective_path.is_file():
        raise ValueError(f"successful cell has no effective config: {cell['design']} {cell['mode']}")
    effective = json.loads(effective_path.read_text()) if effective_path.is_file() else {}
    if mine_succeeded and effective.get("bv_predicates") != cell["mode"]:
        raise ValueError(
            f"effective bv_predicates mismatch for {cell['design']} {cell['mode']}: "
            f"{effective.get('bv_predicates')!r}"
        )
    if mine_succeeded and effective.get("seed") != cell["seed"]:
        raise ValueError(
            f"effective seed mismatch for {cell['design']} {cell['mode']}: "
            f"{effective.get('seed')!r}"
        )
    blocks = [row for row in records if "block" in row]
    for block in blocks:
        missing = {"run", "reason", "seconds"} - block.keys()
        if missing:
            raise ValueError(
                f"block record missing {sorted(missing)} in {cell['design']} {cell['mode']}"
            )
    reasons = collections.Counter(str(row.get("reason", "unknown")) for row in blocks)
    done = next((row for row in reversed(records) if row.get("stage") == "done"), {})
    check = next((row for row in reversed(records) if row.get("stage") == "check"), {})
    if mine_succeeded and not done:
        raise ValueError(f"successful cell lacks done record: {cell['design']} {cell['mode']}")
    # Pipeline.cpp legitimately omits the check stage when there is nothing
    # to check. A non-empty final set without that record is corrupt evidence.
    assertions_path = directory / "work/assertions.txt"
    had_assertions = assertions_path.is_file() and bool(assertions_path.read_text().strip())
    if mine_succeeded and had_assertions and not check:
        raise ValueError(f"successful non-empty cell lacks check record: {cell['design']} {cell['mode']}")
    calls, cvc5_seconds, cvc5_median_block = cvc5_timer(directory / "work/log.txt")
    if mine_succeeded and blocks and calls == 0:
        raise ValueError(f"successful cell has blocks but no CVC5 timer data: {cell['design']} {cell['mode']}")
    evaluation = result.get("evaluation", {})
    evaluation_state = evaluation_metrics(evaluation, cell["fixed_mutants"])
    evaluated = evaluation.get("mutants_without_timeout")
    eval_timeouts = evaluation.get("timeouts")
    block_seconds = [float(row.get("seconds", 0)) for row in blocks]
    return {
        "design": cell["design"], "mode": cell["mode"], "run": run,
        "mine_exit": result.get("mine", {}).get("exit"),
        "mine_seconds": result.get("mine", {}).get("seconds"),
        "evaluation_valid": evaluation_state["valid"],
        "evaluation_invalid_reasons": evaluation_state["invalid_reasons"],
        "md_fixed_percent": evaluation_state["md_fixed_percent"],
        "evaluator_non_timeout_md_percent": evaluation.get(
            "reported_non_timeout_md_percent"
        ),
        "detected": evaluation.get("detected"),
        "fixed_mutants": cell["fixed_mutants"],
        "mutants_without_timeout": evaluated,
        "eval_timeouts": evaluation.get("timeouts"),
        "eval_tool_errors": evaluation.get("tool_errors"),
        "blocks": len(blocks), "candidate_blocks": reasons["verified"] + reasons["refuted"],
        "verified_blocks": reasons["verified"], "refuted_blocks": reasons["refuted"],
        "infeasible_blocks": reasons["infeasible"], "killed_blocks": reasons["killed"],
        "setup_failed_blocks": reasons["setup-failed"],
        "block_seconds_total": sum(block_seconds),
        "block_seconds_median": statistics.median(block_seconds) if block_seconds else None,
        "cvc5_calls": calls, "cvc5_seconds": cvc5_seconds,
        "cvc5_seconds_per_call": cvc5_seconds / calls if calls else None,
        "cvc5_seconds_median_block": cvc5_median_block,
        "verified_assertions": done.get("verified"),
        "comparison_assertions": comparison_count(directory / "work"),
        "check_refuted": check.get("refuted"), "check_errors": check.get("errors"),
        "check_timeouts": check.get("timed_out"),
    }


def stage_a_timing_gate(deltas: list[dict[str, object]], expected_pairs: int) -> dict[str, object]:
    ratios = [row["block_wall_median_ratio"] for row in deltas
              if row["block_wall_median_ratio"] is not None]
    complete = len(deltas) == expected_pairs and len(ratios) == expected_pairs
    median_ratio = statistics.median(ratios) if ratios else None
    return {
        "limit": 2.0,
        "complete": complete,
        "block_wall_median_ratio": median_ratio,
        "passed": median_ratio <= 2.0 if complete else None,
    }


def summarise_manifest(path: Path, quiet: bool) -> int:
    manifest = load_manifest(path)
    rows = [cell_metrics(cell) for cell in manifest["cells"]]
    root = path.parent
    fields = list(rows[0]) if rows else []
    with (root / "summary.csv").open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)

    paired = collections.defaultdict(dict)
    for row in rows:
        paired[row["design"]][row["mode"]] = row
    deltas = []
    invalid_pairs = []
    for design, modes in paired.items():
        if not all(mode in modes for mode in MODES):
            invalid_pairs.append({"design": design, "reasons": ["mode-missing"]})
            continue
        off, unsigned = modes["off"], modes["unsigned"]
        if not off["evaluation_valid"] or not unsigned["evaluation_valid"]:
            reasons = []
            for mode, row in (("off", off), ("unsigned", unsigned)):
                reasons.extend(f"{mode}:{reason}"
                               for reason in row["evaluation_invalid_reasons"])
            invalid_pairs.append({"design": design, "reasons": reasons})
            continue
        md_delta = unsigned["md_fixed_percent"] - off["md_fixed_percent"]
        deltas.append({
            "design": design, "md_delta_pp": md_delta,
            "mine_time_ratio": (unsigned["mine_seconds"] / off["mine_seconds"]
                                if off["mine_seconds"] and unsigned["mine_seconds"] else None),
            "cvc5_time_per_call_ratio": (
                unsigned["cvc5_seconds_per_call"] / off["cvc5_seconds_per_call"]
                if off["cvc5_seconds_per_call"] and unsigned["cvc5_seconds_per_call"] else None
            ),
            "cvc5_median_block_ratio": (
                unsigned["cvc5_seconds_median_block"] / off["cvc5_seconds_median_block"]
                if off["cvc5_seconds_median_block"] and unsigned["cvc5_seconds_median_block"] else None
            ),
            "block_wall_median_ratio": (
                unsigned["block_seconds_median"] / off["block_seconds_median"]
                if off["block_seconds_median"] and unsigned["block_seconds_median"] else None
            ),
            "infeasible_rate_delta_pp": (
                100 * unsigned["infeasible_blocks"] / unsigned["blocks"]
                - 100 * off["infeasible_blocks"] / off["blocks"]
                if off["blocks"] and unsigned["blocks"] else None
            ),
            "comparison_assertions": unsigned["comparison_assertions"],
        })
    md = [row["md_delta_pp"] for row in deltas if row["md_delta_pp"] is not None]
    valid_designs = {row["design"] for row in deltas}
    off_rows = [row for row in rows if row["mode"] == "off"
                and row["design"] in valid_designs]
    unsigned_rows = [row for row in rows if row["mode"] == "unsigned"
                     and row["design"] in valid_designs]
    off_runtime = sum(row["mine_seconds"] or 0 for row in off_rows)
    unsigned_runtime = sum(row["mine_seconds"] or 0 for row in unsigned_rows)
    off_blocks = sum(row["blocks"] for row in off_rows)
    unsigned_blocks = sum(row["blocks"] for row in unsigned_rows)
    off_infeasible = sum(row["infeasible_blocks"] for row in off_rows)
    unsigned_infeasible = sum(row["infeasible_blocks"] for row in unsigned_rows)
    cvc5_ratios = [row["cvc5_median_block_ratio"] for row in deltas
                   if row["cvc5_median_block_ratio"] is not None]
    block_wall_ratios = [row["block_wall_median_ratio"] for row in deltas
                         if row["block_wall_median_ratio"] is not None]
    expected_pairs = len(paired)
    timing_gate = stage_a_timing_gate(deltas, expected_pairs)
    aggregate = {
        "pairs_planned": expected_pairs,
        "pairs_valid": len(deltas),
        "pairs_invalid": len(invalid_pairs),
        "md_mean_delta_pp": statistics.mean(md) if md else None,
        "md_median_delta_pp": statistics.median(md) if md else None,
        "md_better": sum(value > 0.05 for value in md),
        "md_worse": sum(value < -0.05 for value in md),
        "md_unchanged": sum(abs(value) <= 0.05 for value in md),
        "mine_seconds_off": off_runtime,
        "mine_seconds_unsigned": unsigned_runtime,
        "mine_time_ratio": unsigned_runtime / off_runtime if off_runtime else None,
        "infeasible_rate_off": 100 * off_infeasible / off_blocks if off_blocks else None,
        "infeasible_rate_unsigned": (
            100 * unsigned_infeasible / unsigned_blocks if unsigned_blocks else None
        ),
        "infeasible_rate_delta_pp": (
            100 * unsigned_infeasible / unsigned_blocks - 100 * off_infeasible / off_blocks
            if off_blocks and unsigned_blocks else None
        ),
        "cvc5_median_block_ratio": statistics.median(cvc5_ratios) if cvc5_ratios else None,
        "block_wall_median_ratio": (
            statistics.median(block_wall_ratios) if block_wall_ratios else None
        ),
        "comparison_assertions_unsigned": sum(
            row["comparison_assertions"] for row in unsigned_rows
        ),
        "final_check_failures": sum(
            (row["check_refuted"] or 0) + (row["check_errors"] or 0) +
            (row["check_timeouts"] or 0)
            for row in rows
        ),
        "evaluation_tool_errors": sum(row["eval_tool_errors"] or 0 for row in rows),
    }
    stage_a_gate = timing_gate if manifest["stage"] == "A" else None
    (root / "summary.json").write_text(json.dumps(
        {"rows": rows, "deltas": deltas, "invalid_pairs": invalid_pairs,
         "aggregate": aggregate, "stage_a_2x_gate": stage_a_gate},
        indent=2, sort_keys=True,
    ) + "\n")
    if not quiet:
        print(f"{'design':<30} {'mode':<8} {'valid':<7} {'fixed MD%':>10} "
              f"{'inf/blk':>11} {'cmp':>5} {'CVC5 s':>9}")
        for row in rows:
            md_text = ("-" if row["md_fixed_percent"] is None
                       else f"{row['md_fixed_percent']:.2f}")
            valid_text = "yes" if row["evaluation_valid"] else "INVALID"
            print(f"{row['design']:<30} {row['mode']:<8} {valid_text:<7} {md_text:>10} "
                  f"{row['infeasible_blocks']:>4}/{row['blocks']:<6} "
                  f"{row['comparison_assertions']:>5} {row['cvc5_seconds']:>9.2f}")
        print(json.dumps(aggregate, sort_keys=True))
        if stage_a_gate is not None:
            print("Stage-A block-wall 2x gate: " + json.dumps(stage_a_gate, sort_keys=True))
        for invalid in invalid_pairs:
            print(f"INVALID PAIR {invalid['design']}: {', '.join(invalid['reasons'])}")
        print(f"wrote {root / 'summary.csv'} and {root / 'summary.json'}")
    all_cells_finished = all(Path(cell["directory"], "result.json").is_file()
                             for cell in manifest["cells"])
    if all_cells_finished and invalid_pairs:
        return 4
    if (all_cells_finished and stage_a_gate is not None and
            not stage_a_gate["complete"]):
        return 5
    if stage_a_gate is not None and stage_a_gate["passed"] is False:
        return 3
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    prep = commands.add_parser("prepare", help="validate inputs and write a manifest; run nothing")
    prep.add_argument("--stage", choices=("A", "B"), default="A")
    prep.add_argument("--design", action="append", help="override the stage's design list; repeatable")
    prep.add_argument("--repo", type=Path, default=REPO)
    prep.add_argument("--benchmarks", type=Path, default=REPO / "Benchmark")
    prep.add_argument("--mutants", type=Path, default=REPO / "MutationBenchmark")
    prep.add_argument("--config", type=Path, default=REPO / "Config/block_msa.json")
    prep.add_argument("--smart", type=Path, default=REPO / "build/smart")
    prep.add_argument("--output", type=Path, required=True)
    prep.add_argument("--seed", type=int, default=42)
    prep.add_argument("--jobs", type=int, default=16)
    prep.add_argument("--core-timeout", type=int, default=60)
    prep.add_argument("--mine-timeout", type=int, default=3000)
    prep.add_argument("--eval-timeout", type=int, default=3600)
    prep.add_argument("--eval-bound", type=int, default=10)

    runner = commands.add_parser("run", help="explicitly execute a prepared manifest, serially")
    runner.add_argument("manifest", type=Path)
    summary = commands.add_parser("summarise", help="aggregate existing cells by run id")
    summary.add_argument("manifest", type=Path)
    args = parser.parse_args()
    try:
        if args.command == "prepare":
            return prepare(args)
        if args.command == "run":
            return run_experiment(args)
        return summarise_manifest(args.manifest.resolve(), quiet=False)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"bv_ab.py: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
