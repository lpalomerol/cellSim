#include "gtest/gtest.h"
#include "FakeNoise.h"
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/gene/Genome.h"

TEST(NoRevivalTest, DeadCellStaysDeadAfterLiveAndLiveWithTrace) {
    // Preparar un ruido que siempre devuelva 0.99 (alto ruido que normalmente permite mutaciones)
    // FakeNoise espera una secuencia de domain::CellNoise
    std::vector<domain::CellNoise> seq(10, domain::CellNoise{0.99});
    auto noise = std::make_unique<FakeNoise>(std::move(seq));

    // Creamos un genoma por defecto donde BRCA1 está inicialmente +/+
    domain::Genome g = domain::Genome::makeDefaultGenome();
    // Forzamos BRCA1 a -/- para simular célula muerta
    // No existe API pública para forzar, así que emulamos el estado mutando repetidamente
    // (Esto depende de la implementación; aquí asumimos que mutate() alterna hacia -/- cuando se llama)

    // Construir la célula
    domain::AgenticCell cell(std::move(noise), std::move(g));

    // Asegurar que inicialmente la célula está viva (BRCA1 +/+ por defecto)
    // Hacemos un primer liveWithTrace para obtener el estado antes de forzar muerte
    auto tt0 = cell.liveWithTrace(0);

    // Ahora forzamos la muerte: manipulamos el genoma llamando live hasta hacer BRCA1 -/-
    // Llamamos live repetidamente para provocar mutación de BRCA1 (este es un enfoque simple que
    // depende de la implementación de Genome/Gene). Si la implementación no muta, el test puede necesitar
    // ajustarse; en este entorno lo hacemos de forma conservadora.
    for (int i = 0; i < 10 && cell.alive(); ++i) {
        cell.live();
    }

    ASSERT_FALSE(cell.alive()) << "Se esperaba que la célula estuviera muerta después de mutaciones forzadas";

    // Guardamos estado antes
    auto before = cell.alive();
    // Llamamos live y liveWithTrace; la célula no debe revivir
    cell.live();
    auto tt = cell.liveWithTrace(1);

    EXPECT_FALSE(before) << "estado antes debe ser muerto";
    EXPECT_FALSE(cell.alive()) << "La célula no debe revivir después de live";
    EXPECT_FALSE(tt.alive_after) << "liveWithTrace no debe producir alive_after=true si estaba muerta";
}
