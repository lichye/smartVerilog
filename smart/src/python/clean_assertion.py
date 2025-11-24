import re
import sys

from minimise_assertions import run_minimisation

def load_reduced_results(filename):
    results = set()
    with open(filename, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line:
                results.add(line)
    return results

def extract_define_funs(filename):
    entries = set()
    with open(filename, "r", encoding="utf-8") as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line.startswith("(") and line.endswith("):"):
            sva_expr = line[:-2].strip()
            j = i + 1
            body_lines = []
            while j < len(lines) and not lines[j].strip() == ")":
                body_lines.append(lines[j].strip())
                j += 1
            block = " ".join(body_lines)

            # Match "Bool <expr>" where <expr> can have nested parentheses
            expr_match = re.search(r"Bool\s+(.+)\)\s*$", block)
            if expr_match:
                expr = expr_match.group(1).strip()
                entries.add((sva_expr+")", expr))
            i = j
        elif line.endswith(":"):
            sva_expr = line[:-1].strip()
            j = i + 1
            body_lines = []
            while j < len(lines) and not lines[j].strip() == ")":
                body_lines.append(lines[j].strip())
                j += 1
            block = " ".join(body_lines)
            
            # Match "Bool <expr>" where <expr> can have nested parentheses
            expr_match = re.search(r"Bool\s+(.+)\)\s*$", block)
            if expr_match:
                expr = expr_match.group(1).strip()
                entries.add((sva_expr, expr))
            i = j

        i += 1

    return entries

def filter_entries(entries, reduced_results):
    filtered = [
        (sva, expr)
        for sva, expr in entries
        if any(expr in r for r in reduced_results)
    ]
    return filtered

def write_exprs_to_file(entries, output_file="assertions.txt"):
    svas = {sva for sva, _ in entries}  # deduplicate by expr only
    with open(output_file, "w", encoding="utf-8") as f:
        for sva in sorted(svas):
            f.write(sva + "\n")

if __name__ == "__main__":
    if(len(sys.argv)==4):
        reduced_file = sys.argv[1]
        sygus_file = sys.argv[2]
        timeout = int(sys.argv[3])
    else:
        reduced_file = "runtime/reducedResult.sl"
        sygus_file = "runtime/CompareResult.txt"
        timeout = 300

    run_minimisation(sygus_file, reduced_file, timeout=300)
    reduced_results = load_reduced_results(reduced_file)
    entries = extract_define_funs(sygus_file)
    entries = filter_entries(entries, reduced_results)

    write_exprs_to_file(entries, "assertions.txt")
    exit(0)
    print(f"Wrote {len(entries)} assertions to assertions.txt")
