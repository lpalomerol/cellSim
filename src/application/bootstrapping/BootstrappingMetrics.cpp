/// src/application/bootstrapping/BootstrappingMetrics.cpp

#include "BootstrappingMetrics.h"

#include <algorithm>

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

double computeWeightedSSE(const std::vector<RunResult>& results) {
    if (results.empty()) return 0.0;

    // σᵢ = (ci_hi − ci_lo) / 3.92  (95% CI width → 1 standard deviation)
    // weight = 1 / σᵢ²
    double sse_w = 0.0;
    for (const auto& kp : KUCHENBAECKER_BRCA1) {
        double sigma = (kp.ci_hi - kp.ci_lo) / 3.92;
        double diff  = cumulativeRisk(results, kp.age) - kp.risk;
        sse_w += (diff * diff) / (sigma * sigma);
    }
    return sse_w;
}

SaturationMilestones computeMilestones(const std::vector<application::YearlySnapshot>& snapshots) {
    SaturationMilestones m;
    for (const auto& snap : snapshots) {
        if (snap.alive_cells <= 0) continue;
        double pct = 100.0 * snap.neoplastic_alive / snap.alive_cells;
        if (m.year_25 < 0 && pct >= 25.0) m.year_25 = snap.year;
        if (m.year_50 < 0 && pct >= 50.0) m.year_50 = snap.year;
        if (m.year_90 < 0 && pct >= 90.0) m.year_90 = snap.year;
    }
    return m;
}

double medianOnset(const std::vector<RunResult>& results) {
    std::vector<int> onsets;
    onsets.reserve(results.size());
    for (const auto& r : results)
        if (r.onset_year >= 0)
            onsets.push_back(r.onset_year);

    if (onsets.empty()) return -1.0;

    std::sort(onsets.begin(), onsets.end());
    const size_t n = onsets.size();
    if (n % 2 == 1)
        return static_cast<double>(onsets[n / 2]);
    return 0.5 * (onsets[n / 2 - 1] + onsets[n / 2]);
}

double medianSaturation(const std::vector<int>& milestone_years) {
    std::vector<int> reached;
    reached.reserve(milestone_years.size());
    for (int y : milestone_years)
        if (y >= 0)
            reached.push_back(y);

    if (reached.empty()) return -1.0;

    std::sort(reached.begin(), reached.end());
    const size_t n = reached.size();
    if (n % 2 == 1)
        return static_cast<double>(reached[n / 2]);
    return 0.5 * (reached[n / 2 - 1] + reached[n / 2]);
}

unsigned makeCellSeed(int run_seed, int cell_index) {
    return static_cast<unsigned>(run_seed) * 10000u
         + static_cast<unsigned>(cell_index);
}

} // namespace application::bootstrapping
