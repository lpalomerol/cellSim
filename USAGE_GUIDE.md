# 📖 Guía de Uso - Datos de Validación

**Versión:** 1.0  
**Fecha:** Diciembre 4, 2025  
**Objetivo:** Cómo acceder, interpretar y comunicar los resultados de validación

---

## 📁 Estructura de Archivos Generados

```
/home/luis/CLionProjects/cellSim/cmake-build-debug/traces/
│
├── 01_ctrl_baseline_no_mutations_no_division/
│   ├── 01_ctrl_baseline_no_mutations_no_division_run1_POPULATION.md
│   └── 01_ctrl_baseline_no_mutations_no_division_run1_POPULATION.csv
│
├── 02_ctrl_baseline_no_mutations_high_division/
│   ├── 02_ctrl_baseline_no_mutations_high_division_run1_POPULATION.md
│   └── 02_ctrl_baseline_no_mutations_high_division_run1_POPULATION.csv
│
├── 03_ctrl_brca_mutations_high/
├── 04_ctrl_tp53_mutations_high/
├── 05_ctrl_tp53_mutations_high_unstable/
├── 06_realistic_baseline/
├── 07_realistic_low_tp53_instability/
├── 08_realistic_high_tp53_instability/
└── 09_realistic_balanced/
```

**Total:** 18 archivos (9 MD + 9 CSV)

---

## 📊 Formato de Archivos

### MARKDOWN (.md) - Para Presentación

**Uso:** Mostrar al jefe/stakeholders en presentaciones

**Estructura:**
1. **Resumen Ejecutivo** - Parámetros y población inicial/final
2. **Tabla Anual** - Evolución año por año
3. **Tabla de Apoptosis** - Neoplásticas susceptibles vs resistentes
4. **Definiciones** - Explicación de cada columna

**Ejemplo:** 
```
| Año | Total | Vivas | Muertas | Neoplásticas | Protegidas | Min Inest. | Max Inest. | TP53++ | TP53+- | TP53-- |
|-----|-------|-------|---------|--------------|------------|-----------|-----------|--------|--------|--------|
| 0   | 1000  | 1000  | 0       | 0            | 1000       | 0.000     | 0.000     | 100%   | 0%     | 0%     |
| 1   | 1000  | 955   | 45      | 0            | 955        | 0.000     | 0.000     | 99.5%  | 0.5%   | 0%     |
```

### CSV (.csv) - Para Análisis Computacional

**Uso:** Importar en Python/R/Excel para análisis cuantitativo

**Columnas:**
```
scenario            | Nombre del escenario
run                 | Número de run (siempre 1 por ahora)
year                | Año (0-50 ó 0-80)
total_cells         | Vivas + Muertas acumuladas
alive_cells         | Células vivas (Protegidas + Neoplásticas)
dead_cells_cumulative | Total acumulado de muertas
neoplastic_alive    | Neoplásticas vivas
protected_alive     | Protegidas vivas
min_genomic_instability | Inestabilidad mínima en población
max_genomic_instability | Inestabilidad máxima en población
tp53_plus_plus_pct  | % TP53 +/+ (0.0-1.0)
tp53_plus_minus_pct | % TP53 +/- (0.0-1.0)
tp53_minus_minus_pct | % TP53 -/- (0.0-1.0)
neoplastic_apoptosis_susceptible | Neoplásticas que acepta apoptosis
neoplastic_apoptosis_resistant   | Neoplásticas que evaden apoptosis
```

---

## 🔍 Cómo Interpretar los Resultados

### Lectura Rápida (5 minutos)

1. Abre `EXECUTIVE_DASHBOARD.md`
2. Mira la **Matriz de Resultados**
3. Lee las **Conclusiones por Grupo**
4. Observa las **ALERTAS**

### Lectura Detallada (30 minutos)

1. Lee `VALIDATION_RESULTS_ANALYSIS.md`
2. Para cada escenario:
   - Abre el `.md` correspondiente
   - Verifica la tabla anual
   - Busca patrones (crecimientos, caídas, estabilidad)

### Análisis Profundo (1-2 horas)

1. Descarga todos los `.csv`
2. Crea gráficos:
   - Población en tiempo: `plot(year, alive_cells)`
   - Neoplásticas: `plot(year, neoplastic_alive / alive_cells)`
   - Inestabilidad: `plot(year, max_genomic_instability)`
   - TP53 evolución: `plot(year, tp53_minus_minus_pct)`

---

## 💻 Importar en Python

```python
import pandas as pd
import matplotlib.pyplot as plt

# Cargar datos
df = pd.read_csv('06_realistic_baseline_run1_POPULATION.csv')

# Gráfico: Población en el tiempo
fig, ax = plt.subplots(figsize=(12, 6))
ax.plot(df['year'], df['alive_cells'], label='Vivas', linewidth=2)
ax.plot(df['year'], df['neoplastic_alive'], label='Neoplásticas', linewidth=2)
ax.plot(df['year'], df['dead_cells_cumulative'], label='Muertas (acum)', linewidth=2)
ax.set_xlabel('Año')
ax.set_ylabel('Número de Células')
ax.set_title('Escenario 06: Realista Baseline')
ax.legend()
ax.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('escenario_06.png', dpi=150)
plt.show()

# Gráfico: Inestabilidad
fig, ax = plt.subplots(figsize=(10, 5))
ax.plot(df['year'], df['min_genomic_instability'], label='Mín', linewidth=2)
ax.plot(df['year'], df['max_genomic_instability'], label='Máx', linewidth=2)
ax.axhline(y=10.0, color='r', linestyle='--', label='Threshold Apoptosis')
ax.set_xlabel('Año')
ax.set_ylabel('Inestabilidad Genómica')
ax.set_title('Evolución de Inestabilidad')
ax.legend()
ax.grid(True, alpha=0.3)
plt.savefig('inestabilidad.png', dpi=150)
plt.show()
```

---

## 📋 Comparación Entre Escenarios (Excel/Python)

### Tabla Comparativa - Población Final

```python
import pandas as pd

# Crear tabla de resumen
resumen = {
    'Escenario': [
        '01_baseline_no_mut_no_div',
        '02_baseline_no_mut_div',
        '03_brca_high',
        '04_tp53_high',
        '05_tp53_high_inest',
        '06_realistic_baseline',
        '07_realistic_low_tp53',
        '08_realistic_high_inest',
        '09_realistic_balanced'
    ],
    'Pop_Inicial': [1000]*9,
    'Pop_Final': [1000, 4955, 0, 462, 573, 59, 484, 153, 362],
    'Neoplasticas_%': [0, 0, 0, 98.1, 98.4, 20.3, 2.5, 45.8, 4.7],
    'Inmortales_%': [0, 0, 0, 100, 99.5, 100, 100, 100, 94.1],
    'Años': [10, 10, 50, 50, 50, 50, 80, 80, 80]
}

df = pd.DataFrame(resumen)
df.to_csv('RESUMEN_COMPARATIVO.csv', index=False)
print(df.to_string())
```

---

## 🎯 Qué Comunicar al Jefe

### Email Ejecutivo (2 minutos de lectura)

**Asunto:** ✅ Validación completada - 9 escenarios (3.4s)

**Cuerpo:**
```
Estimado [Jefe],

He completado la validación de 9 escenarios del simulador:

✅ RESULTADOS:
- Controles: Funcionan correctamente (población estable sin mutaciones)
- Parámetros: Identifiqué que BRCA1=20% es letal (ajustar a ≤10%)
- Realistas: Parámetros balanceados dan resultados coherentes

⚠️ HALLAZGO IMPORTANTE:
- 100% de neoplásticas son "inmortales" (evaden apoptosis)
- Investigación necesaria: ¿Es realista o threshold muy permisivo?

📊 RECOMENDACIÓN:
- Usar escenario 09 (balanceado) como baseline para producción
- Ejecutar 100 runs adicionales para validación estadística

Archivos adjuntos:
- EXECUTIVE_DASHBOARD.md (resumen con gráficos)
- VALIDATION_RESULTS_ANALYSIS.md (análisis detallado)
- 9 tablas Markdown + 9 CSV para análisis

Saludos,
[Tu nombre]
```

### Presentación en Diapositivas (5 minutos)

**Diapositiva 1:**
- Título: "Validación del Simulador de Células"
- Subtítulo: "9 escenarios, 3.4 segundos, 1000 células cada uno"

**Diapositiva 2: Validación de Modelo**
```
Escenario 01 (Sin nada): Pop=1000 → Pop=1000 ✅
Escenario 02 (Solo división): Pop=1000 → Pop=4955 ✅

Conclusión: El modelo base es correcto
```

**Diapositiva 3: Hallazgos Clave**
- BRCA1=20% → Extinción (ajustar a ≤10%)
- TP53 bajo → Máxima protección ✅
- Inestabilidad alta → Más neoplasias ✅

**Diapositiva 4: Recomendación**
"Usar escenario 09 (balanceado) para producción. Recomiendo 100 runs más para robustez estadística."

---

## 🔄 Próximos Pasos

### Corto Plazo (1 semana)
- [ ] Revisar BRCA1 (ajustar de 0.20 a 0.10)
- [ ] Investigar por qué 100% neoplásticas son inmortales
- [ ] Explorar threshold de apoptosis (5.0, 3.0, 1.0)

### Medio Plazo (2-4 semanas)
- [ ] Ejecutar 100 runs por escenario (estadística robusta)
- [ ] Crear visualizaciones con matplotlib
- [ ] Documentar cambios en README.md

### Largo Plazo (1-2 meses)
- [ ] Publicar resultados
- [ ] Submeter a revisión de pares
- [ ] Iterar basado en feedback

---

## 📞 Preguntas Frecuentes

**P: ¿Por qué el escenario 03 tiene 0 células?**
R: BRCA1=0.20 (20% mutación) es TOO HIGH. Genera daño no reparable. Usar ≤0.10.

**P: ¿Por qué 100% de neoplásticas son resistentes?**
R: Buena pregunta - esto podría indicar que el threshold (10.0) es muy permisivo. Investigar con threshold más bajo.

**P: ¿Puedo usar estos resultados para una publicación?**
R: Sí, pero recomiendo 100 runs por escenario para robustez estadística.

**P: ¿Dónde están las trazas?**
R: En `/home/luis/CLionProjects/cellSim/cmake-build-debug/traces/`

**P: ¿Puedo cambiar parámetros fácilmente?**
R: Sí, están en `app/run_all_scenarios.cpp` líneas 165-211. Recompila y ejecuta.

---

## ✅ Checklist de Validación

- [x] 9 escenarios definidos coherentemente
- [x] Parámetros validados (sin contradicciones)
- [x] Ejecución completada (3.4s)
- [x] Trazas generadas (18 archivos)
- [x] Análisis completado
- [x] Documentación creada
- [x] Dashboard ejecutivo disponible
- [ ] Comunicado al jefe (tuyo para hacer)
- [ ] Próximas iteraciones planificadas

---

**Última actualización:** Diciembre 4, 2025  
**Estado:** ✅ COMPLETADO Y LISTO PARA COMUNICAR


