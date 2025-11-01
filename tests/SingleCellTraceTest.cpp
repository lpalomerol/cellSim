#include <gtest/gtest.h>
#include <memory>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include "../src/domain/gene/Genome.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/adapters/FixedNoise.h"

// Se evita `using namespace domain;` para usar prefijos explícitos `domain::`

// Test/simulación para generar una traza tick-a-tick de una única AgenticCell.
// Implementa dos corridas (noise=0.0 y noise=0.99), exporta CSV enriquecido con metadatos
// y un archivo JSON con la traza completa (auditoría).
TEST(SingleCellTraceTest, TraceSingleCellEnrichedCsvAndJson) {
    const int max_ticks = 10;
    const double neoplasm_k = 0.5; // probabilidad alta para demostrar auditoría de cambio de estado
    const std::vector<double> noise_values = {0.0, 0.99};

    // Directorio de trabajo
    auto cwd = std::filesystem::current_path();
    std::cout << "Current working directory: " << cwd << std::endl;

    for (double noise_val : noise_values) {
        // Prepare filenames
        std::ostringstream name_suffix;
        name_suffix << "fixed" << std::setfill('0') << std::setw(2) << static_cast<int>(noise_val * 100);
        std::string csv_name = "single_cell_trace_" + name_suffix.str() + ".csv";
        std::string json_name = "single_cell_trace_" + name_suffix.str() + ".json";
        std::filesystem::path csv_path = cwd / csv_name;
        std::filesystem::path json_path = cwd / json_name;

        // Recreate genome per run
        domain::Genome genome = genome_factory::makeDefaultGenome();
        domain::AgenticCell cell(std::make_unique<domain::adapters::FixedNoise>(domain::CellNoise{noise_val}), std::move(genome), neoplasm_k);

        // Open CSV and JSON
        std::ofstream csv(csv_path);
        ASSERT_TRUE(csv.is_open());
        std::ofstream json(json_path);
        ASSERT_TRUE(json.is_open());

        // Metadata as JSON string
        std::ostringstream meta;
        meta << "{\"run\":\"" << name_suffix.str() << "\","
             << "\"noise\":" << noise_val << ","
             << "\"neoplasm_k\":" << neoplasm_k << ","
             << "\"ticks\":" << max_ticks << "}";

        // Write metadata as commented JSON in CSV for audit (line starting with #)
        csv << "# " << meta.str() << "\n";

        // CSV header enriched including event_reason
        csv << "tick,alive_before,alive_after,is_neoplastic_before,is_neoplastic_after,neoplasm_sample,neoplasm_threshold,neoplasm_happened,neoplasm_event_reason,";
        csv << "TP53_before,TP53_sample,TP53_threshold,TP53_mutated,TP53_after,TP53_event_reason,";
        csv << "BRCA1_before,BRCA1_sample,BRCA1_threshold,BRCA1_mutated,BRCA1_after,BRCA1_event_reason" << std::endl;

        // JSON array start
        json << "{\"metadata\": " << meta.str() << ",\"ticks\": [\n";

        for (int t = 0; t < max_ticks; ++t) {
            auto tt = cell.liveWithTrace(t);

            // Per-gen traces
            auto tp_it = tt.gene_traces.find("TP53");
            auto br_it = tt.gene_traces.find("BRCA1");

            std::string tp_before = (tp_it!=tt.gene_traces.end()) ? tp_it->second.before : "?";
            double tp_sample = (tp_it!=tt.gene_traces.end()) ? tp_it->second.sample : -1.0;
            double tp_thr = (tp_it!=tt.gene_traces.end()) ? tp_it->second.threshold : 0.0;
            bool tp_mut = (tp_it!=tt.gene_traces.end()) ? tp_it->second.mutated : false;
            std::string tp_after = (tp_it!=tt.gene_traces.end()) ? tp_it->second.after : "?";

            std::string br_before = (br_it!=tt.gene_traces.end()) ? br_it->second.before : "?";
            double br_sample = (br_it!=tt.gene_traces.end()) ? br_it->second.sample : -1.0;
            double br_thr = (br_it!=tt.gene_traces.end()) ? br_it->second.threshold : 0.0;
            bool br_mut = (br_it!=tt.gene_traces.end()) ? br_it->second.mutated : false;
            std::string br_after = (br_it!=tt.gene_traces.end()) ? br_it->second.after : "?";

            // Determine event reasons
            std::string neoplasm_reason;
            if (tt.is_neoplastic_before) neoplasm_reason = "already_neoplastic";
            else if (tt.neoplasm_threshold <= 0.0) neoplasm_reason = "protected_by_TP53_or_zero_p";
            else if (tt.neoplasm_sample < 0.0) neoplasm_reason = "no_sample";
            else if (tt.neoplasm_happened) neoplasm_reason = "sample<p";
            else neoplasm_reason = "sample>=p";

            auto gen_reason = [&](double sample, double thr, bool mutated, const std::string& before)->std::string{
                if (before == "-/-") return std::string("already_minus_minus");
                if (sample < 0.0) return std::string("no_sample");
                if (mutated) return std::string("sample>threshold");
                return std::string("sample<=threshold");
            };

            std::string tp_reason = gen_reason(tp_sample, tp_thr, tp_mut, tp_before);
            std::string br_reason = gen_reason(br_sample, br_thr, br_mut, br_before);

            // CSV write
            csv << tt.tick << ","
                << (tt.alive_before ? "alive" : "dead") << ","
                << (tt.alive_after ? "alive" : "dead") << ","
                << (tt.is_neoplastic_before ? "true" : "false") << ","
                << (tt.is_neoplastic_after ? "true" : "false") << ","
                << tt.neoplasm_sample << ","
                << tt.neoplasm_threshold << ","
                << (tt.neoplasm_happened ? "true" : "false") << ","
                << neoplasm_reason << ","
                << tp_before << "," << tp_sample << "," << tp_thr << "," << (tp_mut?"true":"false") << "," << tp_after << "," << tp_reason << ","
                << br_before << "," << br_sample << "," << br_thr << "," << (br_mut?"true":"false") << "," << br_after << "," << br_reason << std::endl;

            // JSON tick entry (simple manual serialization) - corrected escaping
            json << "  {\"tick\": " << tt.tick
                 << ", \"alive_before\": \"" << (tt.alive_before?"alive":"dead") << "\""
                 << ", \"alive_after\": \"" << (tt.alive_after?"alive":"dead") << "\""
                 << ", \"is_neoplastic_before\": " << (tt.is_neoplastic_before?"true":"false")
                 << ", \"is_neoplastic_after\": " << (tt.is_neoplastic_after?"true":"false")
                 << ", \"neoplasm_sample\": " << tt.neoplasm_sample
                 << ", \"neoplasm_threshold\": " << tt.neoplasm_threshold
                 << ", \"neoplasm_happened\": " << (tt.neoplasm_happened?"true":"false")
                 << ", \"neoplasm_reason\": \"" << neoplasm_reason << "\""
                 << ", \"genes\": {"
                 << "\"TP53\": {\"before\": \"" << tp_before << "\", \"sample\": " << tp_sample << ", \"threshold\": " << tp_thr << ", \"mutated\": " << (tp_mut?"true":"false") << ", \"after\": \"" << tp_after << "\", \"reason\": \"" << tp_reason << "\"},"
                 << "\"BRCA1\": {\"before\": \"" << br_before << "\", \"sample\": " << br_sample << ", \"threshold\": " << br_thr << ", \"mutated\": " << (br_mut?"true":"false") << ", \"after\": \"" << br_after << "\", \"reason\": \"" << br_reason << "\"}"
                 << "} }";
            if (t < max_ticks-1) json << ",\n"; else json << "\n";
        }

        // JSON array end
        json << "]}\n";

        csv.close();
        json.close();

        std::cout << "CSV written to: " << csv_path << std::endl;
        std::cout << "JSON written to: " << json_path << std::endl;

        // Basic checks
        std::ifstream in(csv_path);
        ASSERT_TRUE(in.is_open());
        int csv_lines = 0; std::string l;
        while (std::getline(in, l)) ++csv_lines;
        in.close();
        EXPECT_EQ(csv_lines, max_ticks + 1 + 1); // +1 header, +1 metadata line

        std::ifstream jin(json_path);
        ASSERT_TRUE(jin.is_open());
        jin.close();
    }
}
