/// src/application/bootstrapping/BootstrappingMetrics.cpp

#include "BootstrappingMetrics.h"

namespace application::bootstrapping {

const ClinicalPoint KUCHENBAECKER_BRCA1[] = {
    {30,  4.0,  2.0,  7.0},
    {40, 26.0, 22.0, 30.0},
    {50, 46.0, 41.0, 52.0},
    {60, 58.0, 52.0, 65.0},
    {70, 65.0, 56.0, 73.0},
    {80, 70.0, 60.0, 80.0},
};

double cumulativeRisk(const std::vector<RunResult>& results, int age) {
    if (results.empty()) return 0.0;

    int events = 0;
    for (const auto& r : results)
        if (r.onset_year >= 0 && r.onset_year <= age) ++events;
    return 100.0 * events / static_cast<double>(results.size());
}

double computeSSE(const std::vector<RunResult>& results) {
    if (results.empty()) return 0.0;

    double sse = 0.0;
    for (const auto& kp : KUCHENBAECKER_BRCA1) {
        double diff = cumulativeRisk(results, kp.age) - kp.risk;
        sse += diff * diff;
    }
    return sse;
}

unsigned makeCellSeed(int run_seed, int cell_index) {
    return static_cast<unsigned>(run_seed) * 10000u
         + static_cast<unsigned>(cell_index);
}

} // namespace application::bootstrapping
