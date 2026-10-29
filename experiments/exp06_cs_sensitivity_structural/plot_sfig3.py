#!/usr/bin/env python3
"""
plot_sfig3.py — Supplementary Figure S3: Structural sensitivity analysis.

Visualizes structural sensitivity of the discrete model:
  - Panel a: Structural sensitivity to the BRCA1 contribution parameter (kappa_BRCA1 in {1.0, 2.0, 3.0}).
  - Panel b: Structural sensitivity to somatic TP53 transition probability (mu_TP53 in {0.003, 0.001, 0.0003}).

Outputs:
  - Supplementary_Figure_S3.pdf / .png
  - FigureS3.pdf / .png
"""
from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Kuchenbaecker et al. (2017) clinical BRCA1 data
AGES = np.array([30, 40, 50, 60, 70, 80], dtype=float)
OBSERVED = np.array([4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
CI_LOWER = np.array([2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
CI_UPPER = np.array([7.0, 30.0, 52.0, 65.0, 73.0, 80.0])


def set_publication_style():
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["Arial", "Helvetica", "DejaVu Sans"],
        "font.size": 8.5,
        "axes.titlesize": 10,
        "axes.labelsize": 9,
        "xtick.labelsize": 8,
        "ytick.labelsize": 8,
        "legend.fontsize": 8.0,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.8,
        "xtick.major.width": 0.8,
        "ytick.major.width": 0.8,
    })


def plot_supplementary_figure_s3(data_dir: Path, output_dir: Path):
    set_publication_style()
    output_dir.mkdir(parents=True, exist_ok=True)

    csv_file = data_dir / "S3_structural_recalibration.csv"
    if not csv_file.exists():
        raise FileNotFoundError(f"Missing {csv_file}. Run 'run_structural_sensitivity.py --simulate' first.")

    df = pd.read_csv(csv_file)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10.5, 4.4), dpi=300)

    # -------------------------------------------------------------
    # Panel a: Sensitivity to kappa_BRCA1
    # -------------------------------------------------------------
    df_k = df[df["axis"] == "kappa"].sort_values("value")
    kappa_colors = {1.0: "#2563eb", 2.0: "#dc2626", 3.0: "#16a34a"}
    kappa_labels = {
        1.0: r"$\kappa_{BRCA1} = 1.0$ (equal D2 rate)",
        2.0: r"$\kappa_{BRCA1} = 2.0$ (baseline reference)",
        3.0: r"$\kappa_{BRCA1} = 3.0$ (amplified D2 rate)",
    }

    for _, row in df_k.iterrows():
        val = float(row["value"])
        y = [row[f"risk_{int(a)}"] for a in AGES]
        col = kappa_colors.get(val, "#4b5563")
        lbl = kappa_labels.get(val, rf"$\kappa_{{BRCA1}} = {val:g}$")
        lw = 2.0 if val == 2.0 else 1.4
        ax1.plot(AGES, y, marker="o", markersize=4.5, color=col, linewidth=lw, label=lbl, zorder=3)

    ax1.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1e40af", ecolor="#1e40af", elinewidth=1.2, capsize=3,
        markersize=5, label="Observed penetrance (95% CI)", zorder=5
    )

    ax1.set_xlabel("Age (years)")
    ax1.set_ylabel("Cumulative tumor-positive tissues (%)")
    ax1.set_xlim(25, 85)
    ax1.set_ylim(0, 85)
    ax1.set_title(r"Structural Sensitivity to BRCA1 Contribution to $D_2$" "\n" r"($\kappa_{BRCA1}$ Variation)", pad=8)
    ax1.grid(axis="both", linestyle=":", alpha=0.4, zorder=0)
    ax1.legend(frameon=False, loc="upper left")
    ax1.text(-0.14, 1.08, "a", transform=ax1.transAxes, fontsize=13, fontweight="bold", va="top")

    # -------------------------------------------------------------
    # Panel b: Sensitivity to TP53 transition probability
    # -------------------------------------------------------------
    df_t = df[df["axis"] == "tp53"].sort_values("value", ascending=False)
    tp53_colors = {0.003: "#dc2626", 0.001: "#7c3aed", 0.0003: "#ea580c"}
    tp53_labels = {
        0.003: r"$\mu_{TP53} = 0.0030$ (baseline reference)",
        0.001: r"$\mu_{TP53} = 0.0010$ (3-fold reduction)",
        0.0003: r"$\mu_{TP53} = 0.0003$ (10-fold reduction)",
    }

    for _, row in df_t.iterrows():
        val = float(row["value"])
        y = [row[f"risk_{int(a)}"] for a in AGES]
        col = tp53_colors.get(val, "#4b5563")
        lbl = tp53_labels.get(val, rf"$\mu_{{TP53}} = {val:g}$")
        lw = 2.0 if val == 0.003 else 1.4
        ax2.plot(AGES, y, marker="o", markersize=4.5, color=col, linewidth=lw, label=lbl, zorder=3)

    ax2.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1e40af", ecolor="#1e40af", elinewidth=1.2, capsize=3,
        markersize=5, label="Observed penetrance (95% CI)", zorder=5
    )

    ax2.set_xlabel("Age (years)")
    ax2.set_ylabel("Cumulative tumor-positive tissues (%)")
    ax2.set_xlim(25, 85)
    ax2.set_ylim(0, 85)
    ax2.set_title(r"Structural Sensitivity to Somatic $TP53$ Mutation Rate" "\n" r"($\mu_{TP53}$ Reduction)", pad=8)
    ax2.grid(axis="both", linestyle=":", alpha=0.4, zorder=0)
    ax2.legend(frameon=False, loc="upper left")
    ax2.text(-0.14, 1.08, "b", transform=ax2.transAxes, fontsize=13, fontweight="bold", va="top")

    plt.tight_layout()

    # Save canonical outputs
    pdf_path = output_dir / "FigureS3.pdf"
    png_path = output_dir / "FigureS3.png"
    eps_path = output_dir / "FigureS3.eps"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    fig.savefig(eps_path, format="eps", bbox_inches="tight")
    print(f"✓ Saved: {pdf_path}")
    print(f"✓ Saved: {png_path}")
    print(f"✓ Saved: {eps_path}")

    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description="Render Supplementary Figure S3")
    parser.add_argument("--data-dir", type=str, default=str(Path(__file__).parent / "data"))
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).parent))

    args = parser.parse_args()
    plot_supplementary_figure_s3(Path(args.data_dir), Path(args.output_dir))


if __name__ == "__main__":
    main()
