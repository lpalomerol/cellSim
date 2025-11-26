# 🗂️ Índice Rápido de Documentación

**Ubicación**: `/docs/`

---

## 📍 Acceso Directo

| Recurso | Ruta | Propósito |
|---------|------|----------|
| **Landing Page** | `README.md` | Introducción al simulador |
| **Índice Conceptos** | `biological-concepts/README.md` | Mapa de todos los temas |
| **Genes** | `biological-concepts/genes.md` | Estados, mutaciones, TP53, BRCA1 |
| **Genomas** | `biological-concepts/genome.md` | Colecciones, estabilidad, ciclo de vida |
| **Células** | `biological-concepts/cells.md` | AgenticCell, 3 fases, protección |
| **Tejidos** | `biological-concepts/tissue.md` | Colecciones, coordinación, rastreo |
| **Neoplasias** | `biological-concepts/neoplasm.md` | Cáncer, multihit, transformación |
| **Interacciones** | `biological-concepts/interactions.md` | Señales, broadcasts, comunicación |

---

## 🎯 Búsqueda Rápida por Concepto

| Concepto | Página | Línea |
|----------|--------|------|
| **Mutación** | genes.md | "Máquina de Estados" |
| **Inestabilidad genómica** | genome.md | "Detección de Instabilidad" |
| **Ciclo de vida** | cells.md | "Ciclo de Vida de la Célula" |
| **TP53** | genes.md, genome.md, cells.md | Search "TP53" |
| **BRCA1** | genes.md, neoplasm.md | Search "BRCA1" |
| **Neoplasia** | neoplasm.md | "¿Qué es la Neoplasia?" |
| **Señales** | interactions.md | "Sistema de Señales" |
| **Threads** | tissue.md, interactions.md | "Thread-Safety" |

---

## 📚 Por Nivel de Experiencia

### 👶 Principiante
**Tiempo**: 45 minutos
1. README.md (5 min)
2. biological-concepts/README.md (10 min)
3. genes.md (15 min)
4. genome.md (15 min)

### 👨‍💼 Intermedio
**Tiempo**: 2 horas
1. Todos los anteriores (45 min)
2. cells.md (20 min)
3. tissue.md (20 min)
4. neoplasm.md (15 min)

### 🧑‍🔬 Avanzado
**Tiempo**: 4 horas
1. Todos los anteriores (2 horas)
2. interactions.md detallado (30 min)
3. Revisar código en src/ (1.5 horas)

---

## 🔍 Búsqueda por Palabra Clave

**Mutación**
- genes.md: "Transiciones de Mutación"
- genome.md: "Ciclo de Vida del Genoma"
- cells.md: "Fase 1: Ciclo Genómico"

**Protección**
- genes.md: "Genes Clave: TP53"
- cells.md: "Protección TP53"
- neoplasm.md: "Protección vs. Aceptación"

**Señales**
- tissue.md: "Comunicación y Señales"
- interactions.md: "Sistema de Señales"

**Rastreo**
- tissue.md: "Rastreo de Neoplasias"
- interactions.md: "Rastreo de Eventos"

---

## 📊 Contenido por Tipo

### Conceptos Puros
- genes.md: Estado genético, máquina de estados
- genome.md: Estructura, estabilidad
- neoplasm.md: Multihit hypothesis

### Arquitectura
- cells.md: 3 fases de ciclo
- tissue.md: 3 fases de coordinación
- interactions.md: Arquitectura de señales

### Ejemplos Prácticos
- genes.md: Evolución de TP53 (10 años)
- cells.md: Simulación de célula (100 años)
- tissue.md: Simulación de tejido
- interactions.md: Sistema completo

### Código
- Todos los archivos: 10+ ejemplos C++ cada uno

### Diagramas
- Todos los archivos: 2-5 diagramas ASCII cada uno

---

## 🚀 Flujo de Aprendizaje Recomendado

```
INICIO
  ↓
README.md ◄─── Visión general del proyecto
  ↓
biological-concepts/README.md ◄─── Índice de conceptos
  ↓
genes.md ◄─── Bloques básicos (Gen, estados, mutación)
  ↓
genome.md ◄─── Colecciones (Genoma, inestabilidad)
  ↓
cells.md ◄─── Entidades vivas (AgenticCell, 3 fases)
  ↓
tissue.md ◄─── Sistemas (Tissue, coordinación)
  ↓
neoplasm.md ◄─── Patología (Transformación, multihit)
  ↓
interactions.md ◄─── Comunicación (Señales, broadcast)
  ↓
FIN - Has completado la documentación biológica

OPCIONAL:
  ↓
Revisar código fuente en src/domain/
  ↓
Ejecutar ejemplos en app/main*.cpp
```

---

## 💾 Estructura de Archivos

```
/home/luis/CLionProjects/cellSim/
├── docs/
│   ├── README.md                    ← INICIO AQUÍ
│   └── biological-concepts/
│       ├── README.md                ← ÍNDICE
│       ├── genes.md                 ← §1
│       ├── genome.md                ← §2
│       ├── cells.md                 ← §3
│       ├── tissue.md                ← §4
│       ├── neoplasm.md              ← §5
│       └── interactions.md          ← §6
├── src/
│   └── domain/                      ← Código fuente
├── tests/
│   └── *.cpp                        ← Tests
└── app/
    └── main*.cpp                    ← Ejemplos
```

---

## 🔗 Enlaces Cruzados

Cada página tiene referencias a:
- **Conceptos relacionados** (referencias cruzadas)
- **Página anterior** (back link)
- **Página siguiente** (next link)
- **Índice** (al inicio de cada página)

---

## ✅ Checklist de Lectura

Use esto para rastrear su progreso:

- [ ] README.md (landing page)
- [ ] biological-concepts/README.md (índice)
- [ ] genes.md (conceptos básicos)
- [ ] genome.md (colecciones)
- [ ] cells.md (entidades vivas)
- [ ] tissue.md (sistemas)
- [ ] neoplasm.md (patología)
- [ ] interactions.md (comunicación)

---

## 📞 Referencia Rápida

### Clases Principales
- `Gene`: unidad de herencia, 3 estados
- `Genome`: colección de genes
- `AgenticCell`: célula viva
- `Tissue`: colección de células
- `ISignal`: interfaz de mensajes

### Métodos Clave
- `Gene::live()`: evalúa mutación
- `Genome::liveAllGenes()`: ciclo coordinado
- `AgenticCell::live()`: ciclo celular (3 fases)
- `Tissue::live()`: ciclo de tejido (3 fases)
- `ISignal::type()`: tipo de mensaje

### Parámetros Importantes
- `mutation_threshold`: probabilidad de mutación
- `mutation_instability_k`: factor de inestabilidad
- `neoplasm_k`: probabilidad de transformación
- `genomic_instability`: factor multiplicativo

---

## 🎓 Tips de Estudio

1. **Lee en orden**: Cada página construye sobre las anteriores
2. **Ejecuta ejemplos**: Copia el código C++ y experimenta
3. **Dibuja diagramas**: Crea tus propias máquinas de estados
4. **Haz preguntas**: ¿Por qué TP53 protege? ¿Qué es multihit?
5. **Revisa código**: Compara la documentación con src/domain/

---

## 🆘 Troubleshooting

**P: No entiendo mutaciones**
R: Lee genes.md sección "Máquina de Estados"

**P: ¿Cómo funciona la inestabilidad?**
R: genome.md sección "Inestabilidad Multiplicativa"

**P: ¿Cómo se convierte en cáncer?**
R: neoplasm.md sección "Hipótesis del Multihit"

**P: ¿Cómo se comunican las células?**
R: interactions.md sección "Broadcasts vs. Dirigidos"

---

## 📈 Progresión de Complejidad

```
Básico         │ Intermedio      │ Avanzado
───────────────┼─────────────────┼──────────────
Gen            │ Genome          │ Thread-safety
Estado         │ Ciclo coordinado│ Arquitectura
Mutación       │ Estabilidad     │ Patrones
Threshold      │ Cascadas        │ Concurrencia
               │ Multihit        │ Distribución
```

---

## 📱 Formato Responsivo

Todos los archivos están optimizados para:
- ✅ GitHub web
- ✅ VS Code
- ✅ Navegadores
- ✅ Dispositivos móviles
- ✅ Lectores de Markdown

---

**¡Listo para aprender sobre cellSim!** 🚀

