import os
import sys
import shutil
import subprocess
import json
from pathlib import Path

ROOT = Path(__file__).parent

def bash(cmd):
    subprocess.run(["bash", "-c", cmd], check=True)

def load_env(script):
    # Load environment variables
    command = f"source {script} && env"
    proc = subprocess.Popen(["bash", "-c", command],
                            stdout=subprocess.PIPE, text=True)
    for line in proc.stdout:
        key, _, value = line.partition("=")
        os.environ[key] = value.strip()

def clean_smart():
    smart = Path("smart")

    for pattern in ["*.txt", "*.sby", "*task", "*.log", "*.sl"]:
        for f in smart.glob(pattern):
            f.unlink()
    shutil.rmtree(smart / "result", ignore_errors=True)
    shutil.rmtree(smart / "src/python/__pycache__", ignore_errors=True)
    subprocess.run(["bash", "-c", "cd smart && make all_clean"], check=True)

def run_experiment(target,Config):
    Config = Path(Config).resolve()
    # Read the expriment configuration
    with open(Config) as f:
        config = json.load(f)
        Workflow = config.get("Workflow")
        Minimizer = Workflow.get("Minimizer")
        Minimizer_settings = config.get("Minimizer_settings")
        Move_results = Workflow.get("Move_results", False)
        LTL = Workflow.get("LTL")
        LTL_settings = config.get("LTL_settings")

    print(f"=== Running experiment: {target} ===")

    # 1. Setup environment
    load_env("otherTools/venv/bin/activate")
    load_env("otherTools/oss-cad-suite/environment")

    # 2. Clean previous runs
    clean_smart()

    src_bench = Path("Benchmark")
    mut_dir = Path("MutationBenchmark")

    # find benchmark and mutation
    found_dir = next(src_bench.rglob(target), None)
    found_mut = next(mut_dir.rglob(target), None)

    if not found_dir:
        print(f"Benchmark {target} not found.")
        return

    print(f"Found benchmark: {found_dir}")

    # copy mutation files if exist
    smart_dir = Path("smart")
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
    shutil.rmtree(f"Results/{target}", ignore_errors=True)

    # make clean
    bash("cd smart && make all_clean")

    # copy benchmark files to smart/user
    dest_user = smart_dir / "user"
    shutil.rmtree(dest_user, ignore_errors=True)
    shutil.copytree(found_dir, dest_user)

    os.chdir("smart")

    os.makedirs("user", exist_ok=True)

    bash(f"python setup.py {target}")
    bash("rm -rf *task")

    if LTL == True:
        print("Running LTL synthesis...")
        LTL_depth = LTL_settings.get("LTL_depth")
        LTL_timeout = LTL_settings.get("LTL_timeout")
        for i in range(LTL_depth):
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
    
    if Workflow["Evaluation"] == False:
        print("Skipping evaluation as per config.")
        os.chdir("..")
    else:
        bash(f"python checker.py {target}")
        bash(f"python evaluater.py {target}")
        os.chdir("..")


    # save results
    if not Move_results:
        print("Skipping moving results as per config.")
        return
    else:
        print("Moving results...")
        os.makedirs(f"Results/{target}", exist_ok=True)
        bash(f"mv smart/*.txt Results/{target}")
        bash(f"mv smart/user/* Results/{target}")

    print(f"Done: {target}")

if __name__ == "__main__":
    ## Set up default config
    Config = "Config/basic.json"

    if len(sys.argv) < 2:
        print("Usage: python run.py <benchmark_name>")
        exit(1)
    
    run_experiment(sys.argv[1],Config)
