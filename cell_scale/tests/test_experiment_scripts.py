#!/usr/bin/env python3
"""
test_experiment_scripts.py — Unit test suite for Phase 3 ABC calibration experiment scripts.

Tests:
  1. scripts/experiments/exp_01_abc_smc_global.py in --dry-run mode
  2. scripts/experiments/exp_01_abc_smc_locked_division.py in --dry-run mode
  3. Output artifacts verification (generation_00.csv, diagnostics.csv, parameter_estimation_summary.json, run_manifest.json, summary_report.md)
"""

import json
import os
import shutil
import subprocess
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


class TestExperimentScripts(unittest.TestCase):
    def setUp(self):
        self.tmp_global = REPO_ROOT / "results" / "test_tmp_exp_01_global"
        self.tmp_locked = REPO_ROOT / "results" / "test_tmp_exp_01_locked"
        for p in (self.tmp_global, self.tmp_locked):
            if p.exists():
                shutil.rmtree(p)

    def tearDown(self):
        for p in (self.tmp_global, self.tmp_locked):
            if p.exists():
                shutil.rmtree(p)

    def _verify_artifacts(self, out_dir: Path, expected_params: list[str]):
        self.assertTrue(out_dir.exists(), f"Output directory {out_dir} does not exist")
        self.assertTrue((out_dir / "generation_00.csv").exists(), "generation_00.csv missing")
        self.assertTrue((out_dir / "generation_01.csv").exists(), "generation_01.csv missing")
        self.assertTrue((out_dir / "diagnostics.csv").exists(), "diagnostics.csv missing")
        self.assertTrue((out_dir / "parameter_estimation_summary.json").exists(), "parameter summary missing")
        self.assertTrue((out_dir / "run_manifest.json").exists(), "manifest missing")
        self.assertTrue((out_dir / "summary_report.md").exists(), "summary report missing")

        with open(out_dir / "parameter_estimation_summary.json") as f:
            summary = json.load(f)
            for param in expected_params:
                self.assertIn(param, summary, f"Parameter {param} not in summary")
                self.assertIn("median", summary[param])
                self.assertIn("ci_95_low", summary[param])
                self.assertIn("ci_95_high", summary[param])

    def test_global_calibration_dry_run(self):
        script = REPO_ROOT / "scripts" / "experiments" / "exp_01_abc_smc_global.py"
        cmd = [
            sys.executable,
            str(script),
            "--dry-run",
            "--output-dir",
            str(self.tmp_global),
            "--workers",
            "2",
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, cwd=str(REPO_ROOT), timeout=180)
        self.assertEqual(res.returncode, 0, f"Script failed with output:\nSTDOUT: {res.stdout}\nSTDERR: {res.stderr}")

        expected_params = [
            "brca1_rate", "low_delta", "high_delta", "neoplastic_div_rate",
            "tp53_rate", "d1_threshold", "d2_threshold", "division_rate", "kappa_brca1"
        ]
        self._verify_artifacts(self.tmp_global, expected_params)

    def test_locked_division_calibration_dry_run(self):
        script = REPO_ROOT / "scripts" / "experiments" / "exp_01_abc_smc_locked_division.py"
        cmd = [
            sys.executable,
            str(script),
            "--dry-run",
            "--output-dir",
            str(self.tmp_locked),
            "--workers",
            "2",
        ]
        res = subprocess.run(cmd, capture_output=True, text=True, cwd=str(REPO_ROOT), timeout=180)
        self.assertEqual(res.returncode, 0, f"Script failed with output:\nSTDOUT: {res.stdout}\nSTDERR: {res.stderr}")

        expected_params = [
            "brca1_rate", "low_delta", "high_delta",
            "tp53_rate", "d1_threshold", "d2_threshold", "division_rate", "kappa_brca1"
        ]
        self._verify_artifacts(self.tmp_locked, expected_params)


if __name__ == "__main__":
    unittest.main()
