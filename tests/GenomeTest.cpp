#include "../src/domain/gene/Genome.h"
#include <gtest/gtest.h>
#include <string>

using namespace domain;

TEST(GenomeTest, InitializationAndGeneAccess) {
    Gene tp53("TP53", Gene::State::PlusPlus, 0.01, 0.005);
    Gene brca1("BRCA1", Gene::State::PlusMinus, 0.02, 0.005);
    std::unordered_map<std::string, Gene> genes{
        {tp53.name(), tp53},
        {brca1.name(), brca1}
    };
    Genome genome(genes);
    // Verifica existencia
    EXPECT_TRUE(genome.hasGene("TP53"));
    EXPECT_TRUE(genome.hasGene("BRCA1"));
    EXPECT_FALSE(genome.hasGene("NOTCH1"));
    // Verifica acceso y estado
    const Gene* g1 = genome.getGene("TP53");
    ASSERT_NE(g1, nullptr);
    EXPECT_EQ(g1->name(), "TP53");
    EXPECT_EQ(g1->status(), "+/+");
    const Gene* g2 = genome.getGene("BRCA1");
    ASSERT_NE(g2, nullptr);
    EXPECT_EQ(g2->name(), "BRCA1");
    EXPECT_EQ(g2->status(), "+/-");
}
