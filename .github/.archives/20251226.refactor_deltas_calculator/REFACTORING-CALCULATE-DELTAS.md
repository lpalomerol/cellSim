# Refactorización: Cálculo de Deltas Interno en AgenticCell

**Fecha**: 2025-12-26  
**Estado**: ✅ **COMPLETADO Y VALIDADO**  
**Tests**: 84/84 pasando

---

## 🎯 Objetivo

Simplificar el cálculo de deltas D1/D2 moviendo la lógica desde una clase estática externa (`InstabilityDeltas`) a un método privado interno de `AgenticCell`, mejorando la encapsulación y preparando el terreno para futura extensibilidad.

---

## 📋 Cambios Implementados

### 1. Nuevo Método Privado en AgenticCell

**Archivo**: `src/domain/cell/AgenticCell.h` (línea 156)

```cpp
/// Calculate instability deltas (delta_d1, delta_d2) based on current genetic state
/// @return pair<double, double> where first=delta_d1, second=delta_d2
///
/// Decision matrix based on TP53 and BRCA1 status:
/// - D1 (DNA damage) depends only on TP53
/// - D2 (Immunosuppression) depends on both TP53 and BRCA1 (additive)
///
/// Future scalability: This method can be replaced by a std::function member
/// to allow injection of custom delta calculation strategies.
[[nodiscard]] std::pair<double, double> calculateInstabilityDeltas() const;
```

### 2. Implementación del Método

**Archivo**: `src/domain/cell/AgenticCell.cpp` (línea 359)

**Lógica implementada**:
- Extrae estados genéticos (`TP53`, `BRCA1`) internamente usando `getTP53()` y `getBRCA1()`
- Calcula `delta_tp53` basándose en estado de TP53:
  - `+/+` → 0.0 (wild-type, sin instabilidad)
  - `+/-` → `low_delta_instability_` (heterozigoto, baja instabilidad)
  - `-/-` → `high_delta_instability_` (homozigoto recesivo, alta instabilidad)
- Calcula `delta_brca1` basándose en estado de BRCA1:
  - `+/+` → 0.0 (wild-type, sin contribución)
  - `+/-` → `low_delta_instability_` (heterozigoto, baja contribución)
  - `-/-` → `2 * high_delta_instability_` (homozigoto, contribución muy alta)
- Retorna:
  - `delta_d1 = delta_tp53` (solo depende de TP53)
  - `delta_d2 = delta_tp53 + delta_brca1` (modelo aditivo)

### 3. Simplificación de phase4_CytoplasmicRemodeling()

**Archivo**: `src/domain/cell/AgenticCell.cpp` (línea 292)

**Antes**:
```cpp
std::string tp53_status = getTP53();
std::string brca1_status = getBRCA1();
auto [delta_d1, delta_d2] = InstabilityDeltas::getDeltas(tp53_status, brca1_status,
                                                           low_delta_instability_,
                                                           high_delta_instability_);
```

**Después**:
```cpp
// Calculate instability deltas based on current genetic state
auto [delta_d1, delta_d2] = calculateInstabilityDeltas();
```

**Beneficios**:
- ✅ Código más limpio y legible
- ✅ Menos parámetros explícitos
- ✅ Menor acoplamiento con clases externas
- ✅ Lógica encapsulada dentro de la célula

### 4. Eliminación de Dependencia

**Archivo**: `src/domain/cell/AgenticCell.cpp` (línea 10)

**Eliminado**: `#include "InstabilityDeltas.h"`

La clase `AgenticCell` ya no depende de `InstabilityDeltas` para su funcionamiento normal.

---

## 🧪 Validación

### Tests Ejecutados
- ✅ **84 tests pasando** sin regresiones
- ✅ Compilación exitosa sin errores
- ✅ Solo warnings menores no relacionados (conversiones int→double en `Simulation.cpp`)

### Tests que Siguen Usando InstabilityDeltas
El archivo `tests/CustomDeltaCalculatorTest.cpp` sigue usando `InstabilityDeltas.h` directamente para testear calculadoras personalizadas. Esto es correcto y los tests siguen pasando.

---

## 🔮 Path de Escalabilidad Futura

La implementación actual es simple y directa, pero está diseñada para ser fácilmente extensible cuando sea necesario.

### Opción 1: Strategy Pattern con std::function (Recomendado)

Cuando necesites hacer el cálculo adaptable, puedes añadir un miembro privado:

```cpp
// En AgenticCell.h (sección privada)
std::function<std::pair<double,double>(const AgenticCell&)> delta_calculator_;

// En constructor, asignar lambda por defecto
delta_calculator_ = [](const AgenticCell& cell) {
    // Lógica actual de calculateInstabilityDeltas()
    std::string tp53 = cell.getTP53();
    std::string brca1 = cell.getBRCA1();
    // ... cálculo actual ...
    return {delta_d1, delta_d2};
};

// En calculateInstabilityDeltas() - delegar al calculador
std::pair<double, double> AgenticCell::calculateInstabilityDeltas() const {
    return delta_calculator_(*this);
}

// Opcional: añadir método para inyectar calculadora custom
void AgenticCell::setDeltaCalculator(
    std::function<std::pair<double,double>(const AgenticCell&)> calculator) {
    delta_calculator_ = std::move(calculator);
}
```

**Ventajas**:
- Cambio mínimo al código existente
- Máxima flexibilidad para experimentos
- No rompe código existente
- Fácil de testear con mocks

### Opción 2: Clase Helper Inyectable

Crear `DeltaCalculator` como clase separada que recibe `const AgenticCell&`:

```cpp
class IDeltaCalculator {
public:
    virtual std::pair<double,double> calculate(const AgenticCell& cell) const = 0;
};

// AgenticCell tendría:
std::unique_ptr<IDeltaCalculator> delta_calculator_;
```

**Ventajas**:
- Separación de responsabilidades más clara
- Fácil de testear el calculador independientemente
- Permite polimorfismo si es necesario

---

## 📊 Comparación: Antes vs Después

| Aspecto | Antes | Después |
|---------|-------|---------|
| **Dependencias** | `AgenticCell` → `InstabilityDeltas` (estática) | Autocontenida |
| **Parámetros explícitos** | 4 (tp53, brca1, low_delta, high_delta) | 0 (método sin parámetros) |
| **Líneas en phase4** | 5 líneas | 2 líneas |
| **Testabilidad** | Requiere mockear clase estática | Testeable via comportamiento observable |
| **Extensibilidad** | Calculadoras globales inyectables | Path claro hacia strategy pattern |
| **Encapsulación** | Lógica externa | Lógica interna |

---

## 🎓 Aprendizajes y Decisiones de Diseño

### ¿Por qué método privado y no clase externa?

1. **Encapsulación**: El cálculo de deltas es una responsabilidad interna de la célula
2. **Simplicidad**: Menos archivos, menos clases, menos complejidad
3. **Acceso directo**: Puede acceder a todos los atributos privados sin getters
4. **Evolutivo**: Fácil migrar a strategy pattern cuando sea necesario

### ¿Por qué no eliminar InstabilityDeltas.h?

- Los tests en `CustomDeltaCalculatorTest.cpp` lo usan
- Mantiene compatibilidad hacia atrás si alguien más lo usa
- No causa problemas al no estar incluido en `AgenticCell.cpp`

### Documentación de Escalabilidad

Se añadió documentación explícita en el código sobre cómo evolucionar el diseño:
- Comentarios en el header sobre `std::function`
- Path claro hacia inyección de dependencias
- Sin over-engineering prematuro

---

## ✅ Conclusión

La refactorización ha sido exitosa:

1. ✅ **Código más simple**: Menos dependencias, menos parámetros
2. ✅ **Mejor encapsulación**: Lógica interna a la célula
3. ✅ **Sin regresiones**: 84/84 tests pasando
4. ✅ **Diseño extensible**: Path claro para futura adaptabilidad
5. ✅ **Documentado**: Decisiones y opciones futuras claras

El sistema ahora es más mantenible y está preparado para crecer cuando sea necesario.

---

## 📚 Referencias

- **Diseño original**: `.github/.archives/20251218.agentic_ver2/PLAN-TRABAJO-REFACTORIZACION-D1D2.md`
- **Máquina de estados**: `.github/copilot/CELL_STATE_MACHINE.md`
- **Commits relacionados**: Ver historial de `AgenticCell.cpp` y `AgenticCell.h`

