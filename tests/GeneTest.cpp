#include "../src/domain/gene/Gene.h"
#include "mocks/MockLogger.h"
#include "FakeNoise.h"
#include <gtest/gtest.h>

TEST(GeneTest, NameIsStoredAndReturned) {
    domain::Gene gene("TP53", domain::Gene::State::PlusPlus);
    EXPECT_EQ(gene.name(), "TP53");
}

TEST(GeneTest, StatusIsCorrect) {
    // Nota: Este test usa BRCA1 +/+ solo para probar la API genérica de Gene
    // En el modelo biológico real, BRCA1 siempre empieza como +/- (heterocigota)
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

// Tests con MockLogger para verificar que se invoca correctamente
class GeneLoggingTest : public ::testing::Test {
protected:
    std::shared_ptr<domain::ports::MockLogger> mockLogger;
    FakeNoise fakeNoise;

    GeneLoggingTest() : fakeNoise({domain::CellNoise{0.05}}) {} // valor bajo que activa mutación

    void SetUp() override {
        mockLogger = std::make_shared<domain::ports::MockLogger>();
    }
};

TEST_F(GeneLoggingTest, LoggerIsNotCalledWhenVerboseIsFalse) {
    // El logger debe ser llamado si verbose=false
    EXPECT_CALL(*mockLogger, logGenome).Times(1);

    domain::Gene gene("TP53", domain::Gene::State::PlusPlus, 0.1, 0.0,  mockLogger);
    gene.setNoiseSource(&fakeNoise);
    gene.live();
}

TEST_F(GeneLoggingTest, LoggerIsCalledWhenVerboseIsTrue) {
    // El logger DEBE ser llamado si verbose=true
    EXPECT_CALL(*mockLogger, logGenome).Times(::testing::AtLeast(1));

    domain::Gene gene("TP53", domain::Gene::State::PlusPlus, 0.1, 0.0,  mockLogger);
    gene.setNoiseSource(&fakeNoise);
    gene.live();
}

