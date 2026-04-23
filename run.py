import os
import re
import sys
import shutil
import subprocess
import json
import stat
from pathlib import Path

ROOT = Path(__file__).resolve().parent

size_pat = re.compile('\w*?(\d+)')

def bash(cmd):
    subprocess.run(["bash", "-c", cmd], check=True)

def load_env(script):
    script_path = Path(script)
    if not script_path.is_absolute():
        script_path = ROOT / script_path
    if not script_path.exists():
        print(f"Skipping missing environment script: {script_path}")
        return
    # Load environment variables
    command = f"source {script_path} && env"
    proc = subprocess.Popen(["bash", "-c", command],
                            stdout=subprocess.PIPE, text=True)
    for line in proc.stdout:
        key, _, value = line.partition("=")
        os.environ[key] = value.strip()

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

def run_experiment(target,Config):

    conf = Config.removeprefix('Config/').removesuffix('.json')
    result_name = f"{conf}_{target}"

    Config = (ROOT / Config).resolve() if not Path(Config).is_absolute() else Path(Config)
    # Read the expriment configuration
    with open(Config) as f:
        config = json.load(f)
        Workflow = config.get("Workflow")
        Minimizer = Workflow.get("Minimizer")
        Minimizer_settings = config.get("Minimizer_settings")
        Log_result = Workflow.get("Log", False)
        LTL = Workflow.get("LTL")
        LTL_settings = config.get("LTL_settings")
        Checker = Workflow.get("Checker")
        Checker_settings = config.get("Checker_settings")
        Evaluation = Workflow.get("Evaluation")
        Evaluation_settings = config.get("Evaluation_settings")

    print(f"=== Running experiment: {target} ===")

    # 1. Setup environment
    load_env(ROOT / "otherTools/venv/bin/activate")
    load_env(ROOT / "otherTools/oss-cad-suite/environment")

    # 2. Clean previous runs
    clean_smart()

    src_bench = ROOT / "Benchmark"
    mut_dir = ROOT / "MutationBenchmark"

    # find benchmark and mutation
    found_dir = next(src_bench.rglob(target), None)
    found_mut = next(mut_dir.rglob(target), None)

    if not found_dir:
        print(f"Benchmark {target} not found.")
        return

    print(f"Found benchmark: {found_dir}")

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

    # delete previous results
    shutil.rmtree(ROOT / f"Results/{result_name}", ignore_errors=True)

    # copy benchmark files to smart/user
    dest_user = smart_dir / "user"
    shutil.rmtree(dest_user, ignore_errors=True)
    shutil.copytree(found_dir, dest_user)

    os.chdir(smart_dir)

    os.makedirs("user", exist_ok=True)

    bash(f"python setup.py {target} {Config}")
    bash("rm -rf *task")

    if LTL == True:
        print("Running LTL synthesis...")
        LTL_depth = LTL_settings.get("LTL_depth")
        LTL_timeout = LTL_settings.get("LTL_timeout")
        for i in range(LTL_depth+1):
            print(f"LTL synthesis Latency {i}/{LTL_depth}...")
            bash(f"python smart.py {target} {Config} {i}")
    else:
        bash(f"python smart.py {target} {Config}")


    bash("rm -rf *.sby")
    bash("rm -rf *task")


    if Minimizer == True and Minimizer_settings.get("End_minimizer") == True:
        print("Running assertion minimization...")
        End_Minimizer_timeout = Minimizer_settings.get("End_Minimizer_timeout")
        bash(f"python src/python/clean_assertion.py runtime/reducedResult.sl runtime/CompareResult.txt {End_Minimizer_timeout}")

    # Step 4: Check all assertions correctness on the original design
    if Workflow["Checker"] == False:
        print("Skipping checker as per config.")
    else:
        if Checker_settings.get("Check_unbounded") == True:
            bash(f"python checker.py {target}")
        else:
            bound = Checker_settings.get("bounded_depth")
            bash(f"python checker.py {target} {bound}")

    if Workflow["Evaluation"] == False:
        print("Skipping evaluation as per config.")
    else:
        if Evaluation_settings.get("Check_unbounded") == True:
            bash(f"python evaluater.py {target}")
        else:
            bound = Evaluation_settings.get("bounded_depth")
            bash(f"python evaluater.py {target} {bound}")
    
    # get out of smart directory
    os.chdir(ROOT)

    # save results
    print("Moving results...")
    os.makedirs(ROOT / f"Results/{result_name}", exist_ok=True)
    bash(f"mv {smart_dir}/*.txt {ROOT}/Results/{result_name}")
    bash(f"mv {smart_dir}/user/* {ROOT}/Results/{result_name}")

    print(f"Done run.py: {result_name}")

if __name__ == "__main__":
    ## Set up default config
    Config = "Config/smart.json"

    if len(sys.argv) < 2:
        print("Usage: python run.py <benchmark_name>")
        exit(1)
    elif len(sys.argv) == 3:
        Config = sys.argv[2]

    run_experiment(sys.argv[1],Config)
