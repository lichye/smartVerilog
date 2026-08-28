#!/usr/bin/env python3
"""Experiment driver for the SMART benchmarks.

The tool itself is the `smart` binary; this script is the experiment layer
around it — pick a benchmark, optionally pick a config, run it, and file the
results under Results/. It used to orchestrate the pipeline (setup.py ->
smart.py -> checker.py); all of that now lives in the binary.

    python3 run.py c17                       # smart's built-in defaults
    python3 run.py c17 --config Config/block_msa.json
    python3 run.py --list-benchmarks
    python3 run.py --check-env
"""

import argparse
import os
import re
import shutil
import stat
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
# `Benchmark/` is the small, versioned corpus shipped with SMART.  The fixed
# mutation suite is deliberately a separate checkout so its generated mutants
# and experiment output never become part of the tool repository.
BENCH_DIRS = ["Benchmark", "MutationBenchmark"]


def smart_binary():
    """The tool. Built by `cmake --build build --target smart`."""
    env = os.environ.get("SMART_BIN")
    if env:
        return Path(env)
    for candidate in [ROOT / "build/smart", ROOT / "build/bin/smart"]:
        if candidate.exists():
            return candidate
    print("error: the smart binary is not built. Run:", file=sys.stderr)
    print("    cmake -B build -S . && cmake --build build --target smart",
          file=sys.stderr)
    sys.exit(2)


def load_env(script):
    """Source a shell script and adopt the environment it sets."""
    script_path = Path(script)
    if not script_path.is_absolute():
        script_path = ROOT / script_path
    if not script_path.exists():
        return
    out = subprocess.run(["bash", "-c", 'source "$1" && env -0',
                          "load-env", str(script_path)],
                         stdout=subprocess.PIPE, text=True).stdout
    for entry in out.split("\0"):
        key, sep, value = entry.partition("=")
        if sep and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", key):
            os.environ[key] = value


def setup_env():
    """Put the locally installed tools on PATH, ours ahead of the suite's."""
    load_env(ROOT / "otherTools/oss-cad-suite/environment")

    extra = []
    for path in [ROOT / ".deps/hw-cbmc/src/ebmc",        # EBMC we built
                 ROOT / "otherTools/cvc5/bin"]:          # cvc5 we built
        if path.exists():
            extra.append(str(path))
    # oss-cad-suite ships an old cvc5 and puts itself first on PATH; ours has
    # to win, because solver version changes what gets synthesised.
    if extra:
        os.environ["PATH"] = os.pathsep.join(extra + [os.environ.get("PATH", "")])


def find_benchmark(target):
    matches = []
    for base in BENCH_DIRS:
        base_dir = ROOT / base
        if not base_dir.exists():
            continue
        matches.extend(p for p in base_dir.rglob(target) if p.is_dir())
    if not matches:
        print(f"error: no benchmark named '{target}' under {', '.join(BENCH_DIRS)}",
              file=sys.stderr)
        sys.exit(1)
    if len(matches) > 1:
        print(f"warning: '{target}' is ambiguous, using {matches[0].relative_to(ROOT)}")
    return matches[0]


def list_benchmarks():
    found = set()
    for base in BENCH_DIRS:
        base_dir = ROOT / base
        if not base_dir.exists():
            continue
        for rtl in base_dir.rglob("*.sv"):
            if rtl.stem == rtl.parent.name:
                found.add((rtl.parent.name, str(rtl.parent.relative_to(ROOT))))
    if not found:
        print("No benchmarks found.")
        return
    width = max(len(name) for name, _ in found)
    for name, rel in sorted(found):
        print(f"{name:<{width}}  {rel}")


def list_configs():
    for cfg in sorted((ROOT / "Config").glob("*.json")):
        print(cfg.relative_to(ROOT))


def check_env():
    setup_env()
    # --check-env is the binary's own: it knows what it needs.
    return subprocess.run([str(smart_binary()), "--check-env"]).returncode


def force_rmtree(path):
    def on_error(func, target, _exc):
        try:
            target_path = Path(target)
            target_path.chmod(target_path.stat().st_mode | stat.S_IWUSR)
            func(target)
        except Exception:
            pass
    shutil.rmtree(path, onerror=on_error)


def run_experiment(target, config, keep_work, extra_args):
    bench_dir = find_benchmark(target)
    rtl = bench_dir / f"{target}.sv"
    if not rtl.exists():
        print(f"error: {rtl} not found (the top RTL must be named after the "
              f"benchmark directory)", file=sys.stderr)
        sys.exit(1)

    config_path = None
    if config is not None:
        config_path = Path(config)
        if not config_path.is_absolute():
            config_path = ROOT / config_path
        if not config_path.exists():
            print(f"error: config not found: {config_path}", file=sys.stderr)
            sys.exit(1)

    setup_env()
    binary = smart_binary()

    config_name = config_path.stem if config_path is not None else "default"
    result_name = f"{config_name}_{target}"
    result_dir = ROOT / "Results" / result_name
    if result_dir.exists():
        previous = result_dir.with_name(result_name + ".prev")
        force_rmtree(previous)
        result_dir.rename(previous)
        print(f"Previous results moved to {previous.relative_to(ROOT)}")
    result_dir.mkdir(parents=True)

    # Mutation evaluation needs the FIXED mutant set. mutation.py picks its
    # operators at random, so generating them per run gives the same count with
    # different content (383 mutants on c880, 294 of them different between two
    # runs) and no two runs are comparable. Clone the set with:
    #   git clone https://github.com/lichye/MutationBenchmark.git
    mutants = ROOT / "MutationBenchmark" / target / "benchmarks"
    if mutants.is_dir():
        print(f"Using the fixed mutants in {mutants.relative_to(ROOT)}")

    work_dir = result_dir / "work"
    command = [str(binary), str(rtl), "--top", target]
    if config_path is not None:
        command.extend(["--config", str(config_path)])
    command.extend([
        "--workdir", str(work_dir),
        "--output", str(result_dir / f"{target}_assertion.sv"),
        "--keep-work",
    ])
    command.extend(extra_args)

    config_description = (config_path.name if config_path is not None
                          else "smart built-in defaults")
    print(f"=== {target} with {config_description} ===")
    print("$ " + " ".join(command))
    completed = subprocess.run(command)

    # Hand the evaluator everything it needs, in the layout it expects.
    if mutants.is_dir():
        shutil.copytree(mutants, result_dir / "benchmarks", dirs_exist_ok=True)

    # Lift the interesting artefacts out of the workdir.
    for name in ["assertions.txt", "invariants.txt", "effective-config.json"]:
        source = work_dir / name
        if source.exists():
            shutil.copy(source, result_dir / name)
    if not keep_work and work_dir.exists():
        force_rmtree(work_dir)

    print(f"Results in {result_dir.relative_to(ROOT)}")
    return completed.returncode


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="Unrecognised arguments are passed straight through to `smart`.")
    parser.add_argument("benchmark", nargs="?", help="benchmark name, e.g. c17")
    parser.add_argument(
        "--config",
        help="experiment config (omit to use smart's built-in defaults)")
    parser.add_argument("--keep-work", action="store_true",
                        help="keep the run's working directory under Results/")
    parser.add_argument("--list-benchmarks", action="store_true")
    parser.add_argument("--list-configs", action="store_true")
    parser.add_argument("--check-env", action="store_true")
    args, extra = parser.parse_known_args()

    if args.list_benchmarks:
        list_benchmarks()
        return 0
    if args.list_configs:
        list_configs()
        return 0
    if args.check_env:
        return check_env()
    if not args.benchmark:
        parser.error("benchmark name is required "
                     "(or use --list-benchmarks / --check-env)")

    return run_experiment(args.benchmark, args.config, args.keep_work, extra)


if __name__ == "__main__":
    sys.exit(main())
