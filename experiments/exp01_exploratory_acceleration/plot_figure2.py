#!/usr/bin/env python3
"""Generate publication-quality Figure 2 (panels a and b) identical to the manuscript PDF."""
from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Clinical penetrance data: Kuchenbaecker et al. (2017)
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
        "legend.fontsize": 7.5,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.8,
        "xtick.major.width": 0.8,
        "ytick.major.width": 0.8,
        "xtick.major.size": 3.5,
        "ytick.major.size": 3.5,
    })


def plot_figure_2(data_dir: Path, output_dir: Path):
    set_publication_style()
    output_dir.mkdir(parents=True, exist_ok=True)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10.5, 4.4), dpi=300)

    # -------------------------------------------------------------
    # Panel a: Pre-transformation parameter exploration
    # -------------------------------------------------------------
    file_a = data_dir / "figure2a_extracted_trajectories.csv"
    if file_a.exists():
        df_a = pd.read_csv(file_a)
        
        # Color mapping strictly identical to Adobe Illustrator original:
        # Orange (#ff7f0e): Representative higher-β setting
        # Green  (#2ca02c): Representative lower-β setting
        # Red    (#d62728): Representative higher-δ_low setting
        # Purple (#9467bd): Representative lower-δ_low setting
        color_map_a = {
            "Representative higher-β setting": "#ff7f0e",
            "Representative lower-β setting": "#2ca02c",
            "Representative higher-δ_low setting": "#d62728",
            "Representative lower-δ_low setting": "#9467bd",
        }
        
        for name, col in color_map_a.items():
            sub = df_a[df_a["curve"] == name].sort_values("age")
            if not sub.empty:
                # Filter to valid age range [0, 80]
                sub = sub[(sub["age"] >= 0) & (sub["age"] <= 80)]
                ax1.plot(
                    sub["age"], sub["penetrance"],
                    color=col, linewidth=1.6, label=name
                )

    # Clinical data points and error bars
    ax1.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1f77b4", ecolor="#1f77b4", elinewidth=1.2, capsize=3,
        markersize=4.5, label="Observed penetrance value", zorder=5
    )

    ax1.set_title("Pre-transformation parameter exploration", pad=12, fontweight="normal")
    ax1.set_xlabel("Age (years)")
    ax1.set_ylabel("Cumulative penetrance (%)")
    ax1.set_xlim(0, 80)
    ax1.set_ylim(0, 100)
    ax1.set_xticks(np.arange(0, 81, 10))
    ax1.set_yticks(np.arange(0, 101, 20))

    # Add subplot label 'a'
    ax1.text(-0.14, 1.08, "a", transform=ax1.transAxes, fontsize=13, fontweight="bold", va="top")

    # Legend inside panel a
    handles, labels = ax1.get_legend_handles_labels()
    # Put Observed penetrance first as in the paper
    order = [len(labels)-1] + list(range(len(labels)-1))
    ax1.legend(
        [handles[i] for i in order],
        [labels[i] for i in order],
        loc="upper left", frameon=False
    )

    # -------------------------------------------------------------
    # Panel b: Effect of accelerated Tumoral expansion
    # -------------------------------------------------------------
    file_b = data_dir / "figure2b_extracted_trajectories.csv"
    if file_b.exists():
        df_b = pd.read_csv(file_b)
        
        # Color mapping strictly identical to Adobe Illustrator original:
        # Magenta (#ff00ff): Accelerated expansion (ν = 0.10)
        # Brown   (#c69c6d): Basal expansion (ν = 0.001)
        color_map_b = {
            "Accelerated expansion (ν = 0.10)": "#ff00ff",
            "Basal expansion (ν = 0.001)": "#c69c6d",
        }
        
        for name, col in color_map_b.items():
            sub = df_b[df_b["curve"] == name].sort_values("age")
            if not sub.empty:
                sub = sub[(sub["age"] >= 0) & (sub["age"] <= 80)]
                ax2.plot(
                    sub["age"], sub["penetrance"],
                    color=col, linewidth=1.8, label=name
                )

    ax2.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1f77b4", ecolor="#1f77b4", elinewidth=1.2, capsize=3,
        markersize=4.5, label="Observed penetrance value", zorder=5
    )

    ax2.set_title("Effect of accelerated Tumoral expansion", pad=12, fontweight="normal")
    ax2.set_xlabel("Age (years)")
    ax2.set_ylabel("Cumulative penetrance (%)")
    ax2.set_xlim(0, 80)
    ax2.set_ylim(0, 100)
    ax2.set_xticks(np.arange(0, 81, 10))
    ax2.set_yticks(np.arange(0, 101, 20))

    # Add subplot label 'b'
    ax2.text(-0.14, 1.08, "b", transform=ax2.transAxes, fontsize=13, fontweight="bold", va="top")

    # Legend inside panel b
    handles_b, labels_b = ax2.get_legend_handles_labels()
    order_b = [len(labels_b)-1] + list(range(len(labels_b)-1))
    ax2.legend(
        [handles_b[i] for i in order_b],
        [labels_b[i] for i in order_b],
        loc="upper left", frameon=False
    )

    plt.tight_layout()

    pdf_out = output_dir / "Figure2.pdf"
    png_out = output_dir / "Figure2.png"
    fig.savefig(pdf_out, bbox_inches="tight")
    fig.savefig(png_out, dpi=300, bbox_inches="tight")
    plt.close(fig)

    print(f"✓ Figure 2 generated identical to manuscript:")
    print(f"  -> PDF: {pdf_out}")
    print(f"  -> PNG: {png_out}")


def main():
    parser = argparse.ArgumentParser(description="Render Figure 2 exactly matching the paper")
    parser.add_argument(
        "--data-dir", type=str,
        default=str(Path(__file__).parent / "data"),
        help="Path to input data directory"
    )
    parser.add_argument(
        "--output-dir", type=str,
        default=str(Path(__file__).parent),
        help="Path to output figures directory"
    )

    args = parser.parse_args()
    plot_figure_2(Path(args.data_dir), Path(args.output_dir))


if __name__ == "__main__":
    main()
