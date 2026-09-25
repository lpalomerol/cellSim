# 📑 cellSim: Módulo Autocontenido de Validación Secundaria, Inferencia Bayesiana y Robustez Fenotípica

**Fecha:** 28 de agosto de 2026  
**Módulo:** `docs/paper20260828_1_validation_secondary`  
**Artículo Principal:** [`working_paper_secondary_validation.md`](./working_paper_secondary_validation.md)

---

## 🎯 Estructura Autocontenida del Módulo

Este directorio está completamente autocontenido para permitir la reproducción independiente y la auditoría exhaustiva tanto de los datos brutos como de los scripts de visualización y análisis bayesiano:

```
docs/paper20260828_1_validation_secondary/
├── working_paper_secondary_validation.md  ← Manuscrito científico completo (artículo principal)
├── README.md                              ← Índice, manifiesto de datos y guía de reproducción
├── data/                                  ← Datos brutos de partículas y ensambles ABC-SMC
│   ├── level_1/                           ← Nivel 1 (±10% Fisiológico): gen_00.csv a gen_07.csv
│   ├── level_2/                           ← Nivel 2 (±25% Moderado): gen_00.csv a gen_07.csv
│   ├── level_3/                           ← Nivel 3 (±50% Estrés): gen_00.csv a gen_07.csv
│   └── sensitivity_summary.csv            ← Tabla maestra de estimadores y credibilidad 95% CI
├── scripts/                               ← Scripts ejecutables de inferencia y visualización
│   ├── abc_sensitivity_v5.py              ← Orquestador ABC-SMC de 7 generaciones
│   ├── generate_liepe_posterior_matrix.py ← Generador de matrices posteriores 5x5 y 9x9 (Liepe et al. 2014)
│   └── generate_paper7_figures.py         ← Generador de curvas de incidencia y decaimiento fenotípico
└── figures/                               ← Figuras vectoriales (PDF) y de alta resolución (PNG)
    ├── fig6a_liepe_posterior_matrix_5x5.png/.pdf
    ├── fig_supp_s1_posterior_matrix_9x9_level1.png/.pdf
    ├── fig_supp_s2_posterior_matrix_9x9_level2.png/.pdf
    ├── fig_supp_s3_posterior_matrix_9x9_level3.png/.pdf
    ├── fig3_bimodality_decay_across_levels.png/.pdf
    └── fig4_tp53_threshold_behavior.png/.pdf
```

---

## 🚀 Guía Rápida de Reproducción

Todos los scripts dentro de `scripts/` están configurados para operar directamente sobre la carpeta local `data/` y generar las figuras en `figures/`:

```bash
# 1. Situarse en la raíz del repositorio o dentro de este módulo
cd docs/paper20260828_1_validation_secondary

# 2. Regenerar las matrices posteriores 5x5 y 9x9 (Liepe et al. 2014)
python3 scripts/generate_liepe_posterior_matrix.py

# 3. Regenerar las figuras de sensibilidad, bimodalidad y penetrancia clínica
python3 scripts/generate_paper7_figures.py
```

---

## 📊 Manifiesto de Parámetros y Notación Canónica

| Parámetro | Notación Canónica | Significado Biológico | Estado de Identificabilidad |
| :--- | :---: | :--- | :--- |
| `brca1_rate` | $\beta_{BRCA1}$ | Tasa de mutación somática del 2º alelo BRCA1 | 🟢 **Rígido (*Stiff*):** Dial continuo de latencia temporal |
| `low_delta` | $\delta_{\text{BRCA+}}$ | Inestabilidad basal heterocigótica ($BRCA1^{+/-}$) | 🟢 **Rígido (*Stiff*):** Acoplado homeostáticamente a $d_{\text{neo}}$ |
| `high_delta` | $\delta_{\text{BRCA-}}$ | Inestabilidad genómica post-LOH ($BRCA1^{-/-}$ / $TP53^{-/-}$) | ⚪ **Blando (*Sloppy*):** Satura al cumplir $\delta_{\text{BRCA-}} > \delta_{\text{BRCA+}}$ |
| `neoplastic_div_rate` | $d_{\text{neo}}$ | Tasa de proliferación clonal neoplásica | 🟡 **Bimodal:** Bifurca en Fenotipo A y B |
| `tp53_rate` | $\mu_{TP53}$ | Tasa de degradación/falla del punto de control TP53 | ⚪ **Interruptor Fisiológico:** Fija la meseta clínica a los 80 años (~70%) |
| `division_rate` | $d_{\text{basal}}$ | Tasa de mitosis basal del tejido mamario | ⚪ **Blando (*Sloppy*):** Desacoplado cinéticamente ($150\times$ menor a $d_{\text{neo}}$) |
| `d1_threshold` | $\theta_{D\text{\_intr}}$ | Umbral intrínseco de daño macromolecular (2.0) | ⚪ **Blando (*Sloppy*):** Factor de escala |
| `d2_threshold` | $\theta_{D\text{\_inmune}}$ | Umbral de evasión del estroma e inmunovigilancia (5.0) | ⚪ **Blando (*Sloppy*):** Ratio de contención $2.5\times$ |
| `tumor_threshold` | $\phi_{\text{tumor}}$ | Fracción celular de diagnóstico clínico (5% $\approx 10^9$ céls) | ⚪ **Blando (*Sloppy*):** Resolución anual de cohorte clínica |
