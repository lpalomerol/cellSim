#!/usr/bin/env python3
"""Genera la memoria interna de trabajo combinando Paper 3 + Paper 6 en formato Word."""

from pathlib import Path
from docx import Document
from docx.shared import Inches, Pt, Cm, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
import datetime

ROOT = Path(__file__).resolve().parent.parent
OUT  = ROOT / "docs" / "memoria-interna-calibracion.docx"

P3   = ROOT / "docs" / "paper3"
P6   = ROOT / "docs" / "paper6"

FIG_P3_INITIAL      = str(P3 / "fig1_initial_vs_calibrated.png")
FIG_P3_BIGBANG      = str(P3 / "fig2_bigbang_effect.png")
FIG_P3_CALIB        = str(P3 / "fig3_calibration_kuchenbaecker.png")
FIG_P6_CDF_FIT      = str(P6 / "gen4_cdf_fit.png")
FIG_P6_POSTERIORS   = str(P6 / "gen4_posteriors.png")
FIG_P6_PAIRWISE     = str(P6 / "gen4_pairwise.png")
FIG_P6_PHENOTYPES   = str(P6 / "gen4_phenotypes.png")
FIG_P6_AGGRESSIVENESS = str(P6 / "gen4_aggressiveness.png")

FIG_W = Cm(14)

doc = Document()

# ── Estilos ──────────────────────────────────────────────────────────────────
style = doc.styles['Normal']
font = style.font
font.name = 'Calibri'
font.size = Pt(11)

# ── Portada / Título ─────────────────────────────────────────────────────────
p = doc.add_paragraph()
p.alignment = WD_ALIGN_PARAGRAPH.CENTER
run = p.add_run("Memoria interna de trabajo")
run.bold = True
run.font.size = Pt(14)
run.font.color.rgb = RGBColor(0x44, 0x44, 0x44)

doc.add_paragraph()

title = doc.add_paragraph()
title.alignment = WD_ALIGN_PARAGRAPH.CENTER
run = title.add_run(
    "Fenotipos emergentes de progresión tumoral en portadoras BRCA1:\n"
    "calibración bayesiana de un modelo basado en agentes"
)
run.bold = True
run.font.size = Pt(16)

doc.add_paragraph()

subtitle = doc.add_paragraph()
subtitle.alignment = WD_ALIGN_PARAGRAPH.CENTER
run = subtitle.add_run(
    "Síntesis de los documentos de calibración bootstrap (Paper 3) "
    "y refinamiento ABC-SMC (Paper 6)"
)
run.italic = True
run.font.size = Pt(11)
run.font.color.rgb = RGBColor(0x66, 0x66, 0x66)

meta = doc.add_paragraph()
meta.alignment = WD_ALIGN_PARAGRAPH.CENTER
run = meta.add_run(f"Documento interno — {datetime.date.today().strftime('%B %Y').capitalize()}")
run.font.size = Pt(10)
run.font.color.rgb = RGBColor(0x88, 0x88, 0x88)

doc.add_paragraph()

# ── Helper: añadir figura con caption ────────────────────────────────────────
def add_figure(path, caption, width=FIG_W):
    if not Path(path).exists():
        doc.add_paragraph(f"[Figura no encontrada: {path}]")
        return
    doc.add_picture(path, width=width)
    last_paragraph = doc.paragraphs[-1]
    last_paragraph.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = p.add_run(caption)
    run.italic = True
    run.font.size = Pt(9)
    run.font.color.rgb = RGBColor(0x55, 0x55, 0x55)
    doc.add_paragraph()


# ═════════════════════════════════════════════════════════════════════════════
#  1.  RESUMEN EJECUTIVO
# ═════════════════════════════════════════════════════════════════════════════
doc.add_heading("1. Resumen ejecutivo", level=1)

doc.add_paragraph(
    "CellSim es un simulador estocástico basado en agentes que modela la evolución "
    "de tejido mamario portador de mutación germinal BRCA1. Cada tejido contiene 500 "
    "células autónomas con genes BRCA1 y TP53, que acumulan daño genómico (D1) y "
    "capacidad de evasión inmune (D2) en ticks anuales, progresando por seis estadios "
    "de vida hasta posible transformación neoplásica."
)
doc.add_paragraph(
    "Se calibró el modelo frente a la curva de riesgo acumulado de Kuchenbaecker et al. "
    "(JAMA, 2017). Una primera fase con grid search bootstrap (3 parámetros libres, "
    "ν fijo) demostró que el modo Big Bang —división acelerada de células neoplásicas— "
    "es estructuralmente necesario para reproducir la curva clínica, reduciendo el SSE "
    "de 5295 a 48.5."
)
doc.add_paragraph(
    "Un refinamiento mediante ABC-SMC (4 parámetros libres, ν incluido) confirmó el "
    "ajuste y reveló una estructura oculta en el posterior: la correlación "
    "r(δ_low, ν) = −0.609 (p < 10⁻²¹) define un gradiente continuo que, al "
    "operacionalizarse mediante k-means, identifica dos fenotipos de progresión "
    "tumoral —A (agresivo, sat50=56.5 años) y B (basal, sat50=59.3 años)— que "
    "comparten el mismo onset (~43 años, p=0.25) pero difieren en velocidad de "
    "colonización tisular (p < 0.001). La curva de Kuchenbaecker sería, por tanto, "
    "una distribución marginal que integra dos dinámicas de crecimiento distintas, "
    "indistinguibles con datos de incidencia poblacional."
)

# ═════════════════════════════════════════════════════════════════════════════
#  2.  EL MODELO
# ═════════════════════════════════════════════════════════════════════════════
doc.add_heading("2. El modelo", level=1)

doc.add_heading("2.1 Agentes y tejido", level=2)
doc.add_paragraph(
    "CellSim representa un tejido como una población de 500 células autónomas. "
    "Cada célula ejecuta un ciclo determinista de seis fases por tick anual: "
    "evaluación basal, checkpoint de integridad (apoptosis intrínseca), checkpoint "
    "de apoptosis extrínseca, mutación génica, transformación/división, y "
    "actualización de inestabilidad. El tejido declara onset tumoral cuando ≥5% "
    "de las células alcanzan el estado TUMORAL (~10⁹ células reales, límite "
    "mamográfico). La simulación cubre 80 años (0–80)."
)

doc.add_heading("2.2 Genes y mutaciones", level=2)
doc.add_paragraph(
    "BRCA1 se inicializa en estado heterocigoto (+/-), como corresponde a una "
    "portadora germinal. TP53 se inicializa como salvaje (+/+). Las mutaciones "
    "son unidireccionales (pérdida de función):"
)
doc.add_paragraph("BRCA1: +/-  →  -/-   (tasa β, calibrada)", style='List Bullet')
doc.add_paragraph("TP53:  +/+  →  +/-  →  -/-   (tasa fija 0.003)", style='List Bullet')

doc.add_heading("2.3 Acumuladores D1 y D2", level=2)
doc.add_paragraph(
    "Cada célula mantiene dos acumuladores continuos que se incrementan cada tick "
    "según el estado genético:"
)

tbl = doc.add_table(rows=7, cols=4)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Estado', 'δ_TP53', 'δ_BRCA1', 'D2 (TP53 + BRCA1)']
data = [
    ['TP53 +/+', '0', '—', '0'],
    ['TP53 +/-', 'δ_low', '—', 'δ_low'],
    ['TP53 -/-', 'δ_high', '—', 'δ_high'],
    ['BRCA1 +/-', '—', 'δ_low', 'δ_low'],
    ['BRCA1 -/-', '—', '2·δ_high', '2·δ_high'],
    ['', '', 'D1 = δ_TP53 + edad', 'D2 = δ_TP53 + δ_BRCA1 + edad'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "D1 (daño genómico) depende solo de TP53 y acelera la tasa de mutación cuanto "
    "más alto es. D2 (inmunosupresión) depende de TP53 y BRCA1 aditivamente; al "
    "cruzar el umbral θ_D2 = 5.0 la célula resiste la apoptosis extrínseca y, "
    "si está en estado PRIMER (D1 > θ_D1 = 2.0), se transforma en neoplásica. "
    "La asimetría θ_D1 < θ_D2 codifica que la inestabilidad genómica se inicia "
    "antes que la evasión inmune sostenida."
)

doc.add_paragraph(
    "Una consecuencia importante de que δ_low controle ambos acumuladores "
    "simultáneamente es que un valor alto de δ_low acelera tanto D1 como D2. "
    "Al aumentar la velocidad de acumulación de D2, más células cruzan el umbral "
    "θ_D2 por unidad de tiempo, lo que incrementa la actividad de eliminación "
    "inmune en el tejido. El resultado comportamental es equivalente a un tejido "
    "con mayor presión de vigilancia, pero el mecanismo es cinético (velocidad de "
    "acumulación), no de señalización. Esta distinción es fundamental: la vigilancia "
    "no está programada en el modelo — emerge de la tasa de daño. Es exactamente "
    "el tipo de efecto emergente que buscamos en un modelo basado en agentes: "
    "reglas locales simples producen comportamientos globales biológicamente "
    "interpretables sin haber sido explícitamente codificados."
)

doc.add_heading("2.4 Mecanismo Big Bang", level=2)
doc.add_paragraph(
    "Cuando una célula entra en estado PRIMER, su tasa de división pasa de 0.1% "
    "a ν por tick, donde ν (neoplastic_div_rate) es la probabilidad anual de que "
    "una célula neoplásica genere una célula hija. Con Big Bang activo, ν se "
    "calibra típicamente entre 0.1 y 0.24 (10–24% por tick). Esto modela la "
    "expansión clonal acelerada (dinámica Gompertziana). No afecta las tasas de "
    "mutación — solo la velocidad de crecimiento del clon una vez iniciado."
)

doc.add_heading("2.5 Parámetros", level=2)

tbl = doc.add_table(rows=9, cols=4)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Parámetro', 'Símbolo', 'Rango / Valor', 'Tipo']
data = [
    ['brca1_rate', 'β', '[0.025, 0.080]', 'Libre (calibrado)'],
    ['low_delta', 'δ_low', '[0.060, 0.220]', 'Libre (calibrado)'],
    ['high_delta', 'δ_high', '[0.120, 0.600]', 'Libre (sloppy)'],
    ['neoplastic_div_rate', 'ν', '[0.100, 0.240]', 'Libre (calibrado)'],
    ['tp53_rate', 'μ_TP53', '0.003', 'Fijo (barrido previo)'],
    ['d1_threshold', 'θ_D1', '2.0', 'Fijo'],
    ['d2_threshold', 'θ_D2', '5.0', 'Fijo'],
    ['n_cells / max_t', '', '500 / 80', 'Fijo'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "Restricción biológica dura: δ_high > δ_low (el estado homocigoto daña más "
    "que el heterocigoto)."
)

# ═════════════════════════════════════════════════════════════════════════════
#  3.  CALIBRACIÓN BOOTSTRAP (GRID SEARCH)
#     Versión inicial — documento 3
# ═════════════════════════════════════════════════════════════════════════════
doc.add_heading("3. Calibración bootstrap (grid search)", level=1)

doc.add_paragraph(
    "Los resultados de esta sección provienen de la calibración bootstrap inicial "
    "(documento 3). Establecen los fundamentos del modelo y demuestran la necesidad "
    "estructural del Big Bang."
)

doc.add_heading("3.1 Datos clínicos", level=2)
doc.add_paragraph(
    "Se utilizó la curva de riesgo acumulado de Kuchenbaecker et al. (JAMA, 2017), "
    "cohorte prospectiva de ~6.000 portadoras BRCA1, como referencia clínica."
)

tbl = doc.add_table(rows=7, cols=4)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Edad', 'CDF obs.', 'IC 95% inf.', 'IC 95% sup.']
data = [
    ['30', '4%', '2%', '7%'],
    ['40', '26%', '22%', '30%'],
    ['50', '46%', '41%', '52%'],
    ['60', '58%', '52%', '65%'],
    ['70', '65%', '56%', '73%'],
    ['80', '70%', '60%', '80%'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "La métrica de distancia utilizada es el SSE ponderado por la varianza del "
    "intervalo de confianza: ρ(θ) = Σ [CDF_sim(a) − CDF_obs(a)]² / σ²_a, donde "
    "σ_a = IC_95(a) / (2·1.96). Esto implica que el ajuste es más exigente en "
    "las edades tempranas (30–50 años), donde el IC clínico es más estrecho, y "
    "menos restrictivo en edades avanzadas (70–80 años), donde la incertidumbre "
    "clínica es mayor. El modelo es más estricto donde hay más información clínica."
)

doc.add_heading("3.2 Grid search (sin Big Bang)", level=2)
doc.add_paragraph(
    "Se realizó una búsqueda en rejilla con 36 combinaciones de β × δ_low "
    "(N=200 runs por combo), con ν fijo en 0.001 (tasa basal). El mejor "
    "resultado sin Big Bang fue SSE = 2078."
)
doc.add_paragraph(
    "Problema estructural descubierto: existe una incompatibilidad fundamental "
    "entre la precocidad del onset y la altura del plateau. Subir β adelanta "
    "el onset pero deja el plateau demasiado bajo; bajarlo produce el plateau "
    "correcto (~70%) pero con onset muy tardío. Ninguna combinación lograba "
    "r40 ≈ 26% Y r80 ≈ 70% simultáneamente."
)

add_figure(FIG_P3_INITIAL, "Figura 1. Parámetros iniciales vs. calibrados — "
           "la curva por defecto (rojo) llega muy tarde y se pasa en el plateau.")

doc.add_heading("3.3 Con Big Bang: solución estructural", level=2)
doc.add_paragraph(
    "La causa raíz del problema anterior: sin Big Bang, la tasa de división "
    "neoplásica es idéntica a la normal (0.1% por tick). Una única célula "
    "tumoral tarda décadas en expandirse al 5%, retrasando artificialmente "
    "el onset declarado. El modelo no puede separar 'cuándo aparece la primera "
    "célula tumoral' de 'cuánto tarda el clon en crecer'."
)
doc.add_paragraph(
    "Con Big Bang activo (ν = 0.1), la rejilla gruesa redujo SSE de 2078 a 243 "
    "(mejora ×8.5). La rejilla fina (42 combinaciones, N=500) identificó el "
    "óptimo: β = 0.045, δ_low = 0.120, δ_high = 0.240, con SSE = 48.5."
)

add_figure(FIG_P3_BIGBANG, "Figura 2. Efecto del Big Bang: sin él (azul) "
           "el modelo no puede ajustar la curva clínica (negro).")

tbl = doc.add_table(rows=5, cols=2)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Fase', 'Mejor SSE']
data = [
    ['Baseline (defaults, sin BB)', '5295'],
    ['Rejilla gruesa (sin BB)', '2078'],
    ['Rejilla gruesa (con BB)', '243'],
    ['Rejilla fina (con BB)', '48.5 ✅'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "Confirmado con N=1000 runs: todos los puntos dentro del IC 95% clínico "
    "para edades 40–80. El gap residual en r30 (1.7% vs 4%) se identificó "
    "como limitación estructural del modelo, no artefacto de calibración."
)

add_figure(FIG_P3_CALIB, "Figura 3. Calibración final vs. Kuchenbaecker: "
           "la curva simulada (rojo) cae dentro del IC 95% (azul).")

# ═════════════════════════════════════════════════════════════════════════════
#  4.  REFINAMIENTO ABC-SMC
#     Segunda versión — documento 6
# ═════════════════════════════════════════════════════════════════════════════
doc.add_heading("4. Refinamiento ABC-SMC", level=1)

doc.add_paragraph(
    "Los resultados de esta sección provienen del refinamiento ABC-SMC "
    "(documento 6), que parte de los parámetros identificados en la calibración "
    "bootstrap y añade ν como parámetro libre, además de cuantificar la "
    "incertidumbre de todas las estimaciones."
)

doc.add_paragraph(
    "La calibración bootstrap identifica un punto óptimo pero no cuantifica "
    "la incertidumbre de los parámetros ni explora correlaciones entre ellos. "
    "Para ello se aplicó Approximate Bayesian Computation mediante Sequential "
    "Monte Carlo (ABC-SMC, Toni et al., 2009)."
)

doc.add_heading("4.1 Por qué ABC", level=2)
doc.add_paragraph(
    "La verosimilitud del modelo (probabilidad de los datos clínicos dados "
    "unos parámetros) es analíticamente intratable: el espacio de estados de "
    "500 células durante 80 años es combinatoriamente inmenso. ABC reemplaza "
    "la evaluación de verosimilitud por simulación: se muestrean parámetros, "
    "se simula, y se aceptan si la distancia entre simulación y datos clínicos "
    "es menor que una tolerancia ε."
)

doc.add_heading("4.2 Protocolo SMC", level=2)
doc.add_paragraph(
    "Se utilizaron 4 parámetros libres (β, δ_low, δ_high, ν) con priors "
    "uniformes anclados al óptimo del grid search. N=200 partículas por "
    "generación, tolerancia ε adaptativa (cuantil α=0.6), kernel de "
    "perturbación Gaussiano. La métrica de distancia fue el SSE ponderado "
    "por la incertidumbre clínica (IC 95% de Kuchenbaecker)."
)

tbl = doc.add_table(rows=6, cols=5)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Gen', 'ε', 'dist media', 'dist mín', 'ESS/N']
data = [
    ['0', '20.00', '13.50', '1.94', '1.000'],
    ['1', '15.43', '9.98', '1.17', '0.965'],
    ['2', '11.61', '7.31', '1.08', '0.942'],
    ['3', '8.19', '5.54', '0.57', '0.960'],
    ['4', '6.20', '4.45', '0.92', '0.917'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "El ESS se mantuvo ≥91% en todas las generaciones (sin colapso de pesos). "
    "Gen 4 se considera el posterior final; Gen 5 no completó las 200 partículas "
    "en 100.000 intentos (tasa de aceptación <0.1%)."
)

doc.add_heading("4.3 Posteriores", level=2)

tbl = doc.add_table(rows=5, cols=4)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Parámetro', 'Media', 'Std', 'IC 95%']
data = [
    ['β (brca1_rate)', '0.0445', '0.0041', '[0.0372, 0.0526]'],
    ['δ_low', '0.1093', '0.0189', '[0.0836, 0.1468]'],
    ['δ_high', '0.3504', '0.1258', '[0.1498, 0.5914]'],
    ['ν', '0.1564', '0.0410', '[0.1015, 0.2300]'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "β y δ_low están bien identificados (posterior ~5× más estrecho que el "
    "prior). δ_high es 'sloppy': el IC 95% cubre prácticamente todo el "
    "prior, indicando que el modelo es insensible a este parámetro una vez "
    "fijado δ_low. ν se identifica en el tercio inferior de su prior."
)

add_figure(FIG_P6_POSTERIORS, "Figura 4. Distribuciones posteriores (Gen 4). "
           "β y δ_low están bien constreñidos; δ_high es sloppy.")

doc.add_heading("4.4 Validación clínica", level=2)
doc.add_paragraph(
    "Evaluación con la mejor partícula (SSE=0.92) y N=500 runs:"
)

tbl = doc.add_table(rows=7, cols=4)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Edad', 'Modelo', 'Kuchenbaecker', 'En IC 95%']
data = [
    ['30', '2.6%', '4.0% [2–7%]', '✅'],
    ['40', '25.8%', '26.0% [22–30%]', '✅'],
    ['50', '45.8%', '46.0% [41–52%]', '✅'],
    ['60', '55.8%', '58.0% [52–65%]', '✅'],
    ['70', '61.4%', '65.0% [56–73%]', '✅'],
    ['80', '63.6%', '70.0% [60–80%]', '✅'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "Los 6 puntos caen dentro del IC 95% clínico. La meseta en ~63% "
    "(vs 70% observado) refleja la fracción de tejidos no penetrantes "
    "protegidos por TP53 (~30%), que emerge naturalmente sin programarla "
    "explícitamente."
)

add_figure(FIG_P6_CDF_FIT, "Figura 5. Ajuste final: modelo (rojo) vs. "
           "Kuchenbaecker (negro) con IC 95% (gris).")

# ═════════════════════════════════════════════════════════════════════════════
#  5.  CORRELACIÓN Y FENOTIPOS
# ═════════════════════════════════════════════════════════════════════════════
doc.add_heading("5. Correlación y fenotipos", level=1)

doc.add_paragraph(
    "El aspecto más relevante del posterior es la estructura de correlación "
    "entre parámetros, que codifica información biológica no accesible desde "
    "el punto óptimo del grid search."
)

doc.add_heading("5.1 r(δ_low, ν) = −0.609", level=2)
doc.add_paragraph(
    "La matriz de correlación del posterior revela una anticorrelación "
    "significativa entre la tasa de daño en estado heterocigoto (δ_low) y "
    "la tasa de división neoplásica (ν): r = −0.609 (p < 10⁻²¹)."
)

add_figure(FIG_P6_PAIRWISE, "Figura 6. Pairwise (δ_low, ν) coloreado por SSE. "
           "La correlación negativa es el eje principal del posterior.")

doc.add_heading("5.2 Argumento de los cuatro cuadrantes", level=2)
doc.add_paragraph(
    "Para entender por qué el ABC selecciona esta estructura, considérense "
    "los cuatro cuadrantes posibles en el espacio (δ_low × ν):"
)

tbl = doc.add_table(rows=3, cols=3)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
tbl.cell(0, 0).text = ''
tbl.cell(0, 1).text = 'ν bajo'
tbl.cell(0, 2).text = 'ν alto'
tbl.cell(1, 0).text = 'δ_low alto'
tbl.cell(1, 1).text = 'FENOTIPO B ✅\nCDF ajusta\nSeñal fuerte + expansión lenta'
tbl.cell(1, 2).text = 'EXCESO ❌\nOnset demasiado temprano\n→ CDF sobre Kuchenbaecker'
tbl.cell(2, 0).text = 'δ_low bajo'
tbl.cell(2, 1).text = 'DEFECTO ❌\nOnset tardío Y expansión lenta\n→ CDF bajo Kuchenbaecker'
tbl.cell(2, 2).text = 'FENOTIPO A ✅\nCDF ajusta\nSeñal débil + expansión explosiva'

doc.add_paragraph()
doc.add_paragraph(
    "Los cuadrantes fuera de la diagonal son excluidos por la forma específica "
    "de la curva Kuchenbaecker, no por el prior (que es rectangular y no induce "
    "correlación). La diagonal superviviente representa dos estrategias de "
    "compensación que producen la misma incidencia poblacional mediante "
    "mecanismos opuestos."
)

doc.add_heading("5.3 Identificación de fenotipos", level=2)
doc.add_paragraph(
    "Se aplicó k-means (k=2) sobre las variables (δ_low, ν, β) estandarizadas "
    "para operacionalizar los extremos del gradiente:"
)

tbl = doc.add_table(rows=3, cols=5)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Fenotipo', 'N', 'δ_low (media)', 'ν (media)', 'sat50']
data = [
    ['A — Agresivo', '83', '0.096 ± 0.009', '0.191 ± 0.027', '56.5 ± 1.9 años'],
    ['B — Basal', '117', '0.117 ± 0.015', '0.129 ± 0.021', '59.3 ± 2.3 años'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "β y δ_high no difieren significativamente entre fenotipos — la distinción "
    "está codificada en el plano (δ_low, ν)."
)

add_figure(FIG_P6_PHENOTYPES, "Figura 7. Identificación de fenotipos en el "
           "espacio (δ_low, ν): A (rojo) y B (azul).")

doc.add_heading("5.4 sat50 discrimina donde onset no alcanza", level=2)
doc.add_paragraph(
    "La comparación de métricas de progresión revela una asimetría clave:"
)

tbl = doc.add_table(rows=3, cols=4)
tbl.style = 'Light Grid Accent 1'
tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
headers = ['Métrica', 'Fenotipo A', 'Fenotipo B', 'p-valor']
data = [
    ['onset_age (5% TUMORAL)', '43.7 ± 1.5 años', '43.5 ± 1.4 años', '0.25 ❌'],
    ['sat50 (50% TUMORAL)', '56.5 ± 1.9 años', '59.3 ± 2.3 años', '<0.001 ✅'],
]
for i, h in enumerate(headers):
    tbl.cell(0, i).text = h
for r, row in enumerate(data, 1):
    for c, val in enumerate(row):
        tbl.cell(r, c).text = val

doc.add_paragraph()
doc.add_paragraph(
    "La barrera apoptótica (δ_low) controla la velocidad de expansión clonal, "
    "no el momento de iniciación. Ambos fenotipos 'arrancan' a la misma edad "
    "(onset indistinguible), pero el Fenotipo A alcanza el 50% de saturación "
    "tisular ~3 años antes. sat50 es la métrica correcta para distinguir "
    "agresividad entre subtipos."
)

add_figure(FIG_P6_AGGRESSIVENESS, "Figura 8. onset_age (izquierda) vs. sat50 "
           "(derecha) por fenotipo. La diferencia emerge solo en sat50.")

# ═════════════════════════════════════════════════════════════════════════════
#  6.  INTERPRETACIÓN BIOLÓGICA, PRÓXIMOS EXPERIMENTOS Y LIMITACIONES
# ═════════════════════════════════════════════════════════════════════════════
doc.add_heading("6. Interpretación biológica, próximos experimentos y limitaciones", level=1)

doc.add_heading("6.1 Fenotipo A — Agresivo por escape", level=2)
doc.add_paragraph(
    "δ_low bajo (~0.096): el daño se acumula despacio en el tejido BRCA1+/-. "
    "No hay eliminación activa continua de células dañadas. El tejido se "
    "comporta como un sistema naive: no desarrolla 'memoria' de la amenaza. "
    "Cuando ocurre la transformación (evento estocástico), el clon neoplásico "
    "encuentra un microambiente sin resistencia entrenada y se expande "
    "rápidamente (ν alto, ~0.191). La colonización es explosiva."
)

doc.add_heading("6.2 Fenotipo B — Basal por inevitabilidad", level=2)
doc.add_paragraph(
    "δ_low alto (~0.117): el daño se acumula rápido, lo que genera una "
    "presión selectiva continua. El tejido elimina activamente células "
    "dañadas durante décadas — está 'entrenado'. El tumor que emerge lo "
    "hace después de sobrevivir a esta vigilancia continua, por lo que "
    "crece más despacio (ν bajo, ~0.129). La progresión es lenta pero "
    "inexorable."
)

doc.add_paragraph(
    "Posible relación con cambios hormonales: ambos fenotipos presentan un "
    "onset medio en torno a los 43 años, coincidente con la transición "
    "perimenopáusica. La caída de estrógenos post-menopausia altera el "
    "microambiente mamario (densidad del estroma, perfil inflamatorio, "
    "disponibilidad de factores de crecimiento) y podría favorecer un fenotipo "
    "de progresión sobre otro. Aunque el modelo actual no incluye señalización "
    "hormonal, esta coincidencia entre el rango de sat50 (56.5–59.3 años) y "
    "la transición menopáusica sugiere una posible interacción entre el "
    "deterioro genómico y los cambios endocrinos que merecería exploración "
    "futura. El modelo no puede validar esta hipótesis, pero sí proporciona "
    "un marco cuantitativo para evaluar si la incorporación de señales "
    "hormonales mejoraría el ajuste o sería redundante con los parámetros "
    "actuales."
)

doc.add_heading("6.3 Analogía unificadora", level=2)
doc.add_paragraph(
    "Imaginemos dos tipos de costa frente a la erosión. La primera tiene "
    "gestión activa: vigilancia continua, reparaciones frecuentes. Cuando "
    "ocurre un evento mayor, la infraestructura ya existe y la erosión "
    "progresa gradualmente (Fenotipo B). La segunda costa no tiene gestión: "
    "el desgaste se acumula silenciosamente durante años. Cuando se supera "
    "el umbral crítico, el colapso es rápido y difícil de frenar (Fenotipo A). "
    "La curva de Kuchenbaecker es la suma de ambas dinámicas sobre la "
    "población de portadoras BRCA1."
)

doc.add_heading("6.4 Marco evolutivo", level=2)
doc.add_paragraph(
    "La diferencia entre fenotipos adquiere significado adicional desde la "
    "teoría evolutiva. El Fenotipo B (δ_low alto) es funcionalmente óptimo "
    "dentro de la ventana reproductiva humana (15–40 años): la vigilancia "
    "activa mantiene el tejido limpio durante las décadas de mayor valor "
    "reproductivo. El tumor emerge a edades post-reproductivas, donde la "
    "selección natural no opera. Esto conecta con dos marcos teóricos: "
    "el antagonismo pleiotrópico (Williams, 1957) —un mismo mecanismo puede "
    "ser ventajoso en edad temprana y perjudicial en edad tardía— y la teoría "
    "del soma desechable (Kirkwood, 1977): la inversión en mantenimiento "
    "tisular es suficiente para maximizar la aptitud reproductiva, no para "
    "la longevidad indefinida."
)

doc.add_heading("6.5 Próximos experimentos", level=2)
doc.add_paragraph(
    "El hallazgo de dos fenotipos abre varias líneas de exploración que se "
    "abordarán en fases posteriores del proyecto. Cada una permite evaluar "
    "si la estructura descubierta es robusta o se modifica al incorporar "
    "nueva biología:"
)
doc.add_paragraph(
    "1. Parámetros independientes por gen (δ_tp53_low ≠ δ_brca1_low): "
    "Actualemente δ_low y δ_high son compartidos entre TP53 y BRCA1. "
    "Separarlos en parámetros independientes permitirá evaluar si los dos "
    "fenotipos persisten (serían robustos) o colapsan (eran un artefacto "
    "del parámetro compartido). Es la prioridad más alta por su bajo coste "
    "computacional.", style='List Bullet'
)
doc.add_paragraph(
    "2. Calibrar el ratio D2/D1: Liberar θ_D1 y θ_D2 como parámetros "
    "libres en ABC-SMC. Si el posterior los sitúa cerca del ratio 2.5 "
    "actual, se valida el diseño; si encuentra otro valor, habrá que "
    "reinterpretar la ventana entre inestabilidad genómica y evasión inmune.",
    style='List Bullet'
)
doc.add_paragraph(
    "3. Señal hormonal post-menopausia: Modelar un cambio escalón en "
    "los umbrales o deltas a partir de ~50 años para explorar la "
    "coincidencia entre sat50 y la transición menopáusica. Si un fenotipo "
    "se ve más afectado que el otro, sería una pista biológica directa "
    "sobre el mecanismo subyacente.", style='List Bullet'
)
doc.add_paragraph(
    "4. Heterogeneidad clonal: Introducir variabilidad per-célula en "
    "δ_low o μ_TP53 muestreada de una distribución. Si el Fenotipo A "
    "(agresivo por escape) muestra mayor diversidad clonal que el B, "
    "se refuerza la interpretación de 'explosión estocástica' frente a "
    "'presión lenta e inexorable'.", style='List Bullet'
)
doc.add_paragraph(
    "5. Validación externa: Aplicar la misma metodología de calibración "
    "a otras curvas de penetrancia (BRCA2, portadores PALB2) para evaluar "
    "si la estructura de dos fenotipos es específica de BRCA1 o un patrón "
    "general en la carcinogénesis hereditaria.", style='List Bullet'
)

doc.add_heading("6.6 Limitaciones", level=2)
doc.add_paragraph(
    "1. δ_low y δ_high compartidos entre TP53 y BRCA1: Biológicamente "
    "injustificado — ambos genes tienen funciones distintas y deberían "
    "tener parámetros independientes.", style='List Bullet'
)
doc.add_paragraph(
    "2. Ratio D2/D1 fijo (2.5): Los umbrales θ_D1=2.0 y θ_D2=5.0 son "
    "suposiciones a priori sin respaldo cuantitativo.", style='List Bullet'
)
doc.add_paragraph(
    "3. Multiplicador LOH-BRCA1 fijo (2×): BRCA1 -/- contribuye 2·δ_high "
    "a D2 — es una suposición de diseño, no un parámetro estimado.",
    style='List Bullet'
)
doc.add_paragraph(
    "4. Sin heterogeneidad clonal: Todas las células comparten los mismos "
    "parámetros; no hay selección de subclones, que es el mecanismo "
    "principal de la agresividad tumoral en la realidad.", style='List Bullet'
)
doc.add_paragraph(
    "5. Sin estructura espacial: El tejido es un pocillo bien mezclado, "
    "sin gradientes de oxígeno ni microambiente.", style='List Bullet'
)

# ═════════════════════════════════════════════════════════════════════════════
#  7.  REFERENCIAS
# ═════════════════════════════════════════════════════════════════════════════
doc.add_heading("7. Referencias", level=1)

refs = [
    'Kuchenbaecker KB, Hopper JL, Barnes DR, et al. Risks of breast, ovarian, and contralateral breast cancer for BRCA1 and BRCA2 mutation carriers. JAMA. 2017;317(23):2402–2416.',
    'Toni T, Welch D, Strelkowa N, Ipsen A, Stumpf MPH. Approximate Bayesian computation scheme for parameter inference and model selection in dynamical systems. J R Soc Interface. 2009;6(31):187–202.',
    'Williams GC. Pleiotropy, natural selection, and the evolution of senescence. Evolution. 1957;11(4):398–411.',
    'Kirkwood TBL. Evolution of ageing. Nature. 1977;270:301–304.',
    'Tomasetti C, Li L, Vogelstein B. Stem cell divisions, somatic mutations, cancer etiology, and cancer prevention. Science. 2017;355(6331):1330–1334.',
    'Schreiber RD, Old LJ, Smyth MJ. Cancer immunoediting: integrating immunity\'s roles in cancer suppression and promotion. Science. 2011;331(6024):1565–1570.',
    'Foulkes WD, Smith IE, Reis-Filho JS. Triple-negative breast cancer. N Engl J Med. 2010;363(20):1938–1948.',
    'Slaughter DP, Southwick HW, Smejkal W. "Field cancerization" in oral stratified squamous epithelium. Cancer. 1953;6(5):963–968.',
]

for i, ref in enumerate(refs, 1):
    p = doc.add_paragraph()
    run = p.add_run(f"[{i}] {ref}")
    run.font.size = Pt(9)

# ── Guardar ──────────────────────────────────────────────────────────────────
doc.save(str(OUT))
print(f"✅ Documento guardado en {OUT}")
