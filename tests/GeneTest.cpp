#include "../src/domain/gene/Gene.h"
#include <gtest/gtest.h>

using namespace domain;

TEST(GeneTest, NameIsStoredAndReturned) {
    Gene gene("TP53", Gene::State::PlusPlus);
    EXPECT_EQ(gene.name(), "TP53");
}

TEST(GeneTest, StatusIsCorrect) {
    Gene gene("BRCA1", Gene::State::PlusPlus);
    EXPECT_EQ(gene.status(), "+/+");
    gene.mutate();
    EXPECT_EQ(gene.status(), "+/-");
    gene.mutate();
    EXPECT_EQ(gene.status(), "-/-");
    gene.mutate();
    EXPECT_EQ(gene.status(), "+/+");
}

TEST(GeneTest, EnabledReturnsTrueExceptMinusMinus) {
    Gene gene("TP53", Gene::State::PlusPlus);
    EXPECT_TRUE(gene.enabled());
    gene.mutate();
    EXPECT_TRUE(gene.enabled());
    gene.mutate();
    EXPECT_FALSE(gene.enabled());
}

