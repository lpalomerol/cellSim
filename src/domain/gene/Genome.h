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
    [[nodiscard]] std::string getLogCategory() const override { return "GENOME"; }

    // Check whether a gene with the given name exists
    [[nodiscard]] bool hasGene(const std::string& name) const;

    // Return const pointer to the gene; nullptr if not found
    [[nodiscard]] const Gene* getGene(const std::string& name) const;

    // Access the full map
    [[nodiscard]] const std::unordered_map<std::string, Gene>& genes() const;

    // Factory: returns a default genome (TP53 and BRCA1 with standard states)
    static Genome makeDefaultGenome();

    // Return a shallow copy of the genome (copies genes but shares logger reference)
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
    [[nodiscard]] bool isUnstable() const;

    // ===== Genomic Queries (Information Provider - NOT Decision Maker) =====

    /// Query: Does the cell have BRCA1 mutation (disabled)?
    /// BRCA1 -/- compromises DNA repair capability
    [[nodiscard]] bool hasBRCA1Mutation() const;

    /// Query: Does the cell have functional TP53?
    /// TP53 functional (enabled or partially_enabled) can detect DNA damage
    [[nodiscard]] bool hasTP53Function() const;

    /// Query: Is TP53 completely lost (disabled)?
    /// TP53 -/- means no tumor suppressor checkpoint
    [[nodiscard]] bool hasTP53Loss() const;

    /// Query: Does TP53 indicate genomic instability?
    /// TP53 +/- or -/- suggests compromised genome integrity
    [[nodiscard]] bool hasTP53Instability() const;

    /// Returns whether genome indicates neoplastic protection
    /// TP53 functional (enabled or partially_enabled) can eliminate damaged cells
    /// DEPRECATED: Prefer hasTP53Function() for clarity
    [[nodiscard]] bool hasNeoplasticProtection() const;

private:
    std::unordered_map<std::string, Gene> genes_;
    ports::ILoggerPtr logger_;
};} // namespace domain
