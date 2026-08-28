#!/usr/bin/env python3
"""Plumbing tests for bv_ab.py; no mining, EBMC, or mutant generation."""

import json
from pathlib import Path
import tempfile
import unittest

import bv_ab


class BvAbTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)

    def tearDown(self):
        self.temporary.cleanup()

    def make_cell(self, mode="off", *, eval_exit=0, timed_out=False,
                  tool_errors=0, duplicate_start=False, block_seconds=1.0,
                  check_timeouts=0):
        cell_dir = self.root / mode
        work = cell_dir / "work"
        (work / "runtime").mkdir(parents=True)
        (work / "effective-config.json").write_text(json.dumps({
            "bv_predicates": mode, "seed": 42,
        }))
        records = [
            {"run": 7, "t": 0.0, "stage": "start", "bv_predicates": mode},
            {"run": 7, "t": 0.1, "reason": "verified", "block": "1",
             "seconds": block_seconds},
            {"run": 7, "t": 1.2, "stage": "check", "verified": 1,
             "refuted": 0, "errors": 0, "timed_out": check_timeouts},
            {"run": 7, "t": 1.3, "stage": "done", "verified": 1},
        ]
        if duplicate_start:
            records.insert(1, {
                "run": 7, "t": 0.01, "stage": "start", "bv_predicates": mode,
            })
        (work / "run-log.jsonl").write_text(
            "".join(json.dumps(record) + "\n" for record in records)
        )
        (work / "assertions.txt").write_text("(a < b)\n")
        (work / "invariants.txt").write_text("(a < b)\n")
        (work / "runtime/CompareResult.txt").write_text(
            "(a < b):\n"
            "((define-fun inv ((a (_ BitVec 8)) (b (_ BitVec 8))) Bool (bvult a b)))\n"
        )
        (work / "log.txt").write_text(
            "CVC5 are called 1 times\nCVC5 Timer: 0.25 seconds\n"
        )
        result = {
            "mine": {"exit": 0, "seconds": 1.5, "timed_out": False},
            "evaluation": {
                "exit": eval_exit, "timed_out": timed_out,
                "detected": 5, "mutants_without_timeout": 8, "timeouts": 2,
                "reported_non_timeout_md_percent": 62.5,
                "tool_errors": tool_errors,
            },
        }
        (cell_dir / "result.json").write_text(json.dumps(result))
        return {
            "design": "fixture", "mode": mode, "directory": str(cell_dir),
            "seed": 42, "fixed_mutants": 10,
        }

    def test_primary_md_uses_fixed_denominator(self):
        row = bv_ab.cell_metrics(self.make_cell())
        self.assertTrue(row["evaluation_valid"])
        self.assertEqual(row["md_fixed_percent"], 50.0)
        self.assertEqual(row["evaluator_non_timeout_md_percent"], 62.5)
        self.assertEqual(row["comparison_assertions"], 1)

    def test_evaluation_failures_invalidate_cell(self):
        for evaluation, reason in (
            ({"exit": 1, "timed_out": False, "tool_errors": 0,
              "detected": 1, "mutants_without_timeout": 10, "timeouts": 0},
             "evaluation-exit-1"),
            ({"exit": 0, "timed_out": True, "tool_errors": 0,
              "detected": 1, "mutants_without_timeout": 10, "timeouts": 0},
             "evaluation-timed-out"),
            ({"exit": 0, "timed_out": False, "tool_errors": 2,
              "detected": 1, "mutants_without_timeout": 10, "timeouts": 0},
             "evaluation-tool-errors"),
        ):
            with self.subTest(reason=reason):
                state = bv_ab.evaluation_metrics(evaluation, 10)
                self.assertFalse(state["valid"])
                self.assertIn(reason, state["invalid_reasons"])

    def test_start_record_must_be_unique_and_match_mode(self):
        with self.assertRaisesRegex(ValueError, "exactly one start"):
            bv_ab.cell_metrics(self.make_cell(duplicate_start=True))
        cell = self.make_cell("unsigned")
        run_log = Path(cell["directory"]) / "work/run-log.jsonl"
        records = [json.loads(line) for line in run_log.read_text().splitlines()]
        records[0]["bv_predicates"] = "off"
        run_log.write_text("".join(json.dumps(record) + "\n" for record in records))
        with self.assertRaisesRegex(ValueError, "start bv_predicates mismatch"):
            bv_ab.cell_metrics(cell)

    def test_invalid_cell_cannot_contribute_pair(self):
        off = self.make_cell("off")
        unsigned = self.make_cell("unsigned", tool_errors=1)
        manifest = self.root / "manifest.json"
        manifest.write_text(json.dumps({
            "manifest_version": bv_ab.MANIFEST_VERSION,
            "stage": "A", "cells": [off, unsigned],
        }))
        self.assertEqual(bv_ab.summarise_manifest(manifest, quiet=True), 4)
        summary = json.loads((self.root / "summary.json").read_text())
        self.assertEqual(summary["deltas"], [])
        self.assertEqual(summary["aggregate"]["pairs_valid"], 0)
        self.assertIn("unsigned:evaluation-tool-errors",
                      summary["invalid_pairs"][0]["reasons"])

    def test_stage_a_block_wall_gate(self):
        passing = [{"block_wall_median_ratio": ratio}
                   for ratio in (1.0, 1.5, 2.5)]
        failing = [{"block_wall_median_ratio": ratio}
                   for ratio in (1.9, 2.1, 2.2)]
        self.assertTrue(bv_ab.stage_a_timing_gate(passing, 3)["passed"])
        self.assertFalse(bv_ab.stage_a_timing_gate(failing, 3)["passed"])
        self.assertIsNone(bv_ab.stage_a_timing_gate(passing[:2], 3)["passed"])

    def test_design_identity_covers_sibling_rtl(self):
        source_dir = self.root / "design"
        source_dir.mkdir()
        main = source_dir / "design.sv"
        sibling = source_dir / "dependency.v"
        main.write_text("module design; endmodule\n")
        sibling.write_text("module dependency; endmodule\n")
        before = bv_ab.design_identity(main)
        sibling.write_text("module dependency; wire changed; endmodule\n")
        self.assertNotEqual(before, bv_ab.design_identity(main))

    def test_summary_enforces_stage_a_block_wall_gate(self):
        off = self.make_cell("off", block_seconds=1.0)
        unsigned = self.make_cell("unsigned", block_seconds=2.1)
        manifest = self.root / "manifest.json"
        manifest.write_text(json.dumps({
            "manifest_version": bv_ab.MANIFEST_VERSION,
            "stage": "A", "cells": [off, unsigned],
        }))
        self.assertEqual(bv_ab.summarise_manifest(manifest, quiet=True), 3)
        gate = json.loads((self.root / "summary.json").read_text())["stage_a_2x_gate"]
        self.assertTrue(gate["complete"])
        self.assertFalse(gate["passed"])
        self.assertEqual(gate["block_wall_median_ratio"], 2.1)

    def test_complete_stage_a_rejects_missing_timing_data(self):
        off = self.make_cell("off", block_seconds=0.0)
        unsigned = self.make_cell("unsigned", block_seconds=1.0)
        manifest = self.root / "manifest.json"
        manifest.write_text(json.dumps({
            "manifest_version": bv_ab.MANIFEST_VERSION,
            "stage": "A", "cells": [off, unsigned],
        }))
        self.assertEqual(bv_ab.summarise_manifest(manifest, quiet=True), 5)

    def test_final_check_timeouts_are_failures(self):
        off = self.make_cell("off", check_timeouts=1)
        unsigned = self.make_cell("unsigned")
        manifest = self.root / "manifest.json"
        manifest.write_text(json.dumps({
            "manifest_version": bv_ab.MANIFEST_VERSION,
            "stage": "A", "cells": [off, unsigned],
        }))
        self.assertEqual(bv_ab.summarise_manifest(manifest, quiet=True), 0)
        summary = json.loads((self.root / "summary.json").read_text())
        self.assertEqual(summary["aggregate"]["final_check_failures"], 1)


if __name__ == "__main__":
    unittest.main()
