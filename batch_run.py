import argparse
import os
import subprocess
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
from queue import Queue, Empty


DEFAULT_PARALLEL = 1
DEFAULT_IMAGE = "magna2024/smart"
DEFAULT_CONFIG = "Config/default.json"
DEFAULT_BENCHMARKS = ["c17", "s27"]
FIX_BENCHMARK = False
WORK_FLOW = "md"


def run(cmd, check=True):
    print(f"[cmd] {cmd}")
    subprocess.run(cmd, shell=True, check=check)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Run smartVerilog experiments in Docker containers."
    )
    parser.add_argument(
        "--parallel",
        type=int,
        default=DEFAULT_PARALLEL,
        help="Number of containers to start in parallel (default: 1).",
    )
    parser.add_argument(
        "--image",
        default=DEFAULT_IMAGE,
        help="Docker image name to use (default: magna2024/smart).",
    )
    parser.add_argument(
        "--config",
        default=DEFAULT_CONFIG,
        help="Local config file path (default: Config/default.json).",
    )
    parser.add_argument(
        "--benchmarks",
        nargs="+",
        default=DEFAULT_BENCHMARKS,
        help="Explicit list of benchmarks to run (default: c17 s27).",
    )
    parser.add_argument(
        "--workflow",
        choices=["run", "md"],
        default=WORK_FLOW,
        help="Workflow to run: 'run' (run.py) or 'md' (md.py).",
    )
    parser.add_argument(
        "--fix-benchmark",
        dest="fix_benchmark",
        action="store_true",
        help="Clone MutationBenchmark inside container.",
    )
    parser.add_argument(
        "--no-fix-benchmark",
        dest="fix_benchmark",
        action="store_false",
        help="Skip cloning MutationBenchmark inside container.",
    )
    parser.set_defaults(fix_benchmark=FIX_BENCHMARK)
    parser.add_argument(
        "--workdir",
        default=str(Path.cwd()),
        help="Host project root (default: current directory).",
    )
    return parser.parse_args()


def ensure_results_dir(host_root: Path):
    results_dir = host_root / "Results"
    results_dir.mkdir(parents=True, exist_ok=True)
    return results_dir


def create_container(image: str, config_path: Path, host_results: Path):
    config_name = config_path.name
    container_name = f"smart-batch-{os.getpid()}-{os.urandom(3).hex()}"
    run(
        " ".join(
            [
                "docker run -d --rm",
                f"--name {container_name}",
                f"-v {host_results}:/workspace/smartVerilog/Results",
                f"{image}",
                "sleep infinity",
            ]
        )
    )
    run(f"docker exec {container_name} mkdir -p /workspace/smartVerilog/Config")
    run(
        " ".join(
            [
                "docker cp",
                str(config_path),
                f"{container_name}:/workspace/smartVerilog/Config/{config_name}",
            ]
        )
    )
    return container_name, config_name


def clone_mutation_benchmark(container_name: str):
    run(
        " ".join(
            [
                f"docker exec {container_name}",
                "bash -lc",
                '"cd /workspace/smartVerilog && git clone https://github.com/lichye/MutationBenchmark"',
            ]
        )
    )


def run_benchmark(container_name: str, benchmark: str, config_name: str, workflow: str):
    script = "run.py" if workflow == "run" else "md.py"
    run(
        " ".join(
            [
                f"docker exec {container_name}",
                "bash -lc",
                f'"cd /workspace/smartVerilog && python {script} {benchmark} Config/{config_name}"',
            ]
        )
    )


def collect_results(result_name: str):
    print(f"Results available at host Results/{result_name}")


def main():
    args = parse_args()
    host_root = Path(args.workdir).resolve()
    config_path = (host_root / args.config).resolve()
    if not config_path.exists():
        raise FileNotFoundError(f"Config not found: {config_path}")

    host_results = ensure_results_dir(host_root)
    parallel = max(1, args.parallel)
    work_queue = Queue()
    for benchmark in args.benchmarks:
        work_queue.put(benchmark)

    if args.workflow == "md":
        for benchmark in args.benchmarks:
            result_name = f"{Path(config_path.name).stem}_{benchmark}"
            if not (host_results / result_name).exists():
                raise FileNotFoundError(
                    f"Missing host Results/{result_name} for md workflow."
                )

    def worker(worker_id: int):
        container_name, config_name = create_container(
            args.image, config_path, host_results
        )
        try:
            if args.fix_benchmark:
                clone_mutation_benchmark(container_name)
            completed = 0
            while True:
                try:
                    benchmark = work_queue.get_nowait()
                except Empty:
                    break
                completed += 1
                remaining = work_queue.qsize()
                print(
                    f"=== [worker {worker_id}] Running {benchmark} "
                    f"(done {completed}, remaining {remaining}) ==="
                )
                run_benchmark(container_name, benchmark, config_name, args.workflow)
                result_name = f"{Path(config_name).stem}_{benchmark}"
                collect_results(result_name)
                print(f"[worker {worker_id}] Finished {benchmark}.")
                work_queue.task_done()
        finally:
            print(f"Stopping container {container_name}...")
            run(f"docker stop {container_name}", check=False)

    with ThreadPoolExecutor(max_workers=parallel) as executor:
        futures = [executor.submit(worker, i + 1) for i in range(parallel)]
        for future in as_completed(futures):
            future.result()


if __name__ == "__main__":
    main()