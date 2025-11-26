# 🧠 Tejidos en cellSim

## Introducción

Un **tejido** es una colección de células que viven y evolucionan juntas. El tejido actúa como un **sistema integrado** que coordina el ciclo de vida de múltiples células, gestiona sus identidades, rastrear eventos de neoplasia y facilita la comunicación entre células mediante señales.

---

## ¿Qué es un Tejido?

`Tissue` es la clase que implementa la colección de células en cellSim:

```cpp
class Tissue {
public:
    // Ciclo de vida
    void live();
    
    // Gestión de células
    void addCell(std::unique_ptr<ICell> cell);
    std::size_t size() const;
    ICell* getCell(std::size_t idx);
    
    // Comunicación y señales
    std::vector<std::unique_ptr<ISignal>> stealEmittedSignals();
    void setNeoplasmListener(
        std::function<void(std::uint64_t, const std::string&)> listener
    );
    
    // Rastreo
    std::set<std::uint64_t> getIdentifiedNeoplasms() const;
};
```

### Características Clave

✅ Colección dinámica de células  
✅ Asignación automática de IDs únicos  
✅ Ciclo de vida coordinado  
✅ Integración de señales  
✅ Thread-safety con mutexes  
✅ Rastreo de neoplasias  
✅ Gestión de excepciones por célula  

---

## Ciclo de Vida del Tejido: Método `live()`

El ciclo de vida del tejido tiene **3 fases principales**:

```cpp
void Tissue::live() {
    // FASE 0: Descripción / Logging
    phase0_Description();
    
    // FASE 1: Integración de Señales
    phase1_SignalIntegration();
    
    // FASE 2: Ejecución de Ciclos Celulares
    phase2_ExecuteCellCycles();
}
```

### Visualización del Ciclo

```
┌─────────────────────────────────────────────┐
│      CICLO DE VIDA: Tissue::live()          │
└─────────────────────────────────────────────┘

INICIO
│
├─────► FASE 0: DESCRIPCIÓN
│       │
│       ├─ Registrar estado del tejido
│       ├─ Número de células: [N]
│       ├─ Neoplasias identificadas: [M]
│       └─ Timestamp / Year
│
├─────► FASE 1: INTEGRACIÓN DE SEÑALES
│       │
│       ├─ Procesar señales del ciclo anterior
│       ├─ Distribuir mensajes a células
│       ├─ Actualizar listeners
│       └─ Limpiar cola de señales
│
├─────► FASE 2: EJECUCIÓN DE CICLOS CELULARES
│       │
│       ├─► Para cada célula en el tejido
│       │
│       ├─► cell.live()
│       │   ├─ Fase 1 de célula: Mutación
│       │   ├─ Fase 2 de célula: Viabilidad
│       │   ├─ Fase 3 de célula: Neoplasia
│       │   └─ age++
│       │
│       ├─► Recolectar excepciones si ocurren
│       │   └─ LOG sin interrumpir otras células
│       │
│       ├─► Recolectar señales emitidas
│       │   └─ Almacenar en signals_new_
│       │
│       └─► Identificar neoplasias
│           ├─ Si cell.isNeoplastic()
│           ├─ Registrar en identified_neoplasms_
│           └─ Llamar a neoplasm_listener_
│
└─────► FIN DE CICLO
        Tejido listo para siguiente tick
```

---

## Fase 0: Descripción y Logging

### Propósito

Registrar el estado actual del tejido para trazabilidad:

```cpp
void Tissue::phase0_Description() const {
    if (verbose_) {
        std::cout << "Tissue[" << tissue_id_ << "]: "
                  << size() << " cells, "
                  << identified_neoplasms_.size() << " neoplasms"
                  << std::endl;
    }
}
```

### Información Registrada

- ID del tejido
- Número total de células
- Número de neoplasias identificadas
- Timestamp (en versiones avanzadas)

---

## Fase 1: Integración de Señales

### Propósito

Procesar mensajes y señales emitidas en ciclos anteriores.

### Implementación

```cpp
void Tissue::phase1_SignalIntegration() {
    // 1. Obtener señales pendientes de ciclo anterior
    auto pending_signals = stealEmittedSignals();
    
    // 2. Para cada señal
    for (auto& signal : pending_signals) {
        // 3. Obtener IDs objetivo
        const auto& target_ids = signal->targetIds();
        
        // 4. Si broadcast (vacío), enviar a todas
        if (target_ids.empty()) {
            for (auto& cell : cells_) {
                if (cell) {
                    cell->receiveMessage(
                        cloneSignal(signal)  // Clone para cada célula
                    );
                }
            }
        } else {
            // 5. Si dirigido, enviar a células específicas
            for (uint64_t target_id : target_ids) {
                if (ICell* target_cell = getCellById(target_id)) {
                    target_cell->receiveMessage(
                        cloneSignal(signal)
                    );
                }
            }
        }
    }
}
```

### Tipos de Señales

| Tipo | Broadcast | Dirigida | Uso |
|------|-----------|----------|-----|
| Neoplasm | ✅ | ✅ | Advertir sobre transformación |
| Apoptosis | ✅ | ✅ | Señalizar muerte celular |

---

## Fase 2: Ejecución de Ciclos Celulares

### Propósito

Ejecutar el ciclo de vida (`live()`) de cada célula.

### Implementación

```cpp
void Tissue::phase2_ExecuteCellCycles() {
    for (std::size_t i = 0; i < cells_.size(); ++i) {
        if (!cells_[i]) continue;
        
        try {
            // Ejecutar ciclo de vida
            cells_[i]->live();
            
            // Recolectar señales emitidas
            auto emitted = getEmittedSignalsForCell(i);
            {
                std::lock_guard<std::mutex> lock(signals_mutex_);
                signals_new_.insert(
                    signals_new_.end(),
                    std::make_move_iterator(emitted.begin()),
                    std::make_move_iterator(emitted.end())
                );
            }
            
            // Verificar si se volvió neoplástica
            if (cells_[i]->isNeoplastic()) {
                identified_neoplasms_.insert(cells_[i]->id());
                
                if (neoplasm_listener_) {
                    neoplasm_listener_(
                        cells_[i]->id(),
                        "Cell became neoplastic"
                    );
                }
            }
            
        } catch (const NeoplasticException& e) {
            // Célula transformada durante ciclo
            identified_neoplasms_.insert(cells_[i]->id());
            if (verbose_) {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        } catch (const CellDeathException& e) {
            // Célula murió
            cells_[i] = nullptr;
            if (verbose_) {
                std::cout << "Cell died: " << e.what() << std::endl;
            }
        }
    }
}
```

### Manejo de Excepciones

El tejido **atrapa excepciones por célula** para evitar que una célula problemática deetenga el tejido completo:

```
Si la célula A falla:
├─ Registrar excepción
├─ Continuar con célula B
├─ Continuar con célula C
└─ Todas las otras células se procesan normalmente

Excepciones comunes:
├─ NeoplasticException: Célula se transformó
├─ CellDeathException: Célula murió
└─ Otros errores: Registrar y continuar
```

---

## Gestión de Células

### Añadir Célula

```cpp
auto cell = std::make_unique<AgenticCell>(
    std::make_unique<RandomNoise>(seed),
    genome
);

tissue.addCell(std::move(cell));
```

**Efecto**: Asigna automáticamente un ID único a la célula.

### Acceder a Células

```cpp
// Por índice
ICell* cell = tissue.getCell(0);

// Número total
std::size_t count = tissue.size();

// Iterar (de forma segura)
for (std::size_t i = 0; i < tissue.size(); ++i) {
    ICell* cell = tissue.getCell(i);
    if (cell && cell->alive()) {
        // Procesar célula
    }
}
```

### Limpiar Tejido

```cpp
tissue.clear();  // Elimina todas las células
```

---

## Gestión de Identidades

### Asignación Automática

```cpp
class Tissue {
private:
    std::atomic<std::uint64_t> next_cell_id_{0};
    
    void addCell(std::unique_ptr<ICell> cell) {
        uint64_t cell_id = next_cell_id_.fetch_add(1);
        cell->setId(cell_id);
        cells_.push_back(std::move(cell));
    }
};
```

### Garantías

✅ Cada célula recibe un ID único  
✅ IDs son secuenciales (0, 1, 2, ...)  
✅ Thread-safe (usa `std::atomic`)  
✅ Nunca se reutilizan IDs en una sesión  

---

## Rastreo de Neoplasias

### Identificación

El tejido mantiene un registro de todas las células que se han transformado en neoplasias:

```cpp
std::set<std::uint64_t> identified_neoplasms_;
```

### Acceso

```cpp
std::set<std::uint64_t> neoplasms = tissue.getIdentifiedNeoplasms();

for (uint64_t neopl_id : neoplasms) {
    std::cout << "Neoplasm: Cell " << neopl_id << std::endl;
}
```

### Listener de Neoplasias

Registrar un callback para eventos:

```cpp
tissue.setNeoplasmListener([](uint64_t cell_id, const std::string& msg) {
    std::cout << "ALERT: Cell " << cell_id << " - " << msg << std::endl;
});

// Cuando una célula se vuelve neoplástica:
// ALERT: Cell 5 - Cell became neoplastic
```

---

## Comunicación y Señales

### Emisión de Señales

Las células emiten señales durante su ciclo de vida:

```cpp
// En AgenticCell::live()
if (isNeoplastic()) {
    emitNeoplasmSignal();  // Emite ISignal
}
```

### Recolección de Señales

```cpp
auto signals = tissue.stealEmittedSignals();

for (const auto& signal : signals) {
    std::cout << "Signal from " << signal->sourceId() << std::endl;
    std::cout << "Message: " << signal->message() << std::endl;
}
```

### Thread-Safety

```cpp
private:
    std::vector<std::unique_ptr<ISignal>> signals_new_;
    std::mutex signals_mutex_;
    
    // Thread-safe read
    auto stealEmittedSignals() {
        std::lock_guard<std::mutex> lock(signals_mutex_);
        auto result = std::move(signals_new_);
        signals_new_.clear();
        return result;
    }
```

---

## Rastreo Genético

### Concepto

El tejido puede rastrear información genética agregada:

```cpp
GeneticTrackingData tissue.getGeneticTracking() const {
    // Contar mutaciones por gen
    // Calcular heterogeneidad genética
    // Rastrear diversidad
    // etc.
}
```

### Información

```
Genetic Tracking:
├─ Número de mutaciones BRCA1
├─ Número de mutaciones TP53
├─ Diversidad genética
├─ Edad promedio
└─ Distribución de estados
```

---

## Diagrama: Arquitectura del Tejido

```
┌───────────────────────────────────────────────────────┐
│                    TEJIDO                             │
│   ID: 0   Células: 5   Neoplasias: 2                 │
└───────────────────────────────────────────────────────┘
        │
        ├─────────┬─────────┬─────────┬─────────┬─────────┐
        │         │         │         │         │         │
        ▼         ▼         ▼         ▼         ▼         ▼
    ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐ 
    │Cell 0 │ │Cell 1 │ │Cell 2 │ │Cell 3 │ │Cell 4 │ 
    │ Viva  │ │Viva   │ │Neop.  │ │Viva   │ │Neop.  │
    │ TP53  │ │TP53   │ │TP53   │ │TP53   │ │TP53   │
    │ +/+   │ │ +/-   │ │ -/-   │ │ +/+   │ │ +/-   │
    └───────┘ └───────┘ └───────┘ └───────┘ └───────┘
        │         │         │         │         │
        └────┬────┴────┬────┴────┬────┴────┬────┘
             │         │         │         │
             ▼         ▼         ▼         ▼
        ┌─────────────────────────────────────┐
        │      SEÑALES EMITIDAS               │
        │  ├─ Neoplasm (Cell 2)               │
        │  ├─ Neoplasm (Cell 4)               │
        │  └─ ...                             │
        └─────────────────────────────────────┘
             │
             ▼
        ┌─────────────────────────────────────┐
        │  LISTENERS Y RASTREO                │
        │  ├─ identified_neoplasms: {2, 4}    │
        │  └─ neoplasm_listener() callbacks   │
        └─────────────────────────────────────┘
```

---

## Código: Simulación de un Tejido

```cpp
#include "domain/tissue/Tissue.h"
#include "domain/cell/AgenticCell.h"
#include "domain/gene/GenomeFactory.h"

// Crear tejido
Tissue tissue(true);  // verbose = true
tissue.setId(1);

// Crear y añadir células
for (int i = 0; i < 10; ++i) {
    auto genome = Genome::makeDefaultGenome();
    auto noise = std::make_unique<RandomNoise>(i);
    
    auto cell = std::make_unique<AgenticCell>(
        std::move(noise),
        genome,
        0.002
    );
    
    tissue.addCell(std::move(cell));
}

// Registrar listener
tissue.setNeoplasmListener([](uint64_t id, const std::string& msg) {
    std::cout << "ALERT: Cell " << id << " - " << msg << std::endl;
});

// Simular 100 años
for (int year = 0; year < 100; ++year) {
    tissue.live();
    
    std::cout << "Year " << year << ": "
              << tissue.getIdentifiedNeoplasms().size()
              << " neoplasms detected" << std::endl;
}

// Resultado final
auto neoplasms = tissue.getIdentifiedNeoplasms();
std::cout << "Total neoplasms: " << neoplasms.size() << std::endl;
```

---

## Recapitulación

| Concepto | Explicación |
|----------|-------------|
| **Tejido** | Colección coordinada de células |
| **Fase 0** | Descripción y logging |
| **Fase 1** | Integración de señales |
| **Fase 2** | Ejecución de ciclos celulares |
| **IDs** | Asignación automática y secuencial |
| **Neoplasias** | Rastreo en set de identificadores |
| **Señales** | Cola thread-safe de eventos |
| **Manejo de excepciones** | Por célula, sin interrumpir tejido |

---

## Diagrama: Flujo de Datos

```
Ciclo Anterior
    ↓
    ├─► Recolectar Señales
    │
    ▼
[FASE 0: Descripción]
    │
    ▼
[FASE 1: Integración de Señales]
    │
    ├─► Distribuir a células
    │
    ▼
[FASE 2: Ejecución de Ciclos]
    │
    ├─► Para cada célula
    │   ├─ Ciclo genómico
    │   ├─ Evaluación viabilidad
    │   ├─ Evaluación neoplasia
    │   └─ Recolectar señales emitidas
    │
    ├─► Rastrear neoplasias
    │
    ▼
Ciclo Siguiente
```

---

## Siguiente Paso

👉 **Lee [Neoplasias](./neoplasm.md)** para entender cómo múltiples mutaciones llevan a la progresión del cáncer.

---

## Referencias Cruzadas

- 🔗 [Células](./cells.md) - Unidades dentro del tejido
- 🔗 [Neoplasias](./neoplasm.md) - Transformación dentro del tejido
- 🔗 [Interacciones](./interactions.md) - Comunicación mediante señales

