#include "CellFactory.h"
#include "AgenticCell.h"

namespace domain::cell_factory {

     std::unique_ptr<ICell> createAgenticCell(
         unsigned seed,
         domain::Genome genome,
         double neoplasm_k,
         double low_delta_instability,
         double high_delta_instability,
         double division_rate,
         double neoplastic_division_rate,
         bool enable_big_bang_mode,
         double apoptosis_instability_threshold,
         ports::ILoggerPtr logger){
         // Delegate to overload that accepts an injected noise source
         return createAgenticCell(std::make_unique<domain::adapters::RandomNoise>(seed), std::move(genome), neoplasm_k,
         low_delta_instability, high_delta_instability, division_rate, neoplastic_division_rate, enable_big_bang_mode,
         apoptosis_instability_threshold, logger);
     }

     // Overload: allows directly injecting a noise source
     std::unique_ptr<ICell> createAgenticCell(
         std::unique_ptr<INoiseSource> noise,
         domain::Genome genome,
         double neoplasm_k,
         double low_delta_instability,
         double high_delta_instability,
         double division_rate,
         double neoplastic_division_rate,
         bool enable_big_bang_mode,
         double apoptosis_instability_threshold,
         ports::ILoggerPtr logger) {
         return std::make_unique<domain::AgenticCell>(std::move(noise), std::move(genome), neoplasm_k,
             low_delta_instability, high_delta_instability, division_rate, neoplastic_division_rate,
             enable_big_bang_mode, apoptosis_instability_threshold, logger);
     }

} // namespace domain::cell_factory
