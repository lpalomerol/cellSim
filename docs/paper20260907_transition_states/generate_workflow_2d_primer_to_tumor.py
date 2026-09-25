#!/usr/bin/env python3
"""
generate_workflow_2d_primer_to_tumor.py
Publication-quality 2D workflow diagram: Primer -> Tumoral (Phases I+II unified, Phase III).
Matches exact C++ equations, threshold values (θ_intr=2.0, θ_inmune=5.0, d_neo=0.1584),
and formal biological descriptions from estados_celulares.md.
"""

import os
import matplotlib.pyplot as plt
import matplotlib.patches as patches

def draw_primer_to_tumor_workflow(output_dir):
    plt.rcParams["font.family"] = "DejaVu Sans"
    
    # Dimensions
    fig, ax = plt.subplots(figsize=(23, 11), dpi=300)
    ax.set_facecolor("#ffffff")
    fig.patch.set_facecolor("#ffffff")
    
    # State boxes definition with wide spacing between boxes
    # Layout X:
    # PRIMER: cx = 2.4
    # FUSION TUMORAL (FASES I + II): cx = 10.0
    # SUBCLONES (FASE III): cx = 18.4
    boxes = {
        "PRIMER": {
            "pos": (2.4, 6.0),
            "w": 3.4, "h": 2.3,
            "title": "ESTADO PRIMER (Pre-Tumoral)",
            "sub": "TP53 -/- | BRCA1 +/- o -/-\nCondición de Entrada: D_intr > θ_intr (2.0)\nCompuerta Binaria de 1 Solo Ciclo",
            "delta": "θ_inmune = 5.0 (Barrera Inmune Tisular)\nOrigen T2 (BRCA1+/-) vs Origen T3 (BRCA1-/-)",
            "fc": "#fce4ec", "ec": "#c2185b", "title_c": "#880e4f"
        },
        "APT_EXT": {
            "pos": (2.4, 1.8),
            "w": 3.4, "h": 1.9,
            "title": "Apoptosis Extrínseca (Ω_ext)",
            "sub": "Inmunoedición Tisular\n(extrinsic_apoptosis_immune_surveillance)",
            "delta": "Condición: D_inmune ≤ θ_inmune (5.0)\nFrecuente en origen T2 por menor velocidad",
            "fc": "#eceff1", "ec": "#455a64", "title_c": "#263238"
        },
        "TUMOR_FUNDADOR": {
            "pos": (10.0, 6.0),
            "w": 5.4, "h": 2.7,
            "title": "TRANSFORMACIÓN TUMORAL (Fases I & II Simultáneas)",
            "sub": "• Fase I: develop_neoplasm() → has_evaded_apoptosis_ = true\n  Blindaje irreversible / Pérdida total de checkpoints\n• Fase II: Desacoplamiento Mitótico Inmediato (\"Big Bang\")\n  Tasa mitótica acelerada d_neo = 0.1584 (~158x basal)",
            "delta": "Consumación simultánea en el mismo ciclo del escape tisular",
            "fc": "#e8eaf6", "ec": "#3f51b5", "title_c": "#1a237e"
        },
        "SUBCLON_HET": {
            "pos": (18.4, 7.8),
            "w": 3.8, "h": 2.2,
            "title": "Subclón Portador (BRCA1+/-)",
            "sub": "TP53 -/- | BRCA1 +/-\nRecombinación Homóloga Parcial",
            "delta": "δ_D_inm = 0.321 + edad·10⁻⁵\nPlasticidad somática / Hipermutabilidad",
            "fc": "#fff8e1", "ec": "#ffa000", "title_c": "#e65100"
        },
        "SUBCLON_HOM": {
            "pos": (18.4, 3.8),
            "w": 3.8, "h": 2.2,
            "title": "Subclón LOH Doble Nulo (BRCA1-/-)",
            "sub": "TP53 -/- | BRCA1 -/-\nFenotipo \"BRCAness\" (HR deficiente)",
            "delta": "δ_D_inm = 0.642 + edad·10⁻⁵\nInestabilidad masiva / Resistencia farmacológica",
            "fc": "#f3e5f5", "ec": "#7b1fa2", "title_c": "#4a148c"
        }
    }
    
    # Draw Background Container for Phase III (Intra-Tumoral Heterogeneity)
    phase3_bg = patches.FancyBboxPatch(
        (15.9, 2.3), 5.7, 8.2,
        boxstyle="round,pad=0.12,rounding_size=0.25",
        facecolor="#fafafa", edgecolor="#b0bec5",
        linewidth=1.8, linestyle="--", zorder=1
    )
    ax.add_patch(phase3_bg)
    ax.text(18.75, 10.1, "FASE III: Heterogeneidad Clonal Intra-tumoral", ha="center", va="center",
            fontsize=10.5, fontweight="bold", color="#37474f", zorder=2)
    
    # Draw State Boxes
    for k, data in boxes.items():
        cx, cy = data["pos"]
        w, h = data["w"], data["h"]
        
        rect = patches.FancyBboxPatch(
            (cx - w/2, cy - h/2), w, h,
            boxstyle="round,pad=0.08,rounding_size=0.15",
            facecolor=data["fc"], edgecolor=data["ec"],
            linewidth=2.2, zorder=3
        )
        ax.add_patch(rect)
        
        title_font = 8.5 if k != "TUMOR_FUNDADOR" else 8.8
        sub_font = 7.1 if k != "TUMOR_FUNDADOR" else 7.3
        
        ax.text(cx, cy + h*0.29, data["title"], ha="center", va="center",
                fontsize=title_font, fontweight="bold", color=data["title_c"], zorder=4)
        ax.text(cx, cy - h*0.03, data["sub"], ha="center", va="center",
                fontsize=sub_font, color="#333333", zorder=4)
        ax.text(cx, cy - h*0.35, f"[{data['delta']}]", ha="center", va="center",
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
            zorder=5
        )
        ax.add_patch(arrow)
        if label:
            mx = (x1 + x2) / 2 + lpos[0]
            my = (y1 + y2) / 2 + lpos[1]
            ax.text(mx, my, label, ha=lalign, va="center", fontsize=7.1, fontweight="semibold",
                    color=color, bbox=dict(boxstyle="round,pad=0.25", fc="#ffffff", ec=color, lw=0.8, alpha=0.95), zorder=6)

    # 1. Incoming arrows to PRIMER (Representing input origins)
    ax.annotate("", xy=(0.7, 6.5), xytext=(-0.2, 6.5),
                arrowprops=dict(arrowstyle="-|>", lw=2.0, color="#ad1457"))
    ax.text(-0.25, 6.5, "Desde T2 (BRCA1+/-)\n[δ_D_inm=0.321]", ha="right", va="center",
            fontsize=7.1, fontweight="bold", color="#ad1457")
    
    ax.annotate("", xy=(0.7, 5.5), xytext=(-0.2, 5.5),
                arrowprops=dict(arrowstyle="-|>", lw=2.0, color="#7b1fa2"))
    ax.text(-0.25, 5.5, "Desde T3 (BRCA1-/-)\n[δ_D_inm=0.642]", ha="right", va="center",
            fontsize=7.1, fontweight="bold", color="#7b1fa2")

    # 2. PRIMER -> APT_EXT (Immune editing elimination)
    draw_arrow(2.4, 6.0 - 2.3/2, 2.4, 1.8 + 1.9/2,
               label="Fracaso de Escape Tisular\nD_inmune ≤ θ_inmune (5.0)\n(Eliminación inmediata en el ciclo)",
               color="#455a64", style="--", lw=2.0, lpos=(1.85, 0.0))

    # 3. PRIMER -> TUMOR_FUNDADOR (Successful neoplastic transformation)
    draw_arrow(2.4 + 3.4/2, 6.0, 10.0 - 5.4/2, 6.0,
               label="Escape Inmune Tisular\nD_inmune > θ_inmune (5.0)\n(Ratio θ_inm/θ_intr = 2.5)",
               color="#c2185b", lw=2.4, lpos=(0, 1.15))

    # 4. TUMOR_FUNDADOR -> SUBCLONES (Branching according to founder or post-mitotic evolution)
    # To Heterozygous Subclone
    draw_arrow(10.0 + 5.4/2, 6.0 + 0.35, 18.4 - 3.8/2, 7.8,
               label="Clon Fundador procedente de T2\n(Conserva BRCA1 germinal +/-)",
               color="#ffa000", lw=2.0, rad=-0.06, lpos=(-0.35, 0.90))
    
    # To Homozygous Double Null Subclone
    draw_arrow(10.0 + 5.4/2, 6.0 - 0.35, 18.4 - 3.8/2, 3.8,
               label="Clon Fundador procedente de T3\n(Doble nulo BRCA1-/-)",
               color="#7b1fa2", lw=2.0, rad=0.06, lpos=(-0.35, -0.90))

    # 5. Post-transformation somatic mutation (SUBCLON_HET -> SUBCLON_HOM)
    draw_arrow(18.4, 7.8 - 2.2/2, 18.4, 3.8 + 2.2/2,
               label="Mutación Somática Post-Transformación en BRCA1 (+/- → -/-)\nP = (β_BRCA1 + k_inest) · D_intr\n[has_evaded_apoptosis_ = true: Sin Letalidad Sintética]",
               color="#d81b60", lw=2.2, lpos=(2.7, 0.0))

    # Plot boundaries and styling
    ax.set_xlim(-2.3, 22.8)
    ax.set_ylim(0.4, 10.8)
    ax.axis("off")

    # Title & Subtitle
    plt.title("Workflow 2D de Transición y Progresión: De Estado Primer a Transformación Tumoral Dinámica\n"
              "cellSim Model — Evasión Inmune Tisular, Desacoplamiento Mitótico y Evolución Clonal Intra-tumoral",
              fontsize=14, fontweight="bold", color="#1a202c", pad=22)

    # Biological Summary Box at bottom
    legend_text = (
        "LÓGICA BIOLÓGICA DE LA FASE PRIMER → TUMORAL:\n"
        "• Decisión Instantánea en Primer (1 solo ciclo): Evita sobreparametrización innecesaria. Evalúa el balance agresión genómica vs vigilancia tisular.\n"
        "• Eliminación Tisular (Ω_ext): Si D_inmune ≤ 5.0, la célula pre-maligna es aniquilada de inmediato por inmunoedición (frecuente en origen T2 por menor tasa de D_inmune).\n"
        "• Fases I & II Simultáneas: Al superar D_inmune > 5.0, la célula adquiere en ese mismo ciclo inmunidad permanente (has_evaded_apoptosis_ = true) y tasa neoplásica (d_neo = 0.1584).\n"
        "• Heterogeneidad Intra-tumoral (Fase III): Los clones heterocigotos (BRCA1+/-) sufren el 2º golpe sin riesgo de apoptosis (blindaje tumoral), generando subclones BRCAness resistentes."
    )
    plt.figtext(0.5, 0.02, legend_text, ha="center", fontsize=8.6,
                bbox=dict(boxstyle="round,pad=0.5", fc="#f8fafc", ec="#94a3b8", lw=1.2))

    os.makedirs(output_dir, exist_ok=True)
    pdf_path = os.path.join(output_dir, "fig_workflow_2d_primer_to_tumor.pdf")
    png_path = os.path.join(output_dir, "fig_workflow_2d_primer_to_tumor.png")
    
    plt.savefig(pdf_path, format="pdf", bbox_inches="tight")
    plt.savefig(png_path, format="png", dpi=300, bbox_inches="tight")
    plt.close()
    print(f"[OK] Diagrama Primer -> Tumor guardado en:\n  - {pdf_path}\n  - {png_path}")

if __name__ == "__main__":
    out_dir = "/home/luis/CLionProjects/cellSim/docs/paper20260907_transition_states/figures"
    draw_primer_to_tumor_workflow(out_dir)
