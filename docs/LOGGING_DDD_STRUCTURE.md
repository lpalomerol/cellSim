# Sistema de Logging con Interfaces Loggeable (DDD)

## Resumen de Cambios

Se ha implementado un sistema de logging basado en DDD (Domain-Driven Design) que permite control granular de verbosidad por categoría de componente.

## Nuevas Interfaces

### `ILoggeable` (ports/ILoggeable.h)

```cpp
namespace domain::ports {
    class ILoggeable {
    public:
        virtual ~ILoggeable() = default;
        virtual std::string getLogCategory() const = 0;
    };
}
```

Esta interfaz permite que cualquier entidad del dominio declare su categoría de logging.

## Clases que Implementan ILoggeable

Las siguientes clases ahora heredan de `ILoggeable`:

1. **Tissue** → Categoría: `"TISSUE"`
2. **AgenticCell** → Categoría: `"CELL"`
3. **Genome** → Categoría: `"GENOME"`
4. **Gene** → Categoría: `"GENE"`

## Actualización del Logger

### Nuevo Método: `log(const ILoggeable* obj, const std::string& message)`

```cpp
void log(const ports::ILoggeable* obj, const std::string& message);
```

Este método permite un logging polimórfico:

```cpp
// En Tissue::live()
logger->log(this, "Tejido con " + std::to_string(cells_.size()) + " células");

// En AgenticCell::live()
logger->log(this, "Célula ID: " + std::to_string(id_) + " viviendo");

// En Genome::clone()
logger->log(this, "Genoma clonado exitosamente");

// En Gene::mutate()
logger->log(this, "Mutación detectada en " + name());
```

## Configuración de Verbosidad

### Opción 1: Global (Todos los niveles)

```cpp
auto logger = std::make_shared<domain::adapters::Logger>();
logger->setVerbose(true);  // Activa todos los niveles
```

### Opción 2: Granular por Categoría

```cpp
auto logger = std::make_shared<domain::adapters::Logger>();

// Crear configuración personalizada
domain::adapters::LogLevel config(
    true,   // tissue
    false,  // cell
    false,  // genome
    false   // (gene no está en LogLevel, pero sí en Logger)
);

logger->setLogLevel(config);
```

### Opción 3: Configuración Individual

```cpp
auto logger = std::make_shared<domain::adapters::Logger>();

// Activar solo Tissue
domain::adapters::LogLevel config(true, false, false);
logger->setLogLevel(config);
```

## Ventajas de esta Implementación

1. **Retrocompatibilidad**: Los métodos antiguos (`logTissue()`, `logCell()`, etc.) siguen disponibles
2. **DDD**: Los componentes declaran explícitamente su categoría (no usa `typeid()`)
3. **Flexibilidad**: Cada componente puede ser loggeable sin modificar el Logger
4. **Extensibilidad**: Agregar nuevas categorías es simple
5. **Testing**: Más fácil de testear con mocks de ILoggeable

## Ejemplos de Uso en Componentes

### Tissue

```cpp
void Tissue::live() {
    logger->log(this, "Iniciando ciclo de vida del tejido");
    // ... lógica
    logger->log(this, "Tejido completado con " + std::to_string(cells_.size()) + " células");
}
```

### AgenticCell

```cpp
void AgenticCell::live() {
    logger->log(this, "Célula " + std::to_string(cell_id_) + " iniciando live()");
    // ... fases
    logger->log(this, "Célula completó ciclo, edad: " + std::to_string(age_));
}
```

### Genome

```cpp
Genome Genome::clone() const {
    logger->log(this, "Clonando genoma con " + std::to_string(genes_.size()) + " genes");
    return Genome(genes_, logger_);
}
```

### Gene

```cpp
void Gene::mutate() {
    logger->log(this, "Mutación en " + name_ + ": " + status());
    // ... lógica de mutación
}
```

## Migrando Código Existente

Para migrar de métodos específicos a métodos genéricos:

**Antes:**
```cpp
logger->logTissue("Mensaje");
```

**Después:**
```cpp
logger->log(this, "Mensaje");  // this es una ILoggeable
```

Ambas formas coexisten, permitiendo migración gradual.

## Estructura de LogLevel

```cpp
struct LogLevel {
    bool tissue = false;
    bool cell = false;
    bool genome = false;

    LogLevel() = default;
    LogLevel(bool t, bool c, bool g)
        : tissue(t), cell(c), genome(g) {}

    void enableAll() { /* ... */ }
    void disableAll() { /* ... */ }
};
```

## Notas Importantes

- El campo `gene` aún no está en `LogLevel` (se puede agregar si es necesario)
- El Logger siempre respeta la flag `verbose_` global además de la configuración por categoría
- Los métodos antiguos (`logTissue`, `logCell`, etc.) siguen siendo válidos para compatibilidad

## Próximos Pasos (Opcionales)

1. Agregar `bool gene` a `LogLevel` struct
2. Implementar ILoggeable en más clases según sea necesario
3. Reemplazar todas las llamadas a `logXxx()` con `log(this, message)`
4. Considerar agregar más categorías (ej: SIGNAL, TISSUE_MANAGER, etc.)

