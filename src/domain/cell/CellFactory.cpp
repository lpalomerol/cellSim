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
         bool verbose,
         ports::ILoggerPtr logger){
         // Delegate to overload that accepts an injected noise source
         return createAgenticCell(std::make_unique<adapters::RandomNoise>(seed), std::move(genome), neoplasm_k,
         low_delta_instability, high_delta_instability, division_rate, verbose, logger);
     }

     // Overload: allows directly injecting a noise source
     std::unique_ptr<ICell> createAgenticCell(
         std::unique_ptr<INoiseSource> noise,
         domain::Genome genome,
         double neoplasm_k,
         double low_delta_instability,
         double high_delta_instability,
         double division_rate,
         bool verbose,
         ports::ILoggerPtr logger) {
         return std::make_unique<domain::AgenticCell>(std::move(noise), std::move(genome), neoplasm_k,
             low_delta_instability, high_delta_instability, division_rate, verbose, logger);
     }

} // namespace domain::cell_factory
