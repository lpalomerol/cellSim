#include "../src/domain/gene/Gene.h"
#include <gtest/gtest.h>

TEST(GeneTest, NameIsStoredAndReturned) {
    domain::Gene gene("TP53", domain::Gene::State::PlusPlus);
    EXPECT_EQ(gene.name(), "TP53");
}

TEST(GeneTest, StatusIsCorrect) {
    domain::Gene gene("BRCA1", domain::Gene::State::PlusPlus);
    EXPECT_EQ(gene.status(), "+/+");
    gene.mutate();
    EXPECT_EQ(gene.status(), "+/-");
    gene.mutate();
    EXPECT_EQ(gene.status(), "-/-");
    gene.mutate();
    EXPECT_EQ(gene.status(), "-/-");
}

TEST(GeneTest, EnabledReturnsTrueExceptMinusMinus) {
    domain::Gene gene("TP53", domain::Gene::State::PlusPlus);
    EXPECT_TRUE(gene.enabled());
    gene.mutate();
    EXPECT_TRUE(gene.enabled());
    gene.mutate();
    EXPECT_FALSE(gene.enabled());
}
