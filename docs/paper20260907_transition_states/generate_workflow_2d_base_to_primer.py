#!/usr/bin/env python3
"""
generate_workflow_2d_base_to_primer.py
Publication-quality 2D workflow diagram: Baseline -> T1 -> T2 -> (T3 / Direct) -> Primer.
Updated to match exact C++ equations, calibration parameters (Δ_low=0.107, Δ_high=0.214),
isUnstable/k_inestabilidad dynamic mutation thresholds, and formal naming from estados_celulares.md.
"""

import os
import matplotlib.pyplot as plt
import matplotlib.patches as patches

def draw_2d_workflow(output_dir):
    plt.rcParams["font.family"] = "DejaVu Sans"
    
    fig, ax = plt.subplots(figsize=(20, 9.8), dpi=300)
    ax.set_facecolor("#ffffff")
    fig.patch.set_facecolor("#ffffff")
    
    box_w = 2.6
    box_h = 1.65
    
    states = {
        "BASE": {
            "pos": (1.6, 4.8),
            "title": "1. Baseline (Homeostasis)",
            "sub": "TP53 +/+ | BRCA1 +/-\nΔ_TP53=0.0 | Δ_BRCA1=0.107",
            "delta": "δ_D_intr = edad·10⁻⁵\nδ_D_inm = 0.107 + edad·10⁻⁵",
            "fc": "#e8f5e9",
            "ec": "#2e7d32",
            "title_c": "#1b5e20"
        },
        "T1": {
            "pos": (5.5, 4.8),
            "title": "2. T1 (Inestabilidad Moderada)",
            "sub": "TP53 +/- | BRCA1 +/-\nΔ_TP53=0.107 | Δ_BRCA1=0.107",
            "delta": "δ_D_intr = 0.107 + edad·10⁻⁵\nδ_D_inm = 0.214 + edad·10⁻⁵",
            "fc": "#fffde7",
            "ec": "#fbc02d",
            "title_c": "#f57f17"
        },
        "T2": {
            "pos": (9.4, 4.8),
            "title": "3. T2 (Sin Checkpoints p53)",
            "sub": "TP53 -/- | BRCA1 +/-\nΔ_TP53=0.214 | Δ_BRCA1=0.107",
            "delta": "δ_D_intr = 0.214 + edad·10⁻⁵\nδ_D_inm = 0.321 + edad·10⁻⁵",
            "fc": "#fff3e0",
            "ec": "#fb8c00",
            "title_c": "#e65100"
        },
        "T3": {
            "pos": (13.6, 7.3),
            "title": "4. T3 (LOH Somático Doble Nulo)",
            "sub": "TP53 -/- | BRCA1 -/-\nΔ_TP53=0.214 | Δ_BRCA1=0.428",
            "delta": "δ_D_intr = 0.214 + edad·10⁻⁵\nδ_D_inm = 0.642 + edad·10⁻⁵",
            "fc": "#f3e5f5",
            "ec": "#8e24aa",
            "title_c": "#4a148c"
        },
        "PRIMER": {
            "pos": (17.8, 4.8),
            "title": "5. Primer (Pre-Tumoral)",
            "sub": "TP53 -/- | BRCA1 +/- o -/-\nCondición: D_intr > θ_intr (2.0)",
            "delta": "D_inm ≤ 5.0 → Ω_ext (Apt. Inmune)\nD_inm > 5.0 → Tumor (d_neo)",
            "fc": "#fce4ec",
            "ec": "#d81b60",
            "title_c": "#880e4f"
        },
        "APT_INTR": {
            "pos": (3.55, 1.8),
            "title": "Apoptosis Intrínseca (Ω_int)",
            "sub": "Letalidad Sintética\n(p53 funcional detecta pérdida BRCA1)",
            "delta": "Pérdida 2º alelo BRCA1 con TP53+",
            "fc": "#eceff1",
            "ec": "#546e7a",
            "title_c": "#263238"
        }
    }
    
    # Draw State Boxes
    for k, data in states.items():
        cx, cy = data["pos"]
        w, h = box_w, box_h
        if k == "PRIMER":
            w = 2.85
            h = 1.7
        elif k == "APT_INTR":
            w = 3.3
            h = 1.45
            
        rect = patches.FancyBboxPatch(
            (cx - w/2, cy - h/2), w, h,
            boxstyle="round,pad=0.08,rounding_size=0.15",
            facecolor=data["fc"], edgecolor=data["ec"],
            linewidth=2.2, zorder=3
        )
        ax.add_patch(rect)
        
        # Text inside box
        ax.text(cx, cy + h*0.27, data["title"], ha="center", va="center",
                fontsize=8.5, fontweight="bold", color=data["title_c"], zorder=4)
        ax.text(cx, cy - h*0.04, data["sub"], ha="center", va="center",
                fontsize=7.3, color="#333333", zorder=4)
        ax.text(cx, cy - h*0.34, f"[{data['delta']}]", ha="center", va="center",
                fontsize=6.8, fontweight="bold", color=data["ec"], zorder=4)

    # Helper for styled arrows
    def draw_arrow(x1, y1, x2, y2, label="", color="#37474f", style="-", lw=2.0, rad=0.0, lpos=(0,0), lalign="center"):
        arrow = patches.FancyArrowPatch(
            (x1, y1), (x2, y2),
            arrowstyle="-|>",
            connectionstyle=f"arc3,rad={rad}",
            mutation_scale=18,
            linewidth=lw,
            linestyle=style,
            color=color,
            zorder=2
        )
        ax.add_patch(arrow)
        if label:
            mx = (x1 + x2) / 2 + lpos[0]
            my = (y1 + y2) / 2 + lpos[1]
            ax.text(mx, my, label, ha=lalign, va="center", fontsize=7.1, fontweight="semibold",
                    color=color, bbox=dict(boxstyle="round,pad=0.25", fc="#ffffff", ec=color, lw=0.8, alpha=0.95), zorder=5)

    # 1. Baseline -> T1
    draw_arrow(1.6 + box_w/2, 4.8, 5.5 - box_w/2, 4.8,
               label="Mut TP53 (+/+ → +/-)\nμ_TP53 = 0.0030 año⁻¹\n(k_inest=0, TP53+/+)",
               color="#f57f17", lpos=(0, 1.25))

    # 2. T1 -> T2
    draw_arrow(5.5 + box_w/2, 4.8, 9.4 - box_w/2, 4.8,
               label="Mut TP53 (+/- → -/-)\n(μ_TP53 + k_inest) · D_intr\n[isUnstable=true]",
               color="#e65100", lpos=(0, 1.25))

    # 3. T2 -> T3 (Top Branch: Somatic BRCA1 mutation)
    draw_arrow(9.4 + 0.3, 4.8 + box_h/2, 13.6 - box_w/2, 7.3,
               label="Mut BRCA1 somática (+/- → -/-)\n(β_BRCA1 + k_inest) · D_intr\n[Sin p53: célula sobrevive]",
               color="#7b1fa2", lpos=(-0.55, 0.55), rad=-0.06)

    # 4. T3 -> PRIMER (Top branch convergence: damage accumulation)
    draw_arrow(13.6 + box_w/2, 7.3, 17.8 - 0.4, 4.8 + box_h/2,
               label="Acumulación D_intr > θ_intr (2.0)\n(Máxima velocidad: δ_D_inm = 0.642)",
               color="#d81b60", lpos=(0.4, 0.70), rad=-0.06)

    # 5. T2 -> PRIMER (Bottom / Direct Branch: damage accumulation without LOH)
    draw_arrow(9.4 + box_w/2, 4.8 - 0.2, 17.8 - 1.45, 4.8 - 0.2,
               label="Vía Directa desde T2: Acumulación continua D_intr > θ_intr (2.0)\n(Conservando BRCA1 germinal heterocigoto +/- | δ_D_inm = 0.321)",
               color="#ad1457", lw=2.4, lpos=(0, -0.65))

    # 6. Checkpoint Apoptosis (Letalidad Sintética)
    # Baseline -> APT_INTR
    draw_arrow(1.6, 4.8 - box_h/2, 3.55 - 0.7, 1.8 + 1.45/2,
               label="Mut BRCA1 (-/-)\nP = β_BRCA1 · D_intr\n(k_inest=0)",
               color="#546e7a", style="--", lw=1.5, lpos=(-0.45, 0.0), rad=0.08)
    # T1 -> APT_INTR
    draw_arrow(5.5, 4.8 - box_h/2, 3.55 + 0.7, 1.8 + 1.45/2,
               label="Mut BRCA1 (-/-)\nP = (β_BRCA1 + k_inest) · D_intr\n[isUnstable=true]",
               color="#546e7a", style="--", lw=1.5, lpos=(0.50, 0.0), rad=-0.08)

    # Plot boundaries and styling
    ax.set_xlim(-0.2, 19.5)
    ax.set_ylim(0.4, 9.6)
    ax.axis("off")

    # Title & Subtitle
    plt.title("Workflow 2D de Transición Celular: De Homeostasis (Baseline) a Estado Pre-Tumoral (Primer)\n"
              "cellSim Model — Dinámica Formal de Inestabilidad (D_intrínseca y D_inmune)",
              fontsize=14, fontweight="bold", color="#1a202c", pad=20)

    # Biological Summary Box at bottom
    legend_text = (
        "LÓGICA BIOLÓGICA DE LA FASE BASELINE → PRIMER:\n"
        "• Barrera de p53 (Baseline → T1 → T2): El daño celular solo se acumula de forma crítica tras la inactivación bialélica de TP53 (-/-).\n"
        "• Letalidad Sintética (Ω_int): Si BRCA1 pierde su 2º alelo con p53 funcional (Baseline o T1), se activa Apoptosis Intrínseca inmediata.\n"
        "• Bifurcación en T2: Tras perder TP53, la célula puede transicionar a Primer directamente (BRCA1+/-) o sufrir LOH somático (T3, BRCA1-/-) y luego ir a Primer.\n"
        "• Compuerta de Primer y Ratio de Barrera: Entrada cuando D_intr > θ_intr (2.0). Resolución binaria inmune: D_inmune > θ_inmune (5.0) evade apoptosis (Ratio θ_inmune/θ_intr = 2.5)."
    )
    plt.figtext(0.5, 0.025, legend_text, ha="center", fontsize=8.6,
                bbox=dict(boxstyle="round,pad=0.5", fc="#f8fafc", ec="#94a3b8", lw=1.2))

    os.makedirs(output_dir, exist_ok=True)
    pdf_path = os.path.join(output_dir, "fig_workflow_2d_base_to_primer.pdf")
    png_path = os.path.join(output_dir, "fig_workflow_2d_base_to_primer.png")
    
    plt.savefig(pdf_path, format="pdf", bbox_inches="tight")
    plt.savefig(png_path, format="png", dpi=300, bbox_inches="tight")
    plt.close()
    print(f"[OK] Diagrama 2D guardado en:\n  - {pdf_path}\n  - {png_path}")

if __name__ == "__main__":
    out_dir = "/home/luis/CLionProjects/cellSim/docs/paper20260907_transition_states/figures"
    draw_2d_workflow(out_dir)
