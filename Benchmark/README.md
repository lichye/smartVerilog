# SMART benchmark corpus

This directory contains the small, versioned RTL corpus shipped with SMART.
It is used for examples, regression tests, and reproducible experiments; large
mutation sets and generated experiment results live in separate local
checkouts.

- `fmcad2025/` is the evaluation corpus used by SMART's FMCAD experiments.
- `HWSpec/` contains selected open-source hardware designs. Their original
  copyright and licence notices remain in each source file.
- `user/tiny_and/` is SMART's minimal end-to-end smoke fixture.

Some directories include a legacy `sim.py`. SMART itself does not execute
those files: it generates a Verilog testbench from the RTL. They remain as
frozen oracle fixtures for the frontend-parity development check.

When adding a benchmark, retain its upstream notice and add provenance here or
in the benchmark directory. Do not add generated VCDs, simulators, mutants, or
experiment results.
