#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/gene/Genome.h"
#include <gtest/gtest.h>

using namespace domain;

TEST(GenomeFactoryTest, DefaultGenomeHasTP53AndBRCA1) {
    Genome g = genome_factory::makeDefaultGenome();
    EXPECT_TRUE(g.hasGene("TP53"));
    EXPECT_TRUE(g.hasGene("BRCA1"));
    const Gene* tp53 = g.getGene("TP53");
    ASSERT_NE(tp53, nullptr);
    EXPECT_EQ(tp53->name(), "TP53");
    EXPECT_EQ(tp53->status(), "+/+");
    const Gene* brca1 = g.getGene("BRCA1");
    ASSERT_NE(brca1, nullptr);
    EXPECT_EQ(brca1->name(), "BRCA1");
    EXPECT_EQ(brca1->status(), "+/-");
}

TEST(GenomeFactoryTest, GenomeStaticDelegatesToFactory) {
    Genome g1 = genome_factory::makeDefaultGenome();
    Genome g2 = Genome::makeDefaultGenome();
    // Compare presence and statuses
    EXPECT_TRUE(g2.hasGene("TP53"));
    EXPECT_TRUE(g2.hasGene("BRCA1"));
    const Gene* t1 = g1.getGene("TP53");
    const Gene* t2 = g2.getGene("TP53");
    ASSERT_NE(t1, nullptr);
    ASSERT_NE(t2, nullptr);
    EXPECT_EQ(t1->status(), t2->status());
}

