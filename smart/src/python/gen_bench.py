"""Generate a cocotb sim.py (and an assume-free sim copy) from a single
SystemVerilog module.

The user provides one RTL file that may contain immediate `assume(...)`
statements and `(* anyseq *)` / `(* anyconst *)` free registers (formal-side
environment constraints). This module derives everything the simulation flow
needs from it:

  - parse_module(): ports (both ANSI and non-ANSI headers), parameters,
    free registers, clock/reset guesses
  - strip_assumes(): a copy of the RTL with assume statements removed, for
    the simulator (stimulus must explore freely; assumes only constrain the
    formal side)
  - render_sim_py(): a cocotb testbench that drives every input and free
    register with width-aware random values

Used by run.py when a benchmark directory has no hand-written sim.py.
Can also be run standalone:  python gen_bench.py <bench_dir> [top] [--force]
"""

import json
import os
import re
import sys

CLOCK_NAMES = ["clk", "clock", "clk_i", "i_clk"]
RESET_NAMES = ["rst", "reset", "rst_i", "i_rst", "rst_n", "rst_ni", "reset_n", "resetn"]

COMMENT_RE = re.compile(r"//[^\n]*|/\*.*?\*/", re.DOTALL)


def _strip_comments(text):
    return COMMENT_RE.sub(lambda m: " " if "\n" not in m.group(0) else
                          "\n" * m.group(0).count("\n"), text)


def _find_module(text, top):
    pattern = re.compile(r"\bmodule\s+" + re.escape(top) + r"\b.*?\bendmodule\b",
                         re.DOTALL)
    match = pattern.search(text)
    if not match:
        raise ValueError(f"module '{top}' not found")
    return match.group(0)


def _parse_parameters(module_text):
    params = {}
    for m in re.finditer(r"\b(?:parameter|localparam)\b[^=;,)]*?(\w+)\s*=\s*([^,;)\n]+)",
                         module_text):
        name, expr = m.group(1), m.group(2).strip()
        value = _safe_eval(expr, params)
        if value is not None:
            params[name] = value
    return params


def _safe_eval(expr, params):
    expr = expr.strip()
    # Verilog sized literals like 8'd255 / 4'b1010 / 16'hFF
    m = re.fullmatch(r"\d+'([bodh])([0-9a-fA-F_xz?]+)", expr)
    if m:
        base = {"b": 2, "o": 8, "d": 10, "h": 16}[m.group(1)]
        digits = m.group(2).replace("_", "")
        try:
            return int(digits, base)
        except ValueError:
            return None
    for name, value in params.items():
        expr = re.sub(r"\b" + re.escape(name) + r"\b", str(value), expr)
    if not re.fullmatch(r"[\d\s()+\-*/%]+", expr):
        return None
    try:
        return int(eval(expr, {"__builtins__": {}}, {}))
    except Exception:
        return None


def _range_width(range_text, params):
    if not range_text:
        return 1
    m = re.fullmatch(r"\s*\[(.+):(.+)\]\s*", range_text)
    if not m:
        return None
    msb = _safe_eval(m.group(1), params)
    lsb = _safe_eval(m.group(2), params)
    if msb is None or lsb is None:
        return None
    return abs(msb - lsb) + 1


def _header_and_body(module_text):
    """Split module text into the port header (inside the parens after the
    module name, skipping a #(...) parameter list) and the rest."""
    # skip "module name" and optional #( ... )
    m = re.match(r"\s*module\s+\w+\s*", module_text)
    pos = m.end()
    if module_text[pos:].lstrip().startswith("#"):
        pos = module_text.index("#", pos)
        pos = module_text.index("(", pos)
        depth = 1
        pos += 1
        while depth:
            if module_text[pos] == "(":
                depth += 1
            elif module_text[pos] == ")":
                depth -= 1
            pos += 1
    open_paren = module_text.find("(", pos)
    if open_paren == -1:
        return "", module_text
    depth, i = 1, open_paren + 1
    while depth:
        if module_text[i] == "(":
            depth += 1
        elif module_text[i] == ")":
            depth -= 1
        i += 1
    return module_text[open_paren + 1:i - 1], module_text[i:]


DIR_RE = re.compile(
    r"\b(input|output|inout)\b"
    r"(?:\s+(?:wire|reg|logic|bit|var)\b)*"  # \b: don't eat the "reg" prefix
    r"(?:\s+signed\b|\s+unsigned\b)?"        # of names like regfile_we_o
    r"\s*((?:\[[^\]]+\]\s*)*)"
    r"\s*(\w+(?:\s*,\s*(?!input\b|output\b|inout\b)\w+)*)")


def _parse_port_decls(text, params):
    """Extract (direction, width, name) from input/output/inout declarations
    in `text` (works on both ANSI headers and non-ANSI bodies)."""
    keywords = {"input", "output", "inout", "wire", "reg", "logic", "bit",
                "var", "signed", "unsigned"}
    ports = []
    for m in DIR_RE.finditer(text):
        direction = m.group(1)
        ranges = m.group(2) or ""
        first_range = re.match(r"\s*(\[[^\]]+\])", ranges)
        width = _range_width(first_range.group(1) if first_range else "", params)
        for name in re.split(r"\s*,\s*", m.group(3)):
            name = name.strip()
            if name in keywords:
                # ANSI headers separate ports with commas too; the next
                # declaration's keyword marks the end of this name list
                break
            if name:
                ports.append({"dir": direction, "width": width, "name": name})
    return ports


FREE_REG_RE = re.compile(
    r"\(\*\s*(anyseq|anyconst)\s*\*\)\s*"
    r"(?:reg|logic|wire|bit)\b\s*"
    r"(?:signed\s+|unsigned\s+)?"
    r"((?:\[[^\]]+\]\s*)?)"
    r"(\w+)")


def parse_module(text, top):
    """Return a dict describing the top module: ports, free regs, params."""
    clean = _strip_comments(text)
    module_text = _find_module(clean, top)
    params = _parse_parameters(module_text)
    header, body = _header_and_body(module_text)

    ports = _parse_port_decls(header, params)
    if not ports:
        # non-ANSI style: directions declared in the body
        ports = _parse_port_decls(body, params)

    free_regs = []
    for m in FREE_REG_RE.finditer(module_text):
        width = _range_width(m.group(2).strip(), params)
        free_regs.append({"kind": m.group(1), "width": width, "name": m.group(3)})

    return {
        "top": top,
        "params": params,
        "ports": ports,
        "inputs": [p for p in ports if p["dir"] == "input"],
        "free_regs": free_regs,
        "has_assume": bool(re.search(r"\bassume\b", module_text)),
    }


def strip_assumes(text):
    """Remove immediate `assume(...);` and `assume property (...);`
    statements (formal-only constraints) so the design simulates freely."""
    out = []
    i = 0
    pattern = re.compile(r"\bassume\b(\s+property\b)?\s*\(")
    while True:
        m = pattern.search(text, i)
        if not m:
            out.append(text[i:])
            break
        out.append(text[i:m.start()])
        depth, j = 1, m.end()
        while j < len(text) and depth:
            if text[j] == "(":
                depth += 1
            elif text[j] == ")":
                depth -= 1
            j += 1
        while j < len(text) and text[j] in " \t\n":
            j += 1
        if j < len(text) and text[j] == ";":
            j += 1
        out.append(";")  # keep a null statement so `if (...) assume(x);` stays legal
        i = j
    return "".join(out)


def guess_clock(info, configured=None):
    names = {p["name"] for p in info["inputs"]}
    if configured is not None:
        return configured if configured in names or configured else None
    for cand in CLOCK_NAMES:
        if cand in names:
            return cand
    return None


def guess_reset(info, clock, configured=None):
    if configured is not None:
        return configured
    names = {p["name"] for p in info["inputs"]}
    for cand in RESET_NAMES:
        if cand in names and cand != clock:
            active = 0 if cand.rstrip("i").endswith("n") else 1
            return {"signal": cand, "active": active, "cycles": 2}
    return None


def _spec_for(port, overrides):
    override = overrides.get(port["name"], {})
    if "values" in override:
        return {"values": override["values"]}
    if "const" in override:
        return {"const": override["const"]}
    width = port["width"] if port["width"] else 1
    max_value = override.get("max", (1 << width) - 1)
    return {"max": max_value}


def render_sim_py(info, cfg=None):
    cfg = cfg or {}
    top = info["top"]
    clock = guess_clock(info, cfg.get("clock"))
    reset = guess_reset(info, clock, cfg.get("reset")) if clock else None
    cycles = cfg.get("cycles", 10)
    seed = cfg.get("seed", 42)
    overrides = cfg.get("inputs", {})

    reserved = {clock} | ({reset["signal"]} if reset else set())
    drive_inputs = [p for p in info["inputs"] if p["name"] not in reserved]
    anyseq = [r for r in info["free_regs"] if r["kind"] == "anyseq"]
    anyconst = [r for r in info["free_regs"] if r["kind"] == "anyconst"]

    def spec_entries(ports):
        return ",\n".join(
            f'    ("{p["name"]}", {json.dumps(_spec_for(p, overrides))})'
            for p in ports)

    build_args = ['"--trace"', '"-Wno-WIDTHEXPAND"', '"-Wno-WIDTHTRUNC"',
                  '"-Wno-UNOPTFLAT"', '"-Wno-CASEOVERLAP"']
    if info["free_regs"]:
        build_args.append('"--public-flat-rw"')

    lines = []
    w = lines.append
    w("# Auto-generated by gen_bench.py -- regenerated on every run; do not edit.")
    w(f"# Design: {top}  clock: {clock or 'none (combinational)'}"
      f"  reset: {reset['signal'] if reset else 'none'}")
    w("import glob")
    w("import os")
    w("import random")
    w("")
    w("import cocotb")
    w("from cocotb.runner import get_runner")
    w("from cocotb.triggers import Timer")
    if clock:
        w("from cocotb.clock import Clock")
        w("from cocotb.triggers import RisingEdge")
    w("")
    w(f"SEED = {seed}")
    w(f"CYCLES = {cycles}")
    w("")
    w("INPUTS = [")
    w(spec_entries(drive_inputs))
    w("]")
    w("ANYSEQ = [")
    w(spec_entries(anyseq))
    w("]")
    w("ANYCONST = [")
    w(spec_entries(anyconst))
    w("]")
    w("")
    w("def draw(spec):")
    w('    if "const" in spec:')
    w('        return spec["const"]')
    w('    if "values" in spec:')
    w('        return random.choice(spec["values"])')
    w('    return random.randint(0, spec["max"])')
    w("")
    w("@cocotb.test()")
    w("async def random_stimulus(dut):")
    w("    random.seed(SEED)")
    if clock:
        w(f'    cocotb.start_soon(Clock(dut.{clock}, 2, units="ns").start())')
    w("    for name, spec in ANYCONST:")
    w("        getattr(dut, name).value = draw(spec)")
    if reset:
        w(f"    dut.{reset['signal']}.value = {reset['active']}")
        w(f"    for _ in range({reset['cycles']}):")
        w(f"        await RisingEdge(dut.{clock})")
        w(f"    dut.{reset['signal']}.value = {1 - reset['active']}")
    w("    for _ in range(CYCLES):")
    w("        for name, spec in INPUTS + ANYSEQ:")
    w("            getattr(dut, name).value = draw(spec)")
    if clock:
        w(f"        await RisingEdge(dut.{clock})")
    else:
        w('        await Timer(10, units="ns")')
    w("")
    w("def runner():")
    w('    sim = os.getenv("SIM", "verilator")')
    w("    dir_path = os.path.dirname(os.path.abspath(__file__))")
    w('    sources = sorted(glob.glob(os.path.join(dir_path, "*.sv")))')
    w("    runner = get_runner(sim)")
    w("    runner.build(")
    w("        verilog_sources=sources,")
    w(f'        hdl_toplevel="{top}",')
    w(f"        build_args=[{', '.join(build_args)}],")
    w("    )")
    w(f'    runner.test(hdl_toplevel="{top}", test_module="sim")')
    w("")
    w('if __name__ == "__main__":')
    w("    runner()")
    return "\n".join(lines) + "\n"


def inject_assumes(text, top, assumes, clock=None):
    """Insert immediate assume statements into module `top`, before its
    endmodule. Used for bench.json 'assumes' / run.py --assume."""
    if not assumes:
        return text
    body = "\n".join(f"        assume({a});" for a in assumes)
    trigger = f"@(posedge {clock})" if clock else "@(*)"
    block = f"\n    always {trigger} begin\n{body}\n    end\n"
    module_text = _find_module(_strip_comments(text), top)
    # find the module's endmodule position in the original text
    start = re.search(r"\bmodule\s+" + re.escape(top) + r"\b", text).start()
    end = text.index("endmodule", start)
    return text[:end] + block + text[end:]


def load_bench_config(bench_dir):
    path = os.path.join(bench_dir, "bench.json")
    if not os.path.exists(path):
        return {}
    with open(path) as f:
        return json.load(f)


def generate(bench_dir, top=None, force=False):
    """Generate <bench_dir>/sim.py for the design. Returns the parsed info."""
    cfg = load_bench_config(bench_dir)
    top = top or cfg.get("top") or os.path.basename(os.path.abspath(bench_dir))
    rtl = os.path.join(bench_dir, f"{top}.sv")
    if not os.path.exists(rtl):
        raise FileNotFoundError(f"{rtl} not found (top module RTL must be <top>.sv)")
    with open(rtl) as f:
        text = f.read()

    info = parse_module(text, top)
    if not info["inputs"] and not info["free_regs"]:
        raise ValueError(f"module '{top}' has no inputs or free registers to drive")

    sim_path = os.path.join(bench_dir, "sim.py")
    if os.path.exists(sim_path) and not force:
        print(f"{sim_path} already exists, keeping the hand-written version")
        return info

    content = render_sim_py(info, cfg)
    with open(sim_path, "w") as f:
        f.write(content)

    clock = guess_clock(info, cfg.get("clock"))
    reset = guess_reset(info, clock, cfg.get("reset")) if clock else None
    print(f"Generated {sim_path}:")
    print(f"  top: {top}")
    print(f"  clock: {clock or 'none (combinational, Timer-driven)'}")
    print(f"  reset: {reset if reset else 'none'}")
    print(f"  driven inputs: {[p['name'] for p in info['inputs']]}")
    if info["free_regs"]:
        print(f"  free regs: {[(r['kind'], r['name']) for r in info['free_regs']]}")
    if info["has_assume"]:
        print("  assume statements found: kept for formal, stripped for simulation")
    return info


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if a != "--force"]
    force = "--force" in sys.argv
    if not args:
        print("Usage: python gen_bench.py <bench_dir> [top] [--force]")
        sys.exit(1)
    generate(args[0], args[1] if len(args) > 1 else None, force=force)
