#include "TissueScaleSimulator.h"

int main(int argc, char** argv) {
    if (argc < 11) {
        std::cerr << "Usage: " << argv[0]
                  << " <mu_tp53> <beta_brca1> <delta_low> <delta_high> <theta_d1> <theta_d2> <tau> <sigma> <nsim> <seed>\n";
        return 2;
    }

    tissue_scale::TissueSimulationParameters params {
        std::stod(argv[1]),
        std::stod(argv[2]),
        std::stod(argv[3]),
        std::stod(argv[4]),
        std::stod(argv[5]),
        std::stod(argv[6]),
        std::stod(argv[7]),
        std::stod(argv[8])
    };

    int n_simulations = std::stoi(argv[9]);
    uint64_t seed = std::stoull(argv[10]);

    std::vector<tissue_scale::TissueSimulationResult> results(n_simulations);

#pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < n_simulations; ++i) {
        results[i] = tissue_scale::simulate_tissue(params, seed + static_cast<uint64_t>(i) * 1000003ULL);
    }

    std::cout << "sim,detect_age,founders,founder_null,founder_null_fraction,"
              << "brca1_null_fraction,brca1_wt_fraction,major_founders_1pct,major_founders_5pct,effective_founders\n"
              << std::setprecision(12);

    for (int i = 0; i < n_simulations; ++i) {
        const auto& r = results[i];
        long double total_cells = r.final_unmutated_cells + r.final_null_cells;
        double null_fraction = (total_cells > 0.0L) ? static_cast<double>(r.final_null_cells / total_cells) : 0.0;
        double wt_fraction = 1.0 - null_fraction;
        double founder_null_fraction = (r.total_founders > 0)
            ? static_cast<double>(r.null_founders) / static_cast<double>(r.total_founders) : 0.0;

        std::cout << i << ","
                  << r.age_at_detection << ","
                  << r.total_founders << ","
                  << r.null_founders << ","
                  << founder_null_fraction << ","
                  << null_fraction << ","
                  << wt_fraction << ","
                  << r.major_lineages_1pct << ","
                  << r.major_lineages_5pct << ","
                  << r.effective_founders << "\n";
    }

    return 0;
}
