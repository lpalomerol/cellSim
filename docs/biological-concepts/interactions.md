# 📡 Interacciones Celulares en cellSim

## Introducción

Las **interacciones celulares** son los mecanismos mediante los cuales las células comunican eventos importantes dentro del tejido. En cellSim, esta comunicación se realiza a través de un sistema de **señales** que permite que eventos como neoplasias sean propagados entre células, creando una red de información distribuida.

---

## Sistema de Señales: Interfaz `ISignal`

### Concepto

Una **señal** es un mensaje emitido por una célula que puede ser recibido por otras células:

```cpp
struct ISignal {
    enum class Type {
        Neoplasm,
        Apoptosis
    };
    
    virtual ~ISignal() = default;
    
    // Tipo de señal
    virtual Type type() const noexcept = 0;
    
    // Quién emite
    virtual std::uint64_t sourceId() const noexcept = 0;
    
    // Contenido
    virtual const std::string& message() const noexcept = 0;
    
    // A quién va dirigida
    virtual const std::vector<uint64_t>& targetIds() const noexcept = 0;
};
```

### Características

✅ Polimórfica (puede tener múltiples tipos)  
✅ Direccionable (broadcast o unicast)  
✅ Trazable (sourceId)  
✅ Informativa (mensaje textual)  
✅ Tipificada (neoplasm, apoptosis, etc.)  

---

## Tipos de Señales

### 1. Señal de Neoplasia: `Neoplasm`

**Cuándo se emite**: Cuando una célula se transforma en neoplasia

**Propósito**: Advertir al tejido sobre una transformación cancerosa

**Ejemplo**:
```cpp
ISignal::Type::Neoplasm
sourceId: 42
message: "Cell 42 became neoplastic"
targetIds: {}  (vacío = broadcast)
```

### 2. Señal de Apoptosis: `Apoptosis`

**Cuándo se emite**: Cuando una célula muere (versión futura)

**Propósito**: Notificar muerte celular

**Ejemplo**:
```cpp
ISignal::Type::Apoptosis
sourceId: 15
message: "Cell 15 underwent apoptosis"
targetIds: {}  (vacío = broadcast)
```

### Tabla Comparativa

| Tipo | Evento | Cuando | Propósito |
|------|--------|--------|----------|
| **Neoplasm** | Transformación cancerosa | cell.isNeoplastic = true | Alertar sobre cáncer |
| **Apoptosis** | Muerte celular | (próximamente) | Notificar morte |

---

## Broadcasts vs. Mensajes Dirigidos

### Broadcast (Difusión)

**Características**:
- `targetIds()` devuelve vector **vacío**
- La señal llega a **todas las células** del tejido
- Patrón: publicador-suscriptor global

**Implementación**:

```cpp
std::vector<uint64_t> targetIds() const noexcept override {
    return {};  // Vacío = broadcast
}
```

**Flujo**:

```
Célula A emite Señal (broadcast)
    │
    ▼
Tejido recibe señal
    │
    ├─► targetIds vacío? SÍ
    │
    ├─► Entregar a TODAS las células
    │
    ├─► Célula B recibe
    ├─► Célula C recibe
    └─► Célula D recibe
```

**Ejemplo en cellSim**:

```cpp
// Cuando una célula se vuelve neoplástica
auto signal = std::make_unique<NeoplasmSignal>(
    cell_id,
    "Cell became neoplastic",
    {}  // targetIds vacío = broadcast
);

tissue.broadcastSignal(std::move(signal));
```

### Mensaje Dirigido (Unicast)

**Características**:
- `targetIds()` devuelve vector con IDs específicos
- La señal llega **solo a células designadas**
- Patrón: punto-a-punto

**Implementación**:

```cpp
std::vector<uint64_t> targetIds() const noexcept override {
    return {42, 15, 27};  // Específicas
}
```

**Flujo**:

```
Célula A emite Señal (dirigida a 42, 15, 27)
    │
    ▼
Tejido recibe señal
    │
    ├─► targetIds: {42, 15, 27}
    │
    ├─► Buscar Célula 42 ✓ Encontrada → entregar
    ├─► Buscar Célula 15 ✓ Encontrada → entregar
    ├─► Buscar Célula 27 ✓ Encontrada → entregar
    └─► Resto de células no reciben
```

**Ejemplo hipotético**:

```cpp
// Enviar señal solo a vecinos inmediatos
auto signal = std::make_unique<NeoplasmSignal>(
    cell_id,
    "Aviso a vecinos",
    {neighbor_1, neighbor_2, neighbor_3}
);

tissue.sendDirectedSignal(std::move(signal));
```

### Comparación

| Aspecto | Broadcast | Dirigido |
|--------|-----------|----------|
| **Destinatarios** | Todas | Especificadas |
| **targetIds()** | {} (vacío) | {id1, id2, ...} |
| **Complejidad** | Simple | Moderada |
| **Overhead** | Alto (replicas) | Bajo (específico) |
| **Uso típico** | Alertas globales | Comunicación vecinal |

---

## Arquitectura: Emisión de Señales

### Paso 1: Célula Emite

```cpp
// En AgenticCell::live()
if (is_neoplastic_) {
    // Crear señal
    auto signal = std::make_unique<NeoplasmSignal>(
        id_,  // sourceId
        "Cell " + std::to_string(id_) + " became neoplastic",
        {}    // broadcast (targetIds vacío)
    );
    
    // Emitir a través del callback
    signal_emitter_(std::move(signal));
}
```

### Paso 2: Configurar Emisor

```cpp
// En Tissue::addCell()
auto emitter = [this](std::unique_ptr<ISignal> signal) {
    // Almacenar en cola
    {
        std::lock_guard<std::mutex> lock(signals_mutex_);
        signals_new_.push_back(std::move(signal));
    }
};

cell->setSignalEmitter(emitter);
```

### Paso 3: Tejido Recolecta

```cpp
// En Tissue::live() Fase 2
for (auto& cell : cells_) {
    cell->live();  // Puede emitir señales
    
    // Señales se almacenan en signals_new_
}
```

### Paso 4: Tejido Distribuye

```cpp
// En Tissue::phase1_SignalIntegration()
auto pending = stealEmittedSignals();

for (auto& signal : pending) {
    const auto& targets = signal->targetIds();
    
    if (targets.empty()) {
        // Broadcast: a todas
        for (auto& cell : cells_) {
            cell->receiveMessage(clone(signal));
        }
    } else {
        // Dirigido: a específicas
        for (uint64_t target_id : targets) {
            getCell(target_id)->receiveMessage(clone(signal));
        }
    }
}
```

---

## Arquitectura: Recepción de Señales

### Cola de Mensajes

Cada célula mantiene una cola de mensajes recibidos:

```cpp
class AgenticCell {
private:
    std::queue<std::unique_ptr<ISignal>> incoming_messages_;
};
```

### Recepción

```cpp
void AgenticCell::receiveMessage(std::unique_ptr<ISignal> signal) {
    // Validar que es para esta célula (o broadcast)
    const auto& targets = signal->targetIds();
    
    if (!targets.empty()) {
        // Mensaje dirigido: verificar si es para mí
        bool is_for_me = std::find(
            targets.begin(), targets.end(), id_
        ) != targets.end();
        
        if (!is_for_me) return;  // No es para mí
    }
    
    // Almacenar en cola
    incoming_messages_.push(std::move(signal));
}
```

### Procesamiento

Las señales se procesan en el siguiente `live()`:

```cpp
void AgenticCell::live() {
    // ... ciclo normal ...
    
    // Procesar mensajes pendientes
    while (!incoming_messages_.empty()) {
        auto message = std::move(incoming_messages_.front());
        incoming_messages_.pop();
        
        processMessage(message.get());
    }
}
```

---

## Thread-Safety

### Protección con Mutex

```cpp
class Tissue {
private:
    std::vector<std::unique_ptr<ISignal>> signals_new_;
    std::mutex signals_mutex_;
    
public:
    // Thread-safe read
    std::vector<std::unique_ptr<ISignal>> stealEmittedSignals() {
        std::lock_guard<std::mutex> lock(signals_mutex_);
        auto result = std::move(signals_new_);
        signals_new_.clear();
        return result;
    }
};
```

### Garantías

✅ Múltiples células pueden emitir simultáneamente  
✅ No hay race conditions  
✅ Orden FIFO preservado  
✅ Excepciones seguras (RAII)  

---

## Rastreo de Eventos

### Listeners de Neoplasia

El tejido permite registrar callbacks para eventos de transformación:

```cpp
// Registrar listener
tissue.setNeoplasmListener([](uint64_t cell_id, const std::string& msg) {
    std::cout << "ALERT: Cell " << cell_id << " - " << msg << std::endl;
});

// Cuando una célula se transforma
// Output: ALERT: Cell 42 - Cell became neoplastic
```

### Recolección de Señales

```cpp
// Obtener todas las señales emitidas
auto signals = tissue.stealEmittedSignals();

for (const auto& signal : signals) {
    std::cout << "Signal from " << signal->sourceId() << std::endl;
    std::cout << "Type: " << (signal->type() == ISignal::Type::Neoplasm ? 
                                 "Neoplasm" : "Other") << std::endl;
    std::cout << "Message: " << signal->message() << std::endl;
}
```

### Conjunto de Neoplasias Identificadas

```cpp
// Obtener todas las células que se volvieron neoplásticas
auto neoplasms = tissue.getIdentifiedNeoplasms();

std::cout << "Total neoplasms: " << neoplasms.size() << std::endl;

for (uint64_t neopl_id : neoplasms) {
    std::cout << "- Cell " << neopl_id << std::endl;
}
```

---

## Flujo Completo de una Interacción

### Escenario: Transformación Neoplástica

```
TICK N (Año N)
│
├─► FASE 2: Ejecución de Ciclos
│   │
│   ├─► Célula 42 ejecuta live()
│   │   ├─ Fase 1: Ciclo genómico
│   │ │ ├─ TP53 muta a +/-
│   │ │ └─ Cascada de inestabilidad
│   │ │
│   │ ├─ Fase 2: Viabilidad OK
│   │ │
│   │ ├─ Fase 3: Neoplasia
│   │ │ ├─ TP53 no protege (+/-)
│   │ │ ├─ Muestrear: random = 0.0015 < 0.002
│   │ │ ├─ TRANSFORMACIÓN!
│   │ │ ├─ is_neoplastic_ = true
│   │ │ │
│   │ │ └─ Emitir NeoplasmSignal
│   │ │    │
│   │ │    └─► signal_emitter_() callback
│   │ │        │
│   │ │        └─► Tejido: almacenar en signals_new_
│   │ │
│   │ ├─ Incrementar edad
│   │ │
│   │ └─ Registrar en identified_neoplasms_
│   │
│   └─► Otras células ejecutan live() normalmente
│
├─► Recolectar señales emitidas
│   └─► [NeoplasmSignal(42, "Cell 42 became neoplastic")]
│
└─► Siguiente TICK

TICK N+1
│
├─► FASE 1: Integración de Señales
│   │
│   ├─► signal.targetIds() = {} (broadcast)
│   │
│   ├─► Distribuir a TODAS las células
│   │   ├─► Célula 0 recibe
│   │   ├─► Célula 1 recibe
│   │   ├─► Célula 2 recibe
│   │   └─► ... (todas)
│   │
│   └─► Cells almacenan en incoming_messages_
│
├─► FASE 2: Ejecución de Ciclos
│   │
│   └─► Células pueden procesar mensajes durante live()
│       (en versiones avanzadas con respuesta inmune)
│
└─► Continuar simulación...
```

---

## Código: Sistema Completo

```cpp
#include <iostream>
#include "domain/tissue/Tissue.h"
#include "domain/cell/AgenticCell.h"

int main() {
    // Crear tejido
    Tissue tissue(true);
    tissue.setId(1);
    
    // Crear células
    for (int i = 0; i < 5; ++i) {
        auto genome = Genome::makeDefaultGenome();
        auto noise = std::make_unique<RandomNoise>(i);
        
        auto cell = std::make_unique<AgenticCell>(
            std::move(noise),
            genome,
            0.002
        );
        
        tissue.addCell(std::move(cell));
    }
    
    // Registrar listeners
    tissue.setNeoplasmListener([](uint64_t id, const std::string& msg) {
        std::cout << ">>> NEOPLASM EVENT: Cell " << id << " - " 
                  << msg << std::endl;
    });
    
    // Simular 50 años
    for (int year = 0; year < 50; ++year) {
        tissue.live();
        
        // Procesar señales emitidas
        auto signals = tissue.stealEmittedSignals();
        for (const auto& sig : signals) {
            std::cout << "[Year " << year << "] Signal from Cell " 
                      << sig->sourceId() << ": " 
                      << sig->message() << std::endl;
        }
    }
    
    // Estadísticas finales
    std::cout << "\n=== FINAL STATISTICS ===\n";
    auto neoplasms = tissue.getIdentifiedNeoplasms();
    std::cout << "Total neoplasms detected: " << neoplasms.size() << std::endl;
    
    for (uint64_t id : neoplasms) {
        std::cout << "  - Cell " << id << std::endl;
    }
    
    return 0;
}
```

### Salida Esperada

```
[Year 15] Signal from Cell 1: Cell 1 became neoplastic
>>> NEOPLASM EVENT: Cell 1 - Cell became neoplastic

[Year 23] Signal from Cell 3: Cell 3 became neoplastic
>>> NEOPLASM EVENT: Cell 3 - Cell became neoplastic

[Year 31] Signal from Cell 4: Cell 4 became neoplastic
>>> NEOPLASM EVENT: Cell 4 - Cell became neoplastic

=== FINAL STATISTICS ===
Total neoplasms detected: 3
  - Cell 1
  - Cell 3
  - Cell 4
```

---

## Ventajas del Sistema de Señales

### 1. Desacoplamiento

```
Sin señales:
├─ Célula A necesita referencia a Célula B
├─ Cambios en B afectan A
└─ Altamente acoplado

Con señales:
├─ Célula A emite mensaje
├─ Tejido lo distribuye
└─ Libre acoplamiento
```

### 2. Escalabilidad

```
Broadcast:
├─ Simplicidad O(1) en complejidad conceptual
├─ Extensible a muchas células
└─ Bajo overhead
```

### 3. Trazabilidad

```
Cada señal tiene:
├─ sourceId: origen
├─ type: clasificación
├─ message: contexto
└─ timestamp (potencial)
```

### 4. Extensibilidad

```
Nuevos tipos de señales:
├─ Apoptosis (ya definido)
├─ Estrés celular
├─ Recursos disponibles
└─ Fácil de añadir nuevas
```

---

## Recapitulación

| Concepto | Explicación |
|----------|-------------|
| **ISignal** | Interfaz para mensajes |
| **Broadcast** | Mensaje a todas las células |
| **Dirigido** | Mensaje a células específicas |
| **Emisión** | Célula envía mediante callback |
| **Recepción** | Célula recibe en cola |
| **Distribución** | Tejido (fase 1) distribuye |
| **Thread-safe** | Mutex protege signals_new_ |
| **Listener** | Callback para eventos |

---

## Diagrama: Flujo Completo de Señales

```
Célula A: live()
    │
    ├─ Transforma a neoplasia
    │
    └─► Emitir NeoplasmSignal
        │
        └─► signal_emitter_(signal)

                    ▼

Tejido: signals_new_ += signal
        (almacenar con mutex)

                    ▼

Tissue::stealEmittedSignals()
        (recolectar y limpiar)

                    ▼

Fase 1: Distribution
    │
    ├─► targetIds vacío?
    │   ├─ SÍ: Broadcast a todas
    │   └─ NO: Dirigido a específicas
    │
    └─► cell.receiveMessage(signal)

                    ▼

Cells: incoming_messages_ queue

                    ▼

Siguiente live()
    │
    └─► Procesar mensajes acumulados
        (respuesta inmune, logging, etc.)
```

---

## Próximas Mejoras Potenciales

- ⏳ Sistema de timestamp en señales
- 🔬 Respuesta inmune: células reaccionan a neoplasias
- 🧬 Transferencia horizontal de genes
- 💊 Tratamientos farmacológicos vía señales
- 📊 Análisis de difusión de información

---

## Siguiente Paso

¡Felicidades! Has completado la lectura de los **6 conceptos biológicos fundamentales** de cellSim. 

### Para continuar:

- 🔄 **Relee cualquier sección** si necesitas refrescar conceptos
- 🧪 **Experimenta con el código** en `app/main_interactive.cpp`
- 📖 **Consulta la API reference** (próximamente)
- 🏗️ **Lee ARCHITECTURE.md** para detalles técnicos

---

## Referencias Cruzadas (Índice Completo)

- 🔗 [Genes](./genes.md) - Unidades básicas
- 🔗 [Genomas](./genome.md) - Colecciones de genes
- 🔗 [Células](./cells.md) - Entidades vivas
- 🔗 [Tejidos](./tissue.md) - Sistemas celulares
- 🔗 [Neoplasias](./neoplasm.md) - Transformación cancerosa
- 🔗 [Interacciones](./interactions.md) - Comunicación (AQUÍ)

---

**¡Gracias por leer esta documentación! 🎓**

Esperamos que hayas adquirido una comprensión profunda de cómo funciona cellSim biológicamente.

