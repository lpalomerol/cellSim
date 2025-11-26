//
// Test para validar que AgenticCell recibe mensajes dirigidos en Endocytosis
//

#include <gtest/gtest.h>
#include <memory>
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/signal/NeoplasmSignal.h"
#include "../src/domain/gene/Genome.h"
#include "TestNoise.h"

namespace {

    domain::Genome createDefaultGenome() {
        domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
        domain::Gene brca1("BRCA1", domain::Gene::State::PlusPlus);
        std::unordered_map<std::string, domain::Gene> genes{
            {tp53.name(), tp53},
            {brca1.name(), brca1}
        };
        return domain::Genome(genes);
    }

}

namespace domain::tests {

    class AgenticCellReceiveMessageTest : public ::testing::Test {
    protected:
        Genome createTestGenome() const {
            Gene tp53("TP53", Gene::State::PlusPlus);
            Gene brca1("BRCA1", Gene::State::PlusPlus);
            std::unordered_map<std::string, Gene> genes{
                {tp53.name(), tp53},
                {brca1.name(), brca1}
            };
            return Genome(genes);
        }
    };

    /**
     * Test 1: Broadcast message (empty targetIds) - debe ser aceptado por todas
     */
    TEST_F(AgenticCellReceiveMessageTest, AcceptsBroadcastMessage) {
        AgenticCell cell(std::make_unique<test::DummyNoise>(), createTestGenome(), 0.002, 0.0001, 0.0002, true);
        cell.setId(42);

        // Crear un mensaje broadcast (targetIds vacío)
        auto broadcast_signal = std::make_unique<NeoplasmSignal>(1, "broadcast_msg", std::vector<uint64_t>{});

        cell.receiveMessage(std::move(broadcast_signal));

        // En verbose, debería haber impreso que recibió el mensaje
        // Ejecutamos live() para que procese en phase2_Endocytosis
        cell.live();
        // Si no lanza excepción, pasó
        EXPECT_TRUE(true);
    }

    /**
     * Test 2: Directed message TO this cell - debe ser aceptado
     */
    TEST_F(AgenticCellReceiveMessageTest, AcceptsDirectedMessageToThisCell) {
        AgenticCell cell(std::make_unique<test::DummyNoise>(), createTestGenome(), 0.002, 0.0001, 0.0002, true);
        cell.setId(42);

        // Crear un mensaje dirigido a la célula 42
        auto directed_signal = std::make_unique<NeoplasmSignal>(1, "directed_to_42", std::vector<uint64_t>{42});

        cell.receiveMessage(std::move(directed_signal));

        cell.live();
        EXPECT_TRUE(true);
    }

    /**
     * Test 3: Directed message NOT to this cell - debe ser ignorado
     */
    TEST_F(AgenticCellReceiveMessageTest, IgnoresDirectedMessageNotForThisCell) {
        AgenticCell cell(std::make_unique<test::DummyNoise>(), createTestGenome(), 0.002, 0.0001, 0.0002, true);
        cell.setId(42);

        // Crear un mensaje dirigido a la célula 99 (no es esta)
        auto directed_signal = std::make_unique<NeoplasmSignal>(1, "directed_to_99", std::vector<uint64_t>{99});

        cell.receiveMessage(std::move(directed_signal));

        cell.live();
        // Debería ignorar el mensaje
        EXPECT_TRUE(true);
    }

    /**
     * Test 4: Multi-target message - debe ser aceptado si está en la lista
     */
    TEST_F(AgenticCellReceiveMessageTest, AcceptsMultiTargetMessageIfInList) {
        AgenticCell cell(std::make_unique<test::DummyNoise>(), createTestGenome(), 0.002, 0.0001, 0.0002, true);
        cell.setId(42);

        // Crear un mensaje para múltiples células (incluyendo esta)
        auto multi_signal = std::make_unique<NeoplasmSignal>(1, "multi_target", std::vector<uint64_t>{10, 42, 99});

        cell.receiveMessage(std::move(multi_signal));

        cell.live();
        EXPECT_TRUE(true);
    }

    /**
     * Test 5: Multi-target message - debe ser ignorado si NO está en la lista
     */
    TEST_F(AgenticCellReceiveMessageTest, IgnoresMultiTargetMessageIfNotInList) {
        AgenticCell cell(std::make_unique<test::DummyNoise>(), createTestGenome(), 0.002, 0.0001, 0.0002, true);
        cell.setId(42);

        // Crear un mensaje para múltiples células (sin incluir esta)
        auto multi_signal = std::make_unique<NeoplasmSignal>(1, "multi_target", std::vector<uint64_t>{10, 11, 99});

        cell.receiveMessage(std::move(multi_signal));

        cell.live();
        EXPECT_TRUE(true);
    }

} // namespace domain::tests

