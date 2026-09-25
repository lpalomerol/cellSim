#!/usr/bin/env python3
"""
generate_workflow_3d.py
Deterministic, Publication-Grade 3D Multidimensional State Transition Workflow for cellSim.
Strictly calibrated with working_paper_secondary_validation.md (2026-08-28) and estado_1.png.
"""

import os
import json
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection
from matplotlib.patches import FancyArrowPatch
from mpl_toolkits.mplot3d import proj3d

class Arrow3D(FancyArrowPatch):
    def __init__(self, xs, ys, zs, *args, **kwargs):
        super().__init__((0, 0), (0, 0), *args, **kwargs)
        self._verts3d = xs, ys, zs

    def do_3d_projection(self, renderer=None):
        xs3d, ys3d, zs3d = self._verts3d
        xs, ys, zs = proj3d.proj_transform(xs3d, ys3d, zs3d, self.axes.M)
        self.set_positions((xs[0], ys[0]), (xs[1], ys[1]))
        return np.min(zs)

def create_3d_workflow(output_dir="."):
    plt.rcParams['font.family'] = 'DejaVu Sans'
    
    fig = plt.figure(figsize=(22, 16), dpi=300)
    ax = fig.add_subplot(111, projection='3d')
    ax.set_facecolor('#ffffff')

    # Viewing angle: highly readable isometric perspective
    ax.view_init(elev=23, azim=-48)

    # 1. Semi-transparent Layer Planes
    def draw_plane(z_val, color, border_col, alpha, label_text, sublabel):
        px = [-0.6, 4.4, 4.4, -0.6]
        py = [-0.8, -0.8, 3.4, 3.4]
        pz = [z_val, z_val, z_val, z_val]
        verts = [list(zip(px, py, pz))]
        poly = Poly3DCollection(verts, alpha=alpha, facecolor=color, edgecolor=border_col, linewidth=1.2, linestyle='--')
        ax.add_collection3d(poly)
        
        # Plane badge on the outer left edge
        ax.text(-0.7, 0.4, z_val + 0.18, f"{label_text}\n{sublabel}", color='#1a1a1a', fontsize=9.5, fontweight='bold',
                ha='right', va='center',
                bbox=dict(boxstyle="square,pad=0.4", fc="white", ec=border_col, lw=1.5, alpha=0.95))

    draw_plane(0.0, '#e8f5e9', '#2e7d32', 0.42, "PLANO 0 (Z=0): HOMEOSTASIS Y DAÑO BASAL", "D_intrínseca < 2.0 | Control Tisular")
    draw_plane(2.0, '#fff8e1', '#f57f17', 0.42, "PLANO 1 (Z=1): ESTADO PRE-TUMORAL (PRIMER)", "D_intr ≥ 2.0, D_inm < 5.0 | Vigilancia Tisular")
    draw_plane(4.0, '#ffebee', '#c62828', 0.42, "PLANO 2 (Z=2): DESTINO NEOPLÁSICO (TUMORAL)", "D_inmune ≥ 5.0 | Evasión Inmune & Proliferación d_neo")

    # 2. Node definitions
    # Structure: ID: (x, y, z, title, desc, fc, ec, shape, size, (ox, oy, oz), text_align)
    nodes = {
        # Z = 0: Basal / Instability
        'Base': (0.0, 0.0, 0.0, 'Base (BASELINE)', 'TP53 +/+\nBRCA1 +/-\nHomeostasis', '#c8e6c9', '#2e7d32', 'o', 450, (0.0, -0.42, 0.0), 'center'),
        'St1':  (1.6, 0.0, 0.0, 'St. 1 (UNSTABLE)', 'TP53 +/-\nBRCA1 +/-\n↑ δ_BRCA+ (0.107)', '#fff59d', '#f57f17', 'o', 450, (0.0, -0.42, 0.0), 'center'),
        'St2':  (3.2, 0.0, 0.0, 'St. 2 (UNPROTECTED)', 'TP53 -/-\nBRCA1 +/-\n↑↑ δ_BRCA- (0.353)', '#ffe082', '#e65100', 'o', 450, (0.0, -0.42, 0.0), 'center'),
        'St3':  (3.2, 2.2, 0.0, 'St. 3 (UNPROT. LOH)', 'TP53 -/-\nBRCA1 -/-\n↑↑↑ 2·δ_BRCA- (0.707)', '#ffcc80', '#bf360c', 'o', 450, (0.0, 0.42, 0.0), 'center'),
        
        # Z = 2.0: PRIMER (Pre-tumoral)
        'P1':   (3.2, 0.0, 2.0, 'PRIMER (Pre-T1)', 'TP53 -/-, BRCA1 +/-\nD_intr ≥ 2.0\nVulnerable a Aclaramiento', '#f8bbd0', '#c2185b', 'o', 450, (-0.62, 0.0, 0.0), 'center'),
        'P2':   (3.2, 2.2, 2.0, 'PRIMER (Pre-T2)', 'TP53 -/-, BRCA1 -/-\nD_intr ≥ 2.0\nLOH Hipermutado', '#f48fb1', '#ad1457', 'o', 450, (0.62, 0.0, 0.0), 'center'),
        
        # Z = 4.0: TUMORAL (Evasion)
        'T1':   (3.2, 0.0, 4.0, 'T1 (TUMORAL)', 'TP53 -/-, BRCA1 +/-\nD_inm ≥ 5.0 (Evasión)\nMitosis d_neo (0.158)', '#ef5350', '#b71c1c', 'o', 500, (-0.62, 0.0, 0.0), 'center'),
        'T2':   (3.2, 2.2, 4.0, 'T2 (TUMORAL LOH)', 'TP53 -/-, BRCA1 -/-\nD_inm ≥ 5.0 (Evasión)\nClon LOH Agresivo', '#e53935', '#7f0000', 'o', 500, (0.62, 0.0, 0.0), 'center'),
        
        # Apoptosis Sinks
        'APT_INTR': (0.8, 2.3, 0.0, 'APT. INTRÍNSECA', 'Letalidad Sintética (p53)\nBRCA1-/- con p53+/+ o +/-', '#cfd8dc', '#37474f', 'X', 550, (0.0, 0.38, 0.0), 'center'),
        'APT_EXTR': (4.2, 1.1, 2.0, 'APT. EXTRÍNSECA', 'Aclaramiento Tisular\nD_inmune < 5.0 en PRIMER', '#b0bec5', '#263238', 'X', 550, (0.48, 0.0, 0.0), 'center'),
    }

    # Render Nodes
    for k, (x, y, z, title, desc, fc, ec, marker_shape, size, (ox, oy, oz), align) in nodes.items():
        ax.scatter([x], [y], [z], color=fc, edgecolors=ec, s=size, depthshade=False, linewidth=2.5, marker=marker_shape, zorder=15)
        
        full_text = f"{title}\n{desc}"
        ax.text(x + ox, y + oy, z + oz, full_text, color='#111111', fontsize=7.8, fontweight='bold', ha=align, va='center',
                bbox=dict(boxstyle="round,pad=0.35", fc=fc, ec=ec, lw=1.4, alpha=0.95), zorder=20)

    # 3. Directed 3D Arrows
    def add_arrow(start, end, label, color='#333333', style='-', lw=2.0, label_offset=(0,0,0)):
        xs = [start[0], end[0]]
        ys = [start[1], end[1]]
        zs = [start[2], end[2]]
        
        arrow = Arrow3D(xs, ys, zs, mutation_scale=15, lw=lw, arrowstyle="-|>",
                        linestyle=style, color=color, zorder=12)
        ax.add_artist(arrow)
        
        if label:
            mx = (start[0] + end[0]) / 2.0 + label_offset[0]
            my = (start[1] + end[1]) / 2.0 + label_offset[1]
            mz = (start[2] + end[2]) / 2.0 + label_offset[2]
            ax.text(mx, my, mz, label, fontsize=7.2, color=color, fontweight='bold', ha='center', va='center',
                    bbox=dict(boxstyle="round,pad=0.2", fc='#ffffff', ec=color, lw=0.9, alpha=0.92), zorder=25)

    # --- Z=0: Mutational Degradation ---
    # Base -> St. 1
    add_arrow((0.25, 0.0, 0.0), (1.35, 0.0, 0.0), "Mut TP53 (+/+ → +/-)\n[μ_TP53 = 0.003]", '#f57f17', label_offset=(0, -0.22, 0))
    # St. 1 -> St. 2
    add_arrow((1.85, 0.0, 0.0), (2.95, 0.0, 0.0), "Mut TP53 (+/- → -/-)\n[μ_TP53 · D_intr]", '#e65100', label_offset=(0, -0.22, 0))
    # St. 2 -> St. 3
    add_arrow((3.2, 0.25, 0.0), (3.2, 1.95, 0.0), "Mut BRCA1 (+/- → -/-)\n[β_BRCA1 · D_intr] (Sin p53)", '#bf360c', label_offset=(0.38, 0, 0))

    # --- Z=0 -> APT_INTR (Synthetic Lethality) ---
    add_arrow((0.0, 0.25, 0.0), (0.65, 2.05, 0.0), "Mut BRCA1 (-/-)\nCheckpoint p53 activo", '#37474f', style='--', lw=1.6, label_offset=(-0.15, 0.12, 0))
    add_arrow((1.6, 0.25, 0.0), (0.95, 2.05, 0.0), "Mut BRCA1 (-/-)\nCheckpoint p53 activo", '#37474f', style='--', lw=1.6, label_offset=(0.2, 0.12, 0))

    # --- Z=0 -> Z=2.0 (Threshold D_intr ≥ 2.0) ---
    add_arrow((3.2, 0.0, 0.3), (3.2, 0.0, 1.7), "D_intr ≥ 2.0 (θ_intr)\nAcumulación de daño", '#c2185b', lw=2.4, label_offset=(0.42, 0, 0))
    add_arrow((3.2, 2.2, 0.3), (3.2, 2.2, 1.7), "D_intr ≥ 2.0 (θ_intr)\nColapso LOH masivo", '#ad1457', lw=2.4, label_offset=(0.42, 0, 0))

    # --- Z=2.0: Pre-Tumoral Mutation (P1 -> P2) ---
    add_arrow((3.2, 0.25, 2.0), (3.2, 1.95, 2.0), "Mut BRCA1 (-/-)\n[β_BRCA1 · D_intr]", '#ad1457', style=':', lw=1.8, label_offset=(0.38, 0, 0))

    # --- Z=2.0 -> APT_EXTR (Immune Clearance on Plane 1) ---
    add_arrow((3.35, 0.15, 2.0), (4.0, 0.95, 2.0), "D_inmune < 5.0\nAclaramiento Tisular", '#263238', style='--', lw=1.6, label_offset=(0.15, -0.15, 0))
    add_arrow((3.35, 2.05, 2.0), (4.0, 1.25, 2.0), "D_inmune < 5.0\nAclaramiento Tisular", '#263238', style='--', lw=1.6, label_offset=(0.15, 0.15, 0))

    # --- Z=2.0 -> Z=4.0 (Threshold D_inm ≥ 5.0, Immune Evasion) ---
    add_arrow((3.2, 0.0, 2.3), (3.2, 0.0, 3.7), "D_inmune ≥ 5.0 (θ_inm)\nEvasión & Inmortalización", '#d32f2f', lw=2.8, label_offset=(0.45, 0, 0))
    add_arrow((3.2, 2.2, 2.3), (3.2, 2.2, 3.7), "D_inmune ≥ 5.0 (θ_inm)\nEvasión & Inmortalización", '#b71c1c', lw=2.8, label_offset=(0.45, 0, 0))

    # --- Z=4.0: Horizontal Transition (T1 -> T2) ---
    add_arrow((3.2, 0.25, 4.0), (3.2, 1.95, 4.0), "Mut BRCA1 (+/- → -/-)\n[β_BRCA1 · D_intr]", '#7f0000', lw=2.2, label_offset=(0.38, 0, 0))

    # --- Z=4.0: Proliferative Mitotic Loops (Big Bang) ---
    theta = np.linspace(0, 1.6 * np.pi, 30)
    # T1 loop
    lx1 = 3.2 + 0.35 * np.cos(theta)
    ly1 = 0.0 - 0.35 * np.sin(theta)
    lz1 = np.full_like(lx1, 4.0)
    ax.plot(lx1, ly1, lz1, color='#b71c1c', lw=2.2)
    ax.text(3.6, -0.4, 4.0, "Mitosis Clonal\n[d_neo = 0.158]", color='#b71c1c', fontsize=7.2, fontweight='bold')

    # T2 loop
    lx2 = 3.2 + 0.35 * np.cos(theta)
    ly2 = 2.2 + 0.35 * np.sin(theta)
    lz2 = np.full_like(lx2, 4.0)
    ax.plot(lx2, ly2, lz2, color='#7f0000', lw=2.2)
    ax.text(3.6, 2.6, 4.0, "Mitosis Hipermutada\n[d_neo acelerada]", color='#7f0000', fontsize=7.2, fontweight='bold')

    # 4. Axes, Ticks and Styling
    ax.set_xlim(-0.8, 4.8)
    ax.set_ylim(-0.9, 3.6)
    ax.set_zlim(-0.5, 4.8)

    ax.set_xticks([0.0, 1.6, 3.2])
    ax.set_xticklabels(['TP53 +/+\n(Wild-Type)', 'TP53 +/-\n(Heterocigoto)', 'TP53 -/-\n(Inactivación Bialélica)'], fontsize=9.5, fontweight='semibold')
    ax.set_xlabel('\n\nEje X: Supresión Tumoral y Checkpoint (TP53)', fontsize=11.5, fontweight='bold', labelpad=20)

    ax.set_yticks([0.0, 2.2])
    ax.set_yticklabels(['BRCA1 +/-\n(Portador Germinal)', 'BRCA1 -/-\n(Pérdida Somática LOH)'], fontsize=9.5, fontweight='semibold')
    ax.set_ylabel('\n\nEje Y: Reparación Homóloga de ADN (BRCA1)', fontsize=11.5, fontweight='bold', labelpad=20)

    ax.set_zticks([0.0, 2.0, 4.0])
    ax.set_zticklabels(['Z=0: Daño Basal\n(D_intr < 2.0)', 'Z=1: PRIMER\n(D_intr ≥ 2.0, D_inm < 5.0)', 'Z=2: TUMORAL\n(D_inm ≥ 5.0)'], fontsize=9.5, fontweight='semibold')
    ax.set_zlabel('Eje Z: Espacio Fenotípico / Inestabilidad & Evasión', fontsize=11.5, fontweight='bold', labelpad=22)

    # Panes
    ax.xaxis.pane.fill = True
    ax.yaxis.pane.fill = True
    ax.zaxis.pane.fill = True
    ax.xaxis.pane.set_facecolor('#fafafa')
    ax.yaxis.pane.set_facecolor('#f5f5f5')
    ax.zaxis.pane.set_facecolor('#eeeeee')
    ax.xaxis.pane.set_edgecolor('#d0d0d0')
    ax.yaxis.pane.set_edgecolor('#d0d0d0')
    ax.zaxis.pane.set_edgecolor('#d0d0d0')
    ax.grid(True, linestyle=':', alpha=0.5)

    # Title
    plt.title("Workflow Multidimensional 3D del Modelo cellSim: Estados, Checkpoints y Destinos Celulares\n"
              "Formalización conforme a Working Paper 2026-01 (Validación Secundaria ABC-SMC, JAMA 2017 Cohort)",
              fontsize=13.5, fontweight='bold', pad=25)

    # Biological Rules Banner
    info_text = (
        "REGLAS BIOLÓGICAS FUNDAMENTALES DEL MODELO:\n"
        "1. Letalidad Sintética (Fase 1): Mutación BRCA1 -/- con TP53 funcional (+/+ o +/-) induce Apoptosis Intrínseca inmediata.\n"
        "2. Supervivencia LOH: Requiere pérdida bialélica previa de TP53 (-/-) para evadir el suicidio celular mediado por p53.\n"
        "3. Barrera PRIMER (θ_intr = 2.0): Dispara la fase de reconocimiento tisular y riesgo de aclaramiento por Apoptosis Extrínseca.\n"
        "4. Barrera TUMORAL (θ_inm = 5.0, Ratio 2.5×): Evasión inmune irreversible, inmortalización y división clonal acelerada (d_neo = 0.158).\n"
        "5. Subtipos Tumorales Emergentes: Fenotipo A (Escape Proliferativo, d_neo=0.189) vs. Fenotipo B (Contención Tisular, d_neo=0.133)."
    )
    plt.figtext(0.08, 0.02, info_text, fontsize=8.8,
                bbox=dict(boxstyle="round,pad=0.5", fc="#f8f9fa", ec="#b0bec5", lw=1.3))

    os.makedirs(output_dir, exist_ok=True)
    pdf_path = os.path.join(output_dir, "fig_workflow_3d_transitions.pdf")
    png_path = os.path.join(output_dir, "fig_workflow_3d_transitions.png")

    plt.savefig(pdf_path, format='pdf', bbox_inches='tight')
    plt.savefig(png_path, format='png', dpi=300, bbox_inches='tight')
    plt.close()

    print(f"[OK] Diagrama 3D generado con éxito en:\n  - {pdf_path}\n  - {png_path}")

    # Generate interactive HTML viewer
    html_path = os.path.join(output_dir, "fig_workflow_3d_interactive.html")
    generate_interactive_html(html_path)
    print(f"[OK] Visualizador 3D Interactivo generado en:\n  - {html_path}")

def generate_interactive_html(html_file):
    html_content = """<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>cellSim 3D Workflow Interactivo</title>
    <script src="https://cdn.plot.ly/plotly-2.27.0.min.js"></script>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; margin: 0; padding: 20px; background: #f8f9fa; }
        .header { text-align: center; margin-bottom: 15px; }
        .header h1 { margin: 0 0 8px 0; font-size: 22px; color: #1a202c; }
        .header p { margin: 0; font-size: 14px; color: #4a5568; }
        #plot-container { width: 100%; height: 800px; background: white; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        .rules-card { margin-top: 15px; background: white; padding: 15px 20px; border-radius: 8px; border-left: 4px solid #2b6cb0; font-size: 13px; line-height: 1.6; }
    </style>
</head>
<body>
    <div class="header">
        <h1>Workflow Multidimensional 3D del Modelo cellSim</h1>
        <p>Modelo Basado en Agentes: Estados Genotípicos, Acumulación de Daño y Destinos Celulares</p>
    </div>
    <div id="plot-container"></div>
    <div class="rules-card">
        <strong>Reglas Biológicas Fundamentales:</strong><br>
        • <b>Letalidad Sintética (Fase 1):</b> BRCA1 -/- con TP53 (+/+ o +/-) induce Apoptosis Intrínseca.<br>
        • <b>Supervivencia LOH:</b> Requiere mutación previa de TP53 -/- para franquear la viabilidad celular.<br>
        • <b>Barrera PRIMER (&theta;<sub>intr</sub> = 2.0):</b> Reconocimiento tisular y riesgo de Apoptosis Extrínseca.<br>
        • <b>Barrera TUMORAL (&theta;<sub>inm</sub> = 5.0, Ratio 2.5&times;):</b> Evasión inmune, inmortalización y mitosis desregulada (d<sub>neo</sub> = 0.158).
    </div>

    <script>
        const nodes = [
            { id: 'Base', x: 0, y: 0, z: 0, name: 'Base (BASELINE)<br>TP53 +/+, BRCA1 +/-<br>Homeostasis', color: '#4caf50', size: 14, symbol: 'circle' },
            { id: 'St1', x: 1, y: 0, z: 0, name: 'St. 1 (UNSTABLE)<br>TP53 +/-, BRCA1 +/-<br>&uarr; &delta;<sub>BRCA+</sub> (0.107)', color: '#fbc02d', size: 14, symbol: 'circle' },
            { id: 'St2', x: 2, y: 0, z: 0, name: 'St. 2 (UNPROTECTED)<br>TP53 -/-, BRCA1 +/-<br>&uarr;&uarr; &delta;<sub>BRCA-</sub> (0.353)', color: '#f57c00', size: 14, symbol: 'circle' },
            { id: 'St3', x: 2, y: 1, z: 0, name: 'St. 3 (UNPROT. LOH)<br>TP53 -/-, BRCA1 -/-<br>&uarr;&uarr;&uarr; 2&middot;&delta;<sub>BRCA-</sub> (0.707)', color: '#d84315', size: 14, symbol: 'circle' },
            
            { id: 'P1', x: 2, y: 0, z: 1, name: 'PRIMER (Pre-T1)<br>TP53 -/-, BRCA1 +/-<br>D<sub>intr</sub> &ge; 2.0 (Vulnerable)', color: '#e91e63', size: 14, symbol: 'circle' },
            { id: 'P2', x: 2, y: 1, z: 1, name: 'PRIMER (Pre-T2)<br>TP53 -/-, BRCA1 -/-<br>D<sub>intr</sub> &ge; 2.0 (Hipermutado)', color: '#ad1457', size: 14, symbol: 'circle' },
            
            { id: 'T1', x: 2, y: 0, z: 2, name: 'T1 (TUMORAL)<br>TP53 -/-, BRCA1 +/-<br>D<sub>inm</sub> &ge; 5.0 (Evasión)<br>Mitosis d<sub>neo</sub> = 0.158', color: '#e53935', size: 16, symbol: 'circle' },
            { id: 'T2', x: 2, y: 1, z: 2, name: 'T2 (TUMORAL LOH)<br>TP53 -/-, BRCA1 -/-<br>D<sub>inm</sub> &ge; 5.0 (Evasión)<br>Clon LOH Agresivo', color: '#b71c1c', size: 16, symbol: 'circle' },
            
            { id: 'APT_INTR', x: 0.5, y: 1.0, z: 0, name: 'APT. INTR&Iacute;NSECA<br>Letalidad Sint&eacute;tica (p53)<br>BRCA1-/- con p53 activo', color: '#546e7a', size: 16, symbol: 'x' },
            { id: 'APT_EXTR', x: 2.5, y: 0.5, z: 1, name: 'APT. EXTR&Iacute;NSECA<br>Aclaramiento Tisular<br>D<sub>inm</sub> < 5.0 en PRIMER', color: '#37474f', size: 16, symbol: 'x' }
        ];

        const nodeTrace = {
            x: nodes.map(n => n.x),
            y: nodes.map(n => n.y),
            z: nodes.map(n => n.z),
            text: nodes.map(n => n.name),
            mode: 'markers+text',
            hoverinfo: 'text',
            textposition: 'top center',
            marker: {
                size: nodes.map(n => n.size),
                color: nodes.map(n => n.color),
                symbol: nodes.map(n => n.symbol),
                line: { color: '#ffffff', width: 2 }
            },
            type: 'scatter3d',
            name: 'Estados Celulares'
        };

        const planes = [
            { z: 0, name: 'Plano 0: Homeostasis (Z=0)', color: 'rgba(200, 230, 201, 0.35)' },
            { z: 1, name: 'Plano 1: PRIMER (Z=1)', color: 'rgba(255, 249, 196, 0.35)' },
            { z: 2, name: 'Plano 2: TUMORAL (Z=2)', color: 'rgba(255, 205, 210, 0.35)' }
        ];

        const planeTraces = planes.map(p => ({
            x: [-0.5, 2.8, 2.8, -0.5],
            y: [-0.5, -0.5, 1.5, 1.5],
            z: [p.z, p.z, p.z, p.z],
            i: [0, 0],
            j: [1, 2],
            k: [2, 3],
            type: 'mesh3d',
            color: p.color,
            name: p.name,
            hoverinfo: 'name'
        }));

        const layout = {
            margin: { l: 0, r: 0, b: 0, t: 30 },
            scene: {
                xaxis: { title: 'Eje X: TP53 (+/+, +/-, -/-)', tickvals: [0, 1, 2], ticktext: ['TP53 +/+', 'TP53 +/-', 'TP53 -/-'] },
                yaxis: { title: 'Eje Y: BRCA1 (+/-, -/-)', tickvals: [0, 1], ticktext: ['BRCA1 +/-', 'BRCA1 -/-'] },
                zaxis: { title: 'Eje Z: Fenotipo / Daño', tickvals: [0, 1, 2], ticktext: ['Z=0: Basal', 'Z=1: PRIMER', 'Z=2: TUMORAL'] },
                camera: {
                    eye: { x: 1.6, y: -1.6, z: 1.2 }
                }
            }
        };

        Plotly.newPlot('plot-container', [...planeTraces, nodeTrace], layout, { responsive: true });
    </script>
</body>
</html>
"""
    with open(html_file, "w", encoding="utf-8") as f:
        f.write(html_content)

if __name__ == "__main__":
    out_dir = "/home/luis/CLionProjects/cellSim/docs/paper20260907_transition_states/figures"
    create_3d_workflow(out_dir)
