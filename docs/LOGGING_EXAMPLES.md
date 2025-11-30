// EJEMPLO DE USO: Sistema de Logging con ILoggeable en main_random_cells.cpp

// ============================================
// OPCIÓN 1: Logging Global (Todos los niveles)
// ============================================

auto logger = std::make_shared<domain::adapters::Logger>();
logger->setVerbose(true);  // Habilita: TISSUE, CELL, GENOME, GENE


// ============================================
// OPCIÓN 2: Logging Granular (Selective)
// ============================================

auto logger = std::make_shared<domain::adapters::Logger>();

// Solo ver logs de TISSUE (útil para ver movimientos generales)
domain::adapters::LogLevel tissue_only(true, false, false);
logger->setLogLevel(tissue_only);


// ============================================
// OPCIÓN 3: Múltiples Categorías Seleccionadas
// ============================================

auto logger = std::make_shared<domain::adapters::Logger>();

// Ver TISSUE y CELL, pero no GENOME
domain::adapters::LogLevel tissue_and_cell(true, true, false);
logger->setLogLevel(tissue_and_cell);


// ============================================
// OPCIÓN 4: Solo CELL (para depuración de células)
// ============================================

auto logger = std::make_shared<domain::adapters::Logger>();

// Ver solo logs de células individuales
domain::adapters::LogLevel cell_only(false, true, false);
logger->setLogLevel(cell_only);


// ============================================
// OPCIÓN 5: Deshabilitar todo (silent mode)
// ============================================

auto logger = std::make_shared<domain::adapters::Logger>();

// Sin logging en absoluto
domain::adapters::LogLevel no_logging(false, false, false);
logger->setLogLevel(no_logging);
logger->setVerbose(false);  // Asegurar que verbose está off


// ============================================
// CÓMO SE USA EN LOS COMPONENTES
// ============================================

// En Tissue::live()
void Tissue::live() {
    logger->log(this, "Tejido iniciando ciclo (células: " + std::to_string(size()) + ")");
    // ... lógica
    logger->log(this, "Tejido completó ciclo");
}

// En AgenticCell::live()
void AgenticCell::live() {
    logger->log(this, "Célula " + std::to_string(id()) + " viviendo, edad: " + std::to_string(age_));
    // ... fases
    if (is_neoplastic_) {
        logger->log(this, "¡Célula se volvió neoplásica!");
    }
}

// En Genome::clone()
Genome Genome::clone() const {
    logger->log(this, "Clonando genoma con " + std::to_string(genes_.size()) + " genes");
    return Genome(genes_, logger_);
}

// En Gene::mutate()
void Gene::mutate() {
    std::string old_status = status();
    // ... lógica de mutación
    logger->log(this, "Gen " + name_ + " mutó de " + old_status + " a " + status());
}


// ============================================
// ESCENARIOS RECOMENDADOS DE USO
// ============================================

// ESCENARIO 1: Depuración completa (todas las categorías)
logger->setVerbose(true);

// ESCENARIO 2: Seguimiento de dinámicas poblacionales (solo TISSUE)
domain::adapters::LogLevel config(true, false, false);
logger->setLogLevel(config);

// ESCENARIO 3: Depuración de división celular (TISSUE + CELL)
domain::adapters::LogLevel config(true, true, false);
logger->setLogLevel(config);

// ESCENARIO 4: Análisis genético (GENOME + GENE)
domain::adapters::LogLevel config(false, false, true);  // GENOME
logger->setLogLevel(config);

// ESCENARIO 5: Ejecución limpia (sin logs)
logger->setVerbose(false);


// ============================================
// NOTAS IMPORTANTES
// ============================================

/*
1. La flag global "verbose_" debe estar en true para que cualquier logging funcione
   Esto se controla con logger->setVerbose(bool)

2. LogLevel controla qué categorías específicas se muestran
   Estructura: LogLevel(bool tissue, bool cell, bool genome)

3. El método log(const ILoggeable* obj, const std::string& message) funciona
   con cualquier clase que implemente ILoggeable:
   - Tissue
   - AgenticCell
   - Genome
   - Gene

4. Los métodos antiguos siguen disponibles para retrocompatibilidad:
   - logger->logTissue(message)
   - logger->logCell(message)
   - logger->logGenome(message)
   
5. Para máxima performance en producción, establece:
   logger->setVerbose(false);
*/

