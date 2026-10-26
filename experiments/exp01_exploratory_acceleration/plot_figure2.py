#!/usr/bin/env python3
"""Generate publication-quality Figure 2 (panels a and b) using authentic simulation or paper data."""
from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
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
        "legend.fontsize": 7.2,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.8,
        "xtick.major.width": 0.8,
        "ytick.major.width": 0.8,
        "xtick.major.size": 3.5,
        "ytick.major.size": 3.5,
    })


def plot_figure_2(
    data_dir: Path,
    output_dir: Path,
    mode: str = "simulated",
    show_grid: bool = False,
    max_age: int = 80,
    output_name: str | None = None,
    data_suffix: str = "",
):
    set_publication_style()
    output_dir.mkdir(parents=True, exist_ok=True)

    if output_name is None:
        suffix = "_100years" if max_age > 80 else (data_suffix if data_suffix else "")
        if show_grid:
            output_name = f"Figure2_extra_grid{suffix}"
        elif mode == "simulated":
            output_name = f"Figure2_simulated{suffix}"
        else:
            output_name = f"Figure2_paper{suffix}"

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11.0, 4.6), dpi=300)

    # 100-year vs 80-year file resolution
    if max_age > 80:
        file_100 = data_dir / "figure2_100years_trajectories.csv"
        file_all_grid = data_dir / "figure2a_all_grid_100years.csv"
    else:
        file_100 = None
        file_all_grid = data_dir / f"figure2a_all_grid_trajectories{data_suffix}.csv"

    # File selection based on mode for standard 80-year data
    if mode == "simulated":
        file_a = data_dir / f"figure2a_simulated_trajectories{data_suffix}.csv"
        file_b = data_dir / f"figure2b_simulated_trajectories{data_suffix}.csv"
    else:
        file_a = data_dir / "figure2a_extracted_trajectories.csv"
        file_b = data_dir / "figure2b_extracted_trajectories.csv"

    # -------------------------------------------------------------
    # Panel a: Pre-transformation parameter exploration
    # -------------------------------------------------------------
    # Optional: background ensemble of all 36 grid combinations in faint gray
    if show_grid and file_all_grid.exists():
        df_all = pd.read_csv(file_all_grid)
        first_grid = True
        grid_d = "0.01" if "001" in data_suffix or "0.01" in data_suffix else "0.001"
        for (b, dl), g in df_all.groupby(["beta", "low_delta"]):
            sub = g.sort_values("age")
            sub = sub[(sub["age"] >= 0) & (sub["age"] <= max_age)]
            lbl = rf"Grid combinations ($N=36$, $d={grid_d}$)" if first_grid else None
            ax1.plot(sub["age"], sub["penetrance"], color="#b0bec5", alpha=0.45, lw=0.85, zorder=2, label=lbl)
            first_grid = False

    if max_age > 80 and file_100 and file_100.exists():
        df_100 = pd.read_csv(file_100)
        sub_100 = df_100[(df_100["age"] >= 0) & (df_100["age"] <= max_age)]
        ax1.plot(sub_100["age"], sub_100["higher_beta_orange"], color="#ff7f0e", lw=2.0 if show_grid else 1.7, zorder=4,
                 label=r"Higher-$\beta$ ($\beta=0.060, \delta_{low}=0.150$)")
        ax1.plot(sub_100["age"], sub_100["lower_beta_green"], color="#2ca02c", lw=2.0 if show_grid else 1.7, zorder=4,
                 label=r"Lower-$\beta$ ($\beta=0.010, \delta_{low}=0.150$)")
        ax1.plot(sub_100["age"], sub_100["higher_delta_red"], color="#d62728", lw=2.0 if show_grid else 1.7, zorder=4,
                 label=r"Higher-$\delta_{low}$ ($\beta=0.040, \delta_{low}=0.200$)")
        ax1.plot(sub_100["age"], sub_100["lower_delta_purple"], color="#9467bd", lw=2.0 if show_grid else 1.7, zorder=4,
                 label=r"Lower-$\delta_{low}$ ($\beta=0.040, \delta_{low}=0.080$)")
    elif file_a.exists():
        df_a = pd.read_csv(file_a)
        
        # Color mapping and descriptive parameter labels:
        if mode == "simulated" and show_grid:
            color_map_a = {
                "Representative higher-β setting": ("#ff7f0e", r"Higher-$\beta$ ($\beta=0.060, \delta_{low}=0.150$)"),
                "Representative lower-β setting": ("#2ca02c", r"Lower-$\beta$ ($\beta=0.010, \delta_{low}=0.150$)"),
                "Representative higher-δ_low setting": ("#d62728", r"Higher-$\delta_{low}$ ($\beta=0.040, \delta_{low}=0.150$)"),
                "Representative lower-δ_low setting": ("#9467bd", r"Lower-$\delta_{low}$ ($\beta=0.040, \delta_{low}=0.080$)"),
            }
        else:
            color_map_a = {
                "Representative higher-β setting": ("#ff7f0e", "Representative higher-β setting"),
                "Representative lower-β setting": ("#2ca02c", "Representative lower-β setting"),
                "Representative higher-δ_low setting": ("#d62728", "Representative higher-δ_low setting"),
                "Representative lower-δ_low setting": ("#9467bd", "Representative lower-δ_low setting"),
            }
        
        for name, (col, leg_lbl) in color_map_a.items():
            sub = df_a[df_a["curve"] == name].sort_values("age")
            if not sub.empty:
                sub = sub[(sub["age"] >= 0) & (sub["age"] <= max_age)]
                ax1.plot(
                    sub["age"], sub["penetrance"],
                    color=col, linewidth=2.0 if show_grid else 1.6, zorder=4, label=leg_lbl
                )

    # Clinical data points and error bars
    ax1.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1f77b4", ecolor="#1f77b4", elinewidth=1.2, capsize=3,
        markersize=4.5, label="Observed penetrance value", zorder=5
    )

    if max_age > 80:
        ax1.axvline(80, color="#78909c", linestyle=":", lw=1.0, alpha=0.8, zorder=3, label="Clinical horizon (80y)")

    title_suffix = " (Extended to 100y)" if max_age > 80 else ""
    title_a = f"Pre-transformation parameter exploration{title_suffix}"
    if show_grid:
        title_a = f"Pre-transformation parameter exploration (with grid distribution){title_suffix}"
    ax1.set_title(title_a, pad=12, fontweight="normal")
    ax1.set_xlabel("Age (years)")
    ax1.set_ylabel("Cumulative penetrance (%)")
    ax1.set_xlim(0, max_age)
    ax1.set_ylim(0, 105 if max_age > 80 else 100)
    ax1.set_xticks(np.arange(0, max_age + 1, 10))
    ax1.set_yticks(np.arange(0, 101, 20))
    ax1.text(-0.14, 1.08, "a", transform=ax1.transAxes, fontsize=13, fontweight="bold", va="top")

    # Legend inside panel a
    handles, labels = ax1.get_legend_handles_labels()
    if labels:
        # Put Observed penetrance first as in the paper
        obs_idx = next((i for i, l in enumerate(labels) if "Observed" in l), len(labels)-1)
        order = [obs_idx] + [i for i in range(len(labels)) if i != obs_idx]
        ax1.legend(
            [handles[i] for i in order],
            [labels[i] for i in order],
            loc="upper left", frameon=False, fontsize=6.8 if show_grid else 7.2
        )

    # -------------------------------------------------------------
    # Panel b: Effect of accelerated Tumoral expansion
    # -------------------------------------------------------------
    # Optional: background unaccelerated cloud in panel b to highlight the contrast
    if show_grid and file_all_grid.exists():
        df_all = pd.read_csv(file_all_grid)
        first_grid_b = True
        for (b, dl), g in df_all.groupby(["beta", "low_delta"]):
            sub = g.sort_values("age")
            sub = sub[(sub["age"] >= 0) & (sub["age"] <= max_age)]
            lbl = r"Unaccelerated grid ($N=36$)" if first_grid_b else None
            ax2.plot(sub["age"], sub["penetrance"], color="#b0bec5", alpha=0.35, lw=0.8, zorder=2, label=lbl)
            first_grid_b = False

    if max_age > 80 and file_100 and file_100.exists():
        df_100 = pd.read_csv(file_100)
        sub_100 = df_100[(df_100["age"] >= 0) & (df_100["age"] <= max_age)]
        ax2.plot(sub_100["age"], sub_100["accelerated_expansion"], color="#ff00ff", lw=2.2 if show_grid else 1.9, zorder=4,
                 label=r"Accelerated expansion ($\nu = 0.10$)")
        ax2.plot(sub_100["age"], sub_100["basal_expansion"], color="#c69c6d", lw=2.0 if show_grid else 1.7, zorder=4,
                 label=r"Basal expansion ($\nu = 0.001$)")
    elif file_b.exists():
        df_b = pd.read_csv(file_b)
        
        for name in df_b["curve"].unique():
            if "Accelerated" in name:
                col, leg_lbl = "#ff00ff", r"Accelerated expansion ($\nu = 0.10$)"
            elif "Basal" in name:
                basal_d = "0.01" if "0.01" in name or "001" in data_suffix else "0.001"
                col, leg_lbl = "#c69c6d", rf"Basal expansion ($\nu = d = {basal_d}$)"
            else:
                col, leg_lbl = "#17becf", name
            sub = df_b[df_b["curve"] == name].sort_values("age")
            if not sub.empty:
                sub = sub[(sub["age"] >= 0) & (sub["age"] <= max_age)]
                ax2.plot(
                    sub["age"], sub["penetrance"],
                    color=col, linewidth=2.2 if show_grid else 1.8, zorder=4, label=leg_lbl
                )

    ax2.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1f77b4", ecolor="#1f77b4", elinewidth=1.2, capsize=3,
        markersize=4.5, label="Observed penetrance value", zorder=5
    )

    if max_age > 80:
        ax2.axvline(80, color="#78909c", linestyle=":", lw=1.0, alpha=0.8, zorder=3, label="Clinical horizon (80y)")

    title_b = f"Effect of accelerated Tumoral expansion{title_suffix}"
    ax2.set_title(title_b, pad=12, fontweight="normal")
    ax2.set_xlabel("Age (years)")
    ax2.set_ylabel("Cumulative penetrance (%)")
    ax2.set_xlim(0, max_age)
    ax2.set_ylim(0, 105 if max_age > 80 else 100)
    ax2.set_xticks(np.arange(0, max_age + 1, 10))
    ax2.set_yticks(np.arange(0, 101, 20))
    ax2.text(-0.14, 1.08, "b", transform=ax2.transAxes, fontsize=13, fontweight="bold", va="top")

    # Legend inside panel b
    handles_b, labels_b = ax2.get_legend_handles_labels()
    if labels_b:
        obs_idx_b = next((i for i, l in enumerate(labels_b) if "Observed" in l), len(labels_b)-1)
        order_b = [obs_idx_b] + [i for i in range(len(labels_b)) if i != obs_idx_b]
        ax2.legend(
            [handles_b[i] for i in order_b],
            [labels_b[i] for i in order_b],
            loc="upper left", frameon=False, fontsize=7.2
        )

    plt.tight_layout()

    pdf_out = output_dir / f"{output_name}.pdf"
    png_out = output_dir / f"{output_name}.png"
    fig.savefig(pdf_out, bbox_inches="tight")
    fig.savefig(png_out, dpi=300, bbox_inches="tight")
    plt.close(fig)

    print(f"✓ Figure 2 ({output_name}) generated:")
    print(f"  -> PDF: {pdf_out}")
    print(f"  -> PNG: {png_out}")


def main():
    parser = argparse.ArgumentParser(description="Render Figure 2 (simulated, paper, or extra-grid mode)")
    parser.add_argument(
        "--mode", type=str, choices=["simulated", "paper"], default="simulated",
        help="Data source mode: 'simulated' (real Monte Carlo data) or 'paper' (vectorized PDF)"
    )
    parser.add_argument(
        "--show-grid", action="store_true",
        help="Overlay all 36 unaccelerated grid trajectories in faint gray"
    )
    parser.add_argument(
        "--max-age", type=int, default=80, choices=[80, 100],
        help="Maximum age on horizontal axis: 80 (standard clinical) or 100 (extended)"
    )
    parser.add_argument(
        "--output-name", type=str, default=None,
        help="Custom base name for output files without extension (e.g. Figure2_extra_grid)"
    )
    parser.add_argument(
        "--data-suffix", type=str, default="",
        help="Suffix for input simulation files (e.g. '_d001')"
    )
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
    plot_figure_2(
        Path(args.data_dir), Path(args.output_dir),
        mode=args.mode, show_grid=args.show_grid, max_age=args.max_age, output_name=args.output_name,
        data_suffix=args.data_suffix
    )


if __name__ == "__main__":
    main()
