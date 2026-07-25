import argparse
import os
import re
import sys
import shutil
import subprocess
import json
import stat
from pathlib import Path

ROOT = Path(__file__).resolve().parent

def bash(cmd):
    subprocess.run(["bash", "-c", cmd], check=True)

def load_env(script):
    script_path = Path(script)
    if not script_path.is_absolute():
        script_path = ROOT / script_path
    if not script_path.exists():
        print(f"Skipping missing environment script: {script_path}")
        return
    # Load environment variables. Use env -0 so multi-line values (e.g.
    # exported bash functions) don't get split into broken entries.
    command = f"source {script_path} && env -0"
    out = subprocess.run(["bash", "-c", command],
                         stdout=subprocess.PIPE, text=True).stdout
    for entry in out.split("\0"):
        key, sep, value = entry.partition("=")
        if sep and re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", key):
            os.environ[key] = value

def _on_rm_error(func, path, exc_info):
    path_obj = Path(path)
    try:
        current_mode = path_obj.stat().st_mode
        path_obj.chmod(current_mode | stat.S_IWUSR)
        func(path)
    except Exception:
        pass

def force_rmtree(path: Path):
    if path.exists():
        shutil.rmtree(path, ignore_errors=False, onerror=_on_rm_error)

def clean_smart():
    smart = ROOT / "smart"

    for pattern in ["*.txt", "*.sby", "*task", "*.log", "*.sl"]:
        for f in smart.glob(pattern):
            try:
                f.unlink()
            except FileNotFoundError:
                pass
    for path in [
        smart / "result",
        smart / "runtime",
        smart / "sim_build",
        smart / "src/python/__pycache__",
        smart / "build",
    ]:
        try:
            force_rmtree(path)
        except Exception:
            pass

    proc = subprocess.run(
        ["make", "all_clean"],
        cwd=smart,
        check=False,
    )
    if proc.returncode != 0:
        print("make all_clean failed, continuing with manual cleanup.")
        for extra in ["smart.out", "assertion.out", "sygus.sl"]:
            try:
                (smart / extra).unlink()
            except FileNotFoundError:
                pass
        (smart / "build").mkdir(exist_ok=True)

def load_config(config_path: Path):
    if not config_path.exists():
        sys.exit(f"Config file not found: {config_path}")
    with open(config_path) as f:
        try:
            return json.load(f)
        except json.JSONDecodeError as e:
            sys.exit(f"Invalid JSON in config {config_path}: {e}")

def find_benchmark_dirs(target):
    return [p for p in (ROOT / "Benchmark").rglob(target) if p.is_dir()]

def list_benchmarks():
    found = []
    for sim_py in sorted((ROOT / "Benchmark").rglob("sim.py")):
        bench_dir = sim_py.parent
        found.append((bench_dir.name, bench_dir.relative_to(ROOT)))
    if not found:
        print("No benchmarks found under Benchmark/.")
        return
    width = max(len(name) for name, _ in found)
    for name, rel in found:
        print(f"{name:<{width}}  {rel}")

def list_configs():
    for cfg in sorted((ROOT / "Config").glob("*.json")):
        print(cfg.relative_to(ROOT))

def check_env():
    load_env(ROOT / "otherTools/venv/bin/activate")
    load_env(ROOT / "otherTools/oss-cad-suite/environment")

    ok = True
    for tool in ["verilator", "ebmc", "cvc5", "yosys"]:
        path = shutil.which(tool)
        if path:
            version = ""
            try:
                out = subprocess.run([tool, "--version"], capture_output=True,
                                     text=True, timeout=10)
                version = (out.stdout or out.stderr).strip().splitlines()[0]
            except Exception:
                pass
            print(f"[ok]      {tool:<10} {path}  {version}")
        else:
            print(f"[MISSING] {tool:<10} not found on PATH")
            ok = False

    for module in ["cocotb", "z3"]:
        ret = subprocess.run([sys.executable, "-c", f"import {module}"],
                             capture_output=True)
        if ret.returncode == 0:
            print(f"[ok]      python module {module}")
        else:
            print(f"[MISSING] python module {module}")
            ok = False

    if ok:
        print("Environment looks complete.")
    else:
        print("Environment is incomplete. Use the Docker image or run install.sh.")
        sys.exit(1)

def validate_benchmark_dir(found_dir: Path, target):
    problems = []
    if not (found_dir / f"{target}.sv").exists():
        problems.append(f"missing {target}.sv (the top-level RTL file must be named after the benchmark)")
    if not (found_dir / "sim.py").exists():
        problems.append("missing sim.py (cocotb testbench driving the design)")
    if problems:
        print(f"Benchmark directory {found_dir} is not runnable:", file=sys.stderr)
        for p in problems:
            print(f"  - {p}", file=sys.stderr)
        print(f"  contents: {sorted(f.name for f in found_dir.iterdir())}", file=sys.stderr)
        sys.exit(1)

def run_experiment(target, Config):

    conf = Config.removeprefix('Config/').removesuffix('.json')
    result_name = f"{conf}_{target}"

    Config = (ROOT / Config).resolve() if not Path(Config).is_absolute() else Path(Config)
    # Read the experiment configuration
    config = load_config(Config)
    Workflow = config.get("Workflow", {})
    Minimizer = Workflow.get("Minimizer", False)
    Minimizer_settings = config.get("Minimizer_settings", {})
    LTL = Workflow.get("LTL", False)
    LTL_settings = config.get("LTL_settings", {})
    Checker = Workflow.get("Checker", True)
    Checker_settings = config.get("Checker_settings", {})
    Evaluation = Workflow.get("Evaluation", True)
    Evaluation_settings = config.get("Evaluation_settings", {})

    print(f"=== Running experiment: {target} ===")

    # 1. Find and validate the benchmark before touching any state
    mut_dir = ROOT / "MutationBenchmark"

    matches = find_benchmark_dirs(target)
    if not matches:
        print(f"Benchmark {target} not found under {ROOT / 'Benchmark'}.", file=sys.stderr)
        print("Use --list-benchmarks to see what is available.", file=sys.stderr)
        sys.exit(1)
    if len(matches) > 1:
        print(f"Warning: benchmark name '{target}' is ambiguous, using the first match:")
        for m in matches:
            print(f"  {m.relative_to(ROOT)}")
    found_dir = matches[0]
    found_mut = next((p for p in mut_dir.rglob(target) if p.is_dir()), None) if mut_dir.exists() else None

    print(f"Found benchmark: {found_dir}")
    validate_benchmark_dir(found_dir, target)

    # 2. Setup environment
    load_env(ROOT / "otherTools/venv/bin/activate")
    load_env(ROOT / "otherTools/oss-cad-suite/environment")

    # 3. Clean previous runs
    clean_smart()

    # copy mutation files if exist
    smart_dir = ROOT / "smart"
    if found_mut:
        print(f"Found mutation: {found_mut}")
        for item in found_mut.iterdir():
            if item.is_dir():
                shutil.copytree(item, smart_dir / item.name, dirs_exist_ok=True)
            else:
                shutil.copy(item, smart_dir)
    else:
        print("No mutation found.")

    # archive previous results instead of deleting them
    result_dir = ROOT / "Results" / result_name
    if result_dir.exists():
        prev_dir = result_dir.with_name(result_name + ".prev")
        force_rmtree(prev_dir)
        result_dir.rename(prev_dir)
        print(f"Previous results moved to {prev_dir.relative_to(ROOT)}")

    # copy benchmark files to smart/user
    dest_user = smart_dir / "user"
    shutil.rmtree(dest_user, ignore_errors=True)
    shutil.copytree(found_dir, dest_user)

    os.chdir(smart_dir)
    try:
        bash(f"python setup.py {target} {Config}")
        bash("rm -rf *task")

        if LTL == True:
            print("Running LTL synthesis...")
            LTL_depth = LTL_settings.get("LTL_depth", 1)
            for i in range(LTL_depth+1):
                print(f"LTL synthesis Latency {i}/{LTL_depth}...")
                bash(f"python smart.py {target} {Config} {i}")
        else:
            bash(f"python smart.py {target} {Config}")

        bash("rm -rf *.sby")
        bash("rm -rf *task")

        if Minimizer == True and Minimizer_settings.get("End_minimizer") == True:
            print("Running assertion minimization...")
            End_Minimizer_timeout = Minimizer_settings.get("End_Minimizer_timeout", 300)
            bash(f"python src/python/clean_assertion.py runtime/reducedResult.sl runtime/CompareResult.txt {End_Minimizer_timeout}")

        # Step 4: Check all assertions correctness on the original design
        if Checker == False:
            print("Skipping checker as per config.")
        else:
            if Checker_settings.get("Check_unbounded") == True:
                bash(f"python checker.py {target}")
            else:
                bound = Checker_settings.get("bounded_depth", 10)
                bash(f"python checker.py {target} {bound}")

        if Evaluation == False:
            print("Skipping evaluation as per config.")
        else:
            if Evaluation_settings.get("Check_unbounded") == True:
                bash(f"python evaluater.py {target}")
            else:
                bound = Evaluation_settings.get("bounded_depth", 10)
                bash(f"python evaluater.py {target} {bound}")
    finally:
        # get out of smart directory even if a stage failed
        os.chdir(ROOT)

    # save results
    print("Moving results...")
    result_dir.mkdir(parents=True, exist_ok=True)
    moved = 0
    for f in smart_dir.glob("*.txt"):
        shutil.move(str(f), result_dir / f.name)
        moved += 1
    if dest_user.exists():
        for item in dest_user.iterdir():
            shutil.move(str(item), result_dir / item.name)
            moved += 1
    if moved == 0:
        print("Warning: no result files were produced.", file=sys.stderr)

    # keep per-core smart.out logs for debugging
    logs_dir = smart_dir / "runtime" / "logs"
    if logs_dir.exists():
        shutil.copytree(logs_dir, result_dir / "logs", dirs_exist_ok=True)

    print(f"Done run.py: {result_name}")

def main():
    parser = argparse.ArgumentParser(
        description="Run SMART/MAPminer on a benchmark: mine SVA from simulation "
                    "traces, formally verify them, and (optionally) evaluate "
                    "mutation detection.")
    parser.add_argument("benchmark", nargs="?",
                        help="benchmark name (a directory under Benchmark/)")
    parser.add_argument("config", nargs="?", default="Config/smart.json",
                        help="config JSON (default: Config/smart.json)")
    parser.add_argument("--list-benchmarks", action="store_true",
                        help="list available benchmarks and exit")
    parser.add_argument("--list-configs", action="store_true",
                        help="list available config files and exit")
    parser.add_argument("--check-env", action="store_true",
                        help="check that required tools (verilator, ebmc, cvc5, ...) are available and exit")
    args = parser.parse_args()

    if args.list_benchmarks:
        list_benchmarks()
        return
    if args.list_configs:
        list_configs()
        return
    if args.check_env:
        check_env()
        return

    if not args.benchmark:
        parser.error("benchmark name is required (or use --list-benchmarks / --check-env)")

    run_experiment(args.benchmark, args.config)

if __name__ == "__main__":
    main()
