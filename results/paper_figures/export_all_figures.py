#!/usr/bin/env python3
"""
Centralized Publication Figures Exporter for Palomero et al. (2026) (BRCA1-ONSET).
Copies and validates all canonical figures (PDF, PNG, EPS) from modular vertical
slices in experiments/ into results/paper_figures/.
"""

from __future__ import annotations

import shutil
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent.parent
EXP_DIR = ROOT_DIR / "experiments"
OUT_DIR = Path(__file__).resolve().parent

# Mapping: (source_experiment_dir, filename_stem, canonical_dest_stem, description)
FIGURE_MANIFEST = [
    # Main text figures
    ("exp01_cs_exploratory_acceleration", "Figure2", "Figure2", "Main Figure 2: Exploratory Acceleration Grid"),
    ("exp03_cs_abc_smc_calibration", "Figure3", "Figure3", "Main Figure 3: ABC-SMC Calibration & Posterior Predictive"),
    ("exp04_cs_emergent_features", "Figure4", "Figure4", "Main Figure 4: Emergent Features & Clonal Lineages"),
    ("exp05_cs_cancer_interception", "Figure5", "Figure5", "Main Figure 5: Cancer Interception & Timing"),
    ("exp07_ts_multiobjective_cohort", "Figure6", "Figure6", "Main Figure 6: Multiobjective Tissue-Scale Cohort"),

    # Supplementary figures
    ("exp02_cs_sensitivity_global", "FigureS1", "FigureS1", "Supplementary Figure S1: Global Sensitivity LHS & PRCC"),
    ("exp06_cs_sensitivity_structural", "FigureS2", "FigureS2", "Supplementary Figure S2: Fixed Parameter OAT Sensitivity"),
    ("exp06_cs_sensitivity_structural", "FigureS3", "FigureS3", "Supplementary Figure S3: Structural Sensitivity (Kappa & TP53)"),
    ("exp03_cs_abc_smc_calibration", "FigureS4", "FigureS4", "Supplementary Figure S4: ABC-SMC Corner Matrix (Canonical 4a)"),
    ("exp03_cs_abc_smc_calibration", "FigureS4b", "FigureS4b", "Supplementary Figure S4b: 1D KDEs of All 6 Parameter Pairs"),
    ("exp05_cs_cancer_interception", "FigureS5", "FigureS5", "Supplementary Figure S5: D1-D2 Sub-additive Interaction"),
    ("exp08_ts_homogeneous_restraint", "FigureS6", "FigureS6", "Supplementary Figure S6: Homogeneous Restraint Fitting & Trade-off"),
    ("exp09_ts_checkpoint_relaxation", "FigureS7", "FigureS7", "Supplementary Figure S7: Checkpoint Relaxation Heatmaps"),
]

FORMATS = [".pdf", ".png", ".eps"]


def export_all():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    print("=" * 75)
    print("Exporting All Publication Figures to results/paper_figures/")
    print(f"Destination: {OUT_DIR}")
    print("=" * 75)

    copied_count = 0
    missing_count = 0

    for exp_folder, src_stem, dst_stem, desc in FIGURE_MANIFEST:
        src_dir = EXP_DIR / exp_folder
        print(f"\n▶ {desc}")
        for fmt in FORMATS:
            src_file = src_dir / f"{src_stem}{fmt}"
            dst_file = OUT_DIR / f"{dst_stem}{fmt}"

            if src_file.exists():
                shutil.copy2(src_file, dst_file)
                size_kb = dst_file.stat().st_size / 1024.0
                print(f"  ✓ [{fmt.upper()[1:]}] {dst_file.name} ({size_kb:.1f} KB)")
                copied_count += 1
            else:
                print(f"  ✗ [MISSING] {src_file}")
                missing_count += 1

    print("\n" + "=" * 75)
    print(f"Export summary: {copied_count} files synchronized, {missing_count} missing.")
    print("=" * 75)


if __name__ == "__main__":
    export_all()
