#!/usr/bin/env python3
"""Generate the 12 Figure 2a single-panel exploration variants (d in {0.001, 0.01}, horizon in {80, 100}, 3 grid styles)."""
from __future__ import annotations

import argparse
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from run_exploratory import (
    generate_figure2_trajectories,
    generate_all_grid_trajectories,
    AGES,
    OBSERVED_PENETRANCE as OBSERVED,
    CI_LOWER,
    CI_UPPER,
)


def set_publication_style():
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["Arial", "Helvetica", "DejaVu Sans"],
        "font.size": 9.0,
        "axes.titlesize": 10.5,
        "axes.labelsize": 9.5,
        "xtick.labelsize": 8.5,
        "ytick.labelsize": 8.5,
        "legend.fontsize": 7.8,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.85,
        "xtick.major.width": 0.85,
        "ytick.major.width": 0.85,
        "xtick.major.size": 3.8,
        "ytick.major.size": 3.8,
    })


def plot_single_panel_2a(
    *,
    rep_csv: Path | None,
    grid_csv: Path | None,
    mode: str,
    max_age: int,
    d_value: float,
    output_png: Path,
    output_pdf: Path,
):
    set_publication_style()
    output_png.parent.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(6.2, 4.8), dpi=300)

    # 1. Background grid (if mode in ["rep_and_grid", "grid_only"])
    if mode in ["rep_and_grid", "grid_only"] and grid_csv and grid_csv.exists():
        df_grid = pd.read_csv(grid_csv)
        first_grid = True
        grid_alpha = 0.55 if mode == "grid_only" else 0.40
        grid_color = "#90a4ae" if mode == "grid_only" else "#b0bec5"
        for (b, dl), g in df_grid.groupby(["beta", "low_delta"]):
            sub = g.sort_values("age")
            sub = sub[(sub["age"] >= 0) & (sub["age"] <= max_age)]
            lbl = rf"Grid combinations ($N=36$, $d={d_value}$)" if first_grid else None
            ax.plot(sub["age"], sub["penetrance"], color=grid_color, alpha=grid_alpha, lw=0.9, zorder=2, label=lbl)
            first_grid = False

    # 2. Representative curves (if mode in ["rep_only", "rep_and_grid"])
    if mode in ["rep_only", "rep_and_grid"] and rep_csv and rep_csv.exists():
        df_rep = pd.read_csv(rep_csv)
        color_map = {
            "Representative higher-β setting": ("#ff7f0e", r"Higher-$\beta$ ($\beta=0.060, \delta_{low}=0.150$)"),
            "Representative lower-β setting": ("#2ca02c", r"Lower-$\beta$ ($\beta=0.010, \delta_{low}=0.150$)"),
            "Representative higher-δ_low setting": ("#d62728", r"Higher-$\delta_{low}$ ($\beta=0.040, \delta_{low}=0.200$)"),
            "Representative lower-δ_low setting": ("#9467bd", r"Lower-$\delta_{low}$ ($\beta=0.040, \delta_{low}=0.080$)"),
        }
        for curve_name, (color, leg_lbl) in color_map.items():
            sub = df_rep[df_rep["curve"] == curve_name].sort_values("age")
            if not sub.empty:
                sub = sub[(sub["age"] >= 0) & (sub["age"] <= max_age)]
                lw = 2.0 if mode == "rep_and_grid" else 1.85
                ax.plot(sub["age"], sub["penetrance"], color=color, lw=lw, zorder=4, label=leg_lbl)

    # 3. Clinical Kuchenbaecker (2017) observed reference
    ax.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1f77b4", ecolor="#1f77b4", elinewidth=1.2, capsize=3,
        markersize=4.8, label="Observed penetrance value", zorder=5
    )

    # 4. Clinical horizon boundary line for 100-year plots
    if max_age > 80:
        ax.axvline(80, color="#78909c", linestyle=":", lw=1.1, alpha=0.85, zorder=3, label="Clinical horizon (80y)")

    # 5. Styling, titles and limits
    style_subtitle = {
        "rep_only": "Representative settings",
        "rep_and_grid": "Representative settings + 36 grid models",
        "grid_only": "Full 36-model grid sweep",
    }[mode]

    title = f"Pre-transformation parameter exploration ({style_subtitle}, $d={d_value}$)"
    ax.set_title(title, pad=12, fontweight="normal")
    ax.set_xlabel("Age (years)")
    ax.set_ylabel("Cumulative penetrance (%)")
    ax.set_xlim(0, max_age)
    ax.set_ylim(0, 105 if max_age > 80 else 100)
    ax.set_xticks(np.arange(0, max_age + 1, 10))
    ax.set_yticks(np.arange(0, 101, 20))
    ax.text(-0.10, 1.05, "a", transform=ax.transAxes, fontsize=13, fontweight="bold", va="top")

    # Legend order (Observed at top)
    handles, labels = ax.get_legend_handles_labels()
    if labels:
        obs_idx = next((i for i, l in enumerate(labels) if "Observed" in l), len(labels) - 1)
        order = [obs_idx] + [i for i in range(len(labels)) if i != obs_idx]
        ax.legend(
            [handles[i] for i in order],
            [labels[i] for i in order],
            loc="upper left", frameon=False, fontsize=7.6
        )

    plt.tight_layout()
    fig.savefig(output_pdf, bbox_inches="tight")
    fig.savefig(output_png, dpi=300, bbox_inches="tight")
    plt.close(fig)


def run_campaign(data_dir: Path, output_dir: Path, n_runs: int = 500):
    data_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    d_values = [0.001, 0.01]
    datasets = {}

    for d in d_values:
        d_str = "d0.001" if d == 0.001 else "d0.01"
        rep_csv = data_dir / f"figure2a_simulated_trajectories_{d_str}_100y_n{n_runs}.csv"
        grid_csv = data_dir / f"figure2a_all_grid_trajectories_{d_str}_100y_n{n_runs}.csv"

        if not rep_csv.exists():
            print(f"\n🚀 [Simulation] Generating representative trajectories for d={d} (N={n_runs}, max_t=100)...")
            generate_figure2_trajectories(n_runs=n_runs, output_dir=data_dir, division_rate=d, suffix=f"_{d_str}_100y_n{n_runs}", max_t=100)
        else:
            print(f"✓ Found existing representative dataset: {rep_csv.name}")

        if not grid_csv.exists():
            print(f"\n🚀 [Simulation] Generating 36 grid trajectories for d={d} (N={n_runs}, max_t=100)...")
            generate_all_grid_trajectories(n_runs=n_runs, output_dir=data_dir, division_rate=d, suffix=f"_{d_str}_100y_n{n_runs}", max_t=100)
        else:
            print(f"✓ Found existing grid dataset: {grid_csv.name}")

        datasets[d] = {
            "rep": rep_csv,
            "grid": grid_csv,
        }

    # Generate all 12 figures
    modes = [
        ("rep_only", "rep"),
        ("rep_and_grid", "rep_grid"),
        ("grid_only", "grid_only"),
    ]
    horizons = [80, 100]

    count = 0
    print("\n🎨 [Plotting] Rendering 12 Figure 2a single-panel variations...")
    for d in d_values:
        d_str = "d0.001" if d == 0.001 else "d0.01"
        rep_csv = datasets[d]["rep"]
        grid_csv = datasets[d]["grid"]

        for h in horizons:
            for mode, mode_tag in modes:
                count += 1
                fname = f"Figure2a_{mode_tag}_{d_str}_{h}y"
                out_png = output_dir / f"{fname}.png"
                out_pdf = output_dir / f"{fname}.pdf"

                plot_single_panel_2a(
                    rep_csv=rep_csv,
                    grid_csv=grid_csv,
                    mode=mode,
                    max_age=h,
                    d_value=d,
                    output_png=out_png,
                    output_pdf=out_pdf,
                )
                print(f"  [{count}/12] Generated: {out_png.name}")

    print(f"\n✨ All 12 figures successfully generated in: {output_dir}")


def main():
    parser = argparse.ArgumentParser(description="Generate 12 Figure 2a variants")
    parser.add_argument("--n-runs", type=int, default=500, help="Tissues per run (default: 500)")
    parser.add_argument("--data-dir", type=str, default=str(Path(__file__).parent / "data"))
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).parent / "panel_a_12_variants"))
    args = parser.parse_args()

    run_campaign(Path(args.data_dir), Path(args.output_dir), n_runs=args.n_runs)


if __name__ == "__main__":
    main()
