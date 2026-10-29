#include "../src/domain/gene/Genome.h"
#include "../src/domain/gene/Gene.h"
#include <gtest/gtest.h>

// Test: mutar un gen existente cambia su estado según Gene::mutate()
TEST(GenomeMutateTest, MutateExistingGene) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus, 0.01, 0.0);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}};
    domain::Genome genome(genes);

    const domain::Gene* before = genome.getGene("TP53");
    ASSERT_NE(before, nullptr);
    EXPECT_EQ(before->status(), "+/+");

    // Aplicar mutación directa
    genome.mutate("TP53");

    const domain::Gene* after = genome.getGene("TP53");
    ASSERT_NE(after, nullptr);
    // PlusPlus -> PlusMinus
    EXPECT_EQ(after->status(), "+/-");
}

// Test: mutar un gen no existente no debe hacer nada (no crash) y genes existentes no cambian
TEST(GenomeMutateTest, MutateNonExistingGeneDoesNothing) {
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus, 0.02, 0.0);
    std::unordered_map<std::string, domain::Gene> genes{{brca1.name(), brca1}};
    domain::Genome genome(genes);

    const domain::Gene* before = genome.getGene("BRCA1");
    ASSERT_NE(before, nullptr);
    EXPECT_EQ(before->status(), "+/-");

    // Intentar mutar un gen que no existe
    genome.mutate("NOT_A_GENE");

    const domain::Gene* after = genome.getGene("BRCA1");
    ASSERT_NE(after, nullptr);
    // No debería haber cambiado
    EXPECT_EQ(after->status(), "+/-");
}

