import re
from pathlib import Path
from typing import Dict, Any, List, Optional
import matplotlib.pyplot as plt
import numpy as np


def parse_experiment_log(file_path: str) -> Dict[str, Any]:
    """
    Parse one experiment log file and return metrics as a dict.
    Repeated fields -> keep the last occurrence.
    """
    text = Path(file_path).read_text(encoding="utf-8", errors="ignore")

    key = {
        "total_runtime": r"Total runtime:\s*([0-9]*\.?[0-9]+)",
        "pre_analysis_time": r"Pre analysis Time:\s*([0-9]*\.?[0-9]+)",
        "total_smart_time": r"Total Smart Time:\s*([0-9]*\.?[0-9]+)",
        "total_mus_time": r"Total MUS Time:\s*([0-9]*\.?[0-9]+)",
        "total_minimizer_time": r"Total Minimizer Time:\s*([0-9]*\.?[0-9]+)",

        "assertions_found": r"We found\s+(\d+)\s+assertions",
        "verified_assertions": r"The number of verified assertions is:\s*(\d+)",
        "num_assertions": r"The number of assertions is:\s*(\d+)",
        "vc_rate": r"The Verified correctness\(VC\) rate is:\s*([0-9]*\.?[0-9]+)",

        "fm_calls": r"FM Checker Total Calls:\s*(\d+)",
        "fm_time": r"FM Checker Total Time:\s*([0-9]*\.?[0-9]+)\s*seconds?",

        "cvc5_calls": r"CVC5 Total Calls:\s*(\d+)",
        "cvc5_time": r"CVC5 Total Time:\s*([0-9]*\.?[0-9]+)\s*seconds?",

        "found_mutations": r"Found mutations:\s*(\d+)",
        "total_mutations_wo_timeout": r"Total mutations\(Without timeout file\):\s*(\d+)",
        "md_rate": r"The Mutation Detection\(MD\) rate:\s*([0-9]*\.?[0-9]+)",
    }

    int_fields = {
        "assertions_found",
        "verified_assertions",
        "num_assertions",
        "fm_calls",
        "cvc5_calls",
        "found_mutations",
        "total_mutations_wo_timeout",
    }

    float_fields = set(key.keys()) - int_fields

    result: Dict[str, Any] = {}

    for key, pattern in key.items():
        matches = re.findall(pattern, text)
        if not matches:
            continue

        val = matches[-1]  # take the last occurrence
        if key in int_fields:
            result[key] = int(val)
        else:
            result[key] = float(val)

    # Optional derived values (in case rate is missing)
    if "vc_rate" not in result and \
       "verified_assertions" in result and "num_assertions" in result:
        n = result["num_assertions"]
        result["vc_rate"] = 100.0 * result["verified_assertions"] / n if n else 0.0

    if "md_rate" not in result and \
       "found_mutations" in result and "total_mutations_wo_timeout" in result:
        n = result["total_mutations_wo_timeout"]
        result["md_rate"] = 100.0 * result["found_mutations"] / n if n else 0.0

    return result

def find_result_file(results_root: str, config: str, benchmark: str) -> Optional[Path]:
    base = Path(results_root)
    exp_dir = f"{config}_{benchmark}"
    file_name = f"result_{benchmark}.txt"

    candidates = [
        base / exp_dir / file_name,
    ]

    for p in candidates:
        if p.exists() and p.is_file():
            return p

    print(f"[MISSING] config={config}, bench={benchmark}")
    return None

def collect_results(
    results_root: str,
    config_list: List[str],
    benchmark_list: List[str],
) -> Dict[str, Dict[str, Optional[Dict[str, Any]]]]:
    """
    Return:
      results[config][benchmark] = metrics dict | None
    """
    results: Dict[str, Dict[str, Optional[Dict[str, Any]]]] = {}
    print("Collecting results...")
    for config in config_list:
        results[config] = {}
        for bench in benchmark_list:
            fp = find_result_file(results_root, config, bench)
            if fp is None:
                results[config][bench] = None
                continue

            try:
                metrics = parse_experiment_log(str(fp))
                results[config][bench] = metrics
            except Exception as e:
                # parsing error is still a result, but marked
                results[config][bench] = {
                    "status": "parse_error",
                    "error": f"{type(e).__name__}: {e}",
                }

    return results

def get_config_list(results_root: str = "Results") -> List[str]:
    """
    Auto-detect configs from Results/{config}_{benchmark}/result_{benchmark}.*
    """
    base = Path(results_root)
    configs: Set[str] = set()

    if not base.exists():
        return []

    for exp_dir in base.iterdir():
        if not exp_dir.is_dir():
            continue

        # find result files
        for f in exp_dir.iterdir():
            name = f.name
            if not (name.startswith("result_") and (name.endswith(".txt") or name.endswith(".out") or name.endswith(".log"))):
                continue

            bench = name[len("result_"):].rsplit(".", 1)[0]  # result_{bench}.ext -> bench
            suffix = f"_{bench}"
            dir_name = exp_dir.name

            if dir_name.endswith(suffix):
                config = dir_name[:-len(suffix)]
                if config:
                    configs.add(config)

    return sorted(configs)

def get_benchmark_list(results_root: str = "Results", config_list: List[str] | None = None) -> List[str]:
    """
    Auto-detect benchmarks from Results/{config}_{benchmark}/result_{benchmark}.*
    If config_list is provided, only count those configs.
    """
    base = Path(results_root)
    benches: Set[str] = set()

    if not base.exists():
        return []

    allowed = set(config_list) if config_list else None

    for exp_dir in base.iterdir():
        if not exp_dir.is_dir():
            continue

        # determine config candidate by using the file-derived bench, so we can strip safely even when config has underscores
        for f in exp_dir.iterdir():
            name = f.name
            if not (name.startswith("result_") and (name.endswith(".txt") or name.endswith(".out") or name.endswith(".log"))):
                continue

            bench = name[len("result_"):].rsplit(".", 1)[0]
            suffix = f"_{bench}"
            dir_name = exp_dir.name

            if not dir_name.endswith(suffix):
                continue

            config = dir_name[:-len(suffix)]
            if allowed is not None and config not in allowed:
                continue

            benches.add(bench)

    return sorted(benches)

def get_all_results(results_root: str = "Results") -> Dict[str, Dict[str, Optional[Dict[str, Any]]]]:
    config_list = get_config_list(results_root=results_root)
    benchmark_list = get_benchmark_list(
        results_root=results_root,
        config_list=config_list,
    )

    results = collect_results(
        results_root=results_root,
        config_list=config_list,
        benchmark_list=benchmark_list,
    )

    return results

def collect_metric_by_config(results, config_list, benchmark_list, metric_key):
    """
    Return:
      data[config] = [metric_value_for_each_benchmark]
    Missing -> 0
    """
    data = {}

    for config in config_list:
        values = []
        for bench in benchmark_list:
            d = results.get(config, {}).get(bench)
            if d is None:
                values.append(0.0)
            else:
                v = d.get(metric_key, 0.0)
                values.append(float(v) if v is not None else 0.0)
        data[config] = values

    return data

def plot_metric_bar(
    results,
    config_list,
    benchmark_list,
    metric_key,
    ylabel=None,
    title=None,
    out_file=None,
):
    """
    Generic bar plot for any metric.
    """
    data = collect_metric_by_config(
        results, config_list, benchmark_list, metric_key
    )

    x = np.arange(len(benchmark_list))
    width = 0.8 / len(config_list)

    plt.figure(figsize=(10, 4))

    for i, config in enumerate(config_list):
        plt.bar(
            x + i * width,
            data[config],
            width=width,
            label=config,
        )

    plt.xticks(x + width * (len(config_list) - 1) / 2, benchmark_list)
    plt.xlabel("Benchmark")
    plt.ylabel(ylabel if ylabel else metric_key)
    plt.title(title if title else metric_key)
    plt.legend()

    # heuristic y-limits
    if metric_key.endswith("_rate"):
        plt.ylim(0, 100)

    plt.tight_layout()

    if out_file:
        plt.savefig(out_file, dpi=300)
        plt.close()
    else:
        plt.show()

def writeCSV_flat(
    results,
    config_list,
    benchmark_list,
    key_list,
    out_file,
    float_fmt=".2f",
):
    """
    Write flat CSV:
      one row per (config, benchmark)

    float_fmt:
      ".2f", ".3f", etc.
      None -> no formatting
    """
    with open(out_file, "w", encoding="utf-8") as f:
        header = ["config", "benchmark"] + key_list
        f.write(",".join(header) + "\n")

        for config in config_list:
            for bench in benchmark_list:
                d = results.get(config, {}).get(bench)
                row = [config, bench]

                for k in key_list:
                    if d is None:
                        row.append("0")
                        continue

                    v = d.get(k, 0)
                    if v is None:
                        row.append("0")
                    elif isinstance(v, (int, float)) and float_fmt is not None:
                        row.append(format(v, float_fmt))
                    else:
                        row.append(str(v))

                f.write(",".join(row) + "\n")

def writeCSV_by_benchmark_config(
    results,
    config_list,
    benchmark_list,
    key_list,
    out_file,
    precision: int = 2,
):
    """
    Output tidy CSV:

    benchmark,config,<key1>,<key2>,...

    - key_list is preserved and controls columns
    - all numeric values are formatted with fixed precision
    - missing -> 0.00
    """
    fmt = f"{{:.{precision}f}}"

    def fnum(x) -> str:
        try:
            return fmt.format(float(x))
        except Exception:
            return fmt.format(0.0)

    with open(out_file, "w", encoding="utf-8") as f:
        header = ["benchmark", "config"] + key_list
        f.write(",".join(header) + "\n")

        for bench in benchmark_list:
            for config in config_list:
                d = results.get(config, {}).get(bench)

                row = [bench, config]

                for k in key_list:
                    if d is None or (isinstance(d, dict) and d.get("status") == "parse_error"):
                        row.append(fnum(0.0))
                    else:
                        v = d.get(k, 0.0)
                        row.append(fnum(v))

                f.write(",".join(row) + "\n")

def main():
    config_list = get_config_list("Results")
    benchmark_list = get_benchmark_list("Results", config_list)

    results = collect_results("Results", config_list, benchmark_list)

    plot_metric_bar(
        results,
        config_list,
        benchmark_list,
        metric_key="md_rate",
        ylabel="MD rate (%)",
        title="Mutation Detection Rate by Benchmark",
        out_file="md_rate.png",
    )

    key_list = ["md_rate"]
    writeCSV_by_benchmark_config(
        results,
        config_list,
        benchmark_list,
        key_list,
        out_file="md_rate_flat.csv",
        precision=2,
    )

if __name__ == "__main__":
    main()