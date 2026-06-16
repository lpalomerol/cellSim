#!/usr/bin/env python3
"""
Publication-grade visualization of ABC-SMC Gen 2 results.

Generates:
  1. Marginal posteriors (with priors overlay)
  2. 2D pair plots (correlation structure)
  3. Phenotype clustering (low_delta vs neoplastic_div_rate)
  4. Clinical milestones distribution
"""

import csv
from pathlib import Path

import numpy as np
from scipy.stats import pearsonr

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib.gridspec import GridSpec
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("ERROR: matplotlib required")
    exit(1)


# Priors from abc_final_v4.py
PRIORS = {
    "brca1_rate": (0.008, 0.017),
    "low_delta": (0.15, 0.45),
    "high_delta": (0.05, 1.20),
    "neoplastic_div_rate": (0.10, 0.24),
}
PARAM_NAMES = list(PRIORS.keys())
PARAM_LABELS = {
    "brca1_rate": r"$\beta_{\mathrm{BRCA1}}$ (mutation rate)",
    "low_delta": r"$\delta_{\mathrm{low}}$ (selection pressure)",
    "high_delta": r"$\delta_{\mathrm{high}}$ (basal instability)",
    "neoplastic_div_rate": r"$\nu$ (division rate)",
}


def load_generation(csv_path: Path) -> tuple:
    """Load particles, weights, and clinical metrics from CSV."""
    particles = []
    weights = []
    onset_ages = []
    sat25_vals = []
    sat50_vals = []
    sat90_vals = []
    
    with open(csv_path) as f:
        rows = list(csv.DictReader(f))
    
    for r in rows:
        particles.append([float(r[p]) for p in PARAM_NAMES])
        weights.append(float(r["weight"]))
        onset_ages.append(float(r.get("onset_age", "nan")))
        sat25_vals.append(float(r.get("sat25", "nan")))
        sat50_vals.append(float(r.get("sat50", "nan")))
        sat90_vals.append(float(r.get("sat90", "nan")))
    
    particles = np.array(particles)
    weights = np.array(weights)
    weights /= weights.sum()
    
    return particles, weights, np.array(onset_ages), np.array(sat25_vals), np.array(sat50_vals), np.array(sat90_vals)


def plot_marginals(particles, weights, output_path: Path) -> None:
    """Generate marginal posterior distributions with prior overlays."""
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    axes = axes.flat
    
    for idx, param in enumerate(PARAM_NAMES):
        ax = axes[idx]
        lo, hi = PRIORS[param]
        
        # Posterior histogram
        bins = np.linspace(lo, hi, 30)
        ax.hist(particles[:, idx], bins=bins, weights=weights, density=True,
                alpha=0.7, color='steelblue', edgecolor='black', linewidth=1.2, label='Posterior')
        
        # Prior (uniform)
        ax.axhline(1 / (hi - lo), color='red', ls='--', lw=2.5, label='Prior', alpha=0.8)
        
        # Posterior mean ± std
        w_mean = np.average(particles[:, idx], weights=weights)
        w_std = np.sqrt(np.average((particles[:, idx] - w_mean)**2, weights=weights))
        ax.axvline(w_mean, color='darkblue', ls='-', lw=2, label=f'Mean: {w_mean:.4f}')
        ax.axvline(w_mean - w_std, color='darkblue', ls=':', lw=1.5, alpha=0.6)
        ax.axvline(w_mean + w_std, color='darkblue', ls=':', lw=1.5, alpha=0.6)
        
        ax.set_xlabel(PARAM_LABELS[param], fontsize=11, fontweight='bold')
        ax.set_ylabel('Density', fontsize=10)
        ax.grid(True, alpha=0.3)
        if idx == 0:
            ax.legend(loc='best', fontsize=9)
    
    fig.suptitle('ABC-SMC Gen 2: Marginal Posteriors', fontsize=13, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"  📊 {output_path}")
    plt.close(fig)


def plot_pair_correlations(particles, weights, output_path: Path) -> None:
    """Generate 2D pair scatter plots showing correlations."""
    fig = plt.figure(figsize=(14, 12))
    gs = GridSpec(4, 4, figure=fig, hspace=0.35, wspace=0.35)
    
    for i in range(4):
        for j in range(4):
            if i == j:
                # Diagonal: marginal histogram
                ax = fig.add_subplot(gs[i, j])
                bins = np.linspace(PRIORS[PARAM_NAMES[i]][0], PRIORS[PARAM_NAMES[i]][1], 20)
                ax.hist(particles[:, i], bins=bins, weights=weights, density=True,
                       alpha=0.6, color='steelblue', edgecolor='black')
                ax.set_ylabel('Density', fontsize=8)
            elif i < j:
                # Upper triangle: empty
                ax = fig.add_subplot(gs[i, j])
                ax.axis('off')
            else:
                # Lower triangle: scatter plots
                ax = fig.add_subplot(gs[i, j])
                colors = particles[:, 1]  # Color by low_delta
                sizes = 30 + 150 * weights / weights.max()
                sc = ax.scatter(particles[:, j], particles[:, i], c=colors, s=sizes,
                               alpha=0.5, cmap='viridis', edgecolors='black', linewidth=0.3)
                
                # Compute correlation
                corr, pval = pearsonr(particles[:, j], particles[:, i])
                ax.text(0.05, 0.95, f'r={corr:.2f}**' if pval<0.001 else f'r={corr:.2f}',
                       transform=ax.transAxes, fontsize=8, fontweight='bold',
                       bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8),
                       verticalalignment='top')
            
            # Labels
            if i == 3:
                ax.set_xlabel(PARAM_LABELS[PARAM_NAMES[j]], fontsize=9, fontweight='bold')
            if j == 0:
                ax.set_ylabel(PARAM_LABELS[PARAM_NAMES[i]], fontsize=9, fontweight='bold')
            
            if i < j or i == j:
                pass
            else:
                ax.grid(True, alpha=0.2)
    
    fig.suptitle('ABC-SMC Gen 2: Parameter Correlations', fontsize=13, fontweight='bold')
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"  📊 {output_path}")
    plt.close(fig)


def plot_phenotypes(particles, weights, output_path: Path) -> None:
    """Visualize two tumor phenotypes separated by low_delta."""
    low_delta = particles[:, 1]
    neo_div = particles[:, 3]
    brca1_rate = particles[:, 0]
    high_delta = particles[:, 2]
    
    # Quartile-based clustering
    q25 = np.quantile(low_delta, 0.25)
    q75 = np.quantile(low_delta, 0.75)
    pheno1_mask = low_delta < q25  # Low delta → aggressive
    pheno2_mask = low_delta > q75  # High delta → slow
    
    fig, axes = plt.subplots(2, 2, figsize=(13, 11))
    
    # Panel 1: Main scatter (phenotype space)
    ax = axes[0, 0]
    colors = low_delta
    sizes = 50 + 250 * weights / weights.max()
    
    # All points
    sc = ax.scatter(low_delta, neo_div, c=colors, s=sizes, alpha=0.5,
                   cmap='viridis', edgecolors='black', linewidth=0.5)
    
    # Cluster boundaries
    ax.axvline(q25, color='red', ls='--', lw=2, alpha=0.7, label=f'Q1={q25:.3f}')
    ax.axvline(q75, color='red', ls='--', lw=2, alpha=0.7, label=f'Q3={q75:.3f}')
    
    # Label phenotypes
    ax.text(0.15, 0.22, 'Phenotype 1\n(Aggressive)', fontsize=11, fontweight='bold',
           bbox=dict(boxstyle='round', facecolor='red', alpha=0.2))
    ax.text(0.40, 0.165, 'Phenotype 2\n(Basal)', fontsize=11, fontweight='bold',
           bbox=dict(boxstyle='round', facecolor='blue', alpha=0.2))
    
    ax.set_xlabel(r'$\delta_{\mathrm{low}}$ (selection pressure)', fontsize=11, fontweight='bold')
    ax.set_ylabel(r'$\nu$ (division rate)', fontsize=11, fontweight='bold')
    ax.set_title('Tumor Phenotypes: Selective Pressure vs Aggressiveness', fontsize=11, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right', fontsize=10)
    plt.colorbar(sc, ax=ax, label=r'$\delta_{\mathrm{low}}$')
    
    # Panel 2: Phenotype comparison (box plots)
    ax = axes[0, 1]
    pheno_data = [low_delta[pheno1_mask], low_delta[pheno2_mask]]
    bp = ax.boxplot(pheno_data, labels=['Aggressive', 'Basal'], patch_artist=True)
    for patch, color in zip(bp['boxes'], ['red', 'blue']):
        patch.set_facecolor(color)
        patch.set_alpha(0.3)
    ax.set_ylabel(r'$\delta_{\mathrm{low}}$', fontsize=11, fontweight='bold')
    ax.set_title('Selection Pressure Distribution', fontsize=11, fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    
    # Panel 3: BRCA1 rate by phenotype
    ax = axes[1, 0]
    w1 = weights[pheno1_mask] / weights[pheno1_mask].sum()
    w2 = weights[pheno2_mask] / weights[pheno2_mask].sum()
    brca1_p1 = brca1_rate[pheno1_mask]
    brca1_p2 = brca1_rate[pheno2_mask]
    
    ax.hist(brca1_p1, bins=15, weights=w1, alpha=0.6, label='Aggressive', color='red', edgecolor='black')
    ax.hist(brca1_p2, bins=15, weights=w2, alpha=0.6, label='Basal', color='blue', edgecolor='black')
    ax.set_xlabel(r'$\beta_{\mathrm{BRCA1}}$', fontsize=11, fontweight='bold')
    ax.set_ylabel('Weighted Density', fontsize=10)
    ax.set_title('BRCA1 Mutation Rate by Phenotype', fontsize=11, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3, axis='y')
    
    # Panel 4: High_delta by phenotype
    ax = axes[1, 1]
    high_delta_p1 = high_delta[pheno1_mask]
    high_delta_p2 = high_delta[pheno2_mask]
    
    ax.hist(high_delta_p1, bins=15, weights=w1, alpha=0.6, label='Aggressive', color='red', edgecolor='black')
    ax.hist(high_delta_p2, bins=15, weights=w2, alpha=0.6, label='Basal', color='blue', edgecolor='black')
    ax.set_xlabel(r'$\delta_{\mathrm{high}}$ (basal instability)', fontsize=11, fontweight='bold')
    ax.set_ylabel('Weighted Density', fontsize=10)
    ax.set_title('Basal Instability by Phenotype', fontsize=11, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3, axis='y')
    
    fig.suptitle('ABC-SMC Gen 2: Tumor Phenotype Characterization', fontsize=13, fontweight='bold')
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"  📊 {output_path}")
    plt.close(fig)


def plot_clinical_milestones(onset_ages, sat25_vals, sat50_vals, sat90_vals, weights, output_path: Path) -> None:
    """Visualize clinical outcomes: tumor onset and saturation milestones."""
    fig, axes = plt.subplots(1, 2, figsize=(13, 5))
    
    # Panel 1: Milestone distributions
    ax = axes[0]
    milestone_data = [
        (onset_ages, "Onset (≥5%)", '#2196F3'),
        (sat25_vals, "Saturation 25%", '#4CAF50'),
        (sat50_vals, "Saturation 50%", '#FF9800'),
        (sat90_vals, "Saturation 90%", '#F44336'),
    ]
    
    positions = np.arange(len(milestone_data))
    for pos, (data, label, color) in zip(positions, milestone_data):
        valid = data[~np.isnan(data) & (data > 0)]
        if len(valid) > 0:
            valid_weights = weights[~np.isnan(data) & (data > 0)]
            valid_weights /= valid_weights.sum()
            ax.hist(valid, bins=20, alpha=0.6, label=label, color=color, edgecolor='black', weights=valid_weights, density=True)
    
    ax.set_xlabel('Age (years)', fontsize=11, fontweight='bold')
    ax.set_ylabel('Weighted Density', fontsize=10)
    ax.set_title('Tumor Development Timeline (Posterior)', fontsize=11, fontweight='bold')
    ax.legend(fontsize=10, loc='upper right')
    ax.grid(True, alpha=0.3, axis='y')
    
    # Panel 2: Box plot comparison
    ax = axes[1]
    box_data = []
    box_labels = []
    box_colors = []
    milestone_colors = ['#2196F3', '#4CAF50', '#FF9800', '#F44336']
    
    for data, label, color in milestone_data:
        valid = data[~np.isnan(data) & (data > 0)]
        if len(valid) > 0:
            box_data.append(valid)
            box_labels.append(label)
            box_colors.append(color)
    
    bp = ax.boxplot(box_data, labels=box_labels, patch_artist=True)
    for patch, color in zip(bp['boxes'], box_colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.5)
    
    ax.set_ylabel('Age (years)', fontsize=11, fontweight='bold')
    ax.set_title('Milestone Distribution Summary', fontsize=11, fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    plt.setp(ax.xaxis.get_majorticklabels(), rotation=15, ha='right')
    
    fig.suptitle('ABC-SMC Gen 2: Clinical Milestones vs Kuchenbaecker Data', fontsize=13, fontweight='bold')
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"  📊 {output_path}")
    plt.close(fig)


def main() -> None:
    gen = 2
    out_dir = Path("results/abc_final_v4")
    gen_path = out_dir / f"gen_{gen:02d}.csv"
    
    if not gen_path.exists():
        print(f"[error] {gen_path} not found")
        return
    
    print(f"\n{'='*70}")
    print(f"  Generating Publication-Grade Figures (Gen {gen})")
    print(f"{'='*70}\n")
    
    # Load data
    particles, weights, onset_ages, sat25_vals, sat50_vals, sat90_vals = load_generation(gen_path)
    
    # Generate plots
    plot_marginals(particles, weights, out_dir / f"gen_{gen:02d}_marginals.png")
    plot_pair_correlations(particles, weights, out_dir / f"gen_{gen:02d}_correlations.png")
    plot_phenotypes(particles, weights, out_dir / f"gen_{gen:02d}_phenotypes.png")
    plot_clinical_milestones(onset_ages, sat25_vals, sat50_vals, sat90_vals, weights, 
                             out_dir / f"gen_{gen:02d}_milestones.png")
    
    print(f"\n{'='*70}")
    print(f"  ✅ All figures generated in {out_dir}/")
    print(f"{'='*70}\n")


if __name__ == "__main__":
    main()
