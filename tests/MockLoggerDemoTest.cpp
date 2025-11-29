#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "mocks/MockLogger.h"
#include "../src/domain/gene/Gene.h"

// Test que demuestra que gmock funciona correctamente
class MockLoggerDemoTest : public ::testing::Test {
protected:
    std::shared_ptr<domain::ports::MockLogger> mockLogger;

    void SetUp() override {
        mockLogger = std::make_shared<domain::ports::MockLogger>();
    }
};

TEST_F(MockLoggerDemoTest, MockLoggerCanBeUsed) {
    // No configuramos expectativa: el logger puede o no ser llamado
    // Este test simplemente verifica que el mock puede ser usado sin errores

    // Crear un gen con el mock logger
    domain::Gene gene("TEST_GENE",
                      domain::Gene::State::PlusPlus,
                      0.1,
                      0.0,
                      false,
                      mockLogger);

    // El test pasa si gmock está funcionando correctamente
}

TEST_F(MockLoggerDemoTest, VerifyLoggerNotCalled) {
    // Verificar que el logger NO es llamado cuando es silencioso
    EXPECT_CALL(*mockLogger, logGenome(::testing::_))
        .Times(0);

    domain::Gene gene("SILENT_GENE",
                      domain::Gene::State::PlusPlus,
                      0.1,
                      0.0,
                      false,
                      mockLogger);
}

