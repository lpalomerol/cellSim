#!/usr/bin/env python3
"""
cellSim paper figure generator.
Run from repository root:  python docs/paper/figures/generate_figures.py
Outputs PNG (300 dpi) and PDF for each figure to docs/paper/figures/
"""

import sys
import os
import pathlib
import glob as _glob

import numpy as np
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.patches as FancyArrow
from matplotlib.patches import FancyArrowPatch
import networkx as nx

matplotlib.rcParams.update({
    "font.family": "serif",
    "font.size": 10,
    "axes.titlesize": 11,
    "axes.labelsize": 10,
    "legend.fontsize": 9,
    "figure.dpi": 150,
})

REPO_ROOT = pathlib.Path(__file__).resolve().parents[3]
TRACES_DIR = REPO_ROOT / "cmake-build-debug" / "traces"
OUT_DIR = pathlib.Path(__file__).resolve().parent

# ── colour palette (colour-blind-friendly) ───────────────────────────────────
C_PROTECTED  = "#2196F3"   # blue
C_NEOPLASTIC = "#F44336"   # red
C_TOTAL      = "#212121"   # dark grey
C_DEAD       = "#9E9E9E"   # light grey
C_TP53_PP    = "#4CAF50"   # green  (+/+)
C_TP53_PM    = "#FF9800"   # orange (+/-)
C_TP53_MM    = "#F44336"   # red    (-/-)
C_BB         = "#9C27B0"   # purple (Big Bang)
C_BAL        = "#2196F3"   # blue   (balanced)


def save(fig, name):
    for ext in ("png", "pdf"):
        path = OUT_DIR / f"{name}.{ext}"
        fig.savefig(path, dpi=300, bbox_inches="tight")
        print(f"  saved {path.relative_to(REPO_ROOT)}")
    plt.close(fig)


# ─────────────────────────────────────────────────────────────────────────────
# Fig. 1 — Clean Architecture layers (1A) + Agentic Cell cycle (1B)
# ─────────────────────────────────────────────────────────────────────────────
def fig1_architecture():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
    fig.suptitle("Figure 1 — cellSim Software Architecture", fontweight="bold")

    # ── 1A: Clean Architecture layers ────────────────────────────────────────
    ax1.set_xlim(0, 10)
    ax1.set_ylim(0, 8)
    ax1.axis("off")
    ax1.set_title("(A) Clean Architecture Layers", pad=8)

    layers = [
        ("Ports / Infrastructure", 6.2, "#E3F2FD",
         "ConfigLoader · PopulationCSVWriter\nSimulationRunner · TraceOutput"),
        ("Adapters", 4.4, "#BBDEFB",
         "PopulationRepository · ScenarioAdapter\nCellFactory"),
        ("Application", 2.6, "#90CAF9",
         "SimulationUseCase · EvolutionService\nMutationService"),
        ("Domain", 0.8, "#42A5F5",
         "Cell · Population · CellLifeStage\nD1/D2 model · Signal protocol"),
    ]
    for label, y, colour, sublabel in layers:
        rect = mpatches.FancyBboxPatch(
            (0.3, y), 9.4, 1.5,
            boxstyle="round,pad=0.05",
            facecolor=colour, edgecolor="#1565C0", linewidth=1.2,
        )
        ax1.add_patch(rect)
        ax1.text(5.0, y + 0.95, label, ha="center", va="center",
                 fontsize=10, fontweight="bold", color="#0D47A1")
        ax1.text(5.0, y + 0.35, sublabel, ha="center", va="center",
                 fontsize=7.5, color="#1A237E", style="italic")

    # dependency arrow
    ax1.annotate("", xy=(5, 6.2), xytext=(5, 2.3),
                 arrowprops=dict(arrowstyle="<-", color="#1565C0", lw=1.2,
                                 linestyle="dashed"))
    ax1.text(5.5, 4.2, "dependency\ndirection", ha="left", va="center",
             fontsize=7.5, color="#1565C0", style="italic")

    # ── 1B: Agentic Cell 6-phase cycle ───────────────────────────────────────
    ax2.set_xlim(-1.6, 1.6)
    ax2.set_ylim(-1.8, 1.8)
    ax2.axis("off")
    ax2.set_title("(B) Agentic Cell — 6-Phase Life Cycle", pad=8)

    phases = [
        "1. Sense\nenvironment",
        "2. Evaluate\ngenetic state",
        "3. Mutate\n(stochastic)",
        "4. Age /\nAccumulate",
        "5. Apoptosis\ncheck",
        "6. Divide or\nEmit signal",
    ]
    n = len(phases)
    angles = [np.pi / 2 - i * 2 * np.pi / n for i in range(n)]
    radius = 1.2
    colours = ["#E8F5E9", "#FFF9C4", "#FFF3E0", "#FCE4EC", "#EDE7F6", "#E3F2FD"]

    for i, (phase, angle) in enumerate(zip(phases, angles)):
        x = radius * np.cos(angle)
        y = radius * np.sin(angle)
        circle = plt.Circle((x, y), 0.33, color=colours[i],
                             ec="#455A64", linewidth=1.2, zorder=3)
        ax2.add_patch(circle)
        ax2.text(x, y, phase, ha="center", va="center",
                 fontsize=7.5, color="#212121", zorder=4)

        # arrow to next phase
        next_angle = angles[(i + 1) % n]
        nx_ = radius * np.cos(next_angle)
        ny_ = radius * np.sin(next_angle)
        mid_angle = (angle + next_angle) / 2
        dx = nx_ - x
        dy = ny_ - y
        length = np.hypot(dx, dy)
        # shorten arrow to circle boundary
        offset = 0.34 / length
        ax2.annotate("",
                     xy=(x + dx * (1 - offset), y + dy * (1 - offset)),
                     xytext=(x + dx * offset, y + dy * offset),
                     arrowprops=dict(arrowstyle="-|>", color="#455A64", lw=1.0),
                     zorder=2)

    ax2.text(0, 0, "Agentic\nCell", ha="center", va="center",
             fontsize=9, fontweight="bold", color="#37474F")

    save(fig, "fig1_architecture")


# ─────────────────────────────────────────────────────────────────────────────
# Fig. 2 — State transition graph
# ─────────────────────────────────────────────────────────────────────────────
def fig2_state_transitions():
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.set_title("Figure 2 — Cell Life Stage Transition Graph", fontweight="bold")
    ax.axis("off")

    # Node positions (x, y)
    pos = {
        "BASELINE":   (0.5, 3.0),
        "UNSTABLE":   (2.5, 3.0),
        "UNPROTECTED":(4.5, 3.0),
        "PRIMER":     (6.5, 3.0),
        "TUMORAL":    (8.5, 3.0),
        "DEAD":       (4.5, 0.5),
    }

    node_colours = {
        "BASELINE":    "#C8E6C9",
        "UNSTABLE":    "#FFF9C4",
        "UNPROTECTED": "#FFCCBC",
        "PRIMER":      "#F8BBD0",
        "TUMORAL":     "#EF9A9A",
        "DEAD":        "#B0BEC5",
    }
    node_labels = {
        "BASELINE":    "BASELINE\nTP53 +/+",
        "UNSTABLE":    "UNSTABLE\nTP53 +/-",
        "UNPROTECTED": "UNPROTECTED\nTP53 -/-",
        "PRIMER":      "PRIMER\nTP53 -/- ∧ D1>θ",
        "TUMORAL":     "TUMORAL\nD2>θ (immortal)",
        "DEAD":        "DEAD",
    }

    # Draw nodes manually
    for node, (x, y) in pos.items():
        colour = node_colours[node]
        ec = "#B71C1C" if node in ("TUMORAL", "DEAD") else "#1565C0"
        circle = plt.Circle((x, y), 0.55, color=colour,
                             ec=ec, linewidth=1.5, zorder=3)
        ax.add_patch(circle)
        ax.text(x, y, node_labels[node], ha="center", va="center",
                fontsize=7.5, zorder=4, color="#212121")

    ax.set_xlim(-0.3, 9.8)
    ax.set_ylim(-0.3, 4.5)

    # Edges: (from, to, label, connection_style, colour)
    edges = [
        ("BASELINE",    "UNSTABLE",    "TP53 +/+\n→ +/-",   "arc3,rad=0",   "#1565C0"),
        ("UNSTABLE",    "UNPROTECTED", "TP53 +/-\n→ -/-",   "arc3,rad=0",   "#E65100"),
        ("UNPROTECTED", "PRIMER",      "D1 > θ_D1\n(2.0)",  "arc3,rad=0",   "#880E4F"),
        ("PRIMER",      "TUMORAL",     "D2 > θ_D2\n(5.0)",  "arc3,rad=0",   "#B71C1C"),
        # DEAD edges (from different states)
        ("PRIMER",      "DEAD",        "apoptosis\n(D2≤θ)",  "arc3,rad=-0.3","#546E7A"),
        ("UNPROTECTED", "DEAD",        "immune\nelimination","arc3,rad=-0.2","#546E7A"),
        ("UNSTABLE",    "DEAD",        "apoptosis",          "arc3,rad=0.35","#546E7A"),
        # TUMORAL self-loop (represented separately below)
    ]

    def node_edge_point(src, dst, offset=0.55):
        x0, y0 = pos[src]
        x1, y1 = pos[dst]
        d = np.hypot(x1 - x0, y1 - y0)
        return (x0 + offset * (x1 - x0) / d,
                y0 + offset * (y1 - y0) / d,
                x1 - offset * (x1 - x0) / d,
                y1 - offset * (y1 - y0) / d)

    for src, dst, label, cstyle, colour in edges:
        x0, y0, x1, y1 = node_edge_point(src, dst)
        ax.annotate("", xy=(x1, y1), xytext=(x0, y0),
                    arrowprops=dict(arrowstyle="-|>", color=colour, lw=1.3,
                                    connectionstyle=cstyle),
                    zorder=2)
        mx = (pos[src][0] + pos[dst][0]) / 2
        my = (pos[src][1] + pos[dst][1]) / 2
        if "arc3" in cstyle and "rad=-" in cstyle:
            my -= 0.5
        elif "arc3" in cstyle and "rad=0.3" in cstyle:
            my += 0.5
        ax.text(mx, my, label, ha="center", va="center",
                fontsize=6.5, color=colour,
                bbox=dict(fc="white", ec="none", alpha=0.8, pad=1))

    # TUMORAL self-loop (division)
    tx, ty = pos["TUMORAL"]
    loop = matplotlib.patches.Arc((tx + 0.3, ty + 0.7), 0.6, 0.6,
                                   angle=0, theta1=0, theta2=300,
                                   color="#B71C1C", lw=1.3)
    ax.add_patch(loop)
    ax.annotate("", xy=(tx + 0.05, ty + 0.55), xytext=(tx + 0.6, ty + 0.58),
                arrowprops=dict(arrowstyle="-|>", color="#B71C1C", lw=1.0))
    ax.text(tx + 0.8, ty + 1.1, "divide\n(immortal)", ha="center",
            fontsize=6.5, color="#B71C1C")

    # Legend
    legend_items = [
        mpatches.Patch(color=node_colours["BASELINE"], label="Healthy / wild-type"),
        mpatches.Patch(color=node_colours["UNSTABLE"], label="First TP53 loss (+/-)"),
        mpatches.Patch(color=node_colours["UNPROTECTED"], label="Full TP53 loss (-/-)"),
        mpatches.Patch(color=node_colours["PRIMER"], label="Pre-tumoral (D1 > θ)"),
        mpatches.Patch(color=node_colours["TUMORAL"], label="Immortal / neoplastic"),
        mpatches.Patch(color=node_colours["DEAD"], label="Dead (apoptosis / immune)"),
    ]
    ax.legend(handles=legend_items, loc="upper left",
              framealpha=0.9, fontsize=8)

    save(fig, "fig2_state_transitions")


# ─────────────────────────────────────────────────────────────────────────────
# Helper: load a scenario CSV
# ─────────────────────────────────────────────────────────────────────────────
def load_csv(pattern):
    files = sorted(TRACES_DIR.glob(f"*{pattern}*/*.csv"))
    if not files:
        files = sorted(TRACES_DIR.glob(f"*{pattern}*_POPULATION.csv"))
    if not files:
        print(f"  WARNING: no CSV found for pattern '{pattern}'", file=sys.stderr)
        return None
    return pd.read_csv(files[0])


# ─────────────────────────────────────────────────────────────────────────────
# Fig. 3 — Scenario 09 temporal dynamics (stacked area + alive total)
# ─────────────────────────────────────────────────────────────────────────────
def fig3_scenario09_dynamics():
    df = load_csv("09")
    if df is None:
        return

    fig, ax = plt.subplots(figsize=(9, 4.5))
    ax.set_title("Figure 3 — Scenario 09 (Realistic Balanced): 80-year Cell Population Dynamics",
                 fontweight="bold")

    ax.stackplot(df["year"],
                 df["protected_alive"],
                 df["neoplastic_alive"],
                 labels=["Protected (healthy)", "Neoplastic (immortal)"],
                 colors=[C_PROTECTED, C_NEOPLASTIC], alpha=0.7)

    ax.plot(df["year"], df["alive_cells"], color=C_TOTAL, lw=1.5,
            linestyle="--", label="Total alive", zorder=5)

    # onset marker
    onset_year = 17
    ax.axvline(onset_year, color="#FF6F00", lw=1.5, linestyle=":")
    ax.text(onset_year + 0.5, ax.get_ylim()[1] * 0.92 if ax.get_ylim()[1] > 0 else 500,
            f"First neoplastic\n(year {onset_year})",
            color="#FF6F00", fontsize=8, va="top")

    ax.set_xlabel("Simulation year")
    ax.set_ylabel("Cell count")
    ax.legend(loc="upper left")
    ax.grid(axis="y", alpha=0.3)
    ax.set_xlim(0, df["year"].max())

    save(fig, "fig3_scenario09_dynamics")


# ─────────────────────────────────────────────────────────────────────────────
# Fig. 4 — TP53 genotype erosion (stacked area)
# ─────────────────────────────────────────────────────────────────────────────
def fig4_tp53_erosion():
    df = load_csv("09")
    if df is None:
        return

    fig, ax = plt.subplots(figsize=(9, 4))
    ax.set_title("Figure 4 — Scenario 09: TP53 Genotype Erosion over 80 Years",
                 fontweight="bold")

    ax.stackplot(df["year"],
                 df["tp53_plus_plus_pct"] * 100,
                 df["tp53_plus_minus_pct"] * 100,
                 df["tp53_minus_minus_pct"] * 100,
                 labels=["TP53 +/+ (wild-type)", "TP53 +/- (heterozygous)", "TP53 -/- (lost)"],
                 colors=[C_TP53_PP, C_TP53_PM, C_TP53_MM], alpha=0.8)

    ax.axvline(17, color="#FF6F00", lw=1.2, linestyle=":", label="Neoplastic onset (yr 17)")

    ax.set_xlabel("Simulation year")
    ax.set_ylabel("Fraction of alive cells (%)")
    ax.set_ylim(0, 100)
    ax.legend(loc="upper right", fontsize=8)
    ax.grid(axis="y", alpha=0.3)
    ax.set_xlim(0, df["year"].max())

    save(fig, "fig4_tp53_erosion")


# ─────────────────────────────────────────────────────────────────────────────
# Fig. 5 — Big Bang vs Balanced comparison (years 0–30)
# ─────────────────────────────────────────────────────────────────────────────
def fig5_bigbang_vs_balanced():
    df09 = load_csv("09")
    df10 = load_csv("10_big_bang_tumoral/10")
    if df09 is None or df10 is None:
        return

    max_year = min(df09["year"].max(), df10["year"].max(), 30)
    d09 = df09[df09["year"] <= max_year]
    d10 = df10[df10["year"] <= max_year]

    fig, axes = plt.subplots(1, 2, figsize=(12, 4.5), sharey=False)
    fig.suptitle("Figure 5 — Big Bang (Scenario 10) vs Balanced (Scenario 09): "
                 "Neoplastic Expansion over 30 Years", fontweight="bold")

    for ax, df, label, colour, title in [
        (axes[0], d09, "Scenario 09 — Balanced", C_BAL,
         "Balanced: slow neoplastic progression"),
        (axes[1], d10, "Scenario 10 — Big Bang", C_BB,
         "Big Bang: explosive clonal expansion"),
    ]:
        ax.fill_between(df["year"], df["neoplastic_alive"],
                        color=colour, alpha=0.25, label="Neoplastic alive")
        ax.plot(df["year"], df["neoplastic_alive"],
                color=colour, lw=2, label=label)
        ax.plot(df["year"], df["alive_cells"],
                color=C_TOTAL, lw=1.2, linestyle="--", label="Total alive")
        ax.set_xlabel("Simulation year")
        ax.set_ylabel("Cell count")
        ax.set_title(title)
        ax.legend(fontsize=8)
        ax.grid(alpha=0.3)
        ax.set_xlim(0, max_year)

    # annotate final values
    for ax, df, colour in [(axes[0], d09, C_BAL), (axes[1], d10, C_BB)]:
        last = df.iloc[-1]
        pct = last["neoplastic_alive"] / last["alive_cells"] * 100 if last["alive_cells"] > 0 else 0
        ax.annotate(f"Year {int(last['year'])}\n"
                    f"{int(last['neoplastic_alive'])} neoplastic\n"
                    f"({pct:.1f}% of alive)",
                    xy=(last["year"], last["neoplastic_alive"]),
                    xytext=(-50, 20), textcoords="offset points",
                    fontsize=7.5, color=colour,
                    arrowprops=dict(arrowstyle="->", color=colour, lw=0.8))

    save(fig, "fig5_bigbang_vs_balanced")


# ─────────────────────────────────────────────────────────────────────────────
# Fig. 6 — Scenario comparison bar chart (final neoplastic fraction)
# ─────────────────────────────────────────────────────────────────────────────
SCENARIO_META = {
    "01": ("01 — Baseline\n(no mutations, no div.)", "Control",         "grey"),
    "02": ("02 — Baseline\n(no mutations, high div.)","Control",        "grey"),
    "03": ("03 — BRCA1\nhigh mutations",             "Control",         "grey"),
    "04": ("04 — TP53\nhigh mutations",              "Control",         "#78909C"),
    "05": ("05 — TP53 high\n(unstable)",             "Control",         "#78909C"),
    "06": ("06 — Realistic\nbaseline",               "Realistic",       C_PROTECTED),
    "07": ("07 — Low TP53\ninstability",             "Realistic",       C_PROTECTED),
    "08": ("08 — High TP53\ninstability",            "Realistic",       "#1976D2"),
    "09": ("09 — Balanced\n(reference)",             "Realistic",       "#0D47A1"),
    "10": ("10 — Big Bang",                          "Big Bang",        C_BB),
    "11": ("11 — Big Bang\n(reprod.)",               "Big Bang",        "#7B1FA2"),
    "12": ("12 — Big Bang\n(low threshold)",         "Big Bang",        "#4A148C"),
}

def fig6_scenario_comparison():
    records = []
    for scenario_id, (label, group, colour) in SCENARIO_META.items():
        # find the CSV for this scenario
        pattern = f"{scenario_id}_"
        matches = list(TRACES_DIR.glob(f"{pattern}*/*_POPULATION.csv"))
        if not matches:
            # try flat layout
            matches = list(TRACES_DIR.glob(f"*/{pattern}*_POPULATION.csv"))
        if not matches:
            print(f"  WARNING: no CSV for scenario {scenario_id}", file=sys.stderr)
            continue
        df = pd.read_csv(matches[0])
        last = df.iloc[-1]
        alive = last["alive_cells"]
        neo = last["neoplastic_alive"]
        pct = neo / alive * 100 if alive > 0 else 0
        year = int(last["year"])
        records.append({
            "id": scenario_id,
            "label": label,
            "group": group,
            "colour": colour,
            "pct": pct,
            "year": year,
            "alive": int(alive),
            "neo": int(neo),
        })

    if not records:
        print("  WARNING: no scenario data found for Fig. 6", file=sys.stderr)
        return

    fig, ax = plt.subplots(figsize=(14, 5))
    ax.set_title("Figure 6 — Final Neoplastic Fraction by Scenario",
                 fontweight="bold")

    xs = range(len(records))
    bars = ax.bar(xs,
                  [r["pct"] for r in records],
                  color=[r["colour"] for r in records],
                  edgecolor="white", linewidth=0.8, alpha=0.85)

    # label each bar with time horizon and value
    for i, (bar, r) in enumerate(zip(bars, records)):
        h = bar.get_height()
        alive_str = f"{r['alive']}" if r["alive"] > 0 else "extinct"
        ax.text(bar.get_x() + bar.get_width() / 2, h + 1.5,
                f"{h:.0f}%\n@ yr {r['year']}\n({alive_str} alive)",
                ha="center", va="bottom", fontsize=6.5, color="#212121")

    ax.set_xticks(list(xs))
    ax.set_xticklabels([r["label"] for r in records],
                       fontsize=7.5, rotation=0)
    ax.set_ylabel("Neoplastic fraction among alive cells (%)")
    ax.set_ylim(0, 115)
    ax.axhline(100, color="black", lw=0.8, linestyle="--", alpha=0.4)
    ax.grid(axis="y", alpha=0.3)

    # group dividers
    groups = [r["group"] for r in records]
    for i in range(1, len(groups)):
        if groups[i] != groups[i - 1]:
            ax.axvline(i - 0.5, color="#BDBDBD", lw=1.0, linestyle="--")

    legend_handles = [
        mpatches.Patch(color="grey",    label="Control scenarios (01–05)"),
        mpatches.Patch(color=C_BAL,     label="Realistic scenarios (06–09)"),
        mpatches.Patch(color=C_BB,      label="Big Bang scenarios (10–12)"),
    ]
    ax.legend(handles=legend_handles, loc="upper left", fontsize=8)

    save(fig, "fig6_scenario_comparison")


# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    print("Generating cellSim paper figures...")
    print(f"  Traces dir : {TRACES_DIR}")
    print(f"  Output dir : {OUT_DIR}\n")

    fig1_architecture()
    print("  [1/6] Architecture diagram — done")

    fig2_state_transitions()
    print("  [2/6] State transition graph — done")

    fig3_scenario09_dynamics()
    print("  [3/6] Scenario 09 temporal dynamics — done")

    fig4_tp53_erosion()
    print("  [4/6] TP53 genotype erosion — done")

    fig5_bigbang_vs_balanced()
    print("  [5/6] Big Bang vs Balanced — done")

    fig6_scenario_comparison()
    print("  [6/6] Scenario comparison — done")

    print("\nAll figures generated successfully.")
