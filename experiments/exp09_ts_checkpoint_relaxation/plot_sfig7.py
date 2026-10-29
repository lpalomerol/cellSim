#!/usr/bin/env python3
"""
Generate Supplementary Figure S7 for the BRCA1-ONSET tissue-scale model:
Checkpoint Relaxation Sensitivity Heatmaps.
Matches review manuscript Palomero_SFig7_100126.pdf.

Heatmaps:
- Left: Penetrance deviation (SSE)
- Right: Tumors enabled by checkpoint relaxation (%)
Grid:
- Rows: Somatic driver acquisition rate gamma in {0, 1e-5, 2e-5, 1e-4, 2e-4, 3e-4}
- Columns: Checkpoint bypass probability p_bypass in {0.25, 0.50, 0.75, 1.00}
"""

from __future__ import annotations

import argparse
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1 import make_axes_locatable

# Publication styling
plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["DejaVu Sans", "Helvetica", "Arial"],
    "font.size": 8.5,
    "axes.labelsize": 9.5,
    "axes.titlesize": 10,
    "xtick.labelsize": 8.5,
    "ytick.labelsize": 8.5,
    "figure.titlesize": 11,
    "axes.linewidth": 0.8,
    "pdf.fonttype": 42,
    "ps.fonttype": 42,
})

ROW_LABELS = [
    r"Reference ($\gamma = 0$)",
    r"$1 \times 10^{-5}$",
    r"$2 \times 10^{-5}$",
    r"$1 \times 10^{-4}$",
    r"$2 \times 10^{-4}$",
    r"$3 \times 10^{-4}$",
]
COL_LABELS = ["0.25", "0.5", "0.75", "1.0"]


def plot_figure_s7(data_path: Path, output_dir: Path) -> tuple[Path, Path]:
    df = pd.read_csv(data_path)

    # Pivot into 6x4 matrices
    # Preserve row ordering
    gamma_labels_unique = df["gamma_label"].unique()
    p_bypasses_unique = sorted(df["p_bypass"].unique())

    sse_matrix = np.zeros((len(gamma_labels_unique), len(p_bypasses_unique)))
    pct_matrix = np.zeros((len(gamma_labels_unique), len(p_bypasses_unique)))

    for i, g in enumerate(gamma_labels_unique):
        for j, p in enumerate(p_bypasses_unique):
            row = df[(df["gamma_label"] == g) & (df["p_bypass"] == p)]
            if len(row) > 0:
                sse_matrix[i, j] = row["sse"].values[0]
                pct_matrix[i, j] = row["tumors_enabled_pct"].values[0]

    fig, axes = plt.subplots(1, 2, figsize=(9.2, 5.0), gridspec_kw={"wspace": 0.38})
    fig.suptitle("Figure S7", fontsize=11, fontweight="bold", x=0.10, y=0.98)

    # --- Left Heatmap: Penetrance deviation (SSE) ---
    ax_left = axes[0]
    im_left = ax_left.imshow(sse_matrix, cmap="Blues", aspect="auto", vmin=10, vmax=280)
    ax_left.set_title("Penetrance deviation (SSE)", fontsize=10, pad=12, fontweight="medium")

    ax_left.set_xticks(np.arange(len(COL_LABELS)))
    ax_left.set_xticklabels(COL_LABELS)
    ax_left.set_yticks(np.arange(len(ROW_LABELS)))
    ax_left.set_yticklabels(ROW_LABELS)
    ax_left.set_xlabel(r"Checkpoint bypass probability ($p_{\mathrm{bypass}}$)", fontsize=9, labelpad=8)

    # Annotate values inside left heatmap
    for i in range(sse_matrix.shape[0]):
        for j in range(sse_matrix.shape[1]):
            val = int(round(sse_matrix[i, j]))
            color = "white" if sse_matrix[i, j] > 160 else "black"
            ax_left.text(j, i, f"{val}", ha="center", va="center", color=color, fontsize=8.5)

    # Left colorbar
    divider_left = make_axes_locatable(ax_left)
    cax_left = divider_left.append_axes("bottom", size="5%", pad=0.55)
    cb_left = fig.colorbar(im_left, cax=cax_left, orientation="horizontal")
    cb_left.set_label("SSE", fontsize=8.5)
    cb_left.ax.tick_params(labelsize=8)

    # --- Right Heatmap: Tumors enabled by checkpoint relaxation (%) ---
    ax_right = axes[1]
    im_right = ax_right.imshow(pct_matrix, cmap="Oranges", aspect="auto", vmin=0, vmax=30)
    ax_right.set_title("Tumors enabled\nby checkpoint relaxation", fontsize=10, pad=6, fontweight="medium")

    ax_right.set_xticks(np.arange(len(COL_LABELS)))
    ax_right.set_xticklabels(COL_LABELS)
    ax_right.set_yticks(np.arange(len(ROW_LABELS)))
    ax_right.set_yticklabels([])  # Hide y labels to keep it clean like review PDF
    ax_right.set_xlabel(r"Checkpoint bypass probability ($p_{\mathrm{bypass}}$)", fontsize=9, labelpad=8)

    # Annotate values inside right heatmap
    for i in range(pct_matrix.shape[0]):
        for j in range(pct_matrix.shape[1]):
            val = pct_matrix[i, j]
            color = "white" if pct_matrix[i, j] > 18.0 else "black"
            ax_right.text(j, i, f"{val:.1f}", ha="center", va="center", color=color, fontsize=8.5)

    # Right colorbar
    divider_right = make_axes_locatable(ax_right)
    cax_right = divider_right.append_axes("bottom", size="5%", pad=0.55)
    cb_right = fig.colorbar(im_right, cax=cax_right, orientation="horizontal")
    cb_right.set_label("Percentage (%)", fontsize=8.5)
    cb_right.ax.tick_params(labelsize=8)

    output_dir.mkdir(parents=True, exist_ok=True)
    pdf_path = output_dir / "FigureS7.pdf"
    png_path = output_dir / "FigureS7.png"
    eps_path = output_dir / "FigureS7.eps"

    fig.savefig(pdf_path, dpi=300, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    fig.savefig(eps_path, format="eps", bbox_inches="tight")
    plt.close(fig)

    print(f"  [OK] Saved Figure S7 -> {pdf_path}")
    print(f"  [OK] Saved Figure S7 -> {png_path}")
    print(f"  [OK] Saved Figure S7 -> {eps_path}")
    return pdf_path, png_path


def main():
    parser = argparse.ArgumentParser(description="Plot Supplementary Figure S7 heatmaps.")
    parser.add_argument("--data", default=str(Path(__file__).parent / "data" / "checkpoint_grid_results.csv"))
    parser.add_argument("--outdir", default=str(Path(__file__).parent))
    args = parser.parse_args()

    plot_figure_s7(Path(args.data), Path(args.outdir))


if __name__ == "__main__":
    main()
