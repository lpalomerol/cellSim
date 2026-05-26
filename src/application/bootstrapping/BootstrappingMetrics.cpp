/// src/application/bootstrapping/BootstrappingMetrics.cpp

#include "BootstrappingMetrics.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace application::bootstrapping {
namespace {
using Mat6 = std::array<std::array<double, N_CLINICAL>, N_CLINICAL>;
using Vec6 = std::array<double, N_CLINICAL>;

bool invert6x6(const Mat6& in, Mat6& out) {
    Mat6 a = in;
    out = {};
    for (int i = 0; i < N_CLINICAL; ++i) out[i][i] = 1.0;

    for (int col = 0; col < N_CLINICAL; ++col) {
        int pivot = col;
        double best = std::abs(a[col][col]);
        for (int r = col + 1; r < N_CLINICAL; ++r) {
            const double cand = std::abs(a[r][col]);
            if (cand > best) {
                best = cand;
                pivot = r;
            }
        }
        if (best < 1e-12) return false;

        if (pivot != col) {
            std::swap(a[pivot], a[col]);
            std::swap(out[pivot], out[col]);
        }

        const double div = a[col][col];
        for (int c = 0; c < N_CLINICAL; ++c) {
            a[col][c] /= div;
            out[col][c] /= div;
        }

        for (int r = 0; r < N_CLINICAL; ++r) {
            if (r == col) continue;
            const double f = a[r][col];
            if (std::abs(f) < 1e-18) continue;
            for (int c = 0; c < N_CLINICAL; ++c) {
                a[r][c] -= f * a[col][c];
                out[r][c] -= f * out[col][c];
            }
        }
    }
    return true;
}
} // namespace

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

double computeMahalanobisDistance(const std::vector<RunResult>& results) {
    if (results.empty()) return 0.0;

    Vec6 sim_mean{};
    Vec6 diff{};
    for (int i = 0; i < N_CLINICAL; ++i) {
        sim_mean[i] = cumulativeRisk(results, KUCHENBAECKER_BRCA1[i].age);
        diff[i] = sim_mean[i] - KUCHENBAECKER_BRCA1[i].risk;
    }

    Mat6 sigma{};
    for (int i = 0; i < N_CLINICAL; ++i) {
        const double sigma_i = (KUCHENBAECKER_BRCA1[i].ci_hi - KUCHENBAECKER_BRCA1[i].ci_lo) / 3.92;
        sigma[i][i] += sigma_i * sigma_i;
    }

    // Add empirical covariance of simulated cumulative-risk mean estimator.
    // Per-run indicator vectors are in percentage-point units {0, 100}.
    const size_t n = results.size();
    if (n > 1) {
        Mat6 centered_sum{};
        for (const auto& r : results) {
            Vec6 x{};
            for (int k = 0; k < N_CLINICAL; ++k)
                x[k] = (r.onset_year >= 0 && r.onset_year <= KUCHENBAECKER_BRCA1[k].age) ? 100.0 : 0.0;
            for (int i = 0; i < N_CLINICAL; ++i) {
                for (int j = 0; j < N_CLINICAL; ++j)
                    centered_sum[i][j] += (x[i] - sim_mean[i]) * (x[j] - sim_mean[j]);
            }
        }
        const double denom = static_cast<double>(n) * static_cast<double>(n - 1); // cov(mean)
        for (int i = 0; i < N_CLINICAL; ++i) {
            for (int j = 0; j < N_CLINICAL; ++j)
                sigma[i][j] += centered_sum[i][j] / denom;
        }
    }

    // Ridge for numerical stability.
    for (int i = 0; i < N_CLINICAL; ++i) sigma[i][i] += 1e-6;

    Mat6 sigma_inv{};
    if (!invert6x6(sigma, sigma_inv)) return computeWeightedSSE(results);

    double d2 = 0.0;
    for (int i = 0; i < N_CLINICAL; ++i) {
        double row = 0.0;
        for (int j = 0; j < N_CLINICAL; ++j)
            row += sigma_inv[i][j] * diff[j];
        d2 += diff[i] * row;
    }
    return d2;
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
