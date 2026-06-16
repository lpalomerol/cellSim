#!/usr/bin/env python3
"""
Analyze bimodality in ABC-SMC posterior: detect two tumor subtypes.

Checks for anticorrelation between low_delta (selection pressure) and
neoplastic_div_rate (aggressiveness), which indicates two distinct phenotypes:
  - Low low_delta + High neoplastic_div_rate: Fast aggressive tumors (high pressure)
  - High low_delta + Low neoplastic_div_rate: Slow basal tumors (low pressure)

Usage:
  python scripts/analyze_bimodality.py [--gen 3] [--output-dir results/abc_final_v4]
"""

import argparse
import csv
from pathlib import Path

import numpy as np
from scipy.stats import pearsonr

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False


def load_generation(csv_path: Path) -> tuple[np.ndarray, np.ndarray]:
    """Load particles and weights from generation CSV."""
    particles = []
    weights = []
    param_names = []
    
    with open(csv_path) as f:
        rows = list(csv.DictReader(f))
    
    if not rows:
        return None, None
    
    # Infer parameter names from first row
    param_names = [k for k in rows[0].keys() 
                   if k not in ('weight', 'distance_weighted_sse', 'onset_age', 'sat25', 'sat50', 'sat90')]
    
    for r in rows:
        particles.append([float(r[p]) for p in param_names])
        weights.append(float(r['weight']))
    
    return np.array(particles), np.array(weights), param_names


def analyze_bimodality(particles: np.ndarray, weights: np.ndarray, param_names: list) -> dict:
    """Compute bimodality metrics and phenotype separation."""
    weights /= weights.sum()
    
    # Find indices
    low_delta_idx = param_names.index('low_delta')
    neo_div_idx = param_names.index('neoplastic_div_rate')
    
    low_delta = particles[:, low_delta_idx]
    neo_div = particles[:, neo_div_idx]
    
    # Correlation
    corr, pval = pearsonr(low_delta, neo_div)
    
    # Weighted statistics
    w_mean_ld = np.average(low_delta, weights=weights)
    w_std_ld = np.sqrt(np.average((low_delta - w_mean_ld)**2, weights=weights))
    w_mean_neo = np.average(neo_div, weights=weights)
    w_std_neo = np.sqrt(np.average((neo_div - w_mean_neo)**2, weights=weights))
    
    # Bimodality: quartile-based clustering
    q25_ld = np.quantile(low_delta, 0.25)
    q75_ld = np.quantile(low_delta, 0.75)
    low_cluster_wt = np.sum(weights[low_delta < q25_ld])
    high_cluster_wt = np.sum(weights[low_delta > q75_ld])
    mid_cluster_wt = 1.0 - low_cluster_wt - high_cluster_wt
    
    # Phenotype assignment (if bimodal enough)
    phenotype_1_mask = low_delta < q25_ld
    phenotype_2_mask = low_delta > q75_ld
    
    results = {
        'correlation': float(corr),
        'pvalue': float(pval),
        'low_delta_mean': float(w_mean_ld),
        'low_delta_std': float(w_std_ld),
        'neo_div_mean': float(w_mean_neo),
        'neo_div_std': float(w_std_neo),
        'low_cluster_fraction': float(low_cluster_wt),
        'high_cluster_fraction': float(high_cluster_wt),
        'mid_cluster_fraction': float(mid_cluster_wt),
        'q25_low_delta': float(q25_ld),
        'q75_low_delta': float(q75_ld),
    }
    
    # Phenotype-specific stats
    if np.sum(phenotype_1_mask) > 0:
        w1 = weights[phenotype_1_mask] / weights[phenotype_1_mask].sum()
        results['phenotype_1_low_delta_mean'] = float(np.average(low_delta[phenotype_1_mask], weights=w1))
        results['phenotype_1_neo_div_mean'] = float(np.average(neo_div[phenotype_1_mask], weights=w1))
    
    if np.sum(phenotype_2_mask) > 0:
        w2 = weights[phenotype_2_mask] / weights[phenotype_2_mask].sum()
        results['phenotype_2_low_delta_mean'] = float(np.average(low_delta[phenotype_2_mask], weights=w2))
        results['phenotype_2_neo_div_mean'] = float(np.average(neo_div[phenotype_2_mask], weights=w2))
    
    return results, (low_delta, neo_div, weights, q25_ld, q75_ld)


def print_report(gen: int, results: dict) -> None:
    """Print bimodality analysis report."""
    print(f"\n{'='*70}")
    print(f"  Bimodality Analysis — Generation {gen}")
    print(f"{'='*70}")
    
    print(f"\n  Correlation (low_delta ↔ neoplastic_div_rate):")
    print(f"    r = {results['correlation']:.4f}  (p = {results['pvalue']:.6f})")
    
    if results['pvalue'] < 0.05 and abs(results['correlation']) > 0.3:
        print(f"    ✅ Significant anticorrelation detected")
    else:
        print(f"    ⚠️  Weak or insignificant correlation")
    
    print(f"\n  Weighted posterior stats:")
    print(f"    low_delta:            {results['low_delta_mean']:.5f} ± {results['low_delta_std']:.5f}")
    print(f"    neoplastic_div_rate:  {results['neo_div_mean']:.5f} ± {results['neo_div_std']:.5f}")
    
    print(f"\n  Cluster separation (quartile-based):")
    print(f"    Low cluster  (LD < Q1={results['q25_low_delta']:.4f}):  {results['low_cluster_fraction']:.1%}")
    print(f"    High cluster (LD > Q3={results['q75_low_delta']:.4f}): {results['high_cluster_fraction']:.1%}")
    print(f"    Middle (Q1-Q3):                             {results['mid_cluster_fraction']:.1%}")
    
    # Bimodality verdict
    bimodal_threshold = 0.25
    if results['low_cluster_fraction'] > bimodal_threshold and results['high_cluster_fraction'] > bimodal_threshold:
        print(f"\n  ✅ BIMODAL: Two distinct phenotypes detected")
        print(f"     Phenotype 1 (Low-delta): low_delta ≈ {results.get('phenotype_1_low_delta_mean', 'N/A')}")
        if 'phenotype_1_neo_div_mean' in results:
            print(f"                             neo_div ≈ {results['phenotype_1_neo_div_mean']:.4f}")
        print(f"     Phenotype 2 (High-delta): low_delta ≈ {results.get('phenotype_2_low_delta_mean', 'N/A')}")
        if 'phenotype_2_neo_div_mean' in results:
            print(f"                             neo_div ≈ {results['phenotype_2_neo_div_mean']:.4f}")
    else:
        print(f"\n  ⚠️  UNIMODAL: Phenotypes not yet well-separated (need tighter epsilon)")
    
    print()


def plot_bimodality(low_delta, neo_div, weights, q25_ld, q75_ld, output_path: Path) -> None:
    """Generate bimodality visualization."""
    if not HAS_MATPLOTLIB:
        print(f"[warn] matplotlib not available - skipping plot")
        return
    
    fig, axes = plt.subplots(1, 2, figsize=(12, 4.5))
    
    # Scatter: low_delta vs neoplastic_div_rate
    ax = axes[0]
    colors = low_delta
    sizes = 50 + 200 * weights / weights.max()
    sc = ax.scatter(low_delta, neo_div, c=colors, s=sizes, alpha=0.6, cmap='viridis', edgecolors='black', linewidth=0.5)
    
    # Add cluster boundaries
    ax.axvline(q25_ld, color='red', ls='--', lw=1.5, alpha=0.7, label=f'Q1={q25_ld:.3f}')
    ax.axvline(q75_ld, color='red', ls='--', lw=1.5, alpha=0.7, label=f'Q3={q75_ld:.3f}')
    
    ax.set_xlabel('low_delta (selection pressure)', fontsize=11, fontweight='bold')
    ax.set_ylabel('neoplastic_div_rate (aggressiveness)', fontsize=11, fontweight='bold')
    ax.set_title('Phenotype space: two tumor subtypes', fontsize=11, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(loc='best', fontsize=10)
    plt.colorbar(sc, ax=ax, label='low_delta')
    
    # Histogram: low_delta distribution
    ax = axes[1]
    ax.hist(low_delta, bins=25, weights=weights, alpha=0.7, color='steelblue', edgecolor='black', linewidth=1.2)
    ax.axvline(q25_ld, color='red', ls='--', lw=2, label=f'Q1')
    ax.axvline(q75_ld, color='red', ls='--', lw=2, label=f'Q3')
    
    ax.set_xlabel('low_delta (selection pressure)', fontsize=11, fontweight='bold')
    ax.set_ylabel('Weighted density', fontsize=11, fontweight='bold')
    ax.set_title('Marginal distribution (weighted)', fontsize=11, fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(fontsize=10)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    print(f"  📊 Plot saved: {output_path}")
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gen", type=int, default=None, help="Generation to analyze (default: last saved)")
    parser.add_argument("--output-dir", type=Path, default=Path("results/abc_final_v4"), help="ABC-SMC output directory")
    args = parser.parse_args()
    
    out_dir = args.output_dir
    if not out_dir.exists():
        print(f"[error] Output directory not found: {out_dir}")
        return
    
    # Find generation to analyze
    if args.gen is None:
        gen = -1
        while (out_dir / f"gen_{gen+1:02d}.csv").exists():
            gen += 1
        if gen < 0:
            print(f"[error] No generations found in {out_dir}")
            return
    else:
        gen = args.gen
    
    gen_path = out_dir / f"gen_{gen:02d}.csv"
    if not gen_path.exists():
        print(f"[error] Generation file not found: {gen_path}")
        return
    
    print(f"Loading {gen_path}...")
    particles, weights, param_names = load_generation(gen_path)
    
    if particles is None:
        print(f"[error] Failed to load generation")
        return
    
    # Analyze
    results, plot_data = analyze_bimodality(particles, weights, param_names)
    
    # Report
    print_report(gen, results)
    
    # Plot
    if HAS_MATPLOTLIB:
        plot_path = out_dir / f"gen_{gen:02d}_bimodality.png"
        plot_bimodality(*plot_data, plot_path)


if __name__ == "__main__":
    main()
