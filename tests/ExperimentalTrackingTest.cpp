#include <gtest/gtest.h>

#include "../src/application/ExperimentalTracking.h"
#include "../src/domain/adapters/FixedNoise.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/cell/CellFactory.h"

using namespace domain;
using namespace domain::genome_factory;

TEST(ExperimentalTracking, ObserveCellCounts) {
    // Create genomes and fixed noises
    auto g_defaults = makeDefaultGenome({{"BRCA1",0.01},{"TP53",0.01}}, {{"BRCA1",0.01},{"TP53",0.01}}, nullptr);

    // Cell A: default (BRCA+/- TP53+/+)
    auto noiseA = std::make_unique<adapters::FixedNoise>(domain::CellNoise{1.0});
    auto cellA_ptr = std::make_unique<domain::AgenticCell>(std::move(noiseA), g_defaults.clone(), 0.02, 0.01, 0.02, false);
    domain::AgenticCell* a = cellA_ptr.get();

    // Cell B: make TP53 +/-
    auto noiseB = std::make_unique<adapters::FixedNoise>(domain::CellNoise{1.0});
    auto cellB_ptr = std::make_unique<domain::AgenticCell>(std::move(noiseB), g_defaults.clone(), 0.02, 0.01, 0.02, false);
    domain::AgenticCell* b = cellB_ptr.get();
    b->mutateGene("TP53"); // PP -> +/-

    // Cell C: make TP53 -/- (mutate twice)
    auto noiseC = std::make_unique<adapters::FixedNoise>(domain::CellNoise{1.0});
    auto cellC_ptr = std::make_unique<domain::AgenticCell>(std::move(noiseC), g_defaults.clone(), 0.02, 0.01, 0.02, false);
    domain::AgenticCell* c = cellC_ptr.get();
    c->mutateGene("TP53");
    c->mutateGene("TP53"); // -> -/-

    // Cell D: make BRCA -/-
    auto noiseD = std::make_unique<adapters::FixedNoise>(domain::CellNoise{1.0});
    auto cellD_ptr = std::make_unique<domain::AgenticCell>(std::move(noiseD), g_defaults.clone(), 0.02, 0.01, 0.02, false);
    domain::AgenticCell* d = cellD_ptr.get();
    d->mutateGene("BRCA1"); // +/- -> -/- if mutate twice; mutate once makes -/-, depends on implementation
    d->mutateGene("BRCA1");

    app::ExperimentalTracking track;

    track.observeCell(a);
    track.observeCell(b);
    track.observeCell(c);
    track.observeCell(d);

    EXPECT_EQ(track.brca_het_tp53_hom_plus, 1);
    EXPECT_EQ(track.brca_het_tp53_het, 1);
    EXPECT_EQ(track.brca_het_tp53_hom_minus, 1);
    EXPECT_EQ(track.brca_hom_minus, 1);

    // instability min/max for the +/- category should be set (initial instability is 1.0)
    EXPECT_DOUBLE_EQ(track.min_inst_tp53_het, 1.0);
    EXPECT_DOUBLE_EQ(track.max_inst_tp53_het, 1.0);
}

TEST(ExperimentalTracking, PrintSummaryProducesOutput) {
    auto g_defaults = makeDefaultGenome({{"BRCA1",0.01},{"TP53",0.01}}, {{"BRCA1",0.01},{"TP53",0.01}});
    auto noise = std::make_unique<adapters::FixedNoise>(domain::CellNoise{0.0});
    auto cell_ptr = std::make_unique<domain::AgenticCell>(std::move(noise), g_defaults.clone(), 0.02, 0.01, 0.02, false);
    domain::AgenticCell* ac = cell_ptr.get();

    // Force neoplasia by running a live() with FixedNoise 0.0
    ac->live();

    std::vector<std::unique_ptr<domain::ICell>> cells;
    cells.emplace_back(std::move(cell_ptr));

    app::ExperimentalTracking track;
    // Observe cell (it is brca +/- and tp53 +/+) but became neoplastic
    track.observeCell(ac);

    // Capture stdout
    std::ostringstream oss;
    std::streambuf* old_buf = std::cout.rdbuf(oss.rdbuf());

    track.printSummary(1, 10, /*neoplastic_count=*/1, cells, /*verbose*/ false);

    // restore
    std::cout.rdbuf(old_buf);

    std::string out = oss.str();
    EXPECT_NE(out.find("Resumen genético"), std::string::npos);
    EXPECT_NE(out.find("Año 1 / 10"), std::string::npos);
}

