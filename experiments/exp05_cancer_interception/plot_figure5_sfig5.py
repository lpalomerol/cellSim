#!/usr/bin/env python3
"""
plot_figure5_sfig5.py — Publication-grade visualization for Figure 5 and Supplementary Figure S5.

Reproduces:
  Figure 5 (Cancer Interception):
    Panel a: Modeled interception from age 30 (cumulative risk trajectories for None, D1, D2, D1+D2 50%).
    Panel b: Effect of intervention timing (ARR by age 70 for start ages 30, 40, 50).
    Panel c: Dose-dependent interception effects (ARR by age 70 for doses 10%, 25%, 50%).

  Figure S5 (D1-D2 Sub-additive Interaction):
    Interaction contrast on absolute risk (%) across start ages 30, 40, 50 at 25% and 50% attenuation.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Tuple

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

plt.switch_backend("Agg")

plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["DejaVu Sans", "Helvetica", "Arial"],
    "mathtext.fontset": "dejavusans",
    "axes.edgecolor": "#111827",
    "axes.linewidth": 0.8,
    "xtick.major.width": 0.8,
    "ytick.major.width": 0.8,
    "xtick.color": "#111827",
    "ytick.color": "#111827",
})

# Paper color palette
COLORS = {
    "none": "#1F77B4",      # Blue
    "D1": "#FF7F0E",        # Orange
    "D2": "#2CA02C",        # Green
    "D1+D2": "#D62728",     # Red
}

LABELS = {
    "none": "No intervention",
    "D1": r"$\theta_{D1}$ attenuation",
    "D2": r"$\theta_{D2}$ attenuation",
    "D1+D2": r"$\theta_{D1} + \theta_{D2}$ attenuation",
}


def render_figure5(data_dir: Path, out_dir: Path) -> Tuple[Path, Path]:
    df_curves = pd.read_csv(data_dir / "Figure5a_risk_curves.csv")
    df_arr = pd.read_csv(data_dir / "Figure5b_ARR_age70_50pct.csv")
    df_dose = pd.read_csv(data_dir / "Figure5c_dose_response_age30.csv")

    fig = plt.figure(figsize=(11.0, 4.2), dpi=300)
    gs = fig.add_gridspec(1, 3, left=0.06, right=0.96, top=0.84, bottom=0.18, wspace=0.35)

    # Master title
    fig.text(0.04, 0.96, "Figure 5", fontsize=14, fontweight="bold", va="top")

    # =========================================================================
    # Panel a: Modeled interception from age 30
    # =========================================================================
    ax_a = fig.add_subplot(gs[0, 0])
    ax_a.text(-0.20, 1.12, "a", transform=ax_a.transAxes, fontsize=13, fontweight="bold", va="top")
    ax_a.set_title("Modeled interception from age 30", fontsize=10.5, pad=6)

    for cond in ["none", "D1", "D2", "D1+D2"]:
        sub = df_curves[df_curves["condition"] == cond]
        ax_a.plot(sub["age"], sub["risk"], color=COLORS[cond], label=LABELS[cond], linewidth=1.4)

    ax_a.set_xlim(30, 80)
    ax_a.set_ylim(0, 80)
    ax_a.set_xticks([30, 40, 50, 60, 70, 80])
    ax_a.set_yticks([0, 20, 40, 60, 80])
    ax_a.set_xlabel("Age (years)", fontsize=9.5)
    ax_a.set_ylabel("Subsequent tumor-positive tissues (%)", fontsize=9.5)
    ax_a.tick_params(labelsize=8.5, length=2.5)
    ax_a.grid(True, linestyle=":", alpha=0.5, color="#E5E7EB")
    ax_a.legend(loc="upper left", fontsize=7.5, frameon=True, edgecolor="#E5E7EB", framealpha=0.9)

    # =========================================================================
    # Panel b: Effect of intervention timing (ARR by age 70 for 50% dose)
    # =========================================================================
    ax_b = fig.add_subplot(gs[0, 1])
    ax_b.text(-0.20, 1.12, "b", transform=ax_b.transAxes, fontsize=13, fontweight="bold", va="top")
    ax_b.set_title("Effect of intervention timing", fontsize=10.5, pad=6)

    start_ages = [30, 40, 50]
    x = np.arange(len(start_ages))
    width = 0.24

    for idx, cond in enumerate(["D1", "D2", "D1+D2"]):
        sub = df_arr[df_arr["condition"] == cond]
        vals = [float(sub.loc[sub["intervention_age"] == a, "ARR_pp"].iloc[0]) for a in start_ages]
        pos = x + (idx - 1) * width
        ax_b.bar(pos, vals, width, color=COLORS[cond], label=LABELS[cond], edgecolor="none")

    ax_b.set_xlim(-0.5, 2.5)
    ax_b.set_ylim(0, 30)
    ax_b.set_xticks(x)
    ax_b.set_xticklabels(["30", "40", "50"], fontsize=8.5)
    ax_b.set_yticks([0, 5, 10, 15, 20, 25, 30])
    ax_b.set_xlabel("Intervention start age (years)", fontsize=9.5)
    ax_b.set_ylabel("Absolute risk reduction by age 70\n(percentage points)", fontsize=9.5)
    ax_b.tick_params(labelsize=8.5, length=2.5)
    ax_b.grid(True, linestyle=":", alpha=0.5, color="#E5E7EB")
    ax_b.legend(loc="upper right", fontsize=7.5, frameon=True, edgecolor="#E5E7EB", framealpha=0.9)

    # =========================================================================
    # Panel c: Dose-dependent interception effects
    # =========================================================================
    ax_c = fig.add_subplot(gs[0, 2])
    ax_c.text(-0.20, 1.12, "c", transform=ax_c.transAxes, fontsize=13, fontweight="bold", va="top")
    ax_c.set_title("Dose-dependent interception effects", fontsize=10.5, pad=6)

    for cond in ["D1", "D2", "D1+D2"]:
        sub = df_dose[df_dose["condition"] == cond]
        doses_pct = sub["dose"].to_numpy(float) * 100.0
        arr_vals = sub["ARR_pp"].to_numpy(float)
        ax_c.plot(doses_pct, arr_vals, color=COLORS[cond], marker="o", markersize=4.5,
                  label=LABELS[cond], linewidth=1.4)

    ax_c.set_xlim(5, 55)
    ax_c.set_ylim(0, 30)
    ax_c.set_xticks([10, 25, 50])
    ax_c.set_yticks([0, 5, 10, 15, 20, 25, 30])
    ax_c.set_xlabel(r"Attenuation of $\theta_{D1}$ / $\theta_{D2}$ (%)", fontsize=9.5)
    ax_c.set_ylabel("Absolute risk reduction by age 70\n(percentage points)", fontsize=9.5)
    ax_c.tick_params(labelsize=8.5, length=2.5)
    ax_c.grid(True, linestyle=":", alpha=0.5, color="#E5E7EB")
    ax_c.legend(loc="upper left", fontsize=7.5, frameon=True, edgecolor="#E5E7EB", framealpha=0.9)

    pdf_path = out_dir / "Figure5.pdf"
    png_path = out_dir / "Figure5.png"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    plt.close(fig)
    return pdf_path, png_path


def render_sfig5(data_dir: Path, out_dir: Path) -> Tuple[Path, Path]:
    df_inter = pd.read_csv(data_dir / "FigureS5_interaction.csv")

    fig, ax = plt.subplots(figsize=(5.5, 4.4), dpi=300)

    # Master title
    fig.text(0.08, 0.96, "Figure S5", fontsize=13, fontweight="bold", va="top")

    start_ages = [30, 40, 50]
    x_base = np.arange(len(start_ages))
    offsets = {0.25: -0.08, 0.50: 0.08}
    colors = {0.25: "#111827", 0.50: "#6B7280"}
    labels = {0.25: "25% attenuation", 0.50: "50% attenuation"}

    for dose in [0.25, 0.50]:
        sub = df_inter[np.isclose(df_inter["dose"], dose)]
        y = sub["interaction_pp"].to_numpy(float)
        lo = sub["ci_lo"].to_numpy(float)
        hi = sub["ci_hi"].to_numpy(float)
        yerr = [y - lo, hi - y]

        x_pos = x_base + offsets[dose]
        ax.errorbar(
            x_pos, y, yerr=yerr, fmt="o", color=colors[dose], ecolor=colors[dose],
            elinewidth=1.2, capsize=3.5, capthick=1.2, markersize=5.0,
            label=labels[dose]
        )

    ax.axhline(0, color="#111827", linestyle="--", linewidth=0.9, alpha=0.7)

    ax.set_xlim(-0.4, 2.4)
    ax.set_ylim(-8.5, 0.5)
    ax.set_xticks(x_base)
    ax.set_xticklabels(["30", "40", "50"], fontsize=9)
    ax.set_yticks([0, -2, -4, -6, -8])
    ax.set_xlabel("Intervention start age (years)", fontsize=10)
    ax.set_ylabel(r"$D_1$–$D_2$ interaction on absolute risk (%)", fontsize=10)
    ax.tick_params(labelsize=9, length=2.5)
    ax.grid(True, linestyle=":", alpha=0.5, color="#E5E7EB")
    ax.legend(loc="lower right", fontsize=8.5, frameon=True, edgecolor="#E5E7EB", framealpha=0.9)

    pdf_path = out_dir / "FigureS5.pdf"
    png_path = out_dir / "FigureS5.png"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    plt.close(fig)
    return pdf_path, png_path


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate publication-grade Figure 5 and Supplementary Figure S5.")
    parser.add_argument("--data-dir", type=str, default="experiments/exp05_cancer_interception/data")
    parser.add_argument("--output-dir", type=str, default="experiments/exp05_cancer_interception")
    args = parser.parse_args()

    data_dir = Path(args.data_dir)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print("=========================================================================")
    print("Rendering Figure 5 & Supplementary Figure S5 (Cancer Interception)")
    print(f"Data directory: {data_dir}")
    print(f"Output directory: {out_dir}")
    print("=========================================================================")

    p5_pdf, p5_png = render_figure5(data_dir, out_dir)
    ps5_pdf, ps5_png = render_sfig5(data_dir, out_dir)

    print(f"[SUCCESS] Figure 5 generated: {p5_pdf}")
    print(f"[SUCCESS] Figure S5 generated: {ps5_pdf}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
