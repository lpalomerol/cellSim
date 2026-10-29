#!/usr/bin/env python3
"""
Summarize stochastic validation outputs for the BRCA1-ONSET tissue-scale model.
Computes cumulative penetrance, weighted SSE, BRCA1 status proportions,
and founder lineage distributions.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import numpy as np
import pandas as pd
from scipy.stats import chi2_contingency

CLINICAL_AGES = np.array([30, 40, 50, 60, 70, 80])
AGE_BIN_LABELS = ["<30", "30–39", "40–49", "50–59", "60–69", "70–79", "80"]


def summarize(input_path: Path, clinical_path: Path):
    df = pd.read_csv(input_path)
    total_tissues = len(df)

    pen_vals = np.array([(df["detect_age"].between(0, age)).mean() * 100.0 for age in CLINICAL_AGES])
    print(f"Total virtual tissues analyzed: {total_tissues}")
    print(f"Penetrance (%): " + ", ".join(f"{p:.2f}" for p in pen_vals))

    if clinical_path.exists():
        c = pd.read_csv(clinical_path).set_index("age")
        obs = c.loc[CLINICAL_AGES, "penetrance"].to_numpy(float)
        lo = c.loc[CLINICAL_AGES, "ci_low"].to_numpy(float)
        hi = c.loc[CLINICAL_AGES, "ci_high"].to_numpy(float)
        sd = (hi - lo) / 3.92
        wsse = float(np.sum(((pen_vals - obs) / sd) ** 2))
        print(f"Weighted SSE (vs Kuchenbaecker 2017): {wsse:.3f}")

    pos = df[df["detect_age"] >= 0].copy()
    n_pos = len(pos)
    pos_pct = 100.0 * n_pos / total_tissues
    print(f"Tumor-positive by age 80: {n_pos}/{total_tissues} ({pos_pct:.2f}%)")

    if n_pos == 0:
        print("No tumors detected in sample.")
        return

    nf = pos["brca1_null_fraction"]
    p_wt = 100.0 * (nf <= 0.5).mean()
    p_mixed = 100.0 * ((nf > 0.5) & (nf <= 0.75)).mean()
    p_null = 100.0 * (nf > 0.75).mean()
    loss_majority = p_mixed + p_null

    retain_25_among_loss_majority = 100.0 * p_mixed / loss_majority if loss_majority > 0 else 0.0

    print(f"BRCA1-loss-majority (>50% null): {loss_majority:.2f}%")
    print(f"Retain >=25% BRCA1+/- among loss-majority: {retain_25_among_loss_majority:.2f}%")
    print(f"  - BRCA1+/- predominant: {p_wt:.2f}%")
    print(f"  - Mixed: {p_mixed:.2f}%")
    print(f"  - BRCA1-/- predominant: {p_null:.2f}%")

    sub_founders_2plus = 100.0 * (pos["major_founders_5pct"] >= 2).mean()
    print(f">=2 substantial founders (>=5% each): {sub_founders_2plus:.2f}%")

    pos["age_bin"] = pd.cut(pos["detect_age"], [-np.inf, 29, 39, 49, 59, 69, 79, 80], labels=AGE_BIN_LABELS)
    pos["founder_cat"] = pd.cut(pos["major_founders_5pct"], [0, 1, 2, 3, np.inf], labels=["1", "2", "3", "≥4"])

    tab = pd.crosstab(pos["age_bin"], pos["founder_cat"]).reindex(index=AGE_BIN_LABELS, columns=["1", "2", "3", "≥4"], fill_value=0)

    # Only perform chi-square if there are sufficient positive counts
    if tab.values.sum() > 50 and np.all(tab.sum(axis=0) > 0) and np.all(tab.sum(axis=1) > 0):
        try:
            chi2, p_val, dof, _ = chi2_contingency(tab.values)
            print(f"Founder-age chi-square: chi2={chi2:.3f}, df={dof}, p={p_val:.5e}")
        except Exception:
            pass

    pct = tab.div(tab.sum(axis=1).replace(0, np.nan), axis=0) * 100.0
    print("\nFounder-lineage percentages by age:")
    print(pct.round(2).to_string())


def main():
    base_dir = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description="Summarize validation results.")
    parser.add_argument("--input", default=str(base_dir / "data" / "final_multiobjective_validation_10000.csv"))
    parser.add_argument("--clinical", default=str(base_dir / "data" / "clinical_penetrance.csv"))
    args = parser.parse_args()

    summarize(Path(args.input), Path(args.clinical))


if __name__ == "__main__":
    main()
