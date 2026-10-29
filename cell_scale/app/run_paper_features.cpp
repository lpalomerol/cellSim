// Paper-specific output executable for BRCA1-ONSET emergent features.
// Requires patches/paper_model_extensions.patch applied to the frozen baseline.
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#ifdef BOOTSTRAPPING_USE_OMP
#include <omp.h>
#endif
#include <cxxopts.hpp>

#include "../src/application/simulation/Simulation.h"
#include "../src/application/bootstrapping/BootstrappingMetrics.h"
#include "../src/domain/adapters/NullLogger.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"

namespace {
using application::bootstrapping::makeCellSeed;

class PaperSimulation final : public application::Simulation {
public:
    explicit PaperSimulation(int max_t): application::Simulation(max_t) {}
    std::vector<domain::ICell*> liveCells() { return tissue_.getLiveCells(); }
    bool saturated() const { return tissue_.isSaturated(); }
};

struct Result {
    int seed = 0;
    int onset_year = -1;
    int alive_cells = 0;
    int tumoral_cells = 0;
    int brca1_pm = 0;
    int brca1_mm = 0;
    std::string brca1_category = "none";
    int founder_count = 0;
    double median_d1 = -1.0;
    double median_d2 = -1.0;
};

double median(std::vector<double> x) {
    if (x.empty()) return -1.0;
    std::sort(x.begin(), x.end());
    const auto n=x.size();
    return n%2 ? x[n/2] : 0.5*(x[n/2-1]+x[n/2]);
}

Result oneRun(
    int seed, int n_cells, int max_t,
    double low_delta, double high_delta,
    double brca1_rate, double tp53_rate,
    double d1_threshold, double d2_threshold,
    double division_rate, double neoplastic_div_rate,
    double tumor_threshold, double kappa_brca1, double age_drift
) {
    auto logger=std::make_shared<domain::adapters::NullLogger>();
    PaperSimulation sim(max_t);
    for (int i=0;i<n_cells;++i) {
        auto genome=domain::genome_factory::makeDefaultGenome(
            {{"BRCA1",brca1_rate},{"TP53",tp53_rate}},
            {{"BRCA1",0.0},{"TP53",0.0}}, logger);
        auto noise=std::make_unique<domain::adapters::RandomNoise>(makeCellSeed(seed,i));
        auto cell=domain::CellFactory::createCustomCell(
            std::move(noise),genome,
            low_delta,high_delta,
            division_rate,neoplastic_div_rate,true,
            d1_threshold,d2_threshold,logger,
            0.0,kappa_brca1,age_drift);
        sim.addCell(std::move(cell));
    }

    Result out; out.seed=seed;
    for (int year=1;year<=max_t;++year) {
        sim.executeCellCycle();
        auto cells=sim.liveCells();
        int alive=static_cast<int>(cells.size());
        int neo=0;
        for (auto* c:cells) if (c->isNeoplastic()) ++neo;
        const double frac=alive>0 ? static_cast<double>(neo)/alive : 0.0;
        if (alive>0 && frac>=tumor_threshold) {
            out.onset_year=year;
            out.alive_cells=alive;
            out.tumoral_cells=neo;
            std::vector<double>d1,d2;
            std::set<std::uint64_t> founders;
            for (auto* c:cells) {
                if (!c->isNeoplastic()) continue;
                const auto b=c->getBRCA1Status();
                if (b=="+/-") ++out.brca1_pm;
                else if (b=="-/-") ++out.brca1_mm;
                d1.push_back(c->getD1());
                d2.push_back(c->getD2());
                auto f=c->getFounderLineageId();
                if (f!=0) founders.insert(f);
            }
            out.founder_count=static_cast<int>(founders.size());
            out.median_d1=median(d1); out.median_d2=median(d2);
            if (out.brca1_pm>0 && out.brca1_mm==0) out.brca1_category="BRCA1+/-";
            else if (out.brca1_pm>0 && out.brca1_mm>0) out.brca1_category="mixed";
            else if (out.brca1_pm==0 && out.brca1_mm>0) out.brca1_category="BRCA1-/-";
            return out;
        }
        if (sim.saturated()) break;
    }
    return out;
}
}

int main(int argc,char**argv) {
    cxxopts::Options o("run_paper_features","BRCA1-ONSET emergent-feature simulations");
    o.add_options()
      ("n-runs","Independent tissues",cxxopts::value<int>()->default_value("80"))
      ("seed-offset","First tissue seed",cxxopts::value<int>()->default_value("0"))
      ("n-cells","Initial cells",cxxopts::value<int>()->default_value("500"))
      ("max-t","Maximum cycles",cxxopts::value<int>()->default_value("80"))
      ("brca1-rate","BRCA1 transition parameter",cxxopts::value<double>())
      ("tp53-rate","TP53 transition parameter",cxxopts::value<double>()->default_value("0.003"))
      ("low-delta","Low latent-state increment",cxxopts::value<double>())
      ("high-delta","High latent-state increment",cxxopts::value<double>())
      ("division-rate","Basal division probability",cxxopts::value<double>()->default_value("0.001"))
      ("neoplastic-div-rate","Tumoral division probability",cxxopts::value<double>())
      ("d1-threshold","D1 threshold",cxxopts::value<double>()->default_value("2.0"))
      ("d2-threshold","D2 threshold",cxxopts::value<double>()->default_value("5.0"))
      ("tumor-threshold","Tumor-positive fraction",cxxopts::value<double>()->default_value("0.05"))
      ("kappa-brca1","BRCA1 -/- D2 weighting",cxxopts::value<double>()->default_value("2.0"))
      ("age-drift","Cell-age drift coefficient",cxxopts::value<double>()->default_value("0.00001"))
      ("output","Output CSV",cxxopts::value<std::string>()->default_value("paper_features.csv"))
      ("h,help","Help");
    auto a=o.parse(argc,argv);
    if (a.count("help")) { std::cout<<o.help(); return 0; }
    const int n=a["n-runs"].as<int>(), seed0=a["seed-offset"].as<int>();
    std::vector<Result> res(n);
#ifdef BOOTSTRAPPING_USE_OMP
#pragma omp parallel for schedule(dynamic,4)
#endif
    for(int i=0;i<n;++i) {
        res[i]=oneRun(
            seed0+i,a["n-cells"].as<int>(),a["max-t"].as<int>(),
            a["low-delta"].as<double>(),a["high-delta"].as<double>(),
            a["brca1-rate"].as<double>(),a["tp53-rate"].as<double>(),
            a["d1-threshold"].as<double>(),a["d2-threshold"].as<double>(),
            a["division-rate"].as<double>(),a["neoplastic-div-rate"].as<double>(),
            a["tumor-threshold"].as<double>(),a["kappa-brca1"].as<double>(),
            a["age-drift"].as<double>());
    }
    std::ofstream f(a["output"].as<std::string>());
    f<<"seed,onset_year,alive_cells,tumoral_cells,brca1_pm,brca1_mm,brca1_category,founder_count,median_d1,median_d2\n";
    for(const auto&r:res)
        f<<r.seed<<","<<r.onset_year<<","<<r.alive_cells<<","<<r.tumoral_cells<<","
         <<r.brca1_pm<<","<<r.brca1_mm<<","<<r.brca1_category<<","<<r.founder_count<<","
         <<std::setprecision(12)<<r.median_d1<<","<<r.median_d2<<"\n";
    return 0;
}
