#pragma once

#include <string>
#include <unordered_map>
#include "Gene.h"
#include "../ports/ILogger.h"
#include "../ports/ILoggeable.h"

namespace domain {

class Genome : public ports::ILoggeable {
public:
    // Constructor accepts a map (name -> Gene) and stores it internally.
    explicit Genome(std::unordered_map<std::string, Gene> genes = {}, ports::ILoggerPtr logger = nullptr);

    // ILoggeable implementation
    std::string getLogCategory() const override { return "GENOME"; }

    // Check whether a gene with the given name exists
    bool hasGene(const std::string& name) const;

    // Return const pointer to the gene; nullptr if not found
    const Gene* getGene(const std::string& name) const;

    // Access the full map
    const std::unordered_map<std::string, Gene>& genes() const;

    // Factory: returns a default genome (TP53 and BRCA1 with standard states)
    static Genome makeDefaultGenome();

    // Return a deep copy of the genome
    Genome clone() const;

    // Inject a noise source to every gene in the genome
    void setNoiseSourceForAll(INoiseSource* noise);

    // Advance (live) all genes in the genome
    // genomic_instability: multiplicative factor applied to gene mutation thresholds (default 1.0 = no effect)
    void liveAllGenes(double genomic_instability = 1.0);

    // Print details of all genes (one line per gene), using Gene::details()
    void details() const;


    // Apply mutation to the gene identified by `name`.
    // If the gene does not exist, do nothing.
    void mutate(const std::string& name);

    // Return true if TP53 indicates instability (TP53 == +/- or -/-)
    bool isUnstable() const;

private:
    std::unordered_map<std::string, Gene> genes_;
    ports::ILoggerPtr logger_;
};} // namespace domain
