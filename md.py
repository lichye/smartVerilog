import os
import re
import sys
import shutil
import subprocess
import json
from pathlib import Path

ROOT = Path(__file__).parent

size_pat = re.compile('\w*?(\d+)')

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

    conf = Config.removeprefix('Config/').removesuffix('.json')
    result_name = f"{conf}_{target}"

    Config = Path(Config).resolve()
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
    load_env("otherTools/venv/bin/activate")
    load_env("otherTools/oss-cad-suite/environment")

    # 2. clean smart directory
    clean_smart()
    
    src_bench = Path("Benchmark")
    mut_dir = Path("MutationBenchmark")

    # find benchmark and mutation
    found_dir = next(src_bench.rglob(target), None)
    result_dir = f"Results/{result_name}"
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

    # copy benchmark files to smart/user
    dest_user = smart_dir / "user"
    shutil.rmtree(dest_user, ignore_errors=True)
    shutil.copytree(found_dir, dest_user)

    os.chdir("smart")

    os.makedirs("user", exist_ok=True)

    bash(f"python setup.py {target} {Config}")
    bash("rm -rf *task")

    os.chdir("..")
    ## Step3: copy result from Results\{result_name} to smart/
    bash(f"cp {result_dir}/*.txt smart/")

    os.chdir("smart")

    # Step 4: Check all assertions correctness on the original design
    if Checker_settings.get("Check_unbounded") == True:
        bash(f"python checker.py {target}")
    else:
        bound = Checker_settings.get("bounded_depth")
        bash(f"python checker.py {target} {bound}")

 
    if Evaluation_settings.get("Check_unbounded") == True:
        bash(f"python evaluater.py {target}")
    else:
        bound = Evaluation_settings.get("bounded_depth")
        bash(f"python evaluater.py {target} {bound}")
    
    os.chdir("..")

    # # save results
    # if not Log_result:
    #     print("Skipping moving results as per config.")
    #     return
    # else:
    print("Moving results...")
    os.makedirs(f"Results/{result_name}", exist_ok=True)
    bash(f"mv smart/*.txt Results/{result_name}")
    bash(f"mv smart/user/* Results/{result_name}")

    print(f"Done md.py: {result_name}")

if __name__ == "__main__":
    ## Set up default config
    Config = "Config/default.json"

    if len(sys.argv) < 2:
        print("Usage: python run.py <benchmark_name>")
        exit(1)
    elif len(sys.argv) == 3:
        Config = sys.argv[2]

    run_experiment(sys.argv[1],Config)
